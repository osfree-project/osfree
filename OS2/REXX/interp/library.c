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

#include "regina_c.h"
#include "rexxsaa.h"
#define DONT_TYPEDEF_PFN

#ifdef HAVE_GCI
# include "gci/gci.h"
#endif

#include "rexx.h"
#include "rxiface.h"
#include <assert.h>
#include <string.h>

/*
 * Starting after 3.3RC1 we process both the Rexx???Exe as the Rexx???Dll
 * stuff here and only here. An Exe element is an element with a library of
 * NULL.
 */

#define EP_COUNT 1361   /* should be a prime for distribution */

#define FUNCS   0
#define EXITS   1
#define SUBCOMS 2

typedef struct { /* lib_tsd: static variables of this module (thread-safe) */
   struct library *     first_library;
   struct library *     orphaned;
   struct entry_point  *ep[3][EP_COUNT]; /* FUNCS, EXITS, SUBCOMS */
   streng *             err_message;
} lib_tsd_t; /* thread-specific but only needed by this module. see
              * init_library
              */
/*
 * External functions, exits etc should be global; fixes bug 3489415
 * This is not implemented yet as doing this would require all access
 * to ltGlobal to be protected so multiple threads can't be adding/deleting
 * items fro the list of external functions/exits - TODO
 */
#ifdef TODO_WHEN_TS
static lib_tsd_t *ltGlobal=NULL;
#endif
/*
 * init_library initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_library( tsd_t *TSD )
{
#ifdef TODO_WHEN_TS
   if ( TSD->lib_tsd != NULL )
      return 1;

   if ( ltGlobal == NULL )
   {
      if ( ( ltGlobal = MallocTSD( sizeof( lib_tsd_t ) ) ) == NULL )
         return 0;
      memset( ltGlobal, 0, sizeof( lib_tsd_t ) );  /* correct for all values */
   }
   TSD->lib_tsd = ltGlobal;
#else
   lib_tsd_t *lt;

   if ( TSD->lib_tsd != NULL )
      return 1;

   if ( ( TSD->lib_tsd = MallocTSD( sizeof( lib_tsd_t ) ) ) == NULL )
      return 0;
   lt = (lib_tsd_t *)TSD->lib_tsd;
   memset( lt, 0, sizeof( lib_tsd_t ) );  /* correct for all values */
#endif
   return 1;
}

#ifdef DYNAMIC
/*
 * insert_library inserts the passed library in the linked list of used
 * libraries unconditionally.
 */
static void insert_library( const tsd_t *TSD, struct library *ptr )
{
   lib_tsd_t *lt;

   lt = (lib_tsd_t *)TSD->lib_tsd;
   ptr->prev = NULL;
   ptr->next = lt->first_library;
   lt->first_library = ptr;
   if ( ptr->next != NULL )
      ptr->next->prev = ptr;
}

/*
 * unlink_orphaned_libs attempts to remove the address space from unused
 * DLLs aka shared libraries.
 *
 * We must be extremely careful. Scenario:
 * Load a function package, e.g. w32funcs. This lets several functions
 * (e.g. win32func???) and win32load and win32unload been registered.
 * When the call of win32unload happens, each function name gets unload and
 * on the last call the entire library gets released. But this happens just
 * when RexxDeregisterFunction(win32unload) happens! The address space doesn't
 * exist any longer and the call does a return to unmapped memory. Crash!
 *
 * Therefore the freeing of the library moves the unused lib to the list of
 * orphaned libs which are removed when it's safe to do so.
 *
 * Alternatively we have to maintain a list of used external entry points
 * which lock the associated libraries only (move to orphaned), others are
 * freed immediately. The disadvantage is the maintainance problem with
 * longjmp/sigjmp.
 *
 * If the flag force is set we assume a clean state which should be set only
 * on reforking or terminating.
 */
static void unlink_orphaned_libs( const tsd_t *TSD, lib_tsd_t *lt, int force )
{
   struct library *ptr;

   if ( !lt->orphaned )
      return;

   if ( !force )
   {
      /*
       * The system is ready to remove every lib if TSD->systeminfo->previous
       * is empty AND TSD->systeminfo->input_name is empty.
       * Otherwise we don't catch the plain main() calls or other calls I can
       * imagine that use the first systeminfo directly.
       */
      if ( TSD->systeminfo )
      {
         if ( TSD->systeminfo->previous || TSD->systeminfo->input_file )
            return;
      }
   }

   while ( ( ptr = lt->orphaned ) != NULL )
   {
      lt->orphaned = ptr->next;
      if ( lt->orphaned )
         lt->orphaned->prev = NULL;

      assert( ptr->used == 0 );

      wrapper_unload( TSD, ptr->handle );
      assert( ptr->name );
      Free_stringTSD( ptr->name );
      FreeTSD( ptr );
   }
}

/*
 * remove_library removes the passed library from the linked list of used
 * libraries unconditionally.
 * The library will be unloaded and the name and the passed structure will be
 * freed later when it's safe to do so.
 * See unlink_orphaned_libs.
 */
static void remove_library( const tsd_t *TSD, struct library *ptr )
{
   lib_tsd_t *lt;

   assert( ptr->used == 0 );

   lt = (lib_tsd_t *)TSD->lib_tsd;
   if ( ptr->next )
      ptr->next->prev = ptr->prev;

   if ( ptr->prev )
      ptr->prev->next = ptr->next;
   else
      lt->first_library = ptr->next;

   ptr->next = lt->orphaned;
   if ( lt->orphaned )
      lt->orphaned->prev = ptr;

   lt->orphaned = ptr;

   /*
    * Now try to remove it really.
    */
   unlink_orphaned_libs( TSD, lt, 0 );
}
#endif

/*
 * remove_entry removes the passed library entry from the linked list of used
 * library entries unconditionally.
 * The slot must be either FUNCS, EXITS, or SUBCOMS.
 * Used memory will be freed and the holding library will be removed if this
 * entry was the last entry used of the library.
 */
static void remove_entry( tsd_t *TSD, struct entry_point *fptr, int slot )
{
   lib_tsd_t *lt;

   assert( slot >= FUNCS && slot <= SUBCOMS );

   lt = (lib_tsd_t *)TSD->lib_tsd;
   if ( fptr->name )
      Free_stringTSD( fptr->name );
#if defined(HAVE_GCI) && defined(DYNAMIC)
   if ( ( fptr->special.gci_info != NULL ) && ( slot == FUNCS ) )
      GCI_remove_structure( TSD, (GCI_treeinfo *)fptr->special.gci_info );
#endif
   if ( fptr->next )
      fptr->next->prev = fptr->prev;
   if ( fptr->prev )
      fptr->prev->next = fptr->next;
   else
      lt->ep[slot][fptr->hash % EP_COUNT] = fptr->next;

#ifdef DYNAMIC
   if ( fptr->lib != NULL )
   {
      assert( fptr->lib->used > 0 );
      if ( --fptr->lib->used == 0 )
         remove_library( TSD, fptr->lib );
   }
#endif

   FreeTSD( fptr );
}

/*
 * free_orphaned_libs disconnect from unused DLLs, see unlink_orphaned_libs.
 */
void free_orphaned_libs( tsd_t *TSD )
{
#ifdef DYNAMIC
   lib_tsd_t *lt = (lib_tsd_t *)TSD->lib_tsd;

   unlink_orphaned_libs( TSD, lt, 0 );
#else
   (TSD = TSD);
#endif
}

/*
 * purge_library frees all used memory used by every entry point that is
 * registered and unloads every library.
 * This routine is a little bit slow.
 */
void purge_library( tsd_t *TSD )
{
   struct entry_point *ep, *save_ep;
   lib_tsd_t *lt;
   int i, j;

   lt = (lib_tsd_t *)TSD->lib_tsd;
   if ( lt->first_library != NULL )
   {
      for ( i = FUNCS; i <= SUBCOMS; i++ )
      {
         for ( j = 0; j < EP_COUNT; j++ )
         {
            if ( ( ep = lt->ep[i][j] ) != NULL )
            {
               do {
                  save_ep = ep;
                  remove_entry( TSD, ep, i );
                  if ( ( ep = lt->ep[i][j] ) == save_ep )
                     break;
               } while ( ep != NULL );
               if ( lt->first_library == NULL )
                  goto fastEnd;
            }
         }
      }
   }
   fastEnd:
   assert( lt->first_library == NULL );
   lt->first_library = NULL;
#ifdef DYNAMIC
   unlink_orphaned_libs( TSD, lt, 1 );
#endif
   assert( lt->orphaned == NULL );
   lt->orphaned = NULL;
   memset( lt->ep, 0, sizeof( lt->ep ) );
}

#ifdef DYNAMIC
/*
 * find_library returns the internal structure associated with the passed
 * library name or NULL if such a library doesn't exist.
 */
struct library *find_library( const tsd_t *TSD, const streng *name )
{
   struct library *lptr;
   lib_tsd_t *lt;

   lt = (lib_tsd_t *)TSD->lib_tsd;
   lptr = lt->first_library;
   for ( ; lptr; lptr = lptr->next )
   {
      if ( !Str_cmp( name, lptr->name ) )
         return lptr;
   }

   return NULL;
}
#endif

/*
 * add_entry creates a new library entry from the passed data and inserts it
 * in the linked list of used entries unconditionally.
 * The slot must be either FUNCS, EXITS, or SUBCOMS.
 * rxname is the name that can be used by a REXX script.
 * addr is the entry point of the function/exit hook/subcom hook.
 * lptr is a loaded library or NULL for a call of RexxRegister???Exe.
 * Either gci_info or user_area may be set depending on the kind of the entry.
 *
 * The internal counter of the library isn't incremented.
 */
static void add_entry( const tsd_t *TSD, int slot, const streng *rxname,
                       PFN addr,
                       struct library *lptr, void *gci_info,
                       void *user_area )
{
   int hash0;
   lib_tsd_t *lt;
   struct entry_point *fptr;

   assert( slot >= FUNCS && slot <= SUBCOMS );
   lt = (lib_tsd_t *)TSD->lib_tsd;

   fptr = (struct entry_point *)MallocTSD( sizeof( struct entry_point ) );
   fptr->name = Str_upper( Str_dupstrTSD( rxname ) );
   fptr->hash = hashvalue( rxname->value, rxname->len );
   fptr->addr = addr;
   fptr->lib = lptr;
   memset( &fptr->special, 0, sizeof( fptr->special ) );
   if ( slot == FUNCS )
      fptr->special.gci_info = gci_info;
   else
   {
      if ( user_area != NULL )
         memcpy( fptr->special.user_area, user_area,
                 sizeof ( fptr->special.user_area ) );
   }

   hash0 = fptr->hash % EP_COUNT;
   fptr->next = lt->ep[slot][hash0];
   lt->ep[slot][hash0] = fptr;
   fptr->prev = NULL;
   if ( fptr->next )
      fptr->next->prev = fptr;
}

/*
 * find_entry_point returns NULL if no entry is found. Returns the exact entry
 * if both the name and the library match. Returns any entry with a fitting
 * name if the library doesn't match but the name exists.
 * library may be NULL for entries registered by RexxRegister???Exe.
 * The slot must be either FUNCS, EXITS, or SUBCOMS.
 */
static struct entry_point *find_entry_point( const tsd_t *TSD,
                                             const streng *name,
                                             void *library,
                                             int slot )
{
   struct entry_point *lptr;
   unsigned hash, hash0;
   lib_tsd_t *lt;
   struct entry_point *retval = NULL;

   lt = (lib_tsd_t *)TSD->lib_tsd;
   hash = hashvalue( name->value, name->len );
   hash0 = hash % EP_COUNT;
   for ( lptr = lt->ep[slot][hash0]; lptr; lptr = lptr->next )
   {
      if ( hash == lptr->hash )
         if ( Str_cmp( name, lptr->name ) == 0 )
         {
            if ( lptr->lib == library )
               return lptr;
            else
               retval = lptr;
         }
   }

   return retval;
}

/*
 * find_first_entry_point returns NULL if no entry is found and returns the
 * most recent hook otherwise.
 * The slot must be either FUNCS, EXITS, or SUBCOMS.
 */
static struct entry_point *find_first_entry_point( const tsd_t *TSD,
                                                   const streng *name,
                                                   int slot )
{
   struct entry_point *lptr;
   unsigned hash, hash0;
   lib_tsd_t *lt;

   lt = (lib_tsd_t *)TSD->lib_tsd;
   hash = hashvalue( name->value, name->len );
   hash0 = hash % EP_COUNT;
   for ( lptr = lt->ep[slot][hash0]; lptr; lptr = lptr->next )
   {
      if ( hash == lptr->hash )
         if ( Str_cmp( name, lptr->name ) == 0 )
            return lptr;
   }

   return NULL;
}

/*
 * find_all_entries returns 0 if no entry is found. Otherwise it returns the
 * number of all matching entries with the given name, different in the module
 * name only.
 * The slot must be either FUNCS, EXITS, or SUBCOMS.
 * *list will be set to a list of all available entries.
 *
 * This function is slow.
 */
static int find_all_entries( const tsd_t *TSD, const streng *name, int slot,
                             struct entry_point ***list )
{
   struct entry_point *lptr, **array;
   unsigned hash, hash0;
   lib_tsd_t *lt;
   int cnt;

   lt = (lib_tsd_t *)TSD->lib_tsd;
   hash = hashvalue( name->value, name->len );
   hash0 = hash % EP_COUNT;
   for ( cnt = 0, lptr = lt->ep[slot][hash0]; lptr; lptr = lptr->next )
   {
      if ( hash == lptr->hash )
         if ( Str_cmp( name, lptr->name ) == 0 )
            cnt++;
   }

   if ( cnt == 0 )
   {
      *list = NULL;
      return 0;
   }

   array = (struct entry_point **)MallocTSD( cnt * sizeof( struct entry_point * ) );
   *list = array;

   for ( cnt = 0, lptr = lt->ep[slot][hash0]; lptr; lptr = lptr->next )
   {
      if ( hash == lptr->hash )
         if ( Str_cmp( name, lptr->name ) == 0 )
            array[cnt++] = lptr;
   }

   return cnt;
}

/*
 * set_err_message replaces the current error message by a new one that will
 * be assembled as the concatenation of the two passed messages.
 * The created string will be returned by RxFuncErrMsg().
 */
void set_err_message( const tsd_t *TSD, const char *message1,
                      const char *message2 )
{
   lib_tsd_t *lt;
   int size;

   lt = (lib_tsd_t *)TSD->lib_tsd;
   if ( lt->err_message )
      Free_stringTSD( lt->err_message );

   size = strlen( message1 ) + strlen( message2 );
   lt->err_message = Str_makeTSD( size + 1 );
   if ( lt->err_message )
   {
      strcpy( lt->err_message->value, message1 );
      strcat( lt->err_message->value, message2 );
      lt->err_message->len = size;
   }
}

/*
 * load_entry creates a new library entry from the passed data and inserts it
 * in the linked list of used entries.
 * lptr is a loaded library or NULL for a call of RexxRegister???Exe.
 * rxname is the name that can be used by a REXX script.
 * objnam will be used if lptr != NULL only and is the name of the hook
 * or function that is exported by the library.
 * entry will be used if lptr == NULL only and is the entry point of the hook
 * or function.
 * The slot must be either FUNCS, EXITS, or SUBCOMS.
 * Either gci_info or user_area may be set depending on the kind of the entry.
 *
 * Return codes:
 *    0 on success.
 *    1 if the function is defined already.
 *    1 if the hook is defined already and bound to the same library. The new
 *      hook is rejected.
 *    2 if the hook is defined already and bound to another library. The new
 *      hook is accepted.
 *    3 if objnam isn't exported by lptr.
 *    4 if external libraries are not supported.
 */
static int load_entry( const tsd_t *TSD, struct library *lptr,
                       const streng *rxname, const streng *objnam,
                       PFN entry,
                       int slot, void *gci_info, void *user_area )
{
   int result=0;
   struct entry_point *fptr;

   assert( ( lptr != NULL ) ^ ( entry != NULL ) );
   assert( rxname != NULL );
   assert( slot >= FUNCS && slot <= SUBCOMS );
   /*
    * Check the exceptions first.
    */
   if ( ( fptr = find_entry_point( TSD, rxname, lptr, slot ) ) != NULL )
   {
      /*
       * EXITS and SUBCOMS may have the same callable name bound to different
       * modules.
       */
      if ( ( slot == FUNCS ) || ( fptr->lib == lptr ) )
         return 1;
      /*
       * must be a hook with the same name in a different module.
       */
      result = 2;
   }

   if ( lptr )
   {
      assert( objnam != NULL );
#ifdef DYNAMIC
      if ( ( entry = wrapper_get_addr( TSD, lptr, objnam ) ) == NULL )
         return 3;
      lptr->used++;
#else
      return 4;
#endif
   }

   add_entry( TSD, slot, rxname, entry, lptr, gci_info, user_area );
   return result;
}

/*
 * unload_entry removes a known library entry from the linked list of known
 * entries.
 *
 * rxname is the name that can be used by a REXX script.
 * module is the name of the library and may be NULL for a generic request
 * or is a RexxRegister???Exe registered funcion/hook shall be unloaded.
 * The slot must be either FUNCS, EXITS, or SUBCOMS.
 *
 * Return codes:
 *    0 on success.
 *    1 if the function/hook is not defined or a hook with this name is bound
 *      to different modules and the module name is not given.
 */
static int unload_entry( tsd_t *TSD, const streng *rxname,
                         const streng *module, int slot )
{
   struct entry_point *fptr, **list;
   struct library *lib;
   int cnt;

#ifdef DYNAMIC
   if ( module == NULL )
      lib = NULL;
   else
   {
      if ( ( lib = find_library( TSD, module ) ) == NULL )
         return 1;
   }
#else
   if ( module != NULL )
      return 1;
   lib = NULL;
#endif

   fptr = find_entry_point( TSD, rxname, lib, slot );
   if ( fptr == NULL )
      return 1;

   if ( fptr->lib == lib )
   {
      remove_entry( TSD, fptr, slot );
      return 0;
   }

   /*
    * Not a properly matching function. Check for the "wildcard" library.
    */
   if ( lib != NULL )
      return 1;

   /*
    * We need it the hard way. Check if more than one entry is registered.
    */
   cnt = find_all_entries( TSD, rxname, slot, &list );
   if ( cnt > 1 )
   {
      FreeTSD( list );
      return 1;
   }

   remove_entry( TSD, *list, slot );
   FreeTSD( list );
   return 0;
}

/*
 * loadrxfunc adds a new function to the set of registered entry points.
 *
 * lptr is a loaded library or NULL for a call of RexxRegisterFunctionExe.
 * rxname is the name that can be used by a REXX script.
 * objnam will be used if lptr != NULL only and is the name of the function
 * that is exported by the library.
 * entry will be used if lptr == NULL only and is the entry point of the
 * function.
 * gci_info may be set depending on whether RxFuncDefine is used.
 *
 * Returns a return code suitable for RexxRegisterFunction???.
 */
static int loadrxfunc( const tsd_t *TSD, struct library *lptr,
                       const streng *rxname, const streng *objnam,
                       PFN entry,
                       void *gci_info )
{
   int rc;

   rc = load_entry( TSD, lptr, rxname, objnam, entry, FUNCS, gci_info, NULL );
   switch ( rc )
   {
      case 0:  return 0;   /* RXFUNC_OK */
      case 1:  return 10;  /* RXFUNC_DEFINED */
      case 3:  return 50;  /* RXFUNC_ENTNOTFND */
      case 4:  return 60;  /* RXFUNC_NOTINIT */
   }
   assert ( rc != 0 );
   return 10000 + rc; /* something not recognisable */
}

/*
 * loadrxhook adds a new exit/subcom hook to the set of registered entry
 * points.
 *
 * lptr is a loaded library or NULL for a call of RexxRegister???Exe.
 * rxname is the name that can be used in a hook list.
 * objnam will be used if lptr != NULL only and is the name of the hook that
 * that is exported by the library.
 * entry will be used if lptr == NULL only and is the entry point of the hook.
 * user_area is the passed parameter called UserArea of the Registration.
 * The slot must be either EXITS or SUBCOMS.
 *
 * Returns a return code suitable for RexxRegister???.
 */
static int loadrxhook( const tsd_t *TSD, struct library *lptr,
                       const streng *rxname, const streng *objnam,
                       PFN entry,
                       void *user_area, int slot )
{
   int rc;

   rc = load_entry( TSD, lptr, rxname, objnam, entry, slot, NULL, user_area );
   switch ( rc )
   {
      case 0:  return 0;     /* RX???_OK */
      case 1:  return 30;    /* RX???_NOTREG */
      case 2:  return 10;    /* RX???_DUP */
      case 3:  return 50;    /* RX???_LOADERR */
      case 4:  return 1004;  /* RX???_NOTINIT */
   }
   assert ( rc != 0 );
   return 10000 + rc; /* something not recognisable */
}

/*
 * unloadrxhook removes a registered function entry point.
 *
 * rxname is the name that can be used by a REXX script.
 *
 * Returns a return code suitable for RexxDeregisterFunction.
 */
static int unloadrxfunc( tsd_t *TSD, const streng *rxname )
{
   assert( rxname != NULL );

   if ( unload_entry( TSD, rxname, NULL, FUNCS ) == 0 )
      return 0;
   return 30; /* RXFUNC_NOTREG */
}

/*
 * unloadrxhook removes a registered exit/subcom hook entry point.
 *
 * rxname is the name that can be used in a hook list.
 * module is the name of the module that contains the hook or NULL if either
 * the generic hook should be removed or if a RexxRegister???Exe-hook should
 * be removed. The later one has precedence.
 * The slot must be either EXITS or SUBCOMS.
 *
 * Returns a return code suitable for RexxDeregister???.
 */
static int unloadrxhook( tsd_t *TSD, const streng *rxname,
                         const streng *module, int slot )
{
   assert( rxname != NULL );

   if ( unload_entry( TSD, rxname, module, slot ) == 0 )
      return 0;
   return 30; /* RX???_NOTREG */
}

/*
 * rex_funcadd processes a RexxRegisterFunctionDll() or
 * RexxRegisterFunctionExe() request.
 *
 * rxname is the name that can be used by a REXX script.
 * module is the name of the library and may be NULL only a
 * RexxRegisterFunctionExe is processed.
 * objnam will be used if module != NULL only and is the name of the function
 * that is exported by the library.
 * entry will be used if module == NULL only and is the entry point of the
 * function.
 * gci_info may be set depending on whether RxFuncDefine is used.
 *
 * Returns a return code suitable for RexxRegisterFunction???.
 */
static int rex_funcadd( const tsd_t *TSD, const streng *rxname,
                        const streng *module, const streng *objnam,
                        PFN entry,
                        void *gci_info )
{
   struct library *lptr=NULL;
   int rc;
#ifdef DYNAMIC
   void *handle;
   int newhandle = 0;
#endif
   streng *regutil=Str_crestr( "regutil" );
   streng *rexxutil=Str_crestr( "rexxutil" );

   assert( rxname != NULL );

   if ( module != NULL )
   {
      assert( entry == NULL );
#ifdef DYNAMIC
      if ( Str_ccmp( module, rexxutil ) == 0 )
      {
         if ( ( lptr = find_library( TSD, rexxutil ) ) == NULL )
         {
            newhandle = 1;
            handle = wrapper_load( TSD, rexxutil ) ;
            if ( handle )
            {
               lptr = (struct library *)MallocTSD( sizeof( struct library )) ;
               lptr->name = Str_dupstrTSD( rexxutil ) ;
               lptr->handle = handle ;
               lptr->used = 0l;
            }
            else
            {
               Free_stringTSD( rexxutil );

               if ( Str_ccmp( module, regutil ) == 0 )
               {
                  if ( ( lptr = find_library( TSD, regutil ) ) == NULL )
                  {
                  newhandle = 1;
                  handle = wrapper_load( TSD, regutil ) ;
                  if ( handle )
                  {
                     lptr = (struct library *)MallocTSD( sizeof( struct library )) ;
                     lptr->name = Str_dupstrTSD( regutil ) ;
                     lptr->handle = handle ;
                     lptr->used = 0l;
                  }
                  else
                  {
                     Free_stringTSD( regutil );
                     return 40; /* RXFUNC_MODNOTFND */
                  }
                  insert_library( TSD, lptr ) ;
                  }
               }
            }
            insert_library( TSD, lptr ) ;
         }
      }
      if ( lptr == NULL )
      {
         if ( ( lptr = find_library( TSD, module ) ) == NULL )
         {
            newhandle = 1;
            handle = wrapper_load( TSD, module ) ;
            if ( handle )
            {
               lptr = (struct library *)MallocTSD( sizeof( struct library )) ;
               lptr->name = Str_dupstrTSD( module ) ;
               lptr->handle = handle ;
               lptr->used = 0l;
            }
            else
            {
               Free_stringTSD( regutil );
               Free_stringTSD( rexxutil );
               return 40; /* RXFUNC_MODNOTFND */
            }
            insert_library( TSD, lptr ) ;
         }
      }
#else
      Free_stringTSD( regutil );
      Free_stringTSD( rexxutil );
      return 60; /* RXFUNC_NOTINIT */
#endif
   }
   else
   {
      assert( entry != NULL );
   }
   if ( ( rc = loadrxfunc( TSD, lptr, rxname, objnam, entry, gci_info ) ) != 0 )
   {
#ifdef DYNAMIC
      if ( newhandle )
         remove_library( TSD, lptr );
#endif
   }
   Free_stringTSD( regutil );
   Free_stringTSD( rexxutil );
   return rc;
}

/*
 * rex_hookadd processes a RexxRegisterExitDll(), RexxRegisterExitExe(),
 * RexxRegisterSubcomDll(), or RexxRegisterSubcomExe() request.
 *
 * rxname is the name that can be used in a hook list.
 * module is the name of the library and may be NULL only a RexxRegister???Exe
 * is processed.
 * objnam will be used if module != NULL only and is the name of the hook
 * that is exported by the library.
 * entry will be used if module == NULL only and is the entry point of the
 * hook.
 * user_area is the passed parameter called UserArea of the Registration.
 * The slot must be either EXITS or SUBCOMS.
 *
 * Returns a return code suitable for RexxRegister???.
 */
static int rex_hookadd( const tsd_t *TSD, const streng *rxname,
                        const streng *module, const streng *objnam,
                        PFN entry,
                        void *user_area, int slot )
{
   struct library *lptr=NULL;
   int rc;
#ifdef DYNAMIC
   void *handle;
   int newhandle = 0;
#endif

   assert( rxname != NULL );

   if ( module != NULL )
   {
      assert( entry == NULL );
#ifdef DYNAMIC
      if ( ( lptr = find_library( TSD, module ) ) == NULL )
      {
         newhandle = 1;
         handle = wrapper_load( TSD, module ) ;
         if ( handle )
         {
            lptr = (struct library *)MallocTSD( sizeof( struct library )) ;
            lptr->name = Str_dupstrTSD( module ) ;
            lptr->handle = handle ;
            lptr->used = 0l;
         }
         else
         {
            return 50; /* RX???_LOADERR */
         }
         insert_library( TSD, lptr ) ;
      }
#else
      return 1004; /* RX???_NOTINIT */
#endif
   }
   else
   {
      assert( entry != NULL );
   }
   rc = loadrxhook( TSD, lptr, rxname, objnam, entry, user_area, slot );
   if ( ( rc != 0 ) && ( rc != 10 ) )
   {
#ifdef DYNAMIC
      if ( newhandle )
         remove_library( TSD, lptr );
#endif
   }
   return rc;
}

/*
 * rex_rxfuncerrmsg implements the BIF RxFuncErrMsg.
 */
streng *rex_rxfuncerrmsg( tsd_t *TSD, cparamboxptr parms )
{
#ifdef DYNAMIC
   lib_tsd_t *lt;
#endif

   checkparam( parms, 0, 0, "RXFUNCERRMSG" );

#ifdef DYNAMIC
   lt = (lib_tsd_t *)TSD->lib_tsd;
   if ( lt->err_message )
      return Str_dupTSD( lt->err_message );
   else
      return nullstringptr();
#else
   return Str_creTSD( "Module doesn't support dynamic linking; are you running the \"regina\" executable?" );
#endif
}

/*
 * rex_rxfuncquery implements the BIF RxFuncQuery.
 */
streng *rex_rxfuncquery( tsd_t *TSD, cparamboxptr parms )
{
#ifdef DYNAMIC
   streng *name;
   struct entry_point *fptr;
#endif

   checkparam( parms, 1, 1, "RXFUNCQUERY" );

#ifdef DYNAMIC
   name = Str_upper( Str_dupTSD( parms->value ) );
   fptr = find_entry_point( TSD, name, NULL, FUNCS );
   Free_stringTSD( name );

   if ( fptr )
      return int_to_streng( TSD, 0 );
   return int_to_streng( TSD, 1 );
#else
   return int_to_streng( TSD, 1 );
#endif
}


/*
 * rex_rxfuncadd implements the BIF RxFuncAdd.
 * The returned value is suitable for RexxRegisterFunctionDll.
 *
 * Parameters:
 *   1) name of the function to be added (in Rexx)
 *   2) name of object file to link in
 *   3) name of the function to be added (in the object file)
 */
streng *rex_rxfuncadd( tsd_t *TSD, cparamboxptr parms )
{
#ifdef DYNAMIC
   streng *rxname;
   streng *module, *objnam;
   int rc;
#endif

   if ( TSD->restricted )
      exiterror( ERR_RESTRICTED, 1, "RXFUNCADD" );

   checkparam( parms, 2, 3, "RXFUNCADD" );

#ifdef DYNAMIC
   rxname = Str_upper( Str_dupTSD( parms->value ) );
   objnam = parms->value;
   module = ( parms = parms->next )->value;
   if ( ( parms->next != NULL ) && ( parms->next->value != NULL ) )
      objnam = parms->next->value;

   rc = rex_funcadd( TSD, rxname, module, objnam, NULL, NULL );
   Free_stringTSD( rxname );
   return int_to_streng( TSD, rc );
#else
   return int_to_streng( TSD, 60 ); /* RXFUNC_NOTINIT */
#endif
}

#ifdef HAVE_GCI
/*
 * rex_rxfuncdefine implements the BIF RxFuncDefine.
 *
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
#endif

   if ( TSD->restricted )
      exiterror( ERR_RESTRICTED, 1, "RXFUNCDEFINE" );

   checkparam( parms, 4, 4, "RXFUNCDEFINE" );

#ifdef DYNAMIC
   rxname = Str_upper( Str_dupTSD( parms->value ) );
   objnam = parms->value;
   module = ( parms = parms->next )->value;
   parms = parms->next;
   if ( parms->value != NULL )
      objnam = parms->value;
   def_stem = parms->next->value;

   if ( ( rc = GCI_checkDefinition( TSD, def_stem, &gci_info ) ) != 0 )
   {
      Free_stringTSD( rxname );
      return int_to_streng( TSD, 1 );
   }

   rc = rex_funcadd( TSD, rxname, module, objnam, NULL, gci_info );
   Free_stringTSD( rxname );
   if ( rc )
      GCI_remove_structure( TSD, (GCI_treeinfo *)gci_info );
   return int_to_streng( TSD, rc );
#else
   return int_to_streng( TSD, 60 ); /* RXFUNC_NOTINIT */
#endif
}

/*
 * rex_gciprefixchar implements the BIF GciPrefixChar.
 *
 * parameters:
 *   1) new prefix character
 */
streng *rex_gciprefixchar( tsd_t *TSD, cparamboxptr parms )
{
   static const char valid[] = " !?_#$@"; /* last 3 are Regina specific */
   char oldval[2], newval[2];
   streng *value;

   checkparam( parms, 0, 1, "GCIPREFIXCHAR" );

   oldval[0] = TSD->gci_prefix[0];
   oldval[1] = TSD->gci_prefix[1];
   newval[1] = '\0';

   value = parms->value;
   if ( value )
   {
      if ( Str_len( value ) == 0 )
      {
         newval[0] = '\0';
      }
      else if ( Str_len( value ) > 1 )
      {
         exiterror( ERR_INCORRECT_CALL, 23, "GCIPREFIXCHAR", 1, tmpstr_of( TSD, value ) );
      }
      else
      {
         newval[0] = Str_val( value )[0];
         if ( newval[0] )
         {
            if ( strchr( valid, (int) newval[0] ) == NULL )
            {
               exiterror( ERR_INCORRECT_CALL, 28, "GCIPREFIXCHAR", 1, valid, newval );
            }
         }
         if ( newval[0] == ' ' )
         {
            newval[0] = '\0';
         }
      }
   }
   else
   {
      newval[0] = oldval[0];
   }

   TSD->gci_prefix[0] = newval[0];
   TSD->gci_prefix[1] = newval[1];

   return Str_creTSD( oldval );
}
#endif

/*
 * rex_rxfuncdrop implements the BIF RxFuncDrop.
 * The returned value is suitable for RexxDeregisterFunction.
 */
streng *rex_rxfuncdrop( tsd_t *TSD, cparamboxptr parms )
{
   streng *name;

   checkparam( parms, 1, 1, "RXFUNCDROP" );
   name = Str_upper( parms->value );

   return int_to_streng( TSD, unloadrxfunc( TSD, name ) );
}

/*
 * IfcRegFunc is the interface function for RexxRegisterFunctionExe and
 * RexxRegisterFunctionDll.
 * Either entry or module and objnam must be set.
 */
int IfcRegFunc( const tsd_t *TSD, const char *rxname, const char *module,
                const char *objnam,
                PFN entry )
{
   int rc;
   streng *ext;
   streng *intr=NULL;
   streng *lib=NULL;

   ext = Str_upper( Str_creTSD( rxname ) );
   if ( module && objnam )
   {
      intr = Str_creTSD( objnam );
      lib = Str_creTSD( module );
   }

   rc = rex_funcadd( TSD, ext, lib, intr, entry, NULL );

   Free_stringTSD( ext );
   if ( intr && lib )
   {
      Free_stringTSD( intr );
      Free_stringTSD( lib );
   }

   return rc;
}

/*
 * IfcRegHook is the interface function for RexxRegisterExitExe,
 * RexxRegisterExitDll, RexxRegisterSubcomExe, RexxRegisterSubcomtDll.
 * Either entry or module and objnam must be set.
 */
static int IfcRegHook( const tsd_t *TSD, const char *rxname,
                       const char *module, const char *objnam,
                       PFN entry,
                       void *user_area, int slot )
{
   int rc;
   streng *ext;
   streng *intr=NULL;
   streng *lib=NULL;

   ext = Str_upper( Str_creTSD( rxname ) );
   if ( module && objnam )
   {
      intr = Str_creTSD( objnam );
      lib = Str_creTSD( module );
   }

   rc = rex_hookadd( TSD, ext, lib, intr, entry, user_area, slot );

   Free_stringTSD( ext );
   if ( intr && lib )
   {
      Free_stringTSD( intr );
      Free_stringTSD( lib );
   }

   return rc;
}

/*
 * IfcRegExit is the interface function for RexxRegisterExitExe or
 * RexxRegisterExitDll.
 * Either entry or module and objnam must be set.
 */
int IfcRegExit( const tsd_t *TSD, const char *rxname, const char *module,
                const char *objnam,
                PFN entry,
                void *user_area )
{
   return IfcRegHook( TSD, rxname, module, objnam, entry, user_area, EXITS );
}

/*
 * IfcRegSubcom is the interface function for RexxRegisterSubcomExe or
 * RexxRegisterSubcomDll.
 * Either entry or module and objnam must be set.
 */
int IfcRegSubcom( const tsd_t *TSD, const char *rxname, const char *module,
                  const char *objnam,
                  PFN entry,
                  void *user_area )
{
   streng *env;
   env = Str_creTSD( rxname );
   set_subcomed_envir( TSD, env, 1 );
   Free_stringTSD( env );
   return IfcRegHook( TSD, rxname, module, objnam, entry, user_area, SUBCOMS );
}

/*
 * IfcDelFunc is the interface function for RexxDeregisterFunction.
 */
int IfcDelFunc( tsd_t *TSD, const char *rxname )
{
   int rc;
   streng *ext;

   ext = Str_upper( Str_creTSD( rxname ) );
   rc = unloadrxfunc( TSD, ext );
   Free_stringTSD( ext );

   return rc;
}

/*
 * IfcDelHook is the interface function for RexxDeregisterExit or
 * RexxDeregisterSubcom.
 */
static int IfcDelHook( tsd_t *TSD, const char *rxname, const char *module,
                       int slot )
{
   int rc;
   streng *ext,*mod;

   ext = Str_upper( Str_creTSD( rxname ) );
   if ( module != NULL )
      mod = Str_creTSD( module );
   else
      mod = NULL;
   rc = unloadrxhook( TSD, ext, mod, slot );
   Free_stringTSD( ext );
   if ( mod != NULL )
      Free_stringTSD( mod );

   return rc;
}

/*
 * IfcDelExit is the interface function for RexxDeregisterExit.
 */
int IfcDelExit( tsd_t *TSD, const char *rxname, const char *module )
{
   return IfcDelHook( TSD, rxname, module, EXITS );
}

/*
 * IfcDelSubcom is the interface function for RexxDeregisterSubcom.
 */
int IfcDelSubcom( tsd_t *TSD, const char *rxname, const char *module )
{
   streng *env;
   env = Str_creTSD( rxname );
   set_subcomed_envir( TSD, env, 0 );
   Free_stringTSD( env );
   return IfcDelHook( TSD, rxname, module, SUBCOMS );
}

/*
 * IfcQueryFunc is the interface function for RexxQueryFunction.
 */
int IfcQueryFunc( const tsd_t *TSD, const char *rxname )
{
   int rc;
   streng *ext;

   ext = Str_upper( Str_creTSD( rxname ) );
   rc = ( find_entry_point( TSD, ext, NULL, FUNCS ) != NULL ) ? 0 : 30;
   Free_stringTSD( ext );

   return rc;
}

/*
 * IfcQueryHook is the interface function for RexxQueryExit or RexxQuerySubcom.
 */
static int IfcQueryHook( const tsd_t *TSD, const char *rxname,
                         const char *module, int slot, void *user_area )
{
   streng *ext;
   struct entry_point *fptr,**list;
   struct library *lib;
   int cnt;
#ifdef DYNAMIC
   streng *mod;
#endif

   ext = Str_upper( Str_creTSD( rxname ) );
   if ( module != NULL )
   {
#ifdef DYNAMIC
      mod = Str_creTSD( module );
      lib = find_library( TSD, mod );
      Free_stringTSD( mod );
      if ( lib == NULL )
      {
         Free_stringTSD( ext );
         return 30; /* RX???_NOTREG */
      }
#else
      return 1004; /* RX???_NOTINIT */
#endif
   }
   else
      lib = NULL;

   fptr = find_entry_point( TSD, ext, lib, slot );

   if ( fptr == NULL )
   {
      Free_stringTSD( ext );
      return 30; /* RX???_NOTREG */
   }

   if ( fptr->lib != lib )
   {
      /*
       * Found via wildcard mechanism, check if more than one element exists
       * and if a wildcard is allowed.
       */
      if ( lib != NULL )
      {
         Free_stringTSD( ext );
         return 30; /* RX???_NOTREG */
      }

      cnt = find_all_entries( TSD, ext, slot, &list );
      FreeTSD( list );
      Free_stringTSD( ext );

      if ( cnt > 1 )
         return 30; /* RX???_NOTREG */
   }
   else
      Free_stringTSD( ext );

   if ( user_area != NULL )
      memcpy( user_area, fptr->special.user_area,
              sizeof ( fptr->special.user_area ) );
   return 0;
}

/*
 * IfcQueryExit is the interface function for RexxQueryExit.
 */
int IfcQueryExit( const tsd_t *TSD, const char *rxname, const char *module,
                  void *user_area )
{
   return IfcQueryHook( TSD, rxname, module, EXITS, user_area );
}

/*
 * IfcQuerySubcom is the interface function for RexxQuerySubcom.
 */
int IfcQuerySubcom( const tsd_t *TSD, const char *rxname, const char *module,
                    void *user_area )
{
   return IfcQueryHook( TSD, rxname, module, SUBCOMS, user_area );
}

struct entry_point *loaded_lib_func( const tsd_t *TSD, const streng *name )
{
   struct entry_point *box;
   streng *upp;

   upp = Str_upper( Str_dupTSD( name ) );
   box = find_first_entry_point( TSD, upp, FUNCS );
   Free_stringTSD( upp );

   return box;
}

/*
 * exit_hook returns the most recent exit handler of the given name.
 * The value may be NULL if no hook is registered.
 */
struct entry_point *exit_hook( const tsd_t *TSD, const char *env, int len )
{
   streng *name;
   struct entry_point *ret;

   name = Str_upper( Str_ncreTSD( env, len ) );
   ret = find_first_entry_point( TSD, name, EXITS );
   Free_stringTSD( name );

   return ret;
}

/*
 * subcom_hook returns the most recent subcom handler of the given name.
 * The value may be NULL if no hook is registered.
 */
struct entry_point *subcom_hook( const tsd_t *TSD, const char *com, int len )
{
   streng *name;
   struct entry_point *ret;

   name = Str_upper( Str_ncreTSD( com, len ) );
   ret = find_first_entry_point( TSD, name, SUBCOMS );
   Free_stringTSD( name );

   return ret;
}
