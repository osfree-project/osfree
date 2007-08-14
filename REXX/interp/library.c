#ifndef lint
static char *RCSid = "$Id: library.c,v 1.2 2003/12/11 04:43:12 prokushev Exp $";
#endif

/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Sigh, let's live dangerously. We need to remove the definition of
 * _POSIX_SOURCE, in order to get defined some extenstions to POSIX,
 * since dynamic loading is not a part of POSIX.
 */

#include "rexx_charset.h"
#include "rexxsaa.h"
#define DONT_TYPEDEF_PFN

#ifdef HAVE_GCI
# include "gci/gci.h"
#endif

#include "rexx.h"
#include "rxiface.h"
#include <assert.h>
#include <string.h>


typedef struct { /* lib_tsd: static variables of this module (thread-safe) */
   struct library *     first_library ;
#ifdef DYNAMIC
   struct library_func *libfuncs[133] ;
#endif
   streng *             err_message ;
} lib_tsd_t; /* thread-specific but only needed by this module. see
              * init_library
              */
#define LIBFUNCS_COUNT (sizeof(((lib_tsd_t*)0)->libfuncs) /       \
                        sizeof(((lib_tsd_t*)0)->libfuncs[0]))

/* init_library initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_library( tsd_t *TSD )
{
   lib_tsd_t *lt;

   if (TSD->lib_tsd != NULL)
      return(1);

   if ((lt = TSD->lib_tsd = MallocTSD(sizeof(lib_tsd_t))) == NULL)
      return(0);
   memset(lt,0,sizeof(lib_tsd_t));  /* correct for all values */
   return(1);
}

#ifdef DYNAMIC

/* Operations on the library and library_func structures */

static void insert_library( const tsd_t *TSD, struct library *ptr )
{
   lib_tsd_t *lt;

   lt = TSD->lib_tsd;
   ptr->prev = NULL ;
   ptr->next = lt->first_library ;
   if ((lt->first_library=ptr)->next)
      ptr->next->prev = ptr ;
}

static void remove_function( tsd_t *TSD, struct library_func *fptr )
{
   lib_tsd_t *lt;

   lt = TSD->lib_tsd;
   if ( fptr->name )
      Free_stringTSD( fptr->name );
# ifdef HAVE_GCI
   if ( fptr->gci_info )
      GCI_remove_structure( TSD, fptr->gci_info );
# endif
   if (fptr->next)
      fptr->next->prev = fptr->prev ;
   if (fptr->prev)
      fptr->prev->next = fptr->next ;
   else
      lt->libfuncs[fptr->hash % LIBFUNCS_COUNT] = fptr->next ;

   if (fptr->forw)
      fptr->forw->backw = fptr->backw ;
   if (fptr->backw)
      fptr->backw->forw = fptr->forw ;
   else
      fptr->lib->first = fptr->forw ;
   FreeTSD( fptr );
}

static void remove_library( const tsd_t *TSD, struct library *ptr )
{
   lib_tsd_t *lt;

   lt = TSD->lib_tsd;
   if (ptr->next)
      ptr->next->prev = ptr->prev ;

   if (ptr->prev)
      ptr->prev->next = ptr->next ;
   else
      lt->first_library = ptr->next ;

   /* FIXME: We need a wrapper_unload function */
   assert( ptr->name ) ;
   Free_stringTSD( ptr->name ) ;
   FreeTSD( ptr ) ;
}

void purge_library( tsd_t *TSD )
{
   struct library_func *lptr, *save_lptr ;
   struct library *ptr, *save_ptr ;
   lib_tsd_t *lt;

   lt = TSD->lib_tsd;
   for (ptr = lt->first_library; ptr != NULL; )
   {
      save_ptr = ptr->next ;
      for ( lptr = ptr->first; lptr; )
      {
         save_lptr = lptr->next ;
         remove_function( TSD, lptr ) ;
         lptr = save_lptr ;
      }
      remove_library( TSD, ptr );
      ptr = save_ptr;
   }
   lt->first_library = NULL;
   memset( lt->libfuncs, 0, sizeof(lt->libfuncs) ) ;
}


static struct library *find_library( const tsd_t *TSD, const streng *name )
{
   struct library *lptr=NULL ;
   lib_tsd_t *lt;

   lt = TSD->lib_tsd;
   lptr = lt->first_library ;
   for (;lptr; lptr=lptr->next)
   {
      if (!Str_cmp(name,lptr->name))
         return lptr ;
   }

   return NULL ;
}

static void add_function( const tsd_t *TSD, struct library_func *fptr )
{
   int hash0=0 ;
   lib_tsd_t *lt;

   lt = TSD->lib_tsd;
   hash0 = fptr->hash % LIBFUNCS_COUNT;
   fptr->next = lt->libfuncs[hash0] ;
   lt->libfuncs[hash0] = fptr ;
   fptr->prev = NULL ;
   if (fptr->next)
      fptr->next->prev = fptr ;

   fptr->forw = fptr->lib->first ;
   fptr->backw = NULL ;
   fptr->lib->first = fptr ;
   if (fptr->forw)
      fptr->forw->backw = fptr ;
}

static struct library_func *find_library_func( const tsd_t *TSD, const streng *name )
{
   struct library_func *lptr=NULL ;
   unsigned hash, hash0 ;
   lib_tsd_t *lt;

   lt = TSD->lib_tsd;
   hash = hashvalue( name->value, name->len ) ;
   hash0 = hash % LIBFUNCS_COUNT ;
   for (lptr=lt->libfuncs[hash0]; lptr; lptr=lptr->next)
   {
      if (hash == lptr->hash)
         if (Str_cmp(name,lptr->name) == 0)
            return lptr ;
   }

   return NULL ;
}

void set_err_message( const tsd_t *TSD, const char *message1, const char *message2 )
{
   lib_tsd_t *lt;
   int size;

   lt = TSD->lib_tsd;
   if (lt->err_message)
      Free_stringTSD( lt->err_message ) ;

   size = strlen(message1)+strlen(message2);
   lt->err_message = Str_makeTSD(size+1);
   if ( lt->err_message )
   {
      strcpy( lt->err_message->value, message1 );
      strcat( lt->err_message->value, message2 );
      lt->err_message->len = size;
   }
}

int loadrxfunc( const tsd_t *TSD, struct library *lptr, const streng *rxname,
                const streng *objnam, void *gci_info )
{
   int result=1 ;
   PFN addr=NULL ;
   struct library_func *fptr=NULL ;

   if (lptr)
   {
      fptr = find_library_func( TSD, rxname ) ;
      if (!fptr || fptr->lib!=lptr)
      {
         addr = wrapper_get_addr( TSD, lptr, objnam ) ;
         if (addr)
           {
            fptr = MallocTSD( sizeof( struct library_func )) ;
            fptr->name = Str_upper( Str_dupstrTSD( rxname ) );
            fptr->hash = hashvalue(fptr->name->value, fptr->name->len);
            fptr->addr = addr ;
            fptr->lib = lptr ;
            fptr->gci_info = gci_info;
            add_function( TSD, fptr ) ;
            result = 0 ;
           }
         else
         {
            result = 30 ;
         }
      }
      else
      {
         result = 10;
      }
   }
   else
   {
      result = 30;
   }
   return result ;
}

/*
 * This loads a module as a result of a RexxRegisterFunctionDll() call
 *
 * parameters:
 *   1) name of the function to be added (in Rexx)
 *   2) name of object file to link in
 *   3) name of the function to be added (in the object file)
 */
static int rex_rxfuncdlladd( const tsd_t *TSD, const streng* rxname,
                             const streng* module, const streng* objnam,
                             void *gci_info )
{
   struct library *lptr=NULL ;
   void *handle=NULL ;
   int rc=0;

   if ((lptr=find_library(TSD, module)) == NULL)
   {
      handle = wrapper_load( TSD, module ) ;
      if (handle)
      {
         lptr = MallocTSD( sizeof( struct library )) ;
         lptr->name = Str_dupTSD( module ) ;
         lptr->handle = handle ;
         lptr->funcs = NULL ;
         lptr->first = NULL ;
      }
      else
      {
         return 1;
      }
      insert_library( TSD, lptr ) ;
   }
   rc = loadrxfunc( TSD, lptr, rxname, objnam, gci_info );
   return ( rc );
}
#endif /* DYNAMIC */

streng *rex_rxfuncerrmsg( tsd_t *TSD, cparamboxptr parms )
{
#ifdef DYNAMIC
   lib_tsd_t *lt;
#endif

   checkparam(  parms,  0,  0 , "RXFUNCERRMSG" ) ;

#ifdef DYNAMIC
   lt = TSD->lib_tsd;
   if (lt->err_message)
      return Str_dupTSD( lt->err_message ) ;
   else
      return nullstringptr() ;
#else
   return Str_creTSD( "Platform doesn't support dynamic linking" ) ;
#endif
}

streng *rex_rxfuncquery( tsd_t *TSD, cparamboxptr parms )
{
   streng *name=NULL;
#ifdef DYNAMIC
   struct library_func *fptr=NULL ;
#endif

   checkparam(  parms,  1,  1 , "RXFUNCQUERY" ) ;
   name = Str_upper( parms->value ) ;
#ifdef DYNAMIC
   fptr = find_library_func( TSD, name ) ;

   if (fptr)
      return int_to_streng( TSD, 0 ) ;
   /*
    *... otherwise fall through and try to find the function
    * loaded via RexxRegisterFunctionExe()
    */
#endif
   return int_to_streng( TSD,(external_func( TSD, name ) ) ? 0 : 1);
}


/*
 * parameters:
 *   1) name of the function to be added (in Rexx)
 *   2) name of object file to link in
 *   3) name of the function to be added (in the object file)
 */
streng *rex_rxfuncadd( tsd_t *TSD, cparamboxptr parms )
{
#ifdef DYNAMIC
   streng *rxname=NULL ;
   streng *module=NULL, *objnam=NULL ;
   int rc;

   if ( TSD->restricted )
      exiterror( ERR_RESTRICTED, 1, "RXFUNCADD" )  ;

   checkparam(  parms,  3,  3 , "RXFUNCADD" ) ;

   rxname = Str_upper (Str_dupTSD(parms->value) ) ;
   module = (parms=parms->next)->value ;
   objnam = parms->next->value ;

   rc = rex_rxfuncdlladd( TSD, rxname, module, objnam, NULL );
   Free_stringTSD( rxname );
   return int_to_streng( TSD, rc ) ;
#else
   checkparam(  parms,  3,  3 , "RXFUNCADD" ) ;
   return int_to_streng( TSD, 1 ) ;
#endif
}

#ifdef HAVE_GCI
/*
 * parameters:
 *   1) name of the function to be added (in Rexx)
 *   2) name of object file to link in
 *   3) name of the function to be added (in the object file)
 *   4) name of a stem containing the definition of the function
 */
streng *rex_rxfuncdefine( tsd_t *TSD, cparamboxptr parms )
{
#ifdef DYNAMIC
   streng *rxname,*module,*objnam,*def_stem;
   void *gci_info;
   int rc;

   if ( TSD->restricted )
      exiterror( ERR_RESTRICTED, 1, "RXFUNCDEFINE" );

   checkparam(  parms,  4,  4 , "RXFUNCDEFINE" ) ;

   rxname = Str_upper( Str_dupTSD( parms->value ) );
   module = (parms=parms->next)->value ;
   objnam = (parms=parms->next)->value ;
   def_stem = parms->next->value ;

   if ( ( rc = GCI_checkDefinition( TSD, def_stem, &gci_info ) ) != 0 )
   {
      Free_stringTSD( rxname );
      return int_to_streng( TSD, 1 ) ;
   }

   rc = rex_rxfuncdlladd( TSD, rxname, module, objnam, gci_info );
   Free_stringTSD( rxname );
   if ( rc )
      GCI_remove_structure( TSD, gci_info );
   return int_to_streng( TSD, rc ) ;
#else
   checkparam(  parms,  4,  4 , "RXFUNCDEFINE" ) ;
   return int_to_streng( TSD, 1 ) ;
#endif
}
#endif

int IfcRegDllFunc( const tsd_t *TSD, const char* rxname, const char* module, const char* objnam )
{
#ifdef DYNAMIC
   int rc;
   streng *ext;
   streng *intr;
   streng *lib;

   ext = Str_upper( Str_creTSD( rxname ) ) ;
   intr = Str_creTSD( objnam ) ;
   lib = Str_creTSD( module ) ;

   rc = rex_rxfuncdlladd( TSD, ext, lib, intr, NULL ) ;
   Free_stringTSD( ext );
   Free_stringTSD( intr );
   Free_stringTSD( lib );
   return ( rc ) ;
#else
   TSD = TSD; /* keep compiler happy */
   rxname = rxname; /* keep compiler happy */
   module = module; /* keep compiler happy */
   objnam = objnam; /* keep compiler happy */
   return 1;
#endif
}


streng *rex_rxfuncdrop( tsd_t *TSD, cparamboxptr parms )
{
   streng *name;

   checkparam(  parms,  1,  1 , "RXFUNCDROP" );
   name = Str_upper( parms->value );

   return int_to_streng( TSD, rex_rxfuncdlldrop( TSD, name ) );
}

int rex_rxfuncdlldrop( tsd_t *TSD, const streng* objnam )
{
#ifdef DYNAMIC
   struct library_func *fptr;
   struct library *lib;
   /*
    * try to find the function loaded from a dynamic library
    */
   fptr = find_library_func( TSD, objnam );
   if ( fptr )
   {
      lib = fptr->lib;
      /*
       * if found OK, remove the function. If this was the last function of the
       * library, remove the library, too.
       */
      remove_function( TSD, fptr );
      if ( lib->first == NULL )
         remove_library( TSD, lib );
      return 0;
   }
   /*
    *... otherwise fall through and try to remove from function
    * loaded via RexxRegisterFunctionExe()
    */
#endif
   if ( external_func( TSD, objnam ) )
      return delfunc( TSD, objnam );

   return 1;                   /* value of 1 indicates failure */
}

int rex_rxfuncdllquery( tsd_t *TSD, const streng* objnam )
{
#ifdef DYNAMIC
   struct library_func *fptr=NULL ;
   /*
    * try to find the function loaded from a dynamic library
    */
   fptr=find_library_func(TSD, objnam);
   if (fptr)
      return(0);
   /*
    *... otherwise fall through and try to find the function
    * loaded via RexxRegisterFunctionExe()
    */
#endif
   return((external_func( TSD, objnam )) ? 0 : 1 );
}

void *loaded_lib_func( const tsd_t *TSD, const streng *name )
{
#ifdef DYNAMIC
   struct library_func *box=NULL ;

   box = find_library_func( TSD, name ) ;
   return (void*)(box) ;
#else
   TSD = TSD; /* keep compiler happy */
   name = name; /* keep compiler happy */
   return NULL ;
#endif
}
