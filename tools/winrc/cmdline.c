/*    
	cmdline.c	1.32
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*	Platform-dependent support for rc on Unix and other command-line
*       based systems

 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef _LINUX_
#include <io.h>
#endif
#include "define.h"
#include "rc.h"
#include "buffer.h"
#ifdef _LINUX_
#include "lio.h"
#endif

FILE *out, *ep, *fp, *cp;
extern FILE *yyin;
FILE *fileLog;

extern char in_name[];
extern char out_name[];
extern char export_name[];
extern char cpp_module_name[];
extern char libentry_name[];
extern char tbl_name[];
static char *system_include_path[1] = { "/usr/include" };
static int n_system_include_path = sizeof(system_include_path) / 
				   sizeof(*system_include_path);
static char **user_include_path = NULL;
static int n_user_include_path = 0;
static char **include_path = NULL;
static int n_include_path = 0;
static int rcdebug = 0;
static char *source_dir;
struct binflagvals defbinflags = { 1, little }; /* by default use single-byte alignment, little-endian */

void rcyacc_debug_level(int);
void define_used_module(char *);

int build_app_module = 0;

struct longopts
{
  int opt;
  char *longname;
  int has_arg;
} optionlist[] = 
  {
    { 'a', "align", 1 },
    { 'b', "big-endian", 0 },
    { 'C', "cpp-module", 1 },
    { 'D', "define", 1 },
    { 'e', "endian", 1 },
    { 'E', "entry function", 1 },
    { 'h', "help", 0 },
    { 'I', "include-path", 1 },
    { 'i', "little-endian-compat", 0 },
    { 'l', "little-endian", 0 },
    { 'm', "app-module", 0 },
    { 'n', "table-name", 1 },
    { 'o', "output-file", 1 },
    { 'p', "exportfile", 1 },
    { 'u', "use-module", 1 },
    { 'U', "undefine", 1 },
    { 'v', "verbose", 0 },
    { 'V', "version", 0 },
    { 'x', "debug", 0 },
    { '?', "option-summary", 0 },
    { 0, NULL, 0 }
  };

void usage(void);
void cusage(char *, ...);
char *get_string(char *,int);
void do_option(char opt, char *arg);
char *get_options(int argc, char **argv);
void add_to_include_path(char *);
void merge_include_paths(void);

static int is_verbose = 0;
static int had_error = 0;

void exitfunc(void)
{
  if (had_error)
    unlink(out_name);
}

int main(int argc, char **argv)
{
  char *p;

  /* Give default output file name and table name */
  strcpy(out_name, "");

  /* Define some built-in things */
  define_define("unix", NULL);
  define_define("RC_INVOKED", NULL);
  define_define("TWINRC", NULL);

  strcpy(in_name, get_options(argc, argv));

  /* Change directory to the directory of the source file */
  source_dir = strdup(in_name);
  p = strrchr(source_dir, '/');
  if (p)
  {
      *p = '\0';
      add_to_include_path(source_dir);
  }
  else
  {
      free(source_dir);
      source_dir = ".";
  }
  
  /*Open a log file */
  if (rcdebug)
	fileLog = stdout;
  else
	fileLog = fopen("/dev/null", "w");

  merge_include_paths();

  icompile();

  fclose(fileLog);

  exitfunc();
  return had_error;
}

void do_option(char opt, char *arg)
{
  char *part;

  switch (opt)
    {
    case 'a': /* Alignment */
      defbinflags.align = alignval(arg);
      break;

    case 'b': /* Big-endian */
      defbinflags.endian = big;
      break;

    case 'C': /* C++ Module in library */
      strcpy(cpp_module_name, arg);
      break;

    case 'D': /* Define */
      if ((part = strchr(arg, '=')))
	*part++ = '\0';
      define_define(arg, part);
      if (part)
	*--part = '=';
      break;

    case 'E': /* Library module entry function name */
      strcpy(libentry_name, arg);
      break;

    case 'e': /* Endian, user-specified */
      if (*arg == 'l')
	defbinflags.endian = little;
      else if (*arg == 'b')
	defbinflags.endian = big;
      else
	cusage("--endian must be \"little\" or \"big\"");
      break;

    case 'I': /* Include path */
      add_to_include_path(arg);
      break;

    case 'i':
    case 'l': /* Little-endian */
	defbinflags.endian = little;
	break;

    case 'm': /* Build as a an application module */
      build_app_module = 1;
      break;

    case 'n': /* Resource table name */
      strcpy(tbl_name, arg);
      break;

    case 'o': /* Output file name */
      strcpy(out_name, arg);
      break;

    case 'p': /* Export file name */
      strcpy(export_name, arg);
      break;

    case 'u': /* Use (initialize) named application module */
      define_used_module(arg);
      break;

    case 'U': /* Undefine */
      undefine_define(arg);
      break;

    case 'v': /* Verbose output */
      is_verbose = 1;
      break;

    case 'V': /* Display version number */
      printf("TWIN rc version " RC_VERSION "\n");
      break;

    case 'x': /* Debugging on */
      rcdebug = 1;
      rcyacc_debug_level(1);
      break;

    case '?': /* Just display usage */
    case 'h':
      usage();
      break;

    default: /* Uh-oh... */
      FATAL(("Internal error: do_option got unrecognized option %c", opt));
    }
}

void cusage(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  vfprintf(stderr, str, ap);
  va_end(ap);
  fprintf(stderr, "\n\n");
  usage();
}

void usage(void)
{
  fprintf(stderr, "Usage: rc [-a align] [-b] [-D defineval] [-e endian]\n"
                  "          [-E entry-function-name] [-h] [-I include-path]\n"
                  "          [-i] [-l] [-m] [-n table-name] [-o output-file-name]\n"
                  "          [-U undefineval] [-v] [-V] [-?]\n");
  exit(1);
}

/* This is my getopt(2) replacement, since some platforms don't support it... */
char *get_options(int argc, char **argv)
{
  int i;
  struct longopts *pOpt;
  char *optarg = NULL, *argsearch, *nextpart, *filename;

  filename = NULL;

  for (i = 1; i < argc; i++)
    if (argv[i][0] != '-') /* It's a filename */
      if (!filename)
	filename = argv[i];
      else
	cusage("rc only processes one file");
    else /* It's an option, since it starts with - */
      if (argv[i][1] == '-') /* It's a long option */
	{
	  for (pOpt = optionlist; pOpt->longname; pOpt++)
	    {
	      if ((nextpart = strchr(argv[i], '=')))
		*nextpart++ = '\0';
	      if (strcmp(argv[i] + 2, pOpt->longname))
		{
		  if (nextpart)
		    *--nextpart = '=';
		  continue;
		}
	      if (nextpart && !pOpt->has_arg)
		cusage("Option --%s does not take an argument", pOpt->longname);
	      if (pOpt->has_arg)
		if (nextpart)
		  optarg = nextpart;
		else
		  if (++i < argc)
		    optarg = argv[i];
		  else
		    cusage("Option --%s takes an argument", pOpt->longname);
	      else
		optarg = NULL;
	      do_option(pOpt->opt, optarg);
	      if (nextpart)
		*--nextpart = '=';
	      break; /* Out of the for loop to find an option */
	    }
	  if (!pOpt->longname) /* Didn't find anything */
	    cusage("Unrecognized option %s", argv[i]);
	}
      else /* It's one or more short options */
	for (argsearch = argv[i] + 1; *argsearch; argsearch++) /* Look through each character in the string */
	  {
	    for (pOpt = optionlist; pOpt->longname; pOpt++)
	      {
		if (*argsearch == pOpt->opt) /* Is this one it? */
		  {
		    if (pOpt->has_arg) /* Yes, does it have an argument? */
		      if (*(argsearch + 1)) /* Yes, can we make it from this one (i. e. -Dfoo)? */
			{
			  optarg = ++argsearch;
			  argsearch += strlen(optarg) - 1;
			}
		      else /* No, can it be the next one? */
			if (++i < argc)
			  optarg = argv[i];
			else
			  cusage("Option -%c takes an argument", pOpt->opt);
		    else /* No argument */
		      optarg = NULL;
		    do_option(pOpt->opt, optarg);
		    break; /* Out of the for loop to find an option */
		  }
	      }
	    if (!pOpt->longname) /* Didn't find anything */
	      cusage("Unrecognized option -%c", *argsearch);
	  }
  if (!filename)
    usage();
  return filename;
}

void dolog(char *str, ...)
{
  if( fileLog != NULL )
  {
    va_list ap;
    va_start(ap, str);
    vfprintf(fileLog, str, ap);
    fprintf(fileLog, "\n");
    va_end(ap);
  }
}

void dovprint(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  if (is_verbose)
    {
      vprintf(str, ap);
      printf("\n");
    }
  va_end(ap);
}

void doerror(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  fprintf(stderr, "%s:%d: ", current_file(), current_line());
  vfprintf(stderr, str, ap);
  fprintf(stderr, "\n");
  had_error = 1;
  va_end(ap);
}

void dofatal(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  fprintf(stderr, "%s:%d: ", current_file(), current_line());
  vfprintf(stderr, str, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  had_error = 1;
  exitfunc();
  exit(1);
}

void dowarning(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  fprintf(stderr, "%s:%d: ", current_file(), current_line());
  vfprintf(stderr, str, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

void dooutput(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  vfprintf(out, str, ap);
  va_end(ap);
}

void doeoutput(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  if (ep)
    vfprintf(ep, str, ap);
  va_end(ap);
}

void docppoutput(char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  if (cp)
    vfprintf(cp, str, ap);
  va_end(ap);
}

void open_modulefiles(void)
{
  char *p;
  char filename[256];

  strcpy(filename,in_name);
  p = strrchr(filename,'.');
  *p = 0;
  strcat(filename,".def");
	
  /* Open the input file */
  fp = fopen(filename, "rb");

  ep = NULL;
  if (strlen(export_name) > 0)
  {
    ep = fopen(export_name, "w");
    if (!ep)
      FATAL(("Can't open %s", export_name));
  }
  cp = NULL;
  if (strlen(cpp_module_name) > 0)
  {
    cp = fopen(cpp_module_name, "w");
    if (!cp)
      FATAL(("Can't open %s", cpp_module_name));
  }
}

char *get_string(char *str, int len)
{
  if (fp)
    return fgets(str, len, fp);
  else
    return 0;
}

FILE *open_include_path(char *filename)
{
  char theFile[256];
  FILE *pFile;
  int i;

  for (i = 0; i < n_include_path; i++)
    {
      strcpy(theFile, include_path[i]);
      strcat(theFile, "/");
      strcat(theFile, filename);
      if ((pFile = fopen(theFile, "rb")))
	return pFile;
    }
  return NULL;
}

FILE *open_include(char *filename, int lookHere)
{
  char lowerFile[255];
  char upperFile[255];
  char *p, *dp, *sp;
  FILE *f;

  for (sp = dp = filename; *sp; sp++)
  {
    if (*sp == '\\')
    {
      *dp++ = '/';
      if ( *(sp+1) == '\\')
	sp++;
    }
    else
      *dp++ = *sp;
  }
  *dp = *sp;

  strcpy(lowerFile, filename);
  strcpy(upperFile, filename);
  for (p = lowerFile; *p; p++)
    *p = tolower(*p);
  for (p = upperFile; *p; p++)
    *p = toupper(*p);

  if (lookHere)
    {
      if ((f = fopen(filename, "rb")))
	return f;
      if ((f = fopen(lowerFile, "rb")))
	return f;
      if ((f = fopen(upperFile, "rb")))
	return f;
    }

  if ((f = open_include_path(filename)))
    return f;
  if ((f = open_include_path(lowerFile)))
    return f;
  if ((f = open_include_path(upperFile)))
    return f;
  return NULL;
}

char *load_file(char *filename, int lookHere)
{
  return buff_ind(buff_load(filename, lookHere));
}

buffp buff_load(char *filename, int lookHere)
{
  buffp b;
  FILE *f;
  unsigned len;

  f = open_include(filename, lookHere);
  if (!f)
    FATAL(("Could not find resource file %s", filename));

  fseek(f, 0, SEEK_END);
  len = ftell(f);
  b = buff_alloc(len + 1);
  b->len = b->alloc;

  fseek(f, 0, SEEK_SET);
  fread(b->buffer, 1, len, f);
  fclose(f);

  *((char *)(b->buffer) + len) = '\0';

  str_free(filename);

  return b;
}

void add_to_include_path(char *str)
{
  if (n_user_include_path == 0)
    {
      n_user_include_path = 1;
      user_include_path = (char **) malloc(sizeof(*user_include_path));
    }
  else
    {
      n_user_include_path++;
      user_include_path = (char **) realloc(user_include_path,
					    sizeof(*user_include_path) * 
					    n_user_include_path);
    }
  user_include_path[n_user_include_path - 1] = strdup(str);
}

void merge_include_paths()
{
  int i;
  int j;

  n_include_path = n_system_include_path + n_user_include_path;

  include_path = (char **) malloc(sizeof(*include_path) * n_include_path);

  for (i = 0, j = 0; j < n_user_include_path; i++, j++)
    include_path[i] = user_include_path[j];

  for (j = 0; j < n_system_include_path; i++, j++)
    include_path[i] = system_include_path[j];
}

void open_files(void)
{
  char	*temp;

  /* Check the input filename and append .exe if there is no extension. */
  if (!strchr(in_name,'.'))
    strcat(in_name, ".rc");

  /* Make the output file name <basename>.res.c if it didn't get set by the user */
  if(!out_name[0])
    {
      strcpy(out_name, in_name);
      temp = strrchr(out_name,'.');
      strcpy(temp+1, "res.c"); 
    }
  
  /* See if the file is really there, and open it */
  if (!(yyin = fopen(in_name, "rb")))
    {
      FATAL(("Couldn't open input file %s", in_name));
    }
  
  if (!(out = fopen(out_name, "w+")))
    {
      fclose(yyin);
      FATAL(("Couldn't open output file %s", out_name));
    }
}

/* at_{start,end}_of_{file,macro}
 *
 * Under Unix, etc., we tend not to care that much about what file we're
 * in...the OS handles all of the loading, unloading, etc. that we deal
 * with.  In particular, we open the file, load it, and close it; then
 * we're done with it.  These callbacks are ignored by us.
 */

void at_start_of_macro(void) { }
void at_start_of_file(void) { }
void at_end_of_file(void) { }

void update_info(void)
{
  /* Nothing to do in Unix */
}

/* Various memory stuff: under Unix, we just map to malloc/realloc/free, with error checking */
void *get_mem(unsigned size)
{
  void *p = malloc(size);

  if (!p)
    FATAL(("Out of memory"));

  return p;
}

void free_mem(void *p)
{
  if (p)
    free(p);
}
