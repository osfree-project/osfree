/* find.c */

/* Copyright (C) 2009, osFree.org */
/* Copyright (C) 1994-2002, Jim Hall <jhall@freedos.org> */

/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   */

/* This program locates a string in a text file and prints those lines
 * that contain the string.  Multiple files are clearly separated.
 */

#define INCL_DOSERRORS
#define INCL_VIO

#include <os2.h>                // Include file for osfree
#include <cmd_shared.h>         // Include file for cmd tools

#include <stdio.h>
#include <stdlib.h>                     /* borland needs this for 'exit' */
#include <fcntl.h>                      /* O_RDONLY */
#include <string.h>
#include <ctype.h>

#include <direct.h>                        /* for findfirst, findnext */
#include <dos.h>                        /* for findfirst, findnext */

#include "find_str.h"                   /* find_str() back-end */

/* Show usage */

void usage()
{
  cmd_ShowSystemMessage(cmd_MSG_FIND_HELP,0L);
}


/* Main program */


int main (int argc, char* argv[], char* envp[])
{
  char *s, *needle;
  int c, i, done, ret;

  unsigned drive /* , thisdrive */ ;    /* used to store, change drive */
  char cwd[CCHMAXPATH], thiscwd[CCHMAXPATH];    /* used to store, change cwd */
  char cwd2[CCHMAXPATH];                    /* store cwd of other drive */

  /* char drv[MAXDRIVE]; */             /* temporary buffer */
  unsigned drv;                         /* drive found in argument */
  unsigned maxdrives;                   /* not used */

  int invert_search = 0;                /* flag to invert the search */
  int count_lines = 0;                  /* flag to whether/not count lines */
  int number_output = 0;                /* flag to print line numbers */
  int ignore_case = 0;                  /* flag to be case insensitive */

  /* FILE *pfile; */                    /* file pointer */
  int thefile;                          /* file handler */
  struct find_t ffblk;                   /* findfirst, findnext block */

  /* Scan the command line */
  c = 1; /* argv[0] is the path of the exectutable! */

  /* first, expect all slashed arguments */
  while ((c < argc) && (argv[c][0] == '/') ) {
      /* printf("arg: %s\n",argv[c]); */
      switch (argv[c][1]) {
          case 'c':
          case 'C':             /* Count */
            count_lines = 1;
            break;

          case 'i':
          case 'I':             /* Ignore */
            ignore_case = 1;
            break;

          case 'n':
          case 'N':             /* Number */
            number_output = 1;
            break;

          case 'v':
          case 'V':             /* Not with */
            invert_search = 1;
            break;

          default:
            usage();
            exit (2);           /* syntax error .. return errorlevel 2 */
            break;

      } /* end case */
      c++;      /* next argument */
  } /* end while */


  /* Get the string */

  if (c >= argc)
    {
      /* No string? */
      /* printf("no string"); */
      usage ();
      exit (1);
    }
  else
    {
      /* Assign the string to find */
      needle = argv[c];
      c++; /* next argument(s), if any: file name(s) */
      /* printf("needle: %s\n",needle); */
    }



  /* Store the drive and cwd */

  /* findfirst/findnext do not return drive and cwd information, so we
     have to store the drive & cwd at the beginning, then chdir for
     each file we scan using findfirst, then chdir back to the initial
     drive & cwd at the end.  This is probably not the most efficient
     way of doing it, but it works.  -jh */

  _dos_getdrive(&drive); /* uses dir.h */
  getcwd (cwd, CCHMAXPATH);                 /* also stores drive letter */

  /* Scan the files for the string */

  if ((argc - c) <= 0)
    {
      /* No files on command line - scan stdin */
      ret = find_str (needle, 0 /* stdin */,
        invert_search, count_lines, number_output, ignore_case);
    }

  else
    {
      for (i = c; i < argc; i++)
        {
          /* find drive and wd for each file when using findfirst */

          /* fnsplit (argv[i], drv, thiscwd, NULL, NULL); */
          /* fnsplit is "expensive", so replace it... */

          if (argv[i][1] == ':') {
            drv = toupper(argv[i][0]) - 'A';
            strcpy(thiscwd,argv[i]+2);
          } else {
            drv = drive - 1; /* default drive */
            strcpy(thiscwd,argv[i]);
          }

          if (strrchr(thiscwd,'\\') == NULL) {
            strcpy(thiscwd,"."); /* no dir given */
          } else {
            if (strrchr(thiscwd,'\\') != thiscwd) {
              strrchr(thiscwd,'\\')[0] = '\0'; /* end string at last \\ */
            } else {
              strcpy(thiscwd,"\\"); /* dir is root dir */
            }
          }


          /* use findfirst/findnext to expand the filemask */

          done = _dos_findfirst (argv[i], 0, &ffblk);


          if (done)
            {
              /* We were not able to find a file. Display a message and
                 set the exit status. */

              cmd_ShowSystemMessage(MSG_FILE_NOT_FOUND, 0L);
            }

          while (!done)
            {
              /* We have found a file, so try to open it */

              /* set cwd to the filemask */

              /* _dos_setdrive (('A' - drv[0] + 1), &maxdrives); */
              /* this was the wrong way round! */

              (void) _dos_setdrive (drv, &maxdrives); /* dir.h */

              getcwd(cwd2,CCHMAXPATH); /* remember cwd here, too */

              if (chdir (thiscwd) < 0) {
                  if (strcmp(thiscwd, ".")) {
                    cmd_ShowSystemMessage(1103, 0L);
                  }
              };

              /* open the file, or not */


              if (_dos_open (ffblk.name, O_RDONLY, &thefile) == 0)
                {
                  fprintf (stderr, "---------------- %s\n", ffblk.name);
                  ret = find_str (needle, thefile, invert_search, count_lines, number_output, ignore_case);
                  _dos_close (thefile);
                }
              else
                {
                  cmd_ShowSystemMessage(MSG_FILE_NOT_FOUND, 0L);
                }

              /* return the cwd */

              chdir (cwd2); /* restore cwd on THAT drive */

              /* _dos_setdrive (drive, &maxdrives); */
              (void) _dos_setdrive (drive, &maxdrives); /* dir.h */
              chdir (cwd);

              /* find next file to match the filemask */

              done = _dos_findnext (&ffblk);
            } /* while !done */
        } /* for each argv */
    } /* else */

  /* Done */

  _dos_findclose (&ffblk);

 /* RETURN: If the string was found at least once, returns 0.
  * If the string was not found at all, returns 1.
  * (Note that find_str.c returns the exact opposite values.)
  */

  exit ( (ret ? 0 : 1) );
  return (ret ? 0 : 1);

}
