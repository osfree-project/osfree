/***********************************************************************/
/* THE.C - The Hessling Editor                                         */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2002 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

static char RCSid[] = "$Id: the.c,v 1.42 2002/08/13 09:38:14 mark Exp $";

#define MAIN 1
#include <the.h>
#include <proto.h>
#include <time.h>

#include "mygetopt.h"

#if defined(DOS) || defined(OS2)
#  if !defined(EMX) && !defined(GO32)
#    include <direct.h>
#  endif
#endif

#ifdef HAVE_PROTO
# ifdef UNIX
static RETSIGTYPE handle_signal(int);
# endif
static void display_info(CHARTYPE *);
static void init_signals(void);
#else
# ifdef UNIX
static RETSIGTYPE handle_signal();
# endif
static void display_info();
static void init_signals();
#endif
/*--------------------------- global data -----------------------------*/
   WINDOW *statarea=NULL,*error_window=NULL,*divider=NULL,*filetabs=NULL;
   VIEW_DETAILS *vd_current=(VIEW_DETAILS *)NULL;
   VIEW_DETAILS *vd_first=(VIEW_DETAILS *)NULL;
   VIEW_DETAILS *vd_last=(VIEW_DETAILS *)NULL;
   VIEW_DETAILS *vd_mark=(VIEW_DETAILS *)NULL;
   VIEW_DETAILS *filetabs_start_view=(VIEW_DETAILS *)NULL;
   CHARTYPE number_of_views = 0;                      /* number of views */
   LINETYPE number_of_files = 0;                      /* number of files */
   CHARTYPE display_screens = 1;                    /* number of screens */
   CHARTYPE current_screen = 0;
   SCREEN_DETAILS screen[MAX_SCREENS];            /* 2 screen structures */
   short screen_rows[MAX_SCREENS];
   bool rexx_support;                          /* initially Rexx support */
   bool batch_only=FALSE;                   /* are we running in batch ? */
   bool horizontal=TRUE;
   bool first_screen_display=FALSE;
   bool single_instance_server=FALSE;
   short save_coord_x[VIEW_WINDOWS];
   short save_coord_y[VIEW_WINDOWS];

   _LINE *next_line=NULL,*curr_line=NULL;
   _LINE *first_file_name=NULL,*current_file_name=NULL;
   _LINE *editv=NULL;
   _LINE *first_option=NULL,*last_option=NULL;
   bool error_on_screen=FALSE;
   bool colour_support=TRUE;         /* indicates if colour is supported */
   bool initial=TRUE;
   bool been_interactive=FALSE;
   CHARTYPE *rec=NULL;
   LENGTHTYPE rec_len = 0;                              /* length of rec */
   CHARTYPE *trec=NULL;
   LENGTHTYPE trec_len = 0;
   LENGTHTYPE max_trec_len = 0;
   CHARTYPE *brec=NULL;
   LENGTHTYPE brec_len = 0;
   CHARTYPE *cmd_rec=NULL;
   unsigned short cmd_rec_len = 0;                  /* length of cmd_rec */
   CHARTYPE *pre_rec=NULL;
   unsigned short pre_rec_len = 0;                  /* length of cmd_rec */
   CHARTYPE *profile_command_line=NULL;
   CHARTYPE *target_buffer=NULL;            /* used in get_item_values() */
   unsigned short target_buffer_len=0;        /* length of target buffer */
   bool focus_changed = FALSE;    /* indicates if focus line has changed */
   bool current_changed = FALSE;/* indicates if current line has changed */
   bool in_profile=FALSE;             /* indicates if processing profile */
   bool in_nomsg=FALSE;       /* indicates if running from NOMSG command */
   bool in_reprofile=FALSE;         /* indicates if reprocessing profile */
   int profile_file_executions=0; /* number of times profile file has executed */
   bool execute_profile=TRUE;/* indicates if we are to process a profile */
   bool in_macro=FALSE;            /* indicates if processing Rexx macro */
   bool in_repeat=FALSE;          /* indicates if running REPEAT command */
   bool in_readv=FALSE;         /* indicates if processing READV CMDLINE */
   bool file_read=FALSE;           /* indicates if we have read the file */
   bool curses_started=FALSE;         /* indicates if curses has started */
   bool readonly=FALSE;     /* indicates if running THE in readonly mode */
   bool be_quiet=FALSE;   /* do not display error message header if TRUE */

   CHARTYPE *the_version = (CHARTYPE *)"3.1";
   CHARTYPE *the_release = (CHARTYPE *)"13-August-2002";  /* use FULL month */
   CHARTYPE *the_copyright = (CHARTYPE *)"Copyright 1991-2002 Mark Hessling";
   CHARTYPE term_name[20];  /* $TERM value */
   CHARTYPE *tempfilename = (CHARTYPE *)NULL;
   CHARTYPE *stdinprofile = (CHARTYPE *)NULL;
#if defined(UNIX)
   CHARTYPE user_home_dir[MAX_FILE_NAME+1];
# define THE_PROFILE_FILE ".therc"
#else
# define THE_PROFILE_FILE "PROFILE.THE"
#endif

#ifdef THE_SINGLE_INSTANCE_ENABLED
# define THE_FIFO_FILE ".thefifo"
   CHARTYPE fifo_name[MAX_FILE_NAME+1];
   int fifo_fd;
#endif

#if !defined(MULTIPLE_PSEUDO_FILES)
   CHARTYPE *rexxoutname = (CHARTYPE *)"REXX.$$$";
   CHARTYPE *keyfilename = (CHARTYPE *)"KEY$$$.$$$";
   CHARTYPE _THE_FAR rexx_pathname[MAX_FILE_NAME+1];
   CHARTYPE _THE_FAR rexx_filename[10];
# ifdef VMS
   CHARTYPE *dirfilename = (CHARTYPE *)"DIR.THE";
# else
   CHARTYPE *dirfilename = (CHARTYPE *)"DIR.DIR";
# endif
#endif

#ifndef XCURSES
# define XTERM_PROGRAM "N/A"
#endif

   CHARTYPE _THE_FAR xterm_program[MAX_FILE_NAME+1]; /* default shell for XCURSES */

   CHARTYPE macro_suffix[12] = ".the";   /* default extension for macros */
#if !defined(MULTIPLE_PSEUDO_FILES)
   CHARTYPE _THE_FAR dir_pathname[MAX_FILE_NAME+1];
   CHARTYPE dir_filename[10];
   CHARTYPE _THE_FAR key_pathname[MAX_FILE_NAME+1];
   CHARTYPE key_filename[15];
#endif
   CHARTYPE _THE_FAR curr_path[MAX_FILE_NAME+1];
   CHARTYPE _THE_FAR sp_path[MAX_FILE_NAME+1];
   CHARTYPE _THE_FAR sp_fname[MAX_FILE_NAME+1];
   CHARTYPE _THE_FAR dir_path[MAX_FILE_NAME+1];        /* for dir and ls commands */
   CHARTYPE _THE_FAR dir_files[MAX_FILE_NAME+1];       /* for dir and ls commands */
   CHARTYPE _THE_FAR rexx_macro_name[MAX_FILE_NAME+1]; /* current rexx macro name */
   CHARTYPE _THE_FAR rexx_macro_parameters[MAX_FILE_NAME+1]; /* current rexx macro parameters */

   CHARTYPE _THE_FAR the_home_dir[MAX_FILE_NAME+1];
   CHARTYPE _THE_FAR the_help_file[MAX_FILE_NAME+1];

   CHARTYPE _THE_FAR the_macro_path[MAX_FILE_NAME+1];
   CHARTYPE _THE_FAR the_macro_path_buf[MAX_FILE_NAME+1];
   CHARTYPE **the_macro_dir;
   int max_macro_dirs = 0;
   int total_macro_dirs = 0;

   CHARTYPE _THE_FAR spooler_name[MAX_FILE_NAME+1];

   CHARTYPE *prf_arg=(CHARTYPE *)NULL;
   CHARTYPE *local_prf=(CHARTYPE *)NULL;
   CHARTYPE *specified_prf=(CHARTYPE *)NULL;

   CHARTYPE tabkey_insert='C';
   CHARTYPE tabkey_overwrite='T';
   unsigned short file_start = 38;

   struct stat stat_buf;

   LENGTHTYPE display_length=0;

   short lastrc=0;
   short compatible_look=COMPAT_THE;
   short compatible_feel=COMPAT_THE;
   short compatible_keys=COMPAT_THE;
   short prefix_width=DEFAULT_PREFIX_WIDTH;
   short prefix_gap=DEFAULT_PREFIX_GAP;

   chtype _THE_FAR etmode_table[256];
   bool   _THE_FAR etmode_flag[256];

#ifdef WIN32
   bool StartedPrinter=FALSE;
#endif

#if defined(SIGWINCH) && defined(USE_NCURSES)
   bool ncurses_screen_resized = FALSE;
#endif

   int lastkeys[8];
   int current_key=-1;

#define DEFAULT_LINES 24
#define DEFAULT_COLS  80

   short terminal_lines=DEFAULT_LINES;
   short terminal_cols=DEFAULT_COLS;
   /*
    * Following are for my_getopt function(s).
    */
   extern char *optarg;
   extern int optind;
   /*
    * Following are for original cursor position for EXTRACT /CURSOR/
    */
   LINETYPE original_screen_line = (-1L);
   LINETYPE original_screen_column = (-1L);
   LINETYPE original_file_line = (-1L);
   LINETYPE original_file_column = (-1L);
   /*
    * Following are for startup LINE and COLUMN
    */
   LINETYPE startup_line = 0L;
   LENGTHTYPE startup_column = 0;
   /*
    * Following are XCurses data
    */
#ifdef XCURSES
   char *XCursesProgramName = "the";
# if PDC_BUILD >= 2401
   char *X11_switches=NULL;
   int X11_argc=0;
   char **X11_argv=NULL;
# endif
#endif

   CHARTYPE *linebuf; /* Buffer for one terminal line, at least 81 elems */
   chtype *linebufch; /* Buffer for one terminal line in chtype-mode, >= 81 */
   int max_slk_labels=0;

   LASTOP _THE_FAR lastop[LASTOP_MAX] =
   {
      { "","alter",2   },
      { "","change",1  },
      { "","clocate",2 },
      { "","count",3   },
      { "","find",1    },
      { "","locate",1  },
      { "","schange",3 },
      { "","tfind",2   },
   };
/***********************************************************************/
#ifdef MSWIN
int Themain(argc,argv)
int argc;
char *argv[];
#else
#ifdef HAVE_PROTO
int main(int argc, char *argv[])
#else
int main(argc,argv)
short argc;
char *argv[];
#endif
#endif
/***********************************************************************/
{
#ifdef MSWIN
   extern void efree();
   extern char far *emalloc(unsigned long);
   extern char far *erealloc(void far *,unsigned long);
   extern char far *ecalloc();
#endif
   register short i=0;
   short c=0;
   int length;
   bool trap_signals=TRUE;
   bool pause_for_errors=FALSE;
#ifdef THE_SINGLE_INSTANCE_ENABLED
   bool run_single_instance=FALSE;
#endif
   short rc=RC_OK;
   char *envptr=NULL;
   int slk_format=0;
#if defined(XCURSES) && PDC_BUILD >= 2401
   int initscr_argc=0;
   char **initscr_argv=NULL;
#endif
   char mygetopt_opts[100];

#ifdef __EMX__
   _wildcard(&argc,&argv);
#endif
   TRACE_INITIALISE();
   TRACE_FUNCTION("the.c:     main");
  /*
   * Ensure that CURRENT_VIEW is NULL before starting. This is to ensure
   * that any errors generated before CURRENT_VIEW is assigned are
   * handled gracefully.
   */
   CURRENT_VIEW = (VIEW_DETAILS *)NULL;
  /*
   * Set up our memory management calls. This is where you can specify a
   * debugging memory manager.
   */
#ifdef MSWIN
   the_malloc  = emalloc;
   the_calloc  = ecalloc;
   the_free    = efree;
   the_realloc = erealloc;
   Win31Startup();
#else
   if (getenv("NO_FLISTS"))
   {
      the_malloc  = malloc;
      the_calloc  = calloc;
      the_free    = free;
      the_realloc = realloc;
   }
   else
   {
      the_malloc  = get_a_block;
      the_calloc  = NULL;
      the_free    = give_a_block;
      the_realloc = resize_a_block;
      init_memory_table();
   }
#endif
   /*
    * Set up flag to indicate that we are not interactive...yet.
    */
   in_profile = TRUE;
   execute_profile = TRUE;
   in_macro = FALSE;
   /*
    * Initialise LASTKEY array...
    */
   for (i=0;i<8;i++)
      lastkeys[i] = -1;
   /*
    * Initialise the printer spooler.
    */
#if defined(UNIX)
   strcpy((DEFCHAR *)spooler_name,(DEFCHAR *)"lpr");
#elif defined(WIN32)
   strcpy((DEFCHAR *)spooler_name,(DEFCHAR *)"default");
#else
   strcpy((DEFCHAR *)spooler_name,(DEFCHAR *)"LPT1");
#endif
   /*
    * Get all environment variables here. Some may be overridden by
    * command-line switches. (future possibility)
    */
#if defined(UNIX)
   if ((envptr = getenv("HOME")) != NULL)
      strcpy((DEFCHAR *)user_home_dir,envptr);
   else
      strcpy((DEFCHAR *)user_home_dir,"./");
   if (*(user_home_dir+strlen((DEFCHAR *)user_home_dir)-1) != ISLASH)
      strcat((DEFCHAR *)user_home_dir,(DEFCHAR *)ISTR_SLASH);
   if ((envptr = getenv("TERM")) != NULL)
      strcpy((DEFCHAR *)term_name,envptr);
   else
      strcpy((DEFCHAR *)term_name,"default");
#endif
#if defined(__EMX__)
   if (_osmode == DOS_MODE)
      strcpy((DEFCHAR *)term_name,"DOS");
   else
      strcpy((DEFCHAR *)term_name,"OS2");
#elif defined(DOS)
   strcpy((DEFCHAR *)term_name,"DOS");
#elif defined(OS2)
   strcpy((DEFCHAR *)term_name,"OS2");
#elif defined(XCURSES)
   strcpy((DEFCHAR *)term_name,"X11");
#elif defined(WIN32)
   strcpy((DEFCHAR *)term_name,"WIN32");
#elif defined(AMIGA)
   if ((envptr = getenv("TERM")) != NULL)
      strcpy((DEFCHAR *)term_name,envptr);
   else
      strcpy((DEFCHAR *)term_name,"default");
#endif
   strcpy((DEFCHAR *)xterm_program,XTERM_PROGRAM);
   /*
    * Get THE_HOME_DIR first (as all other paths rely on this value)
    */
   if ((envptr = getenv("THE_HOME_DIR")) != NULL)
   {
      strcpy((DEFCHAR *)the_home_dir,envptr);
      (void *)strrmdup(strtrans(the_home_dir,OSLASH,ISLASH),ISLASH,TRUE);
      if ((the_home_dir[strlen((DEFCHAR *)the_home_dir)-1]) != ISLASH)
         strcat((DEFCHAR *)the_home_dir,(DEFCHAR *)ISTR_SLASH);
   }
   else
   {
#if defined(UNIX)
      strcpy((DEFCHAR *)the_home_dir,(DEFCHAR *)THE_HOME_DIRECTORY);
#else
      strcpy((DEFCHAR *)the_home_dir,(DEFCHAR *)argv[0]);
      (void *)strrmdup(strtrans(the_home_dir,OSLASH,ISLASH),ISLASH,TRUE);
      i = strzreveq(the_home_dir,ISLASH);
      if (i != (-1))
         the_home_dir[i+1] = '\0';
      else
         the_home_dir[0] = '\0';
#endif
   }
   /*
    * Get THE_MACRO_PATH environment variable. If not set set up default
    * to be THE_HOME_DIR followed by the current directory.
    */
   if ((envptr = getenv("THE_MACRO_PATH")) != NULL)
      Macropath((CHARTYPE*)envptr);
   else
   {
      strcpy((DEFCHAR *)the_macro_path,(DEFCHAR *)the_home_dir);
      if (strcmp((DEFCHAR *)the_macro_path,"") == 0)
         strcpy((DEFCHAR *)the_macro_path,".");
#if defined(UNIX)
      strcat((DEFCHAR *)the_macro_path,":.");
#else
      strcat((DEFCHAR *)the_macro_path,";.");
#endif
      Macropath(the_macro_path);
   }
   /*
    * Process the command line arguments.
    */
   strcpy( mygetopt_opts, "qk::sSbmnrl:c:p:w:a:u:h?" );
# if defined(THE_SINGLE_INSTANCE_ENABLED)
   strcat( mygetopt_opts, "1::" );
#endif
#if defined(XCURSES) && PDC_BUILD >= 2401
   strcat( mygetopt_opts, "X:" );
#endif

   while ((c = my_getopt( argc, argv, mygetopt_opts ) ) != EOF )
   {
      switch((char)c)
      {
         case 's':        /* don't trap signals */
            trap_signals = FALSE;
            break;
         case 'q':        /* quiet - don't display profile heading message */
            be_quiet = TRUE;
            break;
         case 'k':        /* allow Soft Label Keys */
            SLKx = TRUE;
            if ( optarg == NULL )
            {
               slk_format = 55;
               max_slk_labels = 10;
               break;
            }
            slk_format = atoi( optarg );
            if ( slk_format == 0 )
            {
               cleanup();
               display_error(4,(CHARTYPE *)optarg,FALSE);
               return(4);
            }
            if ( slk_format > MAX_SLK_FORMAT )
            {
               cleanup();
               sprintf((DEFCHAR *)xterm_program,"SLK format must be >= 1 and <= %d",MAX_SLK_FORMAT );
               display_error(6,(CHARTYPE *)xterm_program,FALSE);
               return(4);
            }
            switch( slk_format )
            {
               case 1:
               case 2:
                  max_slk_labels = 8;
                  slk_format--;
                  break;
               case 3:
               case 4:
                  max_slk_labels = 12;
                  slk_format--;
                  break;
               case 5:
               case 55:
                  slk_format = 55;
                  max_slk_labels = 10;
                  break;
            }
            break;
         case 'S':        /* allow scrollbar */
            SBx = TRUE;
            break;
         case 'l':        /* set current line on startup */
            startup_line = (LINETYPE)atol(optarg);
            if (startup_line < 0L)
            {
               cleanup();
               display_error(5,(CHARTYPE *)"startup line MUST be > 0",FALSE);
               return(4);
            }
            break;
         case 'c':        /* set current column on startup */
            startup_column = (LENGTHTYPE)atoi(optarg);
            if (startup_column == 0)
            {
               cleanup();
               display_error(5,(CHARTYPE *)"startup column MUST be > 0",FALSE);
               return(4);
            }
            break;
         case 'b':        /* batch processing */
            batch_only = TRUE;
            break;
         case 'm':        /* force into MONO */
            colour_support = FALSE;
            break;
         case 'n':        /* do not execute any profile file */
            execute_profile = FALSE;
            break;
         case 'r':        /* run in readonly mode */
            readonly = TRUE;
            break;
         case 'p':        /* profile file name */
            if ((specified_prf = (CHARTYPE *)(*the_malloc)((strlen(optarg)+1)*sizeof(CHARTYPE))) == NULL)
            {
               cleanup();
               display_error(30,(CHARTYPE *)"",FALSE);
               return(2);
            }
               strcpy((DEFCHAR *)specified_prf,(DEFCHAR *)optarg);
               break;
         case 'a':        /* profile arguments */
            if ((prf_arg = (CHARTYPE *)(*the_malloc)((strlen(optarg)+1)*sizeof(CHARTYPE))) == NULL)
            {
               cleanup();
               display_error(30,(CHARTYPE *)"",FALSE);
               return(3);
            }
            strcpy((DEFCHAR *)prf_arg,(DEFCHAR *)optarg);
            break;
         case 'w':        /* width of line */
            max_line_length = (unsigned short)atoi(optarg);
            if (max_line_length < 10)
            {
               cleanup();
               display_error(5,(CHARTYPE *)"- width MUST be >= 10",FALSE);
               return(4);
            }
            if (max_line_length > 32700)
            {
               cleanup();
               display_error(6,(CHARTYPE *)"- width MUST be <= 32700",FALSE);
               return(5);
            }
            break;
         case 'u':        /* display length */
            display_length = (unsigned short)atoi(optarg);
            if (display_length == 0)
            {
               cleanup();
               display_error(5,(CHARTYPE *)"- display length MUST be > 0",FALSE);
               return(4);
            }
            break;
#if defined(XCURSES) && PDC_BUILD >= 2401
         case 'X':        /* X11 switches to be passed to initscrX() */
            X11_switches = (*the_malloc)( strlen( optarg ) + 1 );
            if ( X11_switches == NULL )
            {
               cleanup();
               display_error(30,(CHARTYPE *)"",FALSE);
               return(5);
            }
            strcpy( X11_switches, optarg );
            if ( ( initscr_argv = StringToArgv( &initscr_argc, X11_switches ) ) == NULL )
            {
               (*the_free)( X11_switches );
               cleanup();
               display_error(30,(CHARTYPE *)"",FALSE);
               return(5);
            }
            break;
#endif
         case 'h':
         case '?':
            cleanup();
            display_info((CHARTYPE *)argv[0]);
            return(0);
            break;
#ifdef THE_SINGLE_INSTANCE_ENABLED
         case '1':        /* allow single instances */
            run_single_instance = TRUE;
            /*
             * If no optional parameter supplied, use the user's $HOME
             * for the path, otherwise use the supplied value
             */
            if ( optarg == NULL )
            {
# ifdef WIN32
               strcpy( (DEFCHAR *)fifo_name, (DEFCHAR *)THE_FIFO_FILE );
# else
               strcpy( (DEFCHAR *)fifo_name, (DEFCHAR *)user_home_dir );
               strcat( (DEFCHAR *)fifo_name, (DEFCHAR *)THE_FIFO_FILE );
# endif
               break;
            }
            strcpy( (DEFCHAR *)fifo_name, (DEFCHAR *)optarg );
            break;
#endif
         default:
            break;
         }
      }

   if (optind<argc)
   {
      while(optind<argc)
      {
         if ((current_file_name = add_LINE(first_file_name,
                                         current_file_name,
                                         strrmdup(strtrans((CHARTYPE *)argv[optind],OSLASH,ISLASH),ISLASH,TRUE),
                                         strlen(argv[optind]),0,TRUE)) == NULL)
         {
            cleanup();
            display_error(30,(CHARTYPE *)"",FALSE);
            return(6);
         }
         if (first_file_name == NULL)
            first_file_name = current_file_name;
         optind++;
      }
   }
   else
   {
#if defined(MSWIN)
      return(1);
#else
      if ((current_file_name = add_LINE(first_file_name,
                                      current_file_name,
                                      CURRENT_DIR,
                                      strlen((DEFCHAR *)CURRENT_DIR),0,TRUE)) == NULL)
      {
         cleanup();
         display_error(30,(CHARTYPE *)"",FALSE);
         return(7);
      }
      if (first_file_name == NULL)
         first_file_name = current_file_name;
#endif
   }
   /*
    * Allocate some memory to working variables...
    * Done here before processing single_instance mode because
    * we use trec in initialise_fifo();
    */
#ifdef MSWIN
   rec = trec = brec = cmd_rec = profile_command_line = NULL;
   init_temp_space();
#endif
   rc = allocate_working_memory();
   if (rc)
   {
      cleanup();
      display_error(30,(CHARTYPE *)"",FALSE);
      return(rc);
   }
#ifdef THE_SINGLE_INSTANCE_ENABLED
   /*
    * If running under XCurses, and running in single window mode, then
    * check if the FIFO $HOME/.thefifo exists.
    * If it does, then we are the client, so send the running instance
    * an edit command for each file
    * If not, then create the FIFO and continue on...
    */
   if ( run_single_instance )
   {
      if ( initialise_fifo( first_file_name, startup_line, startup_column, readonly ) )
      {
# ifdef MSWIN
         Win31Cleanup();
         return(0);
# endif
         cleanup();
         return(0);
      }
   }
#endif
   /*
    * Check any command line conflicts...
    */
   if (display_length > 0
   &&  display_length > max_line_length)
   {
      cleanup();
      display_error(6,(CHARTYPE *)"- width MUST be >= display length",FALSE);
      return(8);
   }
   /*
    * Override any default paths,filenames etc if supplied on command line
    */
   if (specified_prf
   &&  *specified_prf == '-'
   &&  batch_only)
   {
      FILE *fp=NULL;
      int num=0;
      if ( ( stdinprofile = (CHARTYPE *)strdup( thetmpnam ( "PRF" ) ) ) == NULL )
      {
         cleanup();
         return(31);
      }
      if ( ( fp = fopen( (DEFCHAR *)stdinprofile , "w" ) ) == NULL )
      {
         (*the_free)(stdinprofile);
         (*the_free)(specified_prf);
         cleanup();
         return(8);
      }
      for ( ; ; )
      {
         num = fread( rexx_pathname, sizeof( char ), MAX_FILE_NAME, stdin );
         fwrite( rexx_pathname, sizeof( char ), num, fp );
         if ( feof( stdin ) )
            break;
      }
      fclose ( fp );
      if ( setup_profile_files( stdinprofile ) != RC_OK )
      {
         cleanup();
         return(8);
      }
   }
   else
   {
      if ( setup_profile_files( specified_prf ) != RC_OK )
      {
         cleanup();
         return(8);
      }
   }
   if (specified_prf != NULL)
      (*the_free)(specified_prf);
   /*
    * Allocate memory to pre_rec and set it to blanks.
    */
   if ((pre_rec = (CHARTYPE *)(*the_malloc)((MAX_PREFIX_WIDTH+1)*sizeof(CHARTYPE))) == NULL)
   {
      cleanup();
      display_error(30,(CHARTYPE *)"",FALSE);
      return(16);
   }
   memset(pre_rec,' ',MAX_PREFIX_WIDTH+1);
   pre_rec_len = 0;
#if !defined(MULTIPLE_PSEUDO_FILES)
   /*
    * Set up filename for directory temporary file (DIR.DIR).
    */
#ifdef UNIX
   strcpy((DEFCHAR *)dir_pathname,(DEFCHAR *)user_home_dir);
#endif
#if defined(DOS) || defined(OS2) || defined(WIN32) || defined(AMIGA)
   strcpy((DEFCHAR *)dir_pathname,(DEFCHAR *)ISTR_SLASH);
#endif
#ifdef VMS
   strcpy((DEFCHAR *)dir_pathname,"");
#endif

   strcat((DEFCHAR *)dir_pathname,(DEFCHAR *)dirfilename);
   if (splitpath(dir_pathname) != RC_OK)
   {
      cleanup();
      return(18);
   }
   strcpy((DEFCHAR *)dir_pathname,(DEFCHAR *)sp_path);
   strcpy((DEFCHAR *)dir_filename,(DEFCHAR *)sp_fname);

#endif
   /*
    * Set up a temporary file name for output from PUT command to go...
    */
   if ( ( tempfilename = (CHARTYPE *)strdup( thetmpnam ( "TMP" ) ) ) == NULL )
   {
      cleanup();
      return(31);
   }
   /*
    * Trap signals to exit gracefully, unless user has specified they not
    * be trapped.
    */
   if (trap_signals)
      init_signals();
   /*
    * Initialise rexx support. If no Rexx available, set flag...
    */
#ifndef MSWIN
   rexx_support = TRUE;
   if (initialise_rexx() != RC_OK)
      rexx_support = FALSE;
#endif
   /*
    * Create the builtin parsers...
    */
   if (construct_default_parsers() != RC_OK)
   {
      cleanup();
      return(32);
   }
   if (construct_default_parser_mapping() != RC_OK)
   {
      cleanup();
      return(33);
   }
    /*
    * Set SCREEN values up...
    */
   for (i=0;i<VIEW_WINDOWS;i++)
       CURRENT_SCREEN.win[i] = (WINDOW *)NULL;
   /*
    * Set up global defaults.
    */
   set_global_defaults();
   /*
    * Initialise command array to empty strings.
    */
   init_command();
   /*
    * Set up default screens using the default values of terminal_lines
    * and terminal_cols. These will be altered after initscr().
    */
   screen[0].sl = screen[1].sl = NULL;
   if (batch_only)
   {
      set_screen_defaults();
      /*
       * Read each file into memory and apply the profile file to each of the
       * files.
       */
      current_file_name = first_file_name;
      while(current_file_name != NULL)
      {
        if ((rc = get_file((CHARTYPE *)current_file_name->line)) != RC_OK)
        {
           cleanup();
           if (rc == RC_DISK_FULL)
              display_error(57,(CHARTYPE *)"...probably",FALSE);
           return(21);
        }
        pre_process_line(CURRENT_VIEW,0L,(_LINE *)NULL);
        if (execute_profile)
        {
           if (local_prf != (CHARTYPE *)NULL)
              rc = get_profile(local_prf,prf_arg);
           if (error_on_screen)
           {
              pause_for_errors = TRUE;
              error_on_screen = FALSE;
           }
#ifdef MSWIN
           (void)get_user_profile();
           if (error_on_screen)
           {
              pause_for_errors = TRUE;
              error_on_screen = FALSE;
           }
#endif
        }
        current_file_name = current_file_name->next;
      }
      first_file_name = lll_free(first_file_name);
      /*
       * If THE has been used only in batch, exit here.
       */
      if (number_of_files != 0)
      {
         sprintf((DEFCHAR *)rec,"%ld",number_of_files);
         display_error(77,rec,FALSE);
      }
      cleanup();
      return(0);
   } /* if (batch_only) */
   /*
    * If the platform supports the mouse, set up the default commands.
    */
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
   initialise_mouse_commands();
#endif
   /*
    * Start up curses. This is done ONLY for interactive sessions!
    */
/*    traceon(); */
#if defined(HAVE_SLK_INIT)
# if MAX_SLK == 0
   if (SLKx) slk_init(1);
# else
   if (SLKx) slk_init(slk_format);
# endif
#endif
#if defined(HAVE_SB_INIT)
   if (SBx) sb_init();
#endif

#if defined(XCURSES) && PDC_BUILD >= 2401
   if ( X11_switches )
   {
      Xinitscr( initscr_argc,initscr_argv );
      (*the_free)( X11_switches );
   }
   else
   {
      if ( ( initscr_argv = StringToArgv( &initscr_argc, "" ) ) == NULL )
      {
         display_error( 30, (CHARTYPE *)"", FALSE );
         cleanup();
         return(1);
      }
      Xinitscr( initscr_argc,initscr_argv );
   }
   if ( initscr_argc )
      (*the_free)( initscr_argv );
#else
   initscr();
#endif
   curses_started = TRUE;
   /*
    * Save the value of LINES and COLS and use these for all screen
    * sizing calculations. This is because BSD scrolls if a character is
    * displayed in the bottom right corner of the screen :-(
    */
   terminal_lines=LINES;
   terminal_cols=COLS;
#ifdef HAVE_BSD_CURSES
   terminal_lines--;
#endif
   if ((length = COLS ) <= 260)
      length = 261;
   if ((linebuf = (CHARTYPE *)(*the_malloc)(length)) == NULL)
   {
      cleanup();
      return(30);
   }
   if ((linebufch = (chtype *)(*the_malloc)(length * sizeof(chtype))) == NULL)
   {
      cleanup();
      return(30);
   }
   /*
    * Determine if colour support available.
    */
   if (colour_support) /* if default setting not overridden on command line */
   {
      colour_support = FALSE;
#ifdef A_COLOR
      if (has_colors())
      {
         start_color();
         colour_support = TRUE;
         init_colour_pairs();
      }
#endif
   }
   /*
    * Set various terminal characteristics...
    */
   cbreak();
   raw();
#if defined(USE_EXTCURSES)
   extended(FALSE);
#endif
#if defined(PDCURSES)
   raw_output(TRUE);
#endif
#if defined(KEY_SHIFT_L) && defined(PDCURSES)
   PDC_return_key_modifiers(TRUE);
#endif
   nonl();
   noecho();
   keypad(stdscr,TRUE);
   notimeout(stdscr,TRUE);
   def_prog_mode();
   (void)THETypeahead((CHARTYPE *)"OFF");
   /*
    * Set up mouse support if enabled in curses library.
    */
#if defined(PDCURSES_MOUSE_ENABLED)
   mouse_set(ALL_MOUSE_EVENTS);
#endif
#if defined(NCURSES_MOUSE_VERSION)
   mousemask(ALL_MOUSE_EVENTS, (mmask_t*)NULL);
#endif
   /*
    * Set up variables and values dependent on LINES and COLS now with
    * values set by initscr().
    */
   set_screen_defaults();

#if defined(HAVE_BROKEN_SYSVR4_CURSES)
   force_curses_background();
   refresh();
#endif
   /*
    * wnoutrefresh() is called here so that the first call to getch() on
    * stdscr does not clear the screen.
    */
   wnoutrefresh(stdscr);
#if defined(HAVE_SLK_INIT)
   if (SLKx) slk_noutrefresh();
#endif
   /*
    * Create the statusline window...
    */
   if (create_statusline_window() != RC_OK)
   {
      cleanup();
      display_error(0,(CHARTYPE *)"creating status line window",FALSE);
      return(23);
   }
   /*
    * Set up ETMODE tables...
    */
#if defined(DOS) || defined(OS2)  || defined(WIN32)
   (void)Etmode((CHARTYPE *)"ON");
#elif defined(XCURSES)
   (void)Etmode((CHARTYPE *)"ON 32-255");
#elif defined(UNIX) || defined(AMIGA)
   (void)Etmode((CHARTYPE *)"OFF");
#endif
   /*
    * Read each file into memory and apply the profile file to each of the
    * files.
    */
   current_file_name = first_file_name;
   while(current_file_name != NULL)
   {
      rc = EditFile((CHARTYPE *)current_file_name->line,TRUE);
      if (rc != RC_OK)
      {
         cleanup();
         return(24);
      }
      current_file_name = current_file_name->next;
   }
   first_file_name = lll_free(first_file_name);
   /*
    * If THE has only been used to process a profile file, then exit.
    */
   if (number_of_files == 0)
   {
      cleanup();
      return(0);
   }
   /*
    * We are no longer executing the profile file.
    */
   in_profile = FALSE;
   been_interactive = TRUE;
   /*
    * Add the default settings to statusline...
    */
   Statopt((CHARTYPE*)"ON NBFILE.1 13 0 Files=");
   Statopt((CHARTYPE*)"ON WIDTH.1 23 0 Width=");

   /*
    * Birthday easter egg ;-)
    */
   if ( BIRTHDAYx == TRUE )
   {
      struct tm *ptr;
      time_t now;
      now = time(NULL);
      ptr = localtime(&now);

      if ( ptr->tm_mday == 16 && ptr->tm_mon == 7 && ptr->tm_year == 102 )
      {
         char buf[300];
         sprintf( buf, "/Happy Birthday to THE,%cHappy Birthday to THE,%cHappy Birthday to THE-EE%cHappy Birthday to THE.%cTHE is 10 years old today!!%cThis message will only occur today, but if you want to turn it off,%cadd 'SET BIRTHDAY OFF' in your profile file./OK",
         10,10,10,10,10,10);
         in_macro = TRUE;
         Dialog( buf );
         in_macro = FALSE;
      }
   }
   /*
    * This is where it all happens!!!
    */
   editor();
   /*
    * Finalise rexx support...
    */
#ifndef MSWIN
   finalise_rexx();
#endif
   /*
    * Free up the dynamically allocated memory.
    */
   if (first_define != NULL)
      first_define = dll_free(first_define);
   if (first_synonym != NULL)
      first_synonym = dll_free(first_synonym);
   if (first_prefix_synonym != NULL)
      first_prefix_synonym = lll_free(first_prefix_synonym);
   if (first_mouse_define != NULL)
      first_mouse_define = dll_free(first_mouse_define);
   (*the_free)(rec);
   (*the_free)(trec);
   (*the_free)(brec);
   (*the_free)(cmd_rec);
   (*the_free)(pre_rec);
   (*the_free)(linebuf);
   (*the_free)(linebufch);
   if (profile_command_line != NULL)
     (*the_free)(profile_command_line);
   if (screen[0].sl != NULL)
      (*the_free)(screen[0].sl);
   if (screen[1].sl != NULL)
      (*the_free)(screen[1].sl);
   if ( the_macro_dir )
      (*the_free)( the_macro_dir );
   /*
    * Free memory for temp_params and tmp_cmd.
    */
   free_temp_space(TEMP_PARAM);
   free_temp_space(TEMP_MACRO);
   free_temp_space(TEMP_TEMP_CMD);
   free_temp_space(TEMP_TMP_CMD);

   if (local_prf != NULL)
      (*the_free)(local_prf);
   if (prf_arg != NULL)
      (*the_free)(prf_arg);
   free_recovery_list();

   if (target_buffer != NULL)
      (*the_free)(target_buffer);

   if (divider != (WINDOW *)NULL)
   {
      delwin(divider);
      divider = (WINDOW *)NULL;
   }
   if (error_window != (WINDOW *)NULL)
   {
      delwin(error_window);
      error_window = (WINDOW *)NULL;
   }
   if (last_message != NULL)
      (*the_free)(last_message);
   /*
    * Destroy the builtin parsers...
    */
   destroy_all_parsers();
   first_parser_mapping = mappingll_free(first_parser_mapping);
   /*
    * If the user wants a clearscreen done before exiting, do it...
    */
   if (CLEARSCREENx)
   {
      wclear(stdscr);
      move(0,0);
      attrset(A_NORMAL);
      refresh();
   }
   else
   /*
    * ...otherwise, get the cursor to the bottom line.
    */
   {
      if (statarea != (WINDOW *)NULL)
      {
         mvwaddstr(statarea,0,4,"     ");
         wattrset(statarea,A_NORMAL);
         mvwaddstr(statarea,0,0,"THE - END");
         wrefresh(statarea);
      }
   }
   if (statarea != (WINDOW *)NULL)
   {
      delwin(statarea);
      statarea = (WINDOW *)NULL;
   }
   if (filetabs != (WINDOW *)NULL)
   {
      delwin(filetabs);
      filetabs = (WINDOW *)NULL;
   }
   last_option = first_option = lll_free(first_option);
#ifdef MSWIN
   Win31Cleanup();
   return(0);
#endif
   cleanup();
   return(0);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static void init_signals(void)
#else
static void init_signals()
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("the.c:     init_signals");
#ifdef UNIX
   signal(SIGQUIT,handle_signal);
   signal(SIGHUP,handle_signal);
   signal(SIGABRT,handle_signal);
   signal(SIGFPE,handle_signal);
   signal(SIGSEGV,handle_signal);
   signal(SIGINT,handle_signal);
   signal(SIGTERM,handle_signal);
# if defined(SIGBUS)
   signal(SIGBUS,handle_signal);
# endif
# if defined(SIGWINCH) && defined(USE_NCURSES)
   signal(SIGWINCH,handle_signal);
# endif
# if defined(SIGPIPE)
   signal(SIGPIPE,SIG_IGN);
# endif
#endif
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void init_colour_pairs(void)
#else
void init_colour_pairs()
#endif
/***********************************************************************/
{
   register int fg=0,bg=0;

   TRACE_FUNCTION("the.c:     init_colour_pairs");
#ifdef A_COLOR
   for (fg=0;fg<COLORS;fg++)
   {
      for (bg=0;bg<COLORS;bg++)
      {
         if (ATTR2PAIR(fg,bg) <= COLOR_PAIRS)
            init_pair(ATTR2PAIR(fg,bg),fg,bg);
      }
   }
#endif
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
int setup_profile_files(CHARTYPE *specified_prf)
#else
int setup_profile_files(specified_prf)
CHARTYPE *specified_prf;
#endif
/***********************************************************************/
{
   int rc=RC_OK;
   char *envptr=NULL;
   short errnum=0;
   /*
    * If a profile specified on the command line, set it up as the local
    * profile. It MUST exist and be readable, otherwise an error.
    */
   if (specified_prf != (CHARTYPE *)NULL)
   {
      if ((local_prf = (CHARTYPE *)(*the_malloc)((MAX_FILE_NAME+1)*sizeof(CHARTYPE))) == NULL)
         return(RC_OUT_OF_MEMORY);
      rc = get_valid_macro_file_name(specified_prf,local_prf,macro_suffix,&errnum);
      if ( rc != RC_OK)
      {
         (*the_free)(local_prf);
         local_prf = NULL;
         display_error(9,specified_prf,FALSE);
         return(rc);
      }
      if (!file_exists(local_prf))
      {
         (*the_free)(local_prf);
         local_prf = NULL;
         display_error(9,local_prf,FALSE);
         return(RC_FILE_NOT_FOUND);
      }
      /*
       * If the file is not readable, error.
       */
      if (!file_readable(local_prf))
      {
         (*the_free)(local_prf);
         local_prf = NULL;
         display_error(8,local_prf,FALSE);
         return(RC_ACCESS_DENIED);
      }
      return(rc);
   }
   /*
    * If a profile specified with THE_PROFILE_FILE, set it up as the local
    * profile. It does not have to exist.
    */
   if ((envptr = getenv("THE_PROFILE_FILE")) != NULL)
   {
      if ((local_prf = (CHARTYPE *)(*the_malloc)((MAX_FILE_NAME+1)*sizeof(CHARTYPE))) == NULL)
         return(RC_OUT_OF_MEMORY);
      strcpy((DEFCHAR *)local_prf,envptr);
      return(rc);
   }
   /*
    * No specific profile, so check for default profiles.
    */
   if ((local_prf = (CHARTYPE *)(*the_malloc)((MAX_FILE_NAME+1)*sizeof(CHARTYPE))) == NULL)
      return(RC_OUT_OF_MEMORY);
   /*
    * For Unix, use a local profile first...
    */
#if defined(UNIX)
   strcpy((DEFCHAR *)local_prf,(DEFCHAR *)user_home_dir);
   strcat((DEFCHAR *)local_prf,(DEFCHAR *)THE_PROFILE_FILE);
   (void *)strrmdup(strtrans(local_prf,OSLASH,ISLASH),ISLASH,TRUE);
   if (file_readable(local_prf))
      return(rc);
#endif
   /*
    * If no local profile, see if a global profile exists...
    */
   strcpy((DEFCHAR *)local_prf,(DEFCHAR *)the_home_dir);
   strcat((DEFCHAR *)local_prf,(DEFCHAR *)THE_PROFILE_FILE);
   (void *)strrmdup(strtrans(local_prf,OSLASH,ISLASH),ISLASH,TRUE);
   if (file_readable(local_prf))
      return(rc);
   /*
    * To get here, no profile files to be executed.
    */
   (*the_free)(local_prf);
   local_prf = (CHARTYPE *)NULL;
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static void display_info(CHARTYPE *argv0)
#else
static void display_info(argv0)
CHARTYPE *argv0;
#endif
/***********************************************************************/
{
   fprintf(stdout,"\nTHE %s %2s %s. All rights reserved.\n",the_version,the_release,the_copyright);
   fprintf(stdout,"THE is distributed under the terms of the GNU General Public License \n");
   fprintf(stdout,"and comes with NO WARRANTY. See the file COPYING for details.\n");
   fprintf(stdout,"\nUsage:\n\n%s [-h?nmrsbq] [-p profile] [-a profile_arg] [-l line_num] [-c col_num] [-w width] [-u display_length] [-k[fmt]] [[dir] [file [...]]]\n",argv0);
   fprintf(stdout,"\nwhere:\n\n");
   fprintf(stdout,"-h,-?                  show this message\n");
   fprintf(stdout,"-n                     do not execute a profile file\n");
   fprintf(stdout,"-m                     force display into mono\n");
   fprintf(stdout,"-r                     run THE in read-only mode\n");
   fprintf(stdout,"-s                     turn off signal trapping (Unix only)\n");
   fprintf(stdout,"-b                     run in batch mode\n");
   fprintf(stdout,"-q                     run quietly in batch\n");
   fprintf(stdout,"-k[fmt]                allow Soft Label Key display and set format\n");
   fprintf(stdout,"-l line_num            specify line number to make current\n");
   fprintf(stdout,"-c column_num          specify column number to make current\n");
   fprintf(stdout,"-p profile             filename of profile file\n");
   fprintf(stdout,"-a profile_arg         argument(s) to profile file (only with Rexx)\n");
   fprintf(stdout,"-w width               maximum width of line (default 512)\n");
   fprintf(stdout,"-u display_length      display length in non-line mode\n");
#ifdef XCURSES
   fprintf(stdout,"-X X11_switches        X11 switches (enclosed in \"\"\n");
   fprintf(stdout,"-1[fifoname]           run in single window mode, optionally supplying FIFO name\n");
#endif
   fprintf(stdout,"[dir [file [...]]]     file(s) and/or directory to be edited\n\n");
   fflush(stdout);
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
int allocate_working_memory(void)
#else
int allocate_working_memory()
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("the.c:     allocate_working_memory");
   /*
    * Allocate some memory to rec.
    */
   if (rec == NULL)
      rec = (CHARTYPE *)(*the_malloc)((max_line_length+5)*sizeof(CHARTYPE));
   else
      rec = (CHARTYPE *)(*the_realloc)(rec,(max_line_length+5)*sizeof(CHARTYPE));
   if (rec == NULL)
      return(10);
   /*
    * Allocate some memory to trec; buffer for file line.
    */
   max_trec_len = trec_len = max((LENGTHTYPE)(30*80),(LENGTHTYPE)((max_line_length+2)*2));
   if (trec == NULL)
      trec = (CHARTYPE *)(*the_malloc)(trec_len*sizeof(CHARTYPE));
   else
      trec = (CHARTYPE *)(*the_realloc)(trec,trec_len*sizeof(CHARTYPE));
   if (trec == NULL)
      return(11);
   /*
    * Allocate some memory to brec; buffer for reading files.
    */
   brec_len = max((LENGTHTYPE)(30*80),(LENGTHTYPE)((max_line_length+2)));
   if (brec == NULL)
      brec = (CHARTYPE *)(*the_malloc)(brec_len*sizeof(CHARTYPE));
   else
      brec = (CHARTYPE *)(*the_realloc)(brec,brec_len*sizeof(CHARTYPE));
   if (brec == NULL)
      return(11);
   /*
    * Allocate memory to cmd_rec and set it to blanks.
    */
   if (cmd_rec == NULL)
      cmd_rec = (CHARTYPE *)(*the_malloc)((max_line_length+2)*sizeof(CHARTYPE));
   else
      cmd_rec = (CHARTYPE *)(*the_realloc)(cmd_rec,(max_line_length+2)*sizeof(CHARTYPE));
   if (cmd_rec == NULL)
      return(12);
   memset(cmd_rec,' ',max_line_length);
   cmd_rec_len = 0;
   /*
    * Allocate some memory for temporary space...
    */
   if (allocate_temp_space(max_line_length,TEMP_PARAM) != RC_OK)
      return(13);
   if (allocate_temp_space(max_line_length,TEMP_TMP_CMD) != RC_OK)
      return(14);
   if (allocate_temp_space(max_line_length,TEMP_TEMP_CMD) != RC_OK)
      return(15);
   /*
    * Allocate some memory to profile_command_line. (only really need to
    * do this if NO Rexx support).
    */
   if (profile_command_line == NULL)
      profile_command_line = (CHARTYPE *)(*the_malloc)((max_line_length+2)*sizeof(CHARTYPE));
   else
      profile_command_line = (CHARTYPE *)(*the_realloc)(profile_command_line,(max_line_length+2)*sizeof(CHARTYPE));
   if (profile_command_line == NULL)
      return(17);
   TRACE_RETURN();
   return(0);
}

/***********************************************************************/
#ifdef HAVE_PROTO
void cleanup(void)
#else
void cleanup()
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("the.c:     cleanup");

   if (curses_started)
   {
      if (error_on_screen
      &&  error_window != NULL)
      {
         display_error(0,(CHARTYPE *)HIT_ANY_KEY,FALSE);
         wrefresh(error_window);
#ifdef KEY_RESIZE
         /*
          * Real hack here. If we have an error caused by editing the first file
          * like line too long, then we need to ignore all KEY_RESIZE events; XCurses
          * sends a resize on startup every time!
          */
         while ( getch() == KEY_RESIZE );
#else
         getch();
#endif
      }
      INSERTMODEx=FALSE;
      draw_cursor(TRUE);
#ifdef HAVE_BSD_CURSES
      nl();
      echo();
#endif
      endwin();
#ifdef XCURSES
      XCursesExit();
#endif
      curses_started = FALSE;
   }
#ifndef XCURSES
   if (!CLEARSCREENx
   && been_interactive)
      printf("\n");
#endif
#ifdef THE_SINGLE_INSTANCE_ENABLED
   /*
    * If we are the initial instance of THE running in single instance
    * mode, remove the FIFO
    */
   if ( single_instance_server )
   {
      close_fifo();
   }
#endif
   if (tempfilename)
   {
      if (file_exists(tempfilename))
         remove_file(tempfilename);
      (*the_free)(tempfilename);
   }
   if (stdinprofile)
   {
      if (file_exists(stdinprofile))
         remove_file(stdinprofile);
      (*the_free)(stdinprofile);
   }

#ifdef WIN32
   if (StartedPrinter)
   {
      StopTextPrnt();
   }
#endif
   /*
    * Free up the working memory
    */
#ifndef MSWIN
   the_free_flists();
#endif
   TRACE_RETURN();
   return;
}

#ifdef UNIX
/***********************************************************************/
#ifdef HAVE_PROTO
static RETSIGTYPE handle_signal(int err)
#else
static RETSIGTYPE handle_signal(err)
int err;
#endif
/***********************************************************************/
{
   FILE_DETAILS *cf;
   VIEW_DETAILS *curr;
   bool process_view=FALSE;
   FILE_DETAILS *first_view_file=NULL;
   register int j=0;

   TRACE_FUNCTION("the.c:     handle_signal");
   /*
    * If a SIGWINCH is caught, set a global flag to indicate that the screen has resized
    * Only do this for NCURSES.
    */
# if defined(SIGWINCH) && defined(USE_NCURSES)
   if (err == SIGWINCH)
   {
      ncurses_screen_resized = TRUE;
      signal(SIGWINCH,handle_signal);
      return;
   }
#endif
  /*
   * For each file in the ring, execute an AUTOSAVE on it and then
   * die.
   */
   if (curses_started)
   {
      endwin();
#ifdef XCURSES
      XCursesExit();
#endif
      curses_started = FALSE;
   }
   fprintf(stderr,"\nTHE terminated with signal: %d\n\n",err);
   signal(err,SIG_IGN); /* ignore any more of these signals while autosaving */
   curr = vd_current;
   for (j=0;j<number_of_files;)
   {
      process_view = TRUE;
      if (curr->file_for_view->file_views > 1)
      {
         if (first_view_file == curr->file_for_view)
            process_view = FALSE;
         else
            first_view_file = curr->file_for_view;
      }
      if (process_view)
      {
         j++;
         cf = curr->file_for_view;
         if (!cf->pseudo_file
         &&  cf->save_alt > 0)
         {
            fprintf(stderr,"Attempting to autosave: %s%s\n",cf->fpath,cf->fname);
            save_file(cf,cf->autosave_fname,TRUE,cf->number_lines,1L,NULL,FALSE,0,max_line_length,TRUE,FALSE,TRUE);
         }
      }
      curr = curr->next;
      if (curr == NULL)
         curr = vd_first;
   }
  /*
   * Lets not push our luck in the signal handler, and just die...
   */
  exit(25);
}
#endif

#if defined(XCURSES) && PDC_BUILD >= 2401
/***********************************************************************/
#ifdef HAVE_PROTO
char **StringToArgv( int *argc, char *string )
#else
char **StringToArgv( argc, string )
int *argc;
char *string;
#endif
/***********************************************************************/
{
#define STA_IN_QUOTE 1
#define STA_IN_SPACE 2
#define STA_IN_WORD  3
   char **argv;
   register int i;
   int max_args = 2;
   int len;
   int args = 0;
   int state;

   /*
    * Convert the option string to an argv[] array
    * We will be conservative in determining how many argv[]'s
    * there are. The most we can have is 1 plus the number of
    * spaces!
    */
   strtrunc( string );
   len = strlen( string );
   for ( i = 0; i < len; i++ )
   {
      if ( string[i] == ' ' )
         max_args++;
   }
   /*
    * Allocate this many char*s
    */
   argv = (*the_malloc)( sizeof(char *) * max_args * max_args );
   if (argv == NULL)
      return NULL;
   /*
    * First argv is programname
    */
   argv[args++] = XCursesProgramName;
   /*
    * Now go and split the string into args
    */
   argv[ args++ ] = string;
   state = STA_IN_WORD;
   for ( i = 0; i < len; i++ )
   {
      switch ( state )
      {
         case STA_IN_QUOTE:
            if ( string[i] == '\"' )
            {
               string[i] = '\0';
               break;
            }
            break;
         case STA_IN_SPACE:
            if ( string[i] != ' ' )
            {
               state = STA_IN_WORD;
               argv[ args++ ] = string+i;
               break;
            }
            break;
         case STA_IN_WORD:
            if ( string[i] == '\"' )
            {
               state = STA_IN_QUOTE;
               break;
            }
            if ( string[i] == ' ' )
            {
               string[i] = '\0';
               state = STA_IN_SPACE;
               break;
            }
            break;
      }
   }
   *argc = args;
   return argv;
}
#endif
