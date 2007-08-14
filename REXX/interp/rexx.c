#ifndef lint
static char *RCSid = "$Id: rexx.c,v 1.2 2003/12/11 04:43:16 prokushev Exp $";
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

#include "rexx_charset.h"

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
# endif
# define INCL_BASE
# include <os2.h>
# define DONT_TYPEDEF_PFN
#endif

#include "rexx.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

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

static void usage( char * );

#ifdef TRACEMEM
void marksubtree( nodeptr ptr )
{
   int i=0 ;
   if ( ptr )
   {
      markmemory(ptr,TRC_TREENODE) ;
      if (ptr->name) markmemory(ptr->name, TRC_TREENODE) ;
      for (i=0;i<sizeof(ptr->p)/sizeof(ptr->p[0]);marksubtree(ptr->p[i++])) ;
      if (ptr->next) marksubtree( ptr->next ) ;

      if (ptr->type == X_STRING || ptr->type == X_CON_SYMBOL)
         if (ptr->u.number)
         {
            markmemory( ptr->u.number, TRC_TREENODE ) ;
            markmemory( ptr->u.number->num, TRC_TREENODE ) ;
         }

      if (ptr->type == X_CEXPRLIST)
         if (ptr->u.strng)
            markmemory( ptr->u.strng, TRC_TREENODE ) ;
   }
}
#endif /* TRACEMEM */


/* FIXME, FGC: This is a useless function! */
nodeptr treadit( cnodeptr tree )
{
/*
   nodeptr left, mid, right ;
 */
   tree = tree;
   return NULL ;
/*
   if (!tree)
      return NULL ;

   left = tree->p[0] ;
   mid = tree->p[1] ;
   right = tree->p[2] ;

   switch (tree->type)
   {
      case X_OTHERWISE:
      case X_PROGRAM:
         treadit( left ) ;
         tree = NULL ;
         break ;

      case X_STATS:
      case X_WHENS:
         left->next = treadit( mid ) ;
         treadit( left ) ;
         tree = left ;
         break ;

      case X_IF:
         treadit( mid ) ;
      case X_DO:
         treadit( right ) ;
         break ;

      case X_SELECT:
         treadit( left ) ;
      case X_WHEN:
         treadit( mid ) ;
         break ;
   }

   return tree ;
 */
}

/* GetArgv0 tries to find the fully qualified filename of the current program.
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

#ifdef HAVE_READLINK
   {
      /*
       * will work on Linux 2.1+
       */
      char buf[1024];
      int result;
      result = readlink("/proc/self/exe", buf, sizeof( buf ) );
      if ( ( result > 0 ) && ( result < sizeof( buf ) ) && ( buf[0] != '[' ) )
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

   if (isalpha(argv0[0]) && (argv0[1] == ':') && (argv0[2] == '\\'))
      return(argv0); /* MS and OS/2 drive letter with path */

   return(NULL); /* not a proven argv0 argument */
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
   FILE *fptr = NULL ;
   streng *string=NULL ;
   int i=0, j=0, stdinput=1, state=0, rcode=0, oldi=0, trace_override=0 ;
   paramboxptr args=NULL, prev ;
   char *arg=NULL ;
   int do_yydebug=0;
   char name[1024];
   internal_parser_type parsing;
   tsd_t *TSD;
   int compiling_to_tokens = 0;
   int executing_from_tokens = 0;

#ifdef MAC
   InitCursorCtl(nil);
#endif

   if (argv0 == NULL)
      argv0 = GetArgv0(argv[0]);


   TSD = GLOBAL_ENTRY_POINT();

   TSD->stddump = stderr ;

   TSD->systeminfo = creat_sysinfo( TSD, Str_creTSD("SYSTEM")) ;
   TSD->systeminfo->called_as = Str_creTSD( argv[0] ) ;

   TSD->systeminfo->currlevel0 = TSD->currlevel = newlevel( TSD, NULL ) ;
   TSD->systeminfo->trace_override = 0;
   TSD->isclient = 0;

   for (i=1; i<argc; i++)
   {
      arg = argv[i] ;
      if ( state == 0 )
      {
         if ( *arg == '-' )
         {
            switch (*(++arg))
            {
               case 'i':
                  starttrace(TSD) ;
                  set_trace_char(TSD, 'A') ;
                  intertrace(TSD) ;
                  intertrace(TSD) ;
                  break ;

               case 'C':
                  if (*(arg+1)=='i')
                  {
                     TSD->isclient = 1 ; /* Other than the default value of 0 */
                  }
                  break ;

               case 'p':
#if defined(WIN32) && !defined(__WINS__) && !defined(__EPOC32__)
                  set_pause_at_exit();
#endif
                  break ;

               case 'v':
                  fprintf( stderr, "%s\n", PARSE_VERSION_STRING );
                  return 0;
                  break ;

               case 'y':
                  do_yydebug = 1 ;
                  break ;

               case 'r': /* safe-rexx */
                  TSD->restricted = 1 ;
                  break ;

               case 't':
                  queue_trace_char(TSD, (char) (*(arg+1)? *(++arg) : 'A')) ;
                  trace_override = 1;
                  break ;

               case 'd':
                  if (*(arg+1)=='m')
                     TSD->listleakedmemory = 1 ;
                  break ;

               case 'a': /* multiple args */
                  TSD->systeminfo->invoked = INVO_SUBROUTINE;
                  break ;

               case 'c': /* compile to tokenised file */
                  compiling_to_tokens = 1;
                  break ;

               case 'e': /* execute from tokenised file */
                  executing_from_tokens = 1;
                  break ;

               case 'l': /* execute from tokenised file */
                  set_locale_info( arg + 1 );
                  break ;

               case 'h': /* usage */
               case '?': /* usage */
                  usage( argv[0] );
                  return 0;
            }
         }
         else
         {
            stdinput = 0 ;
            get_external_routine( TSD, "REGINA_MACROS", argv[i], &fptr, name, 1 );
            if (!fptr)
            {
               get_external_routine( TSD, "PATH", argv[i], &fptr, name, 1 );
               if ( !fptr )
               {
                  TSD->systeminfo->input_file = Str_crestrTSD(argv[i]) ;
                  exiterror( ERR_PROG_UNREADABLE, 1, "Program was not found" )  ;
               }
            }
            TSD->systeminfo->input_file = Str_crestrTSD(name) ;
            TSD->systeminfo->input_fp = fptr ;
            break ;
         }
      }
    }
    /*
     * Under DJGPP setmode screws up Parse Pull and entering code interactively :-(
     */
#if defined(__EMX__) || (defined(_MSC_VER) && !defined(__WINS__)) || (defined(__WATCOMC__) && !defined(__QNX__))
    setmode( fileno( stdin ), O_BINARY );
    setmode( fileno( stdout ), O_BINARY );
    setmode( fileno( stderr ), O_BINARY );
#endif

   if ( stdinput )
   {
      TSD->systeminfo->input_file = Str_crestrTSD( "<stdin>" ) ;
      TSD->systeminfo->input_fp = NULL;
   }

   if ( TSD->isclient )
      return 0 ;

   /*
    * -c switch specified - tokenise the input file before mucking around
    * with parameters etc.
    */
   if ( compiling_to_tokens )
   {
      internal_parser_type ipt;
      streng *SrcStr;
      char *SourceString;
      void *instore_buf=NULL;
      unsigned long SourceStringLen, instore_length=0L;
      streng * volatile command=NULL ;
      FILE *outfp;

      /* must have only one more arg, output file name */
      if ( argc > 4 )
         exiterror( ERR_PROG_UNREADABLE, 1, "Too many arguments when tokenising. Usage: -c inputfile outputfile" ) ;
      if ( argc < 4 )
         exiterror( ERR_PROG_UNREADABLE, 1, "Too few arguments when tokenising. Usage: -c inputfile outputfile" ) ;
      /*
       * Read the file into SourceString
       */
      fseek( TSD->systeminfo->input_fp, 0, SEEK_END );
      SourceStringLen = ftell( TSD->systeminfo->input_fp );
      rewind( TSD->systeminfo->input_fp );
      SourceString = malloc( SourceStringLen );
      if ( SourceString == NULL )
         exiterror( ERR_STORAGE_EXHAUSTED, 0 );
      /* set program file name */
      command = Str_dup_TSD( TSD, TSD->systeminfo->input_file ) ;

      if ( fread( SourceString, SourceStringLen, 1, TSD->systeminfo->input_fp ) != 1 )
         exiterror( ERR_PROG_UNREADABLE, 1, "Unable to read input file" ) ;
      /* why copy ??? */
      SrcStr = Str_makeTSD( SourceStringLen ) ;
      memcpy( SrcStr->value, SourceString, SourceStringLen ) ;
      SrcStr->len = SourceStringLen ;
      /*
       * enter_macro() actually does the tokenising...
       */
      ipt = enter_macro( TSD, SrcStr, command, &instore_buf, &instore_length ) ;
      free( SourceString );
      fclose( TSD->systeminfo->input_fp );
      outfp = fopen( argv[3], "wb" );
      if ( outfp == NULL )
         exiterror( ERR_PROG_UNREADABLE, 1, "Unable to open output file for writing" ) ;
      if ( instore_buf == NULL )
         exiterror( ERR_PROG_UNREADABLE, 1, "Error tokenising input file" ) ;
      if ( fwrite( instore_buf, instore_length, 1, outfp ) != 1 )
         exiterror( ERR_PROG_UNREADABLE, 1, "Unable to write contents of output file" ) ;
      fclose( outfp );
      return 0;
   }

   oldi = ++i ;

   if ( TSD->systeminfo->invoked == INVO_SUBROUTINE )
   {
      prev = NULL;
      for ( i = oldi; i < argc; i++ )
      {
         args = MallocTSD( sizeof(parambox) ) ;
         if ( i == oldi )
            TSD->currlevel->args = args;
         else
            prev->next = args;
         memset( args, 0, sizeof(parambox) ); /* especially ->value */
         args->value = Str_cre_TSD( TSD, argv[i] ) ;
         prev = args;
      }
   }
   else
   {
      for ( j = 1; i < argc; i++ )
         j += strlen( argv[i] ) + 1 ;

      TSD->currlevel->args = args = MallocTSD( sizeof(parambox) ) ;
      memset( args, 0, sizeof(parambox) ); /* especially ->value */
   /*
      args->value = Str_dupTSD(TSD->systeminfo->input_file) ;
      args = args->next = MallocTSD(sizeof(parambox)) ;
    */
      args->next = NULL ;
      if ( oldi >= argc )
         args->value = string = NULL ;
      else
      {
         args->value = string = Str_makeTSD( j ) ;
         string->len = 0 ;
      }

      for ( i = oldi ; i < argc; i++ )
      {
         string = Str_catstrTSD( string, argv[i] ) ;
         string->value[string->len++] = ' ' ;
      }
      if ( string && string->len )
        string->len-- ;
   }

   signal_setup( TSD ) ;

#ifndef NDEBUG
   __reginadebug = do_yydebug ;   /* 1 == yacc-debugging */
#endif

   /*
    * -e switch specified - execute from tokenised code
    */
   if ( executing_from_tokens )
   {
      void *TinnedTree;
      unsigned long TinnedTreeLen;
      streng * volatile command=NULL ;
      volatile streng * volatile result=NULL ;
      volatile streng * volatile environment=NULL ;
      volatile int ctype = TSD->systeminfo->invoked;
      volatile int hooks=0;
      int RetCode=0 ;

      /* cannot run tokenised code from stdin */
      if ( stdinput )
         exiterror( ERR_PROG_UNREADABLE, 1, "Cannot run tokenised code from stdin." ) ;
      /*
       * Read the file into SourceString
       */
      fseek( TSD->systeminfo->input_fp, 0, SEEK_END );
      TinnedTreeLen = ftell( TSD->systeminfo->input_fp );
      rewind( TSD->systeminfo->input_fp );
      TinnedTree = MallocTSD( TinnedTreeLen );
      if ( TinnedTree == NULL )
         exiterror( ERR_STORAGE_EXHAUSTED, 0 );
      if ( fread( TinnedTree, TinnedTreeLen, 1, TSD->systeminfo->input_fp ) != 1 )
         exiterror( ERR_PROG_UNREADABLE, 1, "Unable to read input file" ) ;
      fclose( TSD->systeminfo->input_fp );
      /* check if the file being read is a valid tokenised file */
      if ( !IsValidTin( TinnedTree, TinnedTreeLen ) )
         exiterror( ERR_PROG_UNREADABLE, 1, "The supplied file is not a valid Regina tokenised file" ) ;
      /* set program file name, environment and arguments */
      command = Str_dupTSD( TSD->systeminfo->input_file ) ;
      environment = Str_creTSD( "DEFAULT" ) ;
      if ( !envir_exists( TSD, (streng *)environment ) )
         add_envir( TSD, Str_dupTSD( (streng *)environment), ENVIR_PIPE, 0 ) ;
      args = TSD->currlevel->args;
      /*
       * do_instore() actually does the execution...
       */
      result = do_instore( TSD, command, args, (streng *)environment,
                           &RetCode,
                           hooks,
                           TinnedTree, TinnedTreeLen,
                           NULL, 0, /* source file contents */
                           NULL,
                           ctype);
      FreeTSD( TinnedTree );
      Free_stringTSD( command );
      Free_stringTSD( (streng *) environment );
      return RetCode;
   }

   /*
    * From here we are interpreting...
    */
   fetch_file( TSD, fptr ? fptr : stdin, &parsing );

   if (parsing.result != 0)
      exiterror( ERR_YACC_SYNTAX, 1, parsing.tline ) ;
   else
      TSD->systeminfo->tree = parsing;

   if (trace_override)
      TSD->systeminfo->trace_override = 1;
   else
      TSD->systeminfo->trace_override = 0;

#ifndef R2PERL
#ifndef MINIMAL
#ifndef VMS
#ifndef DOS
#ifndef _MSC_VER
#ifndef __IBMC__
#ifndef MAC
   if ( stdinput )
   {
      struct stat buffer ;

      /*
       * The following line is likely to give a warning when compiled
       * under Ultrix, this can be safely ignored, since it is just a
       * result of Digital not defining their include files properly.
       */
      rcode = fstat( fileno(stdin), &buffer ) ;
      if (rcode==0 && S_ISCHR(buffer.st_mode))
      {
         printf("  \b\b") ;
         fflush(stdout) ;
         rewind(stdin) ;
      }
   }
#endif /* !MAC */
#endif /* !__IBMC__ */
#endif /* !_MSC_VER */
#endif /* !DOS */
#endif /* !VMS */
#endif /* !MINIMAL */
#endif /* !R2PERL */

   treadit( TSD->systeminfo->tree.root ) ;

   flush_trace_chars(TSD) ;
   {
      nodeptr savecurrentnode = TSD->currentnode; /* pgb */
      string = interpret( TSD, TSD->systeminfo->tree.root ) ;
      TSD->currentnode = savecurrentnode; /* pgb */
   }
   rcode = EXIT_SUCCESS ;
   if ( string )
   {
      int error;
      /* fixes bug 657345 */

      rcode = streng_to_int( TSD, string, &error );
      if ( error )
         rcode = EXIT_SUCCESS;
   }

   purge_stacks( TSD );
#if defined(FLISTS) && defined(NEW_FLISTS)
   free_flists();
#endif

#ifdef DYNAMIC
   /*
    * Remove all external function package functions
    * and libraries. Only valid for the DYNAMIC library.
    */
   purge_library( TSD );
#endif

#ifdef TRACEMEM
   if (TSD->listleakedmemory)
      listleaked( TSD, MEMTRC_LEAKED )  ;
#endif

   killsystem( TSD, TSD->systeminfo );
   TSD->systeminfo = NULL ;

   /*
    * Remove all memory allocated by the flists internal memory manager.
    */
#ifdef FLISTS
   purge_flists( TSD );
#endif

   return(rcode) ;

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
      markmemory(sinfo->called_as, TRC_SYSINFO) ;
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
}
#endif


sysinfobox *creat_sysinfo( const tsd_t *TSD, streng *envir )
{
   sysinfobox *sinfo=NULL ;

   sinfo = MallocTSD( sizeof(sysinfobox) ) ;
   sinfo->environment = envir ;
   sinfo->tracing = DEFAULT_TRACING ;
   sinfo->interactive = DEFAULT_INT_TRACING ;
   sinfo->previous = NULL ;
   sinfo->invoked = INVO_COMMAND ;
   sinfo->called_as = NULL ;
   sinfo->input_file = NULL ;
   sinfo->input_fp = NULL ;
   sinfo->panic = NULL ;
   sinfo->hooks = 0 ;
   sinfo->callstack = MallocTSD(sizeof(nodeptr)*10) ;
   sinfo->result = NULL ;
   sinfo->cstackcnt = 0 ;
   sinfo->cstackmax = 10 ;
   sinfo->trace_override = 0 ;
   memset(&sinfo->tree, 0, sizeof(sinfo->tree));

   return sinfo ;
}

#if !defined(RXLIB)

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
   dummy = dummy; /* keep compiler happy */
   TSD = TSD; /* keep compiler happy */
   return 1 ;  /* to keep compiler happy */
}
int hookup_input( tsd_t *TSD, int dummy1, streng **dummy2 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   TSD = TSD; /* keep compiler happy */
   dummy1 = dummy1; /* keep compiler happy */
   dummy2 = dummy2; /* keep compiler happy */
   assert( 0 ) ;
   return 1 ;  /* to keep compiler happy */
}
int hookup_input_output( tsd_t *TSD, int dummy1, const streng *dummy2, streng **dummy3 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   TSD = TSD; /* keep compiler happy */
   dummy1 = dummy1; /* keep compiler happy */
   dummy2 = dummy2; /* keep compiler happy */
   dummy3 = dummy3; /* keep compiler happy */
   assert( 0 ) ;
   return 1 ;  /* to keep compiler happy */
}
int hookup_output( tsd_t *TSD, int dummy1, const streng *dummy2 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   assert( 0 ) ;
   dummy1 = dummy1; /* keep compiler happy */
   dummy2 = dummy2; /* keep compiler happy */
   TSD = TSD; /* keep compiler happy */
   return 1 ;  /* to keep compiler happy */
}
int hookup_output2( tsd_t *TSD, int dummy1, const streng *dummy2, const streng *dummy3 )
{
   /* This should never happen, if we don't have support for SAA API,
    * Then we should never get a system exit!
    */
   assert( 0 ) ;
   dummy1 = dummy1; /* keep compiler happy */
   dummy2 = dummy2; /* keep compiler happy */
   dummy3 = dummy3; /* keep compiler happy */
   TSD = TSD; /* keep compiler happy */
   return 1 ;  /* to keep compiler happy */
}

static void Exit(const tsd_t *TSD)
{
   if (TSD->in_protected)
   {
      jmp_buf h;

      memcpy(h,TSD->protect_return,sizeof(jmp_buf));
      /* cheat about the const, we go away anyway :-) */
      *((int*) &TSD->delayed_error_type) = PROTECTED_DelayedExit;
      *((int*) &TSD->expected_exit_error) = 1;
      longjmp( h, 1 ) ;
   }
   TSD->MTExit( 1 ) ;
}

streng *do_an_external_exe( tsd_t *TSD, const streng *dummy1, cparamboxptr dummy2, char dummy3, char dummy4 )
{
   NoAPI();
   dummy1 = dummy1; /* keep compiler happy */
   dummy2 = dummy2; /* keep compiler happy */
   dummy3 = dummy3; /* keep compiler happy */
   dummy4 = dummy4; /* keep compiler happy */
   Exit( TSD ) ;
   return NULL;
}

streng *do_an_external_dll( tsd_t *TSD, const void *dummy1, cparamboxptr dummy2, char dummy3 )
{
   NoAPI();
   dummy1 = dummy1; /* keep compiler happy */
   dummy2 = dummy2; /* keep compiler happy */
   dummy3 = dummy3; /* keep compiler happy */
   Exit( TSD ) ;
   return NULL;
}


streng *SubCom( tsd_t *TSD, const streng *dummy1, const streng *dummy2, int *dummy3 )
{
   NoAPI();
   dummy1 = dummy1; /* keep compiler happy */
   dummy2 = dummy2; /* keep compiler happy */
   dummy3 = dummy3; /* keep compiler happy */
   Exit( TSD ) ;
   return NULL;
}

int IfcHaveFunctionExit(const tsd_t *TSD)
{
   TSD = TSD; /* keep compiler happy */
   return(0);
}

#endif

static void usage( char *argv0 )
{
   fprintf( stdout, "\nRegina %s. All rights reserved.\n", PARSE_VERSION_STRING );
   fprintf( stdout,"Regina is distributed under the terms of the GNU Library Public License \n" );
   fprintf( stdout,"and comes with NO WARRANTY. See the file COPYING-LIB for details.\n" );
   fprintf( stdout,"\nTo run a Rexx program:\n" );
   fprintf( stdout,"%s [-h?vrt[ir]ap] program [arguments...]\n", argv0 );
   fprintf( stdout,"where:\n\n" );
   fprintf( stdout,"-h,-?                  show this message\n" );
   fprintf( stdout,"-v                     display Regina version and exit\n" );
   fprintf( stdout,"-r                     run Regina in \"safe\" mode\n" );
   fprintf( stdout,"-t[trace_char]         set TRACE any valid TRACE character - default A\n" );
   fprintf( stdout,"-a                     pass command line to Rexx program as separate arguments\n");
   fprintf( stdout,"-p                     pause after execution (Win32 only)\n");
   fprintf( stdout,"-l[locale]             use the system's default charset or a supplied one\n");
   fprintf( stdout,"\nTo tokenise a Rexx program:\n" );
   fprintf( stdout,"%s -c program(input) tokenisedfile(output)\n", argv0 );
   fprintf( stdout,"\nTo execute a tokenised file:\n" );
   fprintf( stdout,"%s -e tokenisedfile [arguments...]\n", argv0 );
   fflush( stdout );
}
