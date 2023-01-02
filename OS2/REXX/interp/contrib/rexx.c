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
 * Bug in LCC complier wchar.h that incorrectly says it defines stat struct
 * but doesn't
 */
#if defined(__LCC__)
# include <sys/stat.h>
#endif

#include "regina_c.h"

#if defined(WIN32) && defined(__IBMC__)
#include <windows.h>
#pragma warning(default: 4115 4201 4214)
#else
# ifdef RXLIB
#  define APIENTRY
#  if defined(__WATCOMC__) && defined(__NT__)
#   undef APIENTRY
#   include <windows.h>
#  endif
#  if defined(__MINGW32__) || defined(__LCC__)
#   undef APIENTRY
#   include <windows.h>
#  endif
#  if defined(WIN32) && defined(__BORLANDC__)
#   undef APIENTRY
#   include <windows.h>
#  endif

#  if defined(_MSC_VER)
#   undef APIENTRY
#   if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#    pragma warning(disable: 4115 4201 4214 4514)
#   endif
#   include <windows.h>
#   if _MSC_VER >= 1100
#    pragma warning(default: 4115 4201 4214)
#   endif
#  endif
# else /* not RXLIB */
#  if defined(__WATCOMC__) && defined(__NT__)
#   include <windows.h>
# endif
#  if defined(__MINGW32__) || defined(__LCC__)
#   include <windows.h>
#  endif
#  if defined(WIN32) && defined(__BORLANDC__)
#   include <windows.h>
#  endif

#  if defined(_MSC_VER) && !defined(__WINS__)
#   if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#    pragma warning(disable: 4115 4201 4214 4514)
#   endif
#   include <windows.h>
#   if _MSC_VER >= 1100
#    pragma warning(default: 4115 4201 4214)
#   endif
#  endif
# endif
#endif

#if defined(OS2) || defined(__EMX__)
# if defined(__WATCOMC__) && defined(RXLIB)
#  undef APIENTRY
# elif defined(__INNOTEK_LIBC__)
#  undef APIENTRY
#  define APIENTRY _System
# endif
# define INCL_BASE
# include <os2.h>
# define DONT_TYPEDEF_PFN
#endif

#include "rexx.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifndef HAVE_GETOPT_LONG
# include "mygetopt.h"
#endif

#ifdef VMS
# include <stat.h>
#elif defined(MAC)
# include "mac.h"
#else
# include <sys/stat.h>
#endif

#if defined(DJGPP) || defined(__EMX__) || defined(_MSC_VER) || (defined(__WATCOMC__) && !defined(__QNX__)) || defined(__EPOC32__)
# include <fcntl.h>
# if !defined(__WINS__) && !defined(__EPOC32__)
#  include <io.h>
# endif
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef __APPLE__
# include <mach-o/dyld.h>
#endif

/*
 * Since development of Ultrix has ceased, and they never managed to
 * fix a few things, we want to define a few things, just in order
 * to kill a few warnings ...
 */
#if defined(FIX_PROTOS) && defined(FIX_ALL_PROTOS) && defined(ultrix)
   int fstat( int fd, struct stat *buf ) ;
   int stat( char *path, struct stat *buf ) ;
#endif


/* Don't terminate the following lines by a semicolon */
GLOBAL_PROTECTION_VAR(regina_globals)


/*
 * Note: must match the settings of NUM_FORM_* in flags.h
 */
const char *numeric_forms[] = { "SCIENTIFIC", "ENGINEERING" } ;

/*
 * Note: these must match the definitions of INVO_* in defs.h
 */
const char *invo_strings[] = { "COMMAND", "FUNCTION", "SUBROUTINE" } ;

const char *argv0 = NULL;

/*
 * The following global is to store the TSD of the currently running
 * interpreter instance.
 * Each API call checks this variable, and if not NULL, this is used
 * as the TSD to use.
 * This enables multiple threads to run within the one interpreter instance.
 * Set by OPTIONS SINGLE_INTERPRETER
 */
const tsd_t *__regina_global_TSD = NULL;

static void usage( char * );

#ifdef TRACEMEM
void marksubtree( nodeptr ptr )
{
   unsigned i;

   while ( ptr )
   {
      markmemory( ptr, TRC_TREENODE );

      if ( ptr->name )
         markmemory( ptr->name, TRC_TREENODE );

      for ( i = 0; i < sizeof( ptr->p ) / sizeof( ptr->p[0] ); i++ )
         marksubtree( ptr->p[i] );


      if ( ( ptr->type == X_STRING ) || ( ptr->type == X_CON_SYMBOL ) )
      {
         if ( ptr->u.number )
         {
            markmemory( ptr->u.number, TRC_TREENODE );
            markmemory( ptr->u.number->num, TRC_TREENODE );
         }
      }
      else if ( ptr->type == X_CEXPRLIST )
      {
         if ( ptr->u.strng )
            markmemory( ptr->u.strng, TRC_TREENODE );
      }

      ptr = ptr->next;
   }
}
#endif /* TRACEMEM */


/*
 * GetArgv0 tries to find the fully qualified filename of the current program.
 * It uses some ugly and system specific tricks and it may return NULL if
 * it can't find any useful value.
 * The argument should be argv[0] of main() and it may be returned.
 * This function must not be called from another as the sole one when starting
 * up.
 */
static const char *GetArgv0(const char *argv0)
{
#ifdef WIN32
   char buf[512];

   if (GetModuleFileName(NULL, buf, sizeof(buf)) != 0)
      return(strdup(buf)); /* never freed up */
#elif defined(__QNX__) && defined(__WATCOMC__)
   char buffer[PATH_MAX];
   char *buf;
   if ( (buf = _cmdname(buffer) ) != NULL )
      return(strdup(buf)); /* never freed up */
#elif defined(OS2)
   char buf[512];
   PPIB ppib;

# ifdef __EMX__
   if (_osmode == OS2_MODE)
   {
# endif
      if (DosGetInfoBlocks(NULL, &ppib) == 0)
         if (DosQueryModuleName(ppib->pib_hmte, sizeof(buf), buf) == 0)
            return(strdup(buf));
# ifdef __EMX__
   }
# endif
#endif

#ifdef __APPLE__
   {
      /*
       * will work on MacOS X
       */
      char buf[1024];
      uint32_t result=sizeof(buf);
      if ( _NSGetExecutablePath( buf, &result ) == 0 )
      {
         return strdup( buf );
      }
   }
#elif defined(HAVE_READLINK)
   {
      /*
       * will work on Linux 2.1+
       */
      char buf[1024];
      int result;
      result = readlink("/proc/self/exe", buf, sizeof( buf ) );
      if ( ( result > 0 ) && ( result < (int) sizeof( buf ) ) && ( buf[0] != '[' ) )
      {
         buf[result] = '\0';
         return strdup( buf );
      }
   }
#endif
   /* No specific code has found the right file name. Maybe, it's coded
    * in argv0. Check it, if it is an absolute path. Be absolutely sure
    * to detect it safely!
    */
   if (argv0 == NULL)
      return(NULL);

   if (argv0[0] == '/') /* unix systems and some others */
      return(argv0);

   if ((argv0[0] == '\\') && (argv0[1] == '\\')) /* MS and OS/2 UNC names */
      return(argv0);

   if (rx_isalpha(argv0[0]) && (argv0[1] == ':') && (argv0[2] == '\\'))
      return(argv0); /* MS and OS/2 drive letter with path */

   return(NULL); /* not a proven argv0 argument */
}


/*
 * setup_system sets up some basics which are needed to use parts of the
 * interpreter.
 * This is a common routine of rexx's or regina's main() and every
 * SAA function.
 * isclient should be set to 1 if called from the SAA interface, 0 otherwise.
 *
 * Note that you have to set TSD->currlevel->script_exit as fast as possible.
 * Otherwise you are at high risk that an error will stop the whole process,
 * which is fatal if an application uses us via SAA API.
 */
void setup_system( tsd_t *TSD, int isclient )
{
   TSD->stddump = stderr;

   TSD->systeminfo = creat_sysinfo( TSD, Str_creTSD( "SYSTEM" ) );

   TSD->systeminfo->currlevel0 = TSD->currlevel = newlevel( TSD, NULL );
   TSD->systeminfo->trace_override = 0;
   TSD->isclient = isclient;
   /*
    * Under DJGPP setmode screws up Parse Pull and entering code
    * interactively :-(
    */
#if defined(__EMX__) || (defined(_MSC_VER) && !defined(__WINS__)) || (defined(__WATCOMC__) && !defined(__QNX__))
   fflush( stdin );
   fflush( stdout );
   fflush( stderr );
   setmode( fileno( stdin ), O_BINARY );
   setmode( fileno( stdout ), O_BINARY );
   setmode( fileno( stderr ), O_BINARY );
# ifdef _MSC_VER
   /* set binary mode for pipes as well! */
   _set_fmode(_O_BINARY);
# endif
#endif
}

static struct my_getopt_option long_options[] =
{
   {"help",        no_argument,       0,  'h' },
   {"debug",       optional_argument, 0,  'D' },
   {"debug",       optional_argument, 0,  'd' },
   {"trace",       optional_argument, 0,  't' },
   {"pause",       no_argument,       0,  'p' },
   {"version",     optional_argument, 0,  'v' },
   {"yaccdbg",     no_argument,       0,  'y' },
   {"restricted",  no_argument,       0,  'r' },
   {"interactive", optional_argument, 0,  'i' },
   {"args",        no_argument,       0,  'a' },
   {"compile",     no_argument,       0,  'c' },
   {"execute",     no_argument,       0,  'e' },
   {"locale",      required_argument, 0,  'l' },
   {"options",     required_argument, 0,  'o' },
   {0,             0,                 0,  0 }
};
/*
 * check_args examines the arguments of the program and assigns the values
 * to the various structures.
 *
 * A process exit of 0 is required if 0 is returned. Otherwise the number
 * of processed args including the zeroth is returned.
 */
static int check_args( tsd_t *TSD, int argc, char **argv,
                       int *compile_to_tokens, int *execute_from_tokens,
                       int *locale_set )
{
   int c;
   while (1)
   {
      int option_index = 0;

      c = my_getopt_long( argc, argv, "+hi::D::t::pv::yrd::acel:o:", long_options, &option_index );
      if ( c == -1 )
         break;

      switch(c)
      {
         case 'i':
            starttrace( TSD );
            set_trace_char( TSD, '?' );
            if ( optarg )
            {
               if ( strlen( optarg ) > 1 )
               {
                  usage( argv[0] );
                  fprintf( stdout, "\nThe passed switch `-t' allows just one additional character, Regina exits.\n" );
                  exit( 1 );
               }
               set_trace_char( TSD, *optarg );
            }
            else
               set_trace_char( TSD, 'A' );
            /*
             * Ensure PARSE SOURCE does not crash; Bug #571
             */
            TSD->systeminfo->input_file = Str_crestrTSD( "<stdin>" );
            TSD->systeminfo->input_fp = NULL;
            intertrace( TSD );
            intertrace( TSD );
            break;
/*
            starttrace( TSD );
            set_trace_char( TSD, '?' );
            set_trace_char( TSD, 'A' );
            intertrace( TSD );
            intertrace( TSD );
            break;
*/
         case 'p':
#if !defined(__WINS__) && !defined(__EPOC32__)
            set_pause_at_exit();
#endif
            break;

         case 'v':
            if ( optarg )
            {
               if ( strlen( optarg ) > 1 )
               {
                  fprintf( stdout, "\nThe passed switch `-v' allows just one additional character, Regina exits.\n" );
                  exit( 1 );
               }
               switch( *optarg )
               {
#if defined(HAVE_REGINA_ADDON_DIR)
                  case 'a':
                     fprintf( stdout, "%s\n", HAVE_REGINA_ADDON_DIR );
                     break;
#endif
                  case 'p':
                     /*
                      * Display any statically linked packages
                      */
#if defined( DYNAMIC_STATIC )
                     static_list_packages();
#else
                     fprintf( stdout, "\nRegina not built with static dynamic loading support.\n" );
#endif
                     break;
                  case 'v':
                     fprintf( stdout, "%s\n", REGINA_VERSION_MAJOR "." REGINA_VERSION_MINOR "." REGINA_VERSION_RELEASE REGINA_VERSION_SUPP );
                     break;
                  case 'b':
                     fprintf( stdout, "%d\n", REGINA_BITS );
                     break;
                  default:
                     fprintf( stdout, "\nUnsupported option to '-v' switch, Regina exits.\n" );
                     exit( 1 );
               }
            }
            else
               fprintf( stdout, "%s: %s (%d bit)\n", argv[0], PARSE_VERSION_STRING, REGINA_BITS );
            return 0;

         case 'y':
#ifndef NDEBUG
            __reginadebug = 1;   /* yacc-debugging */
#endif
            break;

         case 'r': /* safe-rexx */
            TSD->restricted = 1;
            break;

         case 't':
            if ( optarg )
            {
               if ( strlen( optarg ) > 1 )
               {
                  usage( argv[0] );
                  fprintf( stdout, "\nThe passed switch `-t' allows just one additional character, Regina exits.\n" );
                  exit( 1 );
               }
               queue_trace_char( TSD, *optarg );
            }
            else
               queue_trace_char( TSD, 'A' );
            TSD->systeminfo->trace_override = 1;
            break;

         case 'd':
         case 'D':
            if ( optarg )
            {
               if ( *optarg == 'm' )
                  TSD->listleakedmemory = 1;
            }
            break;

         case 'a': /* multiple args */
            TSD->systeminfo->invoked = INVO_SUBROUTINE;
            break;

         case 'c': /* compile to tokenised file */
            if ( *execute_from_tokens )
            {
               usage( argv[0] );
               fprintf( stdout, "\nThe flags `-c' and `-e' are mutually exclusive, Regina exits.\n" );
               exit( 1 );
            }
            *compile_to_tokens = 1;
            break;

         case 'e': /* execute from tokenised file */
            if ( *compile_to_tokens )
            {
               usage( argv[0] );
               fprintf( stdout, "\nThe flags `-c' and `-e' are mutually exclusive, Regina exits.\n" );
               exit( 1 );
            }
            *execute_from_tokens = 1;
            break;

         case 'l': /* set locale information, accept empty string */
            *locale_set = 1;
            set_locale_info( optarg );
            break;

         case 'o': /* command-line OPTIONS */
         {
            streng *opts = Str_creTSD( optarg );
            do_options( TSD, TSD->currlevel, opts, 0 );
            break;
         }

         case 'h': /* usage */
         case '?': /* usage */
            usage( argv[0] );
            return 0;

         default:
            usage( argv[0] );
            fprintf( stdout, "\nThe passed switch `-%c' is unknown, Regina exits.\n", c );
            exit( 1 );
      }
   }
   return optind;
}

/*
 * just_compile does a compile step without execution of the assigned input
 * file (TSD->systeminfo's input_file) to the file named outputname.
 *
 * On exit everything has been done.
 */
static void just_compile( tsd_t *TSD, char *outputname )
{
   int len;
   streng *SrcStr;
   internal_parser_type ipt;
   void *instore_buf;
   unsigned long instore_length;
   FILE *outfp;

   /*
    * Read the file
    */
   fseek( TSD->systeminfo->input_fp, 0, SEEK_END );
   len = (int) ftell( TSD->systeminfo->input_fp );
   rewind( TSD->systeminfo->input_fp );

   SrcStr = Str_makeTSD( len );
   if ( fread( Str_val( SrcStr ), len, 1, TSD->systeminfo->input_fp ) != 1 )
      exiterror( ERR_PROG_UNREADABLE, 1, "Unable to read input file" );
   SrcStr->len = len;

   /*
    * enter_macro() actually does the tokenising...
    */
   ipt = enter_macro( TSD, SrcStr, &instore_buf, &instore_length );
   fclose( TSD->systeminfo->input_fp );

   outfp = fopen( outputname, "wb" );
   if ( outfp == NULL )
      exiterror( ERR_PROG_UNREADABLE, 1, "Unable to open output file for "
                                                                   "writing" );
   if ( instore_buf == NULL )
      exiterror( ERR_PROG_UNREADABLE, 1, "Error tokenising input file" );
   if ( fwrite( instore_buf, instore_length, 1, outfp ) != 1 )
      exiterror( ERR_PROG_UNREADABLE, 1, "Unable to write contents of output "
                                                                      "file" );
   fclose( outfp );
}

/*
 * assign_args sets the current argument list to that one in argv. We count
 * from the next_arg element to to excluding argc.
 *
 * The value is put in TSD->currlevel->args which has to be preeassigned to
 * NULL.
 */
static void assign_args( tsd_t *TSD, int argc, int next_arg, char **argv )
{
   int i, len;
   streng *string;
   paramboxptr args, prev;

   if ( next_arg >= argc )
      return;

   if ( TSD->systeminfo->invoked == INVO_SUBROUTINE )
   {
      prev = NULL;
      for ( i = next_arg; i < argc; i++ )
      {
         args = (paramboxptr)MallocTSD( sizeof( parambox ) );
         memset( args, 0, sizeof( parambox ) );

         if ( i == next_arg )
            TSD->currlevel->args = args;
         else
            prev->next = args;
         args->value = Str_cre_TSD( TSD, argv[i] );
         prev = args;
      }

      return;
   }

   for ( i = next_arg, len = 0; i < argc; i++ )
      len += strlen( argv[i] ) + 1; /* delimiter or terminator */

   TSD->currlevel->args = (paramboxptr)MallocTSD( sizeof( parambox ) );
   if ( TSD->currlevel->args == NULL )
      exiterror( ERR_STORAGE_EXHAUSTED, 0 );
   args = TSD->currlevel->args;
   memset( args, 0, sizeof(parambox) );
   args->value = string = Str_makeTSD( len );

   for ( i = next_arg; i < argc; i++ )
   {
      string = Str_catstrTSD( string, argv[i] );
      string->value[string->len++] = ' ';
   }
   if ( string && string->len )
      string->len--;
}

/*
 * codeFromString translates string into a number and returns it. If this is
 * not possible, EXIT_SUCCESS is returned.
 */
static int codeFromString( tsd_t *TSD, streng *string )
{
   int error, rcode;

   if ( string )
   {
      /* fixes bug 657345 */
      rcode = streng_to_int( TSD, string, &error );
      if ( error )
         rcode = EXIT_SUCCESS;
   }
   else
      rcode = EXIT_SUCCESS;

   return rcode;
}

/*
 * execute_tokenized executes a tokenized script that has already been assigned
 * as the input file.
 *
 * The arguments must have been assigned, too.
 *
 * The return value is the value that the main routine should return to the OS.
 */
static int execute_tokenized( tsd_t *TSD )
{
   void *TinnedTree;
   unsigned long TinnedTreeLen;
   streng *command;
   streng *result;
   streng *environment;
   int err,RetCode;

   /*
    * Read the file into TinnedTree.
    */
   fseek( TSD->systeminfo->input_fp, 0, SEEK_END );
   TinnedTreeLen = ftell( TSD->systeminfo->input_fp );
   rewind( TSD->systeminfo->input_fp );
   TinnedTree = MallocTSD( TinnedTreeLen );
   if ( TinnedTree == NULL )
      exiterror( ERR_STORAGE_EXHAUSTED, 0 );
   if ( fread( TinnedTree, TinnedTreeLen, 1, TSD->systeminfo->input_fp ) != 1 )
      exiterror( ERR_PROG_UNREADABLE, 1, "Unable to read input file" );
   /*
    * Don't close the file because the plain text file remains open as well.
    * This inhibits the deletion or modification on most systems.
    */

   /*
    * Check if the file being read is a valid tokenised file.
    */
   if ( !IsValidTin( (const external_parser_type *)TinnedTree, TinnedTreeLen ) )
      exiterror( ERR_PROG_UNREADABLE,
                 1,
                 "The supplied file is not a valid Regina tokenised file" );

   /*
    * Set program file name and environment. The argument have been assigned
    * already.
    */
   command = Str_dupTSD( TSD->systeminfo->input_file );

   /*
    * Changed after 3.3RC1: The environment is set to SYSTEM instead of
    * the externally bound (ENVIR_PIPE) "DEFAULT".
    */
   environment = Str_creTSD( "SYSTEM" );

   flush_trace_chars( TSD );

   /*
    * do_instore() actually does the execution...
    */
   result = do_instore( TSD, command, TSD->currlevel->args, environment,
                        &err,
                        0,
                        TinnedTree, TinnedTreeLen,
                        NULL, 0, /* source file contents */
                        NULL,
                        TSD->systeminfo->invoked );

   FreeTSD( TinnedTree );
   Free_stringTSD( command );
   Free_stringTSD( environment );

   if ( result )
   {
      RetCode = codeFromString( TSD, result );
      Free_stringTSD( result );
   }
   else
      RetCode = err;

   return RetCode;
}

/*
 * execute_file executes a plain text script that has already been assigned
 * as the input file.
 *
 * The arguments must have been assigned, too.
 *
 * The return value is the value that the main routine should return to the OS.
 */
static int execute_file( tsd_t *TSD )
{
   FILE *fptr = TSD->systeminfo->input_fp;
   internal_parser_type parsing;
   streng *string;
   int RetCode;

   /*
    * From here we are interpreting...
    */
   fetch_file( TSD, fptr ? fptr : stdin, &parsing );
   if ( fptr )
      fclose( fptr );
   TSD->systeminfo->input_fp = NULL;

   if ( parsing.result != 0 )
      exiterror( ERR_YACC_SYNTAX, 1, parsing.tline );
   else
      TSD->systeminfo->tree = parsing;

#if !defined(MINIMAL) && !defined(VMS) && !defined(DOS) && !defined(_MSC_VER) && !defined(__IBMC__) && !defined(MAC)
   if ( !fptr )
   {
      struct stat buffer;
      int rc;

      rc = fstat( fileno( stdin ), &buffer );
      if ( ( rc == 0 ) && S_ISCHR( buffer.st_mode ) )
      {
         /*
          * FIXME. MH and FGC.
          * When does this happen. Add debugging code to determine this, because
          * after 2 glasses of rocket fuel it seems silly to have this code!
          * 13-5-2004.
          */
         printf( "  \b\b" );
         fflush( stdout );
         rewind( stdin );
      }
   }
#endif

   flush_trace_chars( TSD );

   string = interpret( TSD, TSD->systeminfo->tree.root );
   RetCode = codeFromString( TSD, string );
   if ( string )
      Free_stringTSD( string );

   return RetCode;
}

#ifdef RXLIB
# if defined(__LCC__)
int __regina_faked_main(int argc,char *argv[])
# else
int APIENTRY __regina_faked_main(int argc,char *argv[])
# endif
# define CALL_MAIN __regina_faked_main
#else
int main(int argc,char *argv[])
# define CALL_MAIN main
#endif
{
   tsd_t *TSD;
   int processed;
   int compile_to_tokens=0;
   int execute_from_tokens=0;
   int locale_set=0;
   int stdinput, rcode;
   jmp_buf jbuf;

#ifdef MAC
   InitCursorCtl( nil );
#endif

   if ( argv0 == NULL )
      argv0 = GetArgv0( argv[0] );

   TSD = GLOBAL_ENTRY_POINT();

   setup_system( TSD, 0 );

   if ( setjmp( jbuf ) )
   {
      /*
       * We may either be jumped normally after an EXIT instruction or after
       * an error. The first reason means normal continuation, the other
       * means that we have to do an immediate stop.
       */
      if ( !TSD->instore_is_errorfree )
      {
         if ( TSD->systeminfo->result )
         {
            return atoi( TSD->systeminfo->result->value );
         }
         return -1;
      }
      else
      {
         if ( TSD->systeminfo->result )
            rcode = codeFromString( TSD, TSD->systeminfo->result );
         else
            rcode = EXIT_SUCCESS;
      }
   }
   else
   {
      TSD->systeminfo->script_exit = &jbuf;

      processed = check_args( TSD, argc, argv, &compile_to_tokens,
                              &execute_from_tokens, &locale_set );

      if ( processed == 0 )
         return 0;

      if ( !locale_set )
      {
         /*
          * Check for a comma separated default locale in REGINA_LANG.
          */
         char *ptr = getenv( "REGINA_LANG" );
         if ( ptr )
            ptr = strchr( ptr, ',' );
         if ( ptr )
            set_locale_info( ptr + 1 );
      }

      if ( processed < argc )
      {
         stdinput = 0;
         TSD->systeminfo->input_file = get_external_routine( TSD,
                                argv[processed], &TSD->systeminfo->input_fp );
         if ( !TSD->systeminfo->input_file )
         {
            TSD->systeminfo->input_file = Str_crestrTSD( argv[processed] );
            exiterror( ERR_PROG_UNREADABLE, 1, "Program was not found" );
         }
         processed++;
      }
      else
      {
         stdinput = 1;
         TSD->systeminfo->input_file = Str_crestrTSD( "<stdin>" );
         TSD->systeminfo->input_fp = NULL;
         if ( compile_to_tokens )
            exiterror( ERR_PROG_UNREADABLE, 1, "Too few arguments when "
                                "tokenising. Usage: -c inputfile outputfile" );
         if ( execute_from_tokens )
            exiterror( ERR_PROG_UNREADABLE, 1, "Cannot run tokenised code "
                                                               "from stdin." );
      }
      /* RFE #36 - set .FILE reserved name */
      set_reserved_value( TSD, POOL0_FILE, Str_dupTSD( TSD->systeminfo->input_file ), 0, VFLAG_STR );

      /*
       * -c switch specified - tokenise the input file before mucking around
       * with parameters etc.
       */
      if ( compile_to_tokens )
      {
         if ( processed >= argc )
            exiterror( ERR_PROG_UNREADABLE, 1, "Too few arguments when "
                                "tokenising. Usage: -c inputfile outputfile" );
         if ( processed + 1 < argc )
            exiterror( ERR_PROG_UNREADABLE, 1, "Too many arguments when "
                                "tokenising. Usage: -c inputfile outputfile" );

         just_compile( TSD, argv[processed] );
         return 0;
      }


      assign_args( TSD, argc, processed, argv );
      signal_setup( TSD );

      /*
       * -e switch specified - execute from tokenised code
       */
      if ( execute_from_tokens )
         rcode = execute_tokenized( TSD );
      else
         rcode = execute_file( TSD );
   }

#if defined(DEBUG) || defined(TRACEMEM)
   /*
    * Now do the cleanup. We don't need in real life, but for a proper cleanup
    * and for debugging aid it is a good idea to track down the whole beast.
    */
   purge_stacks( TSD );
   purge_filetable( TSD );
# if defined(FLISTS) && defined(NEW_FLISTS)
   free_flists();
# endif

# ifdef DYNAMIC
   /*
    * Remove all external function package functions
    * and libraries. Only valid for the DYNAMIC library.
    */
   purge_library( TSD );
# endif

# ifdef TRACEMEM
   if ( TSD->listleakedmemory )
      listleaked( TSD, MEMTRC_LEAKED );
# endif

   TSD->systeminfo->script_exit = NULL; /* cannot be freed, it's on the stack*/
   killsystem( TSD, TSD->systeminfo );
   TSD->systeminfo = NULL;

   /*
    * Remove all memory allocated by the flists internal memory manager.
    */
# ifdef FLISTS
   purge_flists( TSD );
# endif

#endif /* DEBUG */

   return rcode;
}

/* reexecute_main is possibly called by one of the fork_exec routines.
 * This functions cleans up some stuff to reexecute without problems.
 * The most useful thing to be done here is freeing all used memory.
 * NOTE: usage is always the last thing you should try. Better use
 * spawn or exec to let a fresh interpreter do the work.
 */
int __regina_reexecute_main(int argc, char **argv)
{
   tsd_t *TSD;

   TSD = __regina_get_tsd(); /* hopefully not multithreading! */

   if (TSD != NULL) /* yes! I don't know what happens on forking */
   {                /* and active multi-threading                */

      purge_stacks(TSD);    /* see main above for comments */
      purge_filetable(TSD);
#ifdef DYNAMIC
      purge_library(TSD);
#endif
#if defined(FLISTS)
# if defined(NEW_FLISTS)
      free_flists();
# endif
      purge_flists(TSD);
#endif
   }

   return(CALL_MAIN(argc, argv));
}

#ifdef TRACEMEM
void mark_systeminfo( const tsd_t *TSD )
{
   sysinfo sinfo=NULL ;
   labelbox *lptr=NULL ;
   lineboxptr llptr=NULL ;

   for (sinfo=TSD->systeminfo; sinfo; sinfo=sinfo->previous)
   {
      markmemory(sinfo, TRC_SYSINFO) ;
      markmemory(sinfo->input_file, TRC_SYSINFO) ;
      markmemory(sinfo->environment, TRC_SYSINFO) ;
      markmemory(sinfo->callstack, TRC_SYSINFO) ;

      markvariables( TSD, sinfo->currlevel0 ) ;
      marksource( sinfo->tree.first_source_line ) ;
      /* FGC, FIXME: rewrite this: marksubtree( sinfo->tree.root ) ; */

      for (lptr=sinfo->tree.first_label; lptr; lptr=lptr->next )
      {
         markmemory( lptr, TRC_SYSINFO ) ;
      }

      for (llptr=sinfo->tree.first_source_line; llptr; llptr=llptr->next )
      {
         markmemory( llptr, TRC_SYSINFO ) ;
         markmemory( llptr->line, TRC_SYSINFO ) ;
      }
   }

   markmemory( TSD->mem_tsd, TRC_SYSINFO );
   markmemory( TSD->var_tsd, TRC_SYSINFO );
   markmemory( TSD->stk_tsd, TRC_SYSINFO );
   markmemory( TSD->fil_tsd, TRC_SYSINFO );
   markmemory( TSD->itp_tsd, TRC_SYSINFO );
   markmemory( TSD->tra_tsd, TRC_SYSINFO );
   markmemory( TSD->err_tsd, TRC_SYSINFO );
   if (TSD->vms_tsd )
      markmemory( TSD->vms_tsd, TRC_SYSINFO );
   markmemory( TSD->bui_tsd, TRC_SYSINFO );
   if (TSD->vmf_tsd )
      markmemory( TSD->vmf_tsd, TRC_SYSINFO );
   markmemory( TSD->lib_tsd, TRC_SYSINFO );
   if (TSD->rex_tsd )
      markmemory( TSD->rex_tsd, TRC_SYSINFO );
   markmemory( TSD->shl_tsd, TRC_SYSINFO );
   markmemory( TSD->mat_tsd, TRC_SYSINFO );
   if (TSD->cli_tsd )
      markmemory( TSD->cli_tsd, TRC_SYSINFO );
   markmemory( TSD->arx_tsd, TRC_SYSINFO );
   markmemory( TSD->mt_tsd, TRC_SYSINFO );
}
#endif


sysinfobox *creat_sysinfo( const tsd_t *TSD, streng *envir )
{
   sysinfobox *sinfo;

   sinfo = (sysinfobox *)MallocTSD( sizeof(sysinfobox) );
   sinfo->environment = envir;
   sinfo->tracing = DEFAULT_TRACING;
   sinfo->interactive = DEFAULT_INT_TRACING;
   sinfo->previous = NULL;
   sinfo->invoked = INVO_COMMAND;
   sinfo->input_file = NULL;
   sinfo->input_fp = NULL;
   sinfo->script_exit = NULL ;
   sinfo->hooks = 0;
   sinfo->callstack = (nodeptr *)MallocTSD( sizeof( nodeptr ) * 10 );
   sinfo->result = NULL;
   sinfo->cstackcnt = 0;
   sinfo->cstackmax = 10;
   sinfo->trace_override = 0;
   sinfo->ctrlcounter = 0;
   memset( &sinfo->tree, 0, sizeof( sinfo->tree ) );

   return sinfo;
}
/*
 * The following two functions are used to set and retrieve the value of
 * the global TSD
 */
void setGlobalTSD( const tsd_t *TSD)
{
   __regina_global_TSD = TSD;
}

const tsd_t *getGlobalTSD( void )
{
   return __regina_global_TSD;
}

#if !defined(RXLIB) && !defined(VMS)

static void NoAPI( void )
{
   fprintf (stderr, "Warning: SAA API not compiled into interpreter\n" ) ;
}

int hookup( tsd_t *TSD, int dummy )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   assert( 0 ) ;
   return 1 ;  /* to keep compiler happy */
}
int hookup_input( tsd_t *TSD, int dummy1, streng **dummy2 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   assert( 0 ) ;
   return 1 ;  /* to keep compiler happy */
}
int hookup_input_output( tsd_t *TSD, int dummy1, const streng *dummy2, streng **dummy3 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   assert( 0 ) ;
   return 1 ;  /* to keep compiler happy */
}
int hookup_output( tsd_t *TSD, int dummy1, const streng *dummy2 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   assert( 0 ) ;
   return 1 ;  /* to keep compiler happy */
}
int hookup_output2( tsd_t *TSD, int dummy1, const streng *dummy2, const streng *dummy3 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   assert( 0 ) ;
   return 1 ;  /* to keep compiler happy */
}

static void Exit( const tsd_t *TSD )
{
   /*
    * cheat about the const, we go away anyway :-)
    */
   jump_interpreter_exit( ( tsd_t * ) TSD, 1 );
}

streng *call_unknown_external( tsd_t *TSD, const streng *dummy1, cparamboxptr dummy2, char dummy3 )
{
   NoAPI();
   Exit( TSD ) ;
   return NULL;
}

streng *call_known_external( tsd_t *TSD, const struct entry_point *dummy1, cparamboxptr dummy2, char dummy3 )
{
   NoAPI();
   Exit( TSD ) ;
   return NULL;
}


streng *SubCom( tsd_t *TSD, const streng *dummy1, const streng *dummy2, int *dummy3 )
{
   NoAPI();
   Exit( TSD ) ;
   return NULL;
}

int IfcHaveFunctionExit(const tsd_t *TSD)
{
   return(0);
}

#endif

static void usage( char *argv0 )
{
   fprintf( stdout, "\n%s: %s (%d bit). All rights reserved.\n", argv0, PARSE_VERSION_STRING, REGINA_BITS );
   fprintf( stdout,"Regina is distributed under the terms of the GNU Library General Public License \n" );
   fprintf( stdout,"and comes with NO WARRANTY. See the file COPYING-LIB for details.\n" );
   fprintf( stdout,"\nTo run a Rexx program:\n" );
   fprintf( stdout,"%s [switches] [program] [arguments...]\n", argv0 );
   fprintf( stdout,"where switches are:\n\n" );
   fprintf( stdout,"  --help, -h                      show this message\n" );
   fprintf( stdout,"  --version[=option], -v[option]  display various Regina details and exit\n" );
   fprintf( stdout,"    option one of:\n" );
   fprintf( stdout,"       no option - full Regina version\n" );
   fprintf( stdout,"       a - ADDONS directory\n" );
   fprintf( stdout,"       b - bits; 32 or 64\n" );
   fprintf( stdout,"       p - external packages statically linked\n" );
   fprintf( stdout,"       v - version only\n" );
   fprintf( stdout,"  --restricted, -r                run Regina in \"safe\" mode\n" );
   fprintf( stdout,"  --trace[=char], -t[char]        set TRACE to any valid TRACE character - default A\n" );
   fprintf( stdout,"  --interactive[=char], -i[char]  set TRACE to any valid TRACE character and run interactively - default A\n" );
   fprintf( stdout,"  --args, -a                      pass command line to Rexx program as separate arguments\n");
   fprintf( stdout,"  --pause, -p                     pause after execution (Win32 only)\n");
   fprintf( stdout,"  --locale=locale, -llocale       use the system's default charset or a supplied one\n");
   fprintf( stdout,"  --options=OPTIONS, -oOPTIONS    specify OPTIONS in same format as OPTIONS instruction\n");
   fprintf( stdout,"  --compile, -c                   see \"To tokenise a Rexx program:\" below\n");
   fprintf( stdout,"  --execute, -e                   see \"To execute a tokenised file:\" below\n");
   fprintf( stdout,"\"program\" is the file containing Rexx code to execute\n");
   fprintf( stdout,"\"arguments\" are arguments passed to program\n");
   fprintf( stdout,"\nTo tokenise a Rexx program:\n" );
   fprintf( stdout,"%s -c program(input) tokenisedfile(output)\n", argv0 );
   fprintf( stdout,"\nTo execute a tokenised file:\n" );
   fprintf( stdout,"%s -e tokenisedfile [arguments...]\n", argv0 );
   fprintf( stdout,"\nIf you intend using external functions (using RxFuncAdd) you need to run the \"regina\" executable\n" );
   fflush( stdout );
}
