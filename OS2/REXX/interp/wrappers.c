/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1993-1994  Anders Christensen <anders@pvv.unit.no>
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
 * This file is 'a hell of a file'. It contain _anything_ that is neither
 * POSIX nor ANSI. The meaning is that when these things are needed in
 * the code, they are located in a wrapper in this file. Thus, if you need
 * to hack the code, it's likely that you only need to hack this file.
 *
 * At least, that's the theory ...
 */

#include "regina_c.h"

#if defined(WIN32)
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
#  endif
# endif
# include <windows.h>
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
#  endif
# endif
#endif

#if defined(OS2)
# define INCL_DOSMODULEMGR
# include <os2.h>
# define DONT_TYPEDEF_PFN
#endif

#ifdef _POSIX_SOURCE
# undef _POSIX_SOURCE
#endif

#include "rexx.h"

#ifdef DYNAMIC

/*
 * Most Unix systems have dlopen(), so set this as the default and
 * unset it for platforms that don't have it - except for HPUX
 */
# if defined(__hpux) && !defined(HAVE_DLFCN_H)
#  define DYNAMIC_HPSHLOAD
# endif

# if defined(HAVE_DLFCN_H) && !defined(DYNAMIC_HPSHLOAD)
#  define DYNAMIC_DLOPEN
# endif

# if defined(DYNAMIC_STATIC)
   typedef void * handle_type;
#  define DYNLIBPRE ""
#  define DYNLIBPST ""
# elif defined(DYNAMIC_DLOPEN)
#  include <dlfcn.h>
   typedef void *handle_type ;
#  ifndef RTLD_LAZY
#   define RTLD_LAZY 1
#  endif
#  if defined(__CYGWIN__)
#   define DYNLIBPRE ""
#   define DYNLIBPST ".dll"
#   define DYNLIBLEN 4
#  elif defined(__APPLE__) && defined(__MACH__)
#   define DYNLIBPRE "lib"
#   define DYNLIBPST ".dylib"
#   define DYNLIBLEN 9
#  elif defined(AIX)
#   define DYNLIBPRE "lib"
#   define DYNLIBPST ".a"
#   define DYNLIBLEN 5
#  else
#   define DYNLIBPRE "lib"
#   define DYNLIBPST ".so"
#   define DYNLIBLEN 6
#  endif
# elif defined(DYNAMIC_AIXLOAD)
#  include <sys/types.h>
#  include <sys/ldr.h>
   typedef int (*handle_type) ;
#  define DYNLIBPRE "lib"
#  define DYNLIBPST ".a"
#  define DYNLIBLEN 5

# elif defined(DYNAMIC_HPSHLOAD)
#  include <dl.h>
   typedef shl_t handle_type ;
#  define DYNLIBPRE "lib"
#  define DYNLIBPST ".sl"
#  define DYNLIBLEN 6

# elif defined(__BEOS__) || defined(__HAIKU__)
#  include <be/kernel/image.h>
#  define DYNAMIC_BEOS  1
   typedef image_id handle_type ;
/*
 * BeOS/HAIKU add-ons are all in a directory specified in $ADDON_PATH
 */
#  define DYNLIBPRE "lib"
#  define DYNLIBPST ".so"
#  define DYNLIBLEN 6

# elif defined(DYNAMIC_SKYOS)
   typedef sDllHandle * handle_type ;
#  define DYNLIBPRE "/boot/programs/rexx/extensions/lib"
#  define DYNLIBPST ".dll"
#  define DYNLIBLEN 40

# elif defined(DYNAMIC_OS2)
   typedef HMODULE handle_type ;
/*  typedef PHMODULE handle_type ; */

# elif defined(DYNAMIC_WIN32)
   typedef HINSTANCE handle_type ;

# elif defined(DYNAMIC_VMS)
#  define DYNLIBPRE "LIB"
#  define DYNLIBPST ".EXE"
#  define DYNLIBLEN 7
#endif /* DYNAMIC */

#include <errno.h>
#include <string.h>

#ifdef DYNAMIC_HPSHLOAD_NOT_NEEDED
/*
 * HP/UX doesn't find symbols unless they are 'true data'; entrypoints
 * for functions are not sufficient! Thus, we need to force it to
 * 'release' that these symbols are defined. There's no need to do
 * anything more, once it sets some flag somewhere ... it'll find'em
 * later. Weird!
 *
 * Datatype char only used to save space ... this is never used, so
 * there is no reason to waste space.
 *
 * Removed by MH 0.08h. Causes problems on HPUX 10.20 linker, and may
 * not be needed anymore.
 */

char Food_For_Fuzzy_HP_Linkers[] = {
   (char)exiterror, (char)getonechar,
   (char)nullstringptr,
   (char)atozpos, (char)checkparam, (char)find_library } ;
#endif

#if defined(DYNAMIC_AIXLOAD)
void *wrapper_dummy_for_aix( void )
{
}
#endif

void *wrapper_load( const tsd_t *TSD, const streng *module )
{
   handle_type handle=(handle_type)NULL ;
#if defined(DYNAMIC_OS2)
   CHAR LoadError[256];
   APIRET rc=0L;
#endif
#if defined(DYNAMIC_WIN32)
   char LoadError[256];
#endif
#if defined(DYNAMIC_HPSHLOAD) || defined(DYNAMIC_BEOS)
   char buf[1024];
#endif
#ifdef DYNAMIC_SKYOS
   sDllHandle tmp_handle;
#endif
#ifdef DYNAMIC_DLOPEN
   char *addon_dir;
#endif
   char *file_name, *module_name, *udpart, *orig_module;
   char *addon_name, *ptmp;
   int addon_len=0;

   orig_module = str_ofTSD( module );
#ifdef DYNLIBLEN
   module_name = (char *)MallocTSD( Str_len( module ) + strlen(DYNLIBPRE) +
                                    strlen(DYNLIBPST) + 1 ) ;
   strcpy(module_name, DYNLIBPRE );
   udpart = module_name + strlen(DYNLIBPRE);
   memcpy(udpart, module->value, Str_len(module) );
   strcpy(udpart + Str_len(module), DYNLIBPST );
   file_name = module_name;
# if defined(DYNAMIC_HPSHLOAD)
   file_name = buf;
# endif
#else
   file_name = module_name = str_ofTSD(module);
#endif

#if defined(DYNAMIC_STATIC)
   handle = static_dlopen( file_name );
   if (handle == NULL)
   {
      set_err_message(TSD, "static_dlopen() failed loading:", file_name );
      handle = (handle_type)NULL;
   }
#elif defined(DYNAMIC_DLOPEN)
   /*
    * If the REGINA_ADDON_DIR environment variable is set, use it to explicitly specify
    * the location of Regina external function packages
    */
   addon_dir = getenv( "REGINA_ADDON_DIR" );
   if ( addon_dir )
   {
      addon_len = strlen(addon_dir);
      addon_name = (char *)MallocTSD( Str_len( module ) + strlen(DYNLIBPRE) + strlen(DYNLIBPST) + addon_len + 2 ) ;
      strcpy(addon_name, addon_dir );
      if ( addon_name[addon_len-1] != '/' )
      {
         strcat( addon_name, "/" );
         addon_len++;
      }
      strcat(addon_name, DYNLIBPRE );
      ptmp = addon_name + strlen(DYNLIBPRE) + addon_len;
      memcpy(ptmp, module->value, Str_len(module) );
      strcpy(ptmp + Str_len(module), DYNLIBPST );
      handle = dlopen(addon_name, RTLD_LAZY );
      FreeTSD( addon_name );
   }
# if defined(HAVE_REGINA_ADDON_DIR)
   /*
    * Use the compiled in location of Regina addons to locate the external function package
    */
   if ( handle == NULL )
   {
      addon_len = strlen(HAVE_REGINA_ADDON_DIR);
      addon_name = (char *)MallocTSD( Str_len( module ) + strlen(DYNLIBPRE) + strlen(DYNLIBPST) + addon_len + 2 ) ;
      strcpy(addon_name, HAVE_REGINA_ADDON_DIR );
      if ( addon_name[addon_len-1] != '/' )
      {
         strcat( addon_name, "/" );
         addon_len++;
      }
      strcat(addon_name, DYNLIBPRE );
      ptmp = addon_name + strlen(DYNLIBPRE) + addon_len;
      memcpy(ptmp, module->value, Str_len(module) );
      strcpy(ptmp + Str_len(module), DYNLIBPST );
      handle = dlopen(addon_name, RTLD_LAZY );
      FreeTSD( addon_name );
   }
# endif
   if ( handle == NULL )
   {
      /*
       * Try and load the module name exactly as specified (before wrapping it
       * in lib*.so)
       */
      handle = dlopen( orig_module, RTLD_LAZY ) ;
      if (handle == NULL)
      {
         handle = dlopen( file_name, RTLD_LAZY ) ;
         /* deal with incorrect case in call */
         if (handle == NULL)
         {
            mem_lower( udpart, Str_len( module ) );
            handle = dlopen(module_name, RTLD_LAZY);

            if (handle == NULL)
            {
               mem_upper( udpart, Str_len( module ) );
               handle = dlopen(module_name, RTLD_LAZY);
               if ( handle == NULL )
               {
                  /*
                   * Reset the original module portion of the filename to be
                   * searched again so that any error message returned uses the
                   * original module name
                   */
                  memcpy(udpart, module->value, Str_len(module) );
                  handle = dlopen(module_name, RTLD_LAZY);
               }
            }
         }
      }
   }

   /* or maybe it's just not there */
   if (handle==NULL)
   {
      char *msg=NULL;
      msg = dlerror();
      if (msg)
         set_err_message(TSD, "dlopen() failed: ", msg ) ;
      else
         set_err_message(TSD, "", "");
   }
#elif defined(DYNAMIC_HPSHLOAD)
   /*
    * Try and load the module name exactly as specified (before wrapping it
    * in lib*.sl)
    */
   find_shared_library( TSD, orig_module, "SHLIB_PATH", buf );
   handle = shl_load( file_name, BIND_IMMEDIATE | DYNAMIC_PATH, 0L ) ;
   if (handle == NULL)
   {
      find_shared_library(TSD,module_name,"SHLIB_PATH",buf);
      handle = shl_load( file_name, BIND_IMMEDIATE | DYNAMIC_PATH, 0L ) ;
      if (handle == NULL)
      {
         mem_lower( udpart, Str_len( module ) );
         find_shared_library( TSD, module_name, "SHLIB_PATH", buf );
         handle = shl_load( file_name, BIND_IMMEDIATE | DYNAMIC_PATH ,0L ) ;

         if (handle == NULL)
         {
            mem_upper( udpart, Str_len( module ) );
            find_shared_library( TSD, module_name, "SHLIB_PATH", buf );
            handle = shl_load( file_name, BIND_IMMEDIATE | DYNAMIC_PATH ,0L ) ;
         }
      }
   }

   if (handle==NULL)
      set_err_message(TSD,  "shl_load() failed: ", strerror(errno)) ;
#elif defined(DYNAMIC_AIXLOAD)
   /*
    * Try and load the module name exactly as specified (before wrapping it
    * in lib*.a)
    */
   handle = load( orig_module, 1, NULL ) ;
   if ( handle == NULL )
   {
      handle = load( file_name, 1, NULL ) ;
      if ( handle == NULL )
      {
         set_err_message(TSD,  "load() failed: ", strerror( errno )) ;
      }
      else
      {
         int rc=loadbind( 0, (void *)wrapper_dummy_for_aix, (void *)handle ) ;
         if (rc)
         {
            handle = NULL ;
            set_err_message(TSD,  "loadbind() failed: ", strerror( errno )) ;
         }
      }
#elif defined(DYNAMIC_OS2)
   rc = DosLoadModule( LoadError, sizeof(LoadError),
                       file_name, &handle ) ;
   if (rc)
   {
      set_err_message(TSD, "DosLoadModule() unable to load DLL: ", LoadError);
      handle = (handle_type)NULL;
   }
#elif defined(DYNAMIC_WIN32)
   handle = LoadLibrary( file_name ) ;
   if (handle==NULL)
   {
      char buf[150];
      FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
      sprintf( buf, "Failed to load \"%s\" library: LoadLibrary() failed: ", file_name );
      set_err_message(TSD, buf, LoadError);
   }
#elif defined(DYNAMIC_BEOS)
   handle = load_add_on( orig_module );
   if (handle < B_NO_ERROR)
   {
      handle = load_add_on( file_name );
      if (handle < B_NO_ERROR)
      {
         sprintf( buf, "load_add_on() failed loading \"%s\" with error:", file_name );
         set_err_message(TSD, buf, strerror( handle ) );
         handle = (handle_type)NULL;
      }
   }
#elif defined(DYNAMIC_SKYOS)
   handle = (handle_type)MallocTSD( sizeof( sDllHandle ) );
   /*
    * Don't try and load the module without the trimmings.
    * You get a binary popup window otherwise!
    */
fprintf(stderr,"%s %d %x %s:\n",__FILE__,__LINE__,handle,file_name);
   if ( DllLoad( file_name, &tmp_handle ) != 0 )
   {
      char buf[150];
      sprintf( buf, "Failed to load \"%s\" library: DllLoad() failed: ", file_name );
      set_err_message(TSD, buf, strerror( errno ) );
      FreeTSD( handle );
      handle = (handle_type)NULL;
   }
fprintf(stderr,"%s %d:\n",__FILE__,__LINE__);
   memcpy( handle, &tmp_handle, sizeof( sDllHandle ) );
fprintf(stderr,"%s %d:\n",__FILE__,__LINE__);
#endif

   FreeTSD( module_name );
   FreeTSD( orig_module );

   return (void *)handle ;
}

void wrapper_unload( const tsd_t *TSD, void *libhandle )
{
#ifdef DYNAMIC_STATIC

   libhandle = libhandle;

#elif defined(DYNAMIC_DLOPEN)

   dlclose((handle_type) libhandle);

#elif defined(DYNAMIC_HPSHLOAD)

   shl_unload((handle_type) libhandle);

#elif defined(DYNAMIC_AIXLOAD)

   unload((handle_type) libhandle);

#elif defined(DYNAMIC_OS2)

   DosFreeModule((handle_type) libhandle);

#elif defined(DYNAMIC_WIN32)

   FreeLibrary((handle_type) libhandle);

#elif defined(DYNAMIC_BEOS)

   unload_add_on((handle_type) libhandle);

#elif defined(DYNAMIC_SKYOS)

fprintf(stderr,"%s %d:\n",__FILE__,__LINE__);
   FreeTSD( libhandle );

#else
   (libhandle = libhandle);
#endif
}

PFN wrapper_get_addr( const tsd_t *TSD, const struct library *lptr, const streng *name )
{
   PFN addr;
   handle_type handle=(handle_type)lptr->handle;
   char *funcname ;
#if defined(DYNAMIC_WIN32)
   char LoadError[256];
   char *entryname;
   unsigned u;
   char c;
#endif
#if defined(DYNAMIC_OS2)
   char *entryname;
   unsigned u;
   char c;
   ULONG ordinal;
   APIRET rc=0L;
#endif
#if defined(DYNAMIC_BEOS)
   status_t rc=0;
#endif
#if defined(DYNAMIC_STATIC)
   int rc=0;
#endif
#if defined(MODULES_NEED_USCORE)
   streng *us_func;
#endif

   funcname = str_of( TSD, name ) ;

#if defined(DYNAMIC_STATIC)
   rc = static_dlsym( handle, funcname,(void **)&addr );
   if ( rc != 0 )
   {
      char buf[150];
      sprintf(buf,"static_dlsym() failed with %d looking for %s", rc, funcname );
      set_err_message(TSD,  buf, "" ) ;
      addr = NULL;
   }
#elif defined(DYNAMIC_DLOPEN)
# if defined(MODULES_NEED_USCORE)
   /*
    * Some platforms need to have an underscore prepended to the function
    * name to be found in a loadable module.
    */
   FreeTSD( funcname );
   us_func = Str_makeTSD( Str_len( name ) + 1 );
   memcpy( us_func->value, "_", 1 );
   us_func->len = 1;
   Str_catTSD( us_func, name );
   funcname = str_of( TSD, us_func );
   Free_stringTSD( us_func );
# endif
   /*
    * Note, the following assignment is not allowed by ANSI, but SVR4.2
    * includes it as an example, so it is probably safe in this context
    */
   addr = (PFN)(dlsym( handle, funcname )) ;

   /* deal with, eg 'SysLoadFuncs' when the function is 'sysloadfuncs' or 'SYSLOADFUNCS' */
   if (addr == NULL)
   {
      mem_upper( funcname, strlen( funcname ) );
      addr = (PFN)(dlsym( handle, funcname )) ;

      if (addr == NULL)
      {
         mem_lower( funcname, strlen( funcname ) );
         addr = (PFN)(dlsym( handle, funcname )) ;

         if (addr==NULL)
            set_err_message( TSD,  "dlsym() failed: ", dlerror() );
      }
   }

#elif defined(DYNAMIC_HPSHLOAD)
   {
      long eaddr ;
      int rc;

      if (rc = shl_findsym( &handle, funcname, TYPE_PROCEDURE, &eaddr ))
      {
         mem_upper( funcname, strlen( funcname ) );
         if (rc = shl_findsym( &handle, funcname, TYPE_PROCEDURE, &eaddr ))
         {
            mem_lower( funcname, strlen( funcname ) );
            if (rc = shl_findsym( &handle, funcname, TYPE_PROCEDURE, &eaddr ))
            {
               addr = NULL ;
               set_err_message( TSD,  "shl_findsym() failed: ", strerror(errno) );
            }
         }
      }

      if (!rc)
         addr = (PFN)eaddr ;
   }

#elif defined(DYNAMIC_AIXLOAD)
   addr = (PFN)handle ;

#elif defined(DYNAMIC_OS2)
   if ( ( sscanf( funcname, "#%u%c", &u, &c ) == 1 ) && ( u != 0 ) )
   {
      ordinal = (ULONG) u;
      entryname = NULL;
   }
   else
   {
      ordinal = 0L;
      mem_upper( funcname, strlen( funcname ) );
      entryname = funcname;
   }
   rc = DosQueryProcAddr(handle,ordinal,entryname,&addr);
   if (rc)
   {
      mem_upper( entryname, strlen( entryname ) );
      rc = DosQueryProcAddr(handle,ordinal,entryname,&addr);
      if (rc)
      {
         mem_lower( entryname, strlen( entryname ) );
         rc = DosQueryProcAddr(handle,ordinal,entryname,&addr);
         if (rc)
         {
            char buf[150];
            /* in case of an error, DosQueryProcAddr returns addr == -1, so set it to 0 to be safe */
            addr = NULL;
            sprintf(buf,"DosQueryProcAddr() failed with %lu looking for %.90s", (long) rc, funcname );
            set_err_message(TSD, buf, "" ) ;
         }
      }
   }

#elif defined(DYNAMIC_WIN32)
   /*  13/12/1999 JH moved cast, (HMODULE), from second parm to first.  Removed
    * a compiler warning,
    */
   if ( ( sscanf( funcname, "#%u%c", &u, &c ) == 1 ) && ( u != 0 ) &&
                                                             ( u <= 0xFFFF ) )
      entryname = (char *) u;
   else
      entryname = funcname;

   addr = (PFN) GetProcAddress( (HMODULE) handle, entryname );

   if ( ( addr == NULL ) && ( funcname == entryname ) )
   {
      strlwr(funcname);
      addr = (PFN)GetProcAddress((HMODULE)handle,funcname);

      if (addr == NULL)
      {
         strupr(funcname);
         addr = (PFN)GetProcAddress((HMODULE)handle, funcname);
      }
   }
   if (addr == NULL)
   {
      char buf[150];
      FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL );
      sprintf( buf, "Failed to find \"%s\" in external library: GetProcAddress() failed: ", funcname );
      set_err_message( TSD, buf, LoadError );
   }

#elif defined(DYNAMIC_BEOS)
   rc = get_image_symbol(handle,funcname,B_SYMBOL_TYPE_TEXT,(void **)&addr);
   if (rc == B_BAD_IMAGE_ID)
   {
      char buf[150];
      sprintf(buf,"get_image_symbol() failed with %d looking for %s", rc, funcname );
      set_err_message( TSD,  buf, "" );
      addr = NULL;
   }

#elif defined(DYNAMIC_SKYOS)
fprintf(stderr,"%s %d:\n",__FILE__,__LINE__);
   addr = (PFN)GetDllFunction( handle, funcname );
   if ( addr == NULL )
   {
      char buf[150];
      sprintf(buf,"GetDllFunction() failed looking for %s", funcname );
      set_err_message( TSD,  buf, "" );
      addr = NULL;
   }
fprintf(stderr,"%s %d:\n",__FILE__,__LINE__);
#endif

   FreeTSD( funcname );

   if (addr)
      return (PFN)addr ;
   else
      return NULL ;
}
# ifdef SKYOS
/*
 * Required as entry point for DLL under SkyOS
 */
int DllMain( void )
{
   return 0;
}
# endif

#endif /* DYNAMIC */
