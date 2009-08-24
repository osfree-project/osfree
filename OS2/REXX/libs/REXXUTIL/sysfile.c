/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysSearchPath
*      SysFileSystemType
*      SysFileDelete
*      SysFileSearch
*      SysFileTree
*      SysGetFileDateTime
*      SysSetFileDateTime
*
*  Michael Greene, December 2007
*
*  17 Jan 2008 MKG:
*    I had to fix SysSearchPath matches oorexx sources - has option arg.
*
------------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/******************************************************************************/

#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <direct.h>
#include <alloca.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header


/*********************************************************************/
/* RxTree Structure used by SysTree.                                 */
/*********************************************************************/

typedef struct RxTreeData {
    unsigned long count;       /* Number of lines processed       */
    SHVBLOCK shvb;             /* Request block for RxVar         */
    unsigned long open_action; /* Action taken opening the file   */
    unsigned long stemlen;     /* Length of stem                  */
    unsigned long vlen;        /* Length of variable value        */
    char TargetSpec[MAX];      /* Filespec RxTree is looking for  */
    char truefile[MAX];        /* Used to hold actual file name   */
    char tempfile[MAX];        /* Used to hold temp file name     */
    char Temp[MAX];            /* Used when building stem element */
    char ibuf[IBUF_LEN];       /* Input buffer                    */
    char varname[MAX];         /* Buffer for the variable name    */
    unsigned long j;           /* Temp counter                    */
    unsigned long nattrib;     /* New attrib, diff for each file  */
} RXTREEDATA;


static FILESTATUS3 FileInfo;

//static long RecursiveFindFile(char *FileSpec, RXTREEDATA *ldp,
//                         int *smask, int *dmask, unsigned long options);

static long RecursiveFindFile(char *FileSpec, char *path, RXTREEDATA *ldp,
                        int *smask, int *dmask, unsigned long options);
static unsigned long mystrstr(char *haystack, char *needle,
                          unsigned long hlen, unsigned long nlen, bool sensitive);
static void getpath(char *string, char *path, char *filename);
static unsigned long SameAttr(int *mask, unsigned long attr);
static unsigned long NewAttr(int *mask, unsigned long attr);
static int SetFileMode(char *file, unsigned long attr);
static unsigned long FormatFile(RXTREEDATA *ldp, int *smask, int *dmask,
                          unsigned long options, FILEFINDBUF3 *finfo);

RexxFunctionHandler SysSearchPath;
RexxFunctionHandler SysFileSystemType;
RexxFunctionHandler SysFileDelete;
RexxFunctionHandler SysFileSearch;
RexxFunctionHandler SysFileTree;
RexxFunctionHandler SysGetFileDateTime;
RexxFunctionHandler SysSetFileDateTime;


/*************************************************************************
* Function:  SysSearchPath                                               *
*                                                                        *
* Syntax:    call SysSearchPath path, file [, options]                   *
*                                                                        *
* Params:    path - Environment variable name which specifies a path     *
*                    to be searched (ie 'PATH', 'DPATH', etc).           *
*            file - The file to search for.                              *
*            options -  'C' - Current directory search first (default).  *
*                       'N' - No Current directory search. Only searches *
*                             the path as specified.                     *
*                                                                        *
* Return:    other  - Full path and filespec of found file.              *
*            ''     - Specified file not found along path.               *
*************************************************************************/
/*
 * NOTE: This doesn't match the OS/2 docs but works as described on the
 * IBM site and in oorexx sources.
 */

unsigned long SysSearchPath(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long flags = SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT;
    unsigned long retbuffsize = (_MAX_PATH + _MAX_FNAME + _MAX_EXT);
    unsigned long rc;

    char *RetBufferbuf = NULL;

    if (numargs < 2 || numargs > 3 || !RXVALIDSTRING(args[0]) ||
        !RXVALIDSTRING(args[1])) return INVALID_ROUTINE;

    // do this the long way just so I can check for a bad 3rd arg
    if (numargs == 3) {

        switch(args[2].strptr[0]) {

        case 'c':
        case 'C':
            flags |= SEARCH_CUR_DIRECTORY;
            break;

        case 'n':
        case 'N':
            break;

        default:
            return INVALID_ROUTINE;
        }
    } else flags |= SEARCH_CUR_DIRECTORY; // default

    RetBufferbuf = alloca(retbuffsize);

    rc = DosSearchPath(flags, args[0].strptr,
                  args[1].strptr,
                  RetBufferbuf,
                  retbuffsize);

    // Don't even ask me why this happens, but while trying to
    // get this to work with ecsmt I kept getting a 203 error.
    // It was passing the LIBPATH in a env var {_temp} and it did
    // not make sense I was getting a failure. Just by chance I found
    // if the env var was passed upper case it worked. I just upper
    // case the 2nd arg for the hell of it -- 17 Jan 2008 MKG
    if (rc == ERROR_ENVVAR_NOT_FOUND) {
        strupr(args[0].strptr);
        strupr(args[1].strptr);

        rc = DosSearchPath(flags, args[0].strptr,
                      args[1].strptr,
                      RetBufferbuf,
                      retbuffsize);
    }

    retstr->strlength = sprintf(retstr->strptr, "%s", RetBufferbuf);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysFileSystemType                                           *
*                                                                        *
* Syntax:    result = SysFileSystemType("drive")                         *
*                                                                        *
* Params:    drive - drive letter (in form of 'D:')                      *
*        or  none - current drive                                        *
*                                                                        *
* Return:    result - File System Name attached to the specified drive   *
*                     (FAT, HPFS ....)                                   *
*            '' - Empty string in case of any error                      *
*                                                                        *
*************************************************************************/

unsigned long SysFileSystemType(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;
    unsigned char fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
    PFSQBUFFER2 pfsqBuffer  = (PFSQBUFFER2) fsqBuffer;
    unsigned long cbBuffer  = sizeof(fsqBuffer);
    char chDriveLetter[3];

    /* validate arguments         */
    if (numargs > 1 || ((numargs == 1) && (args[0].strlength > 2 ||
        args[0].strlength == 0))) return INVALID_ROUTINE;

    /* validate the arg, drive letter? */
    if ((numargs == 1) && (strlen(args[0].strptr) == 2 &&
        args[0].strptr[1] != ':')) return INVALID_ROUTINE;

    // Process drive letter, skip all the crap and just put in the colon
    if (numargs == 1) chDriveLetter[0]=args[0].strptr[0];   /* letter passed in  */
    else {
        char *cwd;

        cwd = getcwd( NULL, 0 );
        chDriveLetter[0]=cwd[0];
        free(cwd);
    }
    chDriveLetter[1]=':';
    chDriveLetter[2]='\0';

    DosError(0);                         /* Disable hard-error pop-up  */

    rc = DosQueryFSAttach(chDriveLetter, 0L, FSAIL_QUERYNAME, pfsqBuffer, &cbBuffer);

    if (rc == NO_ERROR) {
        sprintf(retstr->strptr, "%s", (pfsqBuffer->szName + pfsqBuffer->cbName + 1));
        retstr->strlength = strlen(retstr->strptr);
    } else retstr->strlength = 0;        /* return a null string       */

    DosError(1);                         /* Enable hard-error pop-up   */

    return VALID_ROUTINE;                /* good completion            */
}


/*************************************************************************
* Function:  SysFileDelete                                               *
*                                                                        *
* Syntax:    call SysFileDelete file                                     *
*                                                                        *
* Params:    file - file to be deleted.                                  *
*                                                                        *
* Return:    Return code from DosDelete() function.                      *
*************************************************************************/

unsigned long SysFileDelete(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    unsigned long rc;                        /* Return code */

    /* If no args, then its an  incorrect call */
    if (numargs != 1) return INVALID_ROUTINE;

    /* delete the file            */
    rc = DosDelete(args[0].strptr);

    /* result is return code      */
    sprintf(retstr->strptr, "%d", rc);
    retstr->strlength = strlen(retstr->strptr);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysFileSearch                                               *
*                                                                        *
* Syntax:    call SysFileSearch target, file, stem [, options]           *
*                                                                        *
* Params:    target  - String to search for.                             *
*            file    - Filespec to search.                               *
*            stem    - Stem variable name to place results in.           *
*            options - Any combo of the following:                       *
*                       'C' - Case sensitive search (non-default).       *
*                       'N' - Preceed each found string in result stem   *
*                              with it line number in file (non-default).*
*                                                                        *
* Return:    NO_UTIL_ERROR   - Successful.                               *
*            ERROR_NOMEM     - Out of memory.                            *
*************************************************************************/

unsigned long SysFileSearch(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    PSZ         target;                  /* search string              */
    PSZ         file;                    /* search file                */
    PSZ         opts;                    /* option string              */
    char        line[MAX];               /* Line read from file        */
    ULONG       ptr;                     /* Pointer to char str found  */
    ULONG       num = 0;                 /* Line number                */
    ULONG       len;                     /* Length of string           */
    unsigned long       len2;                    /* Length of string           */
//    ULONG       rc = 0;                  /* Return code of this func   */
    BOOL        linenums = FALSE;        /* Set TRUE for linenums in   */
                                         /* output                     */
    BOOL        sensitive = FALSE;       /* Set TRUE for case-sens     */
                                         /* search                     */
    FILE       *fp;                      /* Pointer to file to search  */
    RXSTEMDATA  ldp;                     /* stem data                  */

    BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */
                                         /* validate arguments         */
    if (numargs < 3 || numargs > 4 ||
        !RXVALIDSTRING(args[0]) ||
        !RXVALIDSTRING(args[1]) ||
        !RXVALIDSTRING(args[2]))
      return INVALID_ROUTINE;            /* raise an error             */

    target = args[0].strptr;             /* get target pointer         */
    file = args[1].strptr;               /* get file name              */

    if (numargs == 4) {                  /* process options            */
        opts = args[3].strptr;           /* point to the options       */
        if (strstr(opts, "N") || strstr(opts, "n")) linenums = TRUE;
        if (strstr(opts, "C") || strstr(opts, "c")) sensitive = TRUE;
    }
                                          /* Initialize data area       */
    ldp.count = 0;
    strcpy(ldp.varname, args[2].strptr);
    ldp.stemlen = args[2].strlength;
    strupr(ldp.varname);                 /* uppercase the name         */

    if (ldp.varname[ldp.stemlen-1] != '.') ldp.varname[ldp.stemlen++] = '.';

    fp = fopen(file, "r");               /* Open the file              */

    if (fp == NULL) {                    /* Open error?                */
        BUILDRXSTRING(retstr, ERROR_FILEOPEN);
        return VALID_ROUTINE;            /* finished                   */
    }
                                         /* do the search...found lines*/
                                         /* are saved in stem vars     */
    while (fgets(line, MAX - 1, fp) != NULL) {
        len = strlen(line);
        num++;
        ptr = mystrstr(line, target, len, args[0].strlength, sensitive);

        if (ptr != '\0') {
            if (linenums) {
                sprintf(ldp.ibuf, "%d ", num);
                len2 = strlen(ldp.ibuf);
                memcpy(ldp.ibuf+len2, line, len);
                ldp.vlen = len+len2;
            } else {
                memcpy(ldp.ibuf, line, len);
                ldp.vlen = len;
            }

            ldp.count++;
            sprintf(ldp.varname+ldp.stemlen, "%d", ldp.count);

            if (ldp.ibuf[ldp.vlen-1] == '\n') ldp.vlen--;
            ldp.shvb.shvnext = NULL;
            ldp.shvb.shvname.strptr = ldp.varname;
            ldp.shvb.shvname.strlength = strlen(ldp.varname);
            ldp.shvb.shvnamelen = ldp.shvb.shvname.strlength;
            ldp.shvb.shvvalue.strptr = ldp.ibuf;
            ldp.shvb.shvvalue.strlength = ldp.vlen;
            ldp.shvb.shvvaluelen = ldp.vlen;
            ldp.shvb.shvcode = RXSHV_SET;
            ldp.shvb.shvret = 0;

            /* error on non-zero          */
            if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN) return INVALID_ROUTINE;
        }
    }

    fclose(fp);                        /* Close that file            */
                                       /* set stem.0 to lines read   */
    sprintf(ldp.ibuf, "%d", ldp.count);
    ldp.varname[ldp.stemlen] = '0';
    ldp.varname[ldp.stemlen+1] = 0;
    ldp.shvb.shvnext = NULL;
    ldp.shvb.shvname.strptr = ldp.varname;
    ldp.shvb.shvname.strlength = ldp.stemlen+1;
    ldp.shvb.shvnamelen = ldp.stemlen+1;
    ldp.shvb.shvvalue.strptr = ldp.ibuf;
    ldp.shvb.shvvalue.strlength = strlen(ldp.ibuf);
    ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
    ldp.shvb.shvcode = RXSHV_SET;
    ldp.shvb.shvret = 0;
    if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
      return INVALID_ROUTINE;            /* error on non-zero          */

    return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  SysGetFileDateTime                                          *
*                                                                        *
* Syntax:    result = SysGetFileDateTime(filename [,timesel])            *
* Params:    filename - name of the file to query                        *
*            timesel  - What filetime to query: Created/Access/Write     *
*            default, no timesel, returns Write
*                                                                        *
* Return:    -1 - file date/time query failed                            *
*            other - date and time as YYYY-MM-DD HH:MM:SS                *
*************************************************************************/

unsigned long SysGetFileDateTime(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    PFTIME fileTime;
    PFDATE fileDate;

    /* we expect one to three parameters, if three parameters are      */
    /* specified then the second may be omitted to set only a new time,*/
    /* if only one is specified then the file is set to current time   */
    if ( (numargs < 1) || (numargs > 2) ||
         ((numargs == 2) && !RXVALIDSTRING(args[1])) )
      return INVALID_ROUTINE;            /* raise error condition      */

    // Does the file of directory exist?
    if(access(args[0].strptr, R_OK) != 0) RETVAL(-1)

    // clear the structure
    memset(&FileInfo, 0, sizeof(FILESTATUS3));

    // get info, error if not found
    if(DosQueryPathInfo(args[0].strptr, FIL_STANDARD,
         &FileInfo, sizeof(FILESTATUS3))) RETVAL(-1)

    // Get info requested by timesel - Write is default if no timesel
    if (numargs > 1) {

        switch (args[1].strptr[0]) {

        case 'c':
        case 'C':
            fileTime = &FileInfo.ftimeCreation;
            fileDate = &FileInfo.fdateCreation;
            break;

        case 'a':
        case 'A':
            fileTime = &FileInfo.ftimeLastAccess;
            fileDate = &FileInfo.fdateLastAccess;
            break;

        case 'w':
        case 'W':
            fileTime = &FileInfo.ftimeLastWrite;
            fileDate = &FileInfo.fdateLastWrite;
            break;

        default:
            return INVALID_ROUTINE;
        }

    } else {
        fileTime = &FileInfo.ftimeLastWrite;
        fileDate = &FileInfo.fdateLastWrite;
    }

    sprintf(retstr->strptr, "%4d-%02d-%02d %02d:%02d:%02d",
            (fileDate->year+1980L), /* Years since 1980 */
            fileDate->month,        /* Month            */
            fileDate->day,          /* Day              */
            fileTime->hours,        /* Hours            */
            fileTime->minutes,      /* Minutes          */
            (fileTime->twosecs*2)); /* Seconds          */
    retstr->strlength = strlen(retstr->strptr);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysSetFileDateTime                                          *
*                                                                        *
* Syntax:    result = SysGetFileDateTime(filename [,timesel])            *
* Params:    filename - name of the file to query                        *
*            timesel  - What filetime to query: Created/Access/Write     *
*            default, no timesel, returns Write
*                                                                        *
* Return:    -1 - file date/time query failed                            *
*            other - date and time as YYYY-MM-DD HH:MM:SS                *
*************************************************************************/

unsigned long SysSetFileDateTime(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    bool fOk = TRUE;

    PFTIME fileTime;
    PFDATE fileDate;

    /* we expect one to three parameters, if three parameters are      */
    /* specified then the second may be omitted to set only a new time,*/
    /* if only one is specified then the file is set to current time   */
    if ( (numargs < 1) || (numargs > 3) ||
         ((numargs == 2) && !RXVALIDSTRING(args[1])) ||
         ((numargs == 3) && !RXVALIDSTRING(args[2])) )
      return INVALID_ROUTINE;            /* raise error condition      */

    // Does the file of directory exist?
    if(access(args[0].strptr, R_OK) != 0) RETVAL(-1)

    // clear the structure
    memset(&FileInfo, 0, sizeof(FILESTATUS3));

    // get info, error if not found
    if(DosQueryPathInfo(args[0].strptr, FIL_STANDARD,
         &FileInfo, sizeof(FILESTATUS3))) RETVAL(-1)

    fileTime = &FileInfo.ftimeLastWrite;
    fileDate = &FileInfo.fdateLastWrite;

    // 2 or more args so handle arg 2 - date
    if ((numargs >= 2) && RXVALIDSTRING(args[1])) {
        /* parse new date */
        unsigned int year;
        unsigned int month;
        unsigned int day;

        if (sscanf(args[1].strptr, "%4hu-%2hu-%2hu",
                          &year, &month, &day) != 3) fOk = FALSE;

        // load date info into structure
        fileDate = &FileInfo.fdateLastWrite;
        fileDate->year  = (year - 1980);
        fileDate->month = month;
        fileDate->day   = day;
    }

    // 3 args so handle arg 3 - time
    if ((numargs == 3) && RXVALIDSTRING(args[2]) && fOk) {
        /* parse new time */
        unsigned int hours;
        unsigned int minutes;
        unsigned int seconds;

        if (sscanf(args[2].strptr, "%2hu:%2hu:%2hu",
                          &hours, &minutes, &seconds) != 3) fOk = FALSE;

        // load time info into structure
        fileTime = &FileInfo.ftimeLastWrite;
        fileTime->hours   = hours;
        fileTime->minutes = minutes;
        fileTime->twosecs = (seconds < 2)?0:(seconds/2);
    }

    // only received the file name so get current date time
    if (numargs == 1) {
        /* we set the timestamp to the current time and date */
        DATETIME DateTime;

        DosGetDateTime(&DateTime);

        // load date info into structure
        fileDate = &FileInfo.fdateLastWrite;
        fileDate->year  = (DateTime.year - 1980);
        fileDate->month = DateTime.month;
        fileDate->day   = DateTime.day;

        // load time info into structure
        fileTime = &FileInfo.ftimeLastWrite;
        fileTime->hours   = DateTime.hours;
        fileTime->minutes = DateTime.minutes;
        fileTime->twosecs = (DateTime.seconds < 2)?0:(DateTime.seconds/2);
    }

    // return error if the previous had problems
    if(!fOk) RETVAL(-1)

    // write the info back to the file or directory
    if(DosSetPathInfo(args[0].strptr, FIL_STANDARD, &FileInfo,
         sizeof(FILESTATUS3), DSPI_WRTTHRU)) RETVAL(-1)

    RETVAL(0);
}


/*************************************************************************
* Function:  SysFileTree                                                 *
*                                                                        *
* Syntax:    call SysFileTree filespec, stem [, options]                 *
*                                                                        *
* Params:    filespec - Filespec to search for (may include * and ?).    *
*            stem     - Name of stem var to store results in.            *
*            options  - Any combo of the following:                      *
*                        'B' - Search for files and directories.         *
*                        'D' - Search for directories only.              *
*                        'F' - Search for files only.                    *
*                        'O' - Only output file names.                   *
*                        'S' - Recursively scan subdirectories.          *
*                        'T' - Combine time & date fields into one.      *
*                                                                        *
* Return:    NO_UTIL_ERROR   - Successful.                               *
*            ERROR_NOMEM     - Out of memory.                            *
*************************************************************************/

unsigned long SysFileTree(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    int  smask[5];
    int  dmask[5];

    char buff1[MAX] = {0};
    char buff2[MAX] = {0};
    char *FileSpec  = buff1;
    char *path      = buff2;

    unsigned char *optptr;
    unsigned long options = FIRST_TIME | DO_FILES | DO_DIRS;
    unsigned long y;

    RXTREEDATA  ldp;

    smask[0] = RXIGNORE;
    dmask[0] = RXIGNORE;

    if (numargs < 2 || numargs > 5 || !RXVALIDSTRING(args[0]) ||
        !RXVALIDSTRING(args[1]) || args[0].strlength > 255)
        return INVALID_ROUTINE;

    if (args[0].strlength > 255) {
        FileSpec = (char*) malloc(sizeof(char)*(args[0].strlength+8));
        if (FileSpec == NULL) return INVALID_ROUTINE;
        path = (char*) malloc(sizeof(char)*(args[0].strlength+8));
        if (path == NULL) {
            free(FileSpec);
            return INVALID_ROUTINE;
        }
    }

    ldp.count = 0;
    strcpy(ldp.varname, args[1].strptr);
    ldp.stemlen = args[1].strlength;
    strupr(ldp.varname);

    if (ldp.varname[ldp.stemlen-1] != '.') ldp.varname[ldp.stemlen++] = '.';

    strcpy(FileSpec, args[0].strptr);

    if (FileSpec[args[0].strlength]) FileSpec[args[0].strlength] = 0x00;

    if (FileSpec[args[0].strlength-1] == '\\') strcat(FileSpec, "*.*");
    else if (FileSpec[args[0].strlength-1] == '.') strcat(FileSpec, "\\*.*");

    if (numargs >= 3 && !RXNULLSTRING(args[2])) {
        if (!args[2].strlength) {
            if (FileSpec != buff1) {
                free(FileSpec);
                free(path);
            }
            return INVALID_ROUTINE;
        }
        optptr = args[2].strptr;

        while (*optptr) {

            switch(toupper(*optptr)) {

            case 'S':
                options |= RECURSE;
                break;

            case 'O':
                options |= NAME_ONLY;
                break;

            case 'T':
                options |= EDITABLE_TIME;
                break;

            case 'L':
                options |= LONG_TIME;
                break;

            case 'F':
                options &= ~DO_DIRS;
                options |= DO_FILES;
                break;

            case 'D':
                options |= DO_DIRS;
                options &= ~DO_FILES;
                break;

            case 'B':
                options |= DO_DIRS;
                options |= DO_FILES;
                break;

            default:
                if (FileSpec != buff1) {
                    free(FileSpec);
                    free(path);
                }
                return INVALID_ROUTINE;
            }
            optptr++;
        }
    }

    if (numargs >= 4 && !RXNULLSTRING(args[3])) {
        optptr = args[3].strptr;

        smask[0] = smask[1] = smask[2] = smask[3] = smask[4] = 0;

        if (strlen((const char *)optptr) > 5) return INVALID_ROUTINE;

        y = 0;

        while (*optptr) {
            if (*optptr == '+') smask[y] = 1;
            else if (*optptr == '-') smask[y] = -1;
            else if (*optptr == '*') smask[y] = 0;
            else {
                if (FileSpec != buff1) {
                    free(FileSpec);
                    free(path);
                }
                return INVALID_ROUTINE;
            }
            y++;
            optptr++;
        }
    }

    if (numargs == 5) {
        dmask[0] = dmask[1] = dmask[2] = dmask[3] = dmask[4] = 0;
        optptr = args[4].strptr;
        if (strlen(optptr) > 5) {
            if (FileSpec != buff1) {
                free(FileSpec);
                free(path);
            }
            return INVALID_ROUTINE;
        }
        y = 0;
        while (*optptr) {

            if (*optptr == '+') dmask[y] = 1;
            else if (*optptr == '-') dmask[y] = -1;
            else if (*optptr == '*') dmask[y] = 0;
            else {
                if (FileSpec != buff1) {
                    free(FileSpec);
                    free(path);
                }
                return INVALID_ROUTINE;
            }
            y++;
            optptr++;
        }
        dmask[1] = 0;

    }

    getpath(FileSpec, path, ldp.TargetSpec);

    if (RecursiveFindFile(FileSpec, path, &ldp, smask, dmask, options))
              return INVALID_ROUTINE;

    ltoa(ldp.count, ldp.Temp, 10);
    ldp.varname[ldp.stemlen] = '0';
    ldp.varname[ldp.stemlen+1] = 0;
    ldp.shvb.shvnext = NULL;
    ldp.shvb.shvname.strptr = ldp.varname;
    ldp.shvb.shvname.strlength = ldp.stemlen+1;
    ldp.shvb.shvnamelen = ldp.stemlen+1;
    ldp.shvb.shvvalue.strptr = ldp.Temp;
    ldp.shvb.shvvalue.strlength = strlen(ldp.Temp);
    ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
    ldp.shvb.shvcode = RXSHV_SET;
    ldp.shvb.shvret = 0;

    if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
            return INVALID_ROUTINE;

    RETVAL(0)
}


/******************* Helpers follow ********************************/

/********************************************************************
* Function:  mystrstr(haystack, needle, hlen, nlen, sensitive)      *
*                                                                   *
* Purpose:   Determines if the string 'needle' is in the            *
*            string 'haystack' by returning it's position or        *
*            a NULL if not found.  The length of haystack and       *
*            needle are given by 'hlen' and 'nlen' respectively.    *
*                                                                   *
*            If 'sensitive' is true, then the search is case        *
*            sensitive, else it is case insensitive.                *
*                                                                   *
* RC:        num  - The pos where needle was found.                 *
*            NULL - needle not found.                               *
*                                                                   *
* Used By:   SysFileSearch()                                        *
*********************************************************************/

static unsigned long mystrstr(char *haystack, char *needle, unsigned long hlen,
                       unsigned long nlen, bool sensitive)
{
    char line[MAX];
    char target[MAX];
    unsigned long p;

    /* Copy line  - Change nulls to spaces and uppercase if needed      */
    for (p = 0; p < hlen; p++) {
        if (haystack[p] == '\0') line[p] = ' ';
        else if (sensitive) line[p] = haystack[p];
        else line[p] = (char)toupper(haystack[p]);
    }
    line[p] = '\0';

    /* Copy target  - Change nulls to spaces and uppercase if needed    */

    for (p = 0; p < nlen; p++) {
        if (needle[p] == '\0') target[p] = ' ';
        else if (sensitive) target[p] = needle[p];
        else target[p] = (char)toupper(needle[p]);
    }
    target[p] = '\0';

    return ((unsigned long)strstr(line, target));
}


/*****************************************************************************
* Function: RecursiveFindFile( FileSpec, lpd, smask, dmask, options )        *
*                                                                            *
* Purpose:  Finds all files starting with FileSpec, and will look down the   *
*           directory tree if required.                                      *
*                                                                            *
* Params:   FileSpec - ASCIIZ string which designates filespec to search     *
*                       for.                                                 *
*                                                                            *
*           ldp      - Pointer to local data structure.                      *
*                                                                            *
*           smask    - Array of integers which describe the source attribute *
*                       mask.  Only files with attributes matching this mask *
*                       will be found.                                       *
*                                                                            *
*           dmask    - Array of integers which describe the target attribute *
*                       mask.  Attributes of all found files will be set     *
*                       using this mask.                                     *
*                                                                            *
*             Note:  Both source and targets mask are really arrays of       *
*                    integers.  Each index of the mask corresponds           *
*                    to a different file attribute.  Each indexe and         *
*                    its associated attribute follows:                       *
*                                                                            *
*                         mask[0] = FILE_ARCHIVED                            *
*                         mask[1] = FILE_DIRECTORY                           *
*                         mask[2] = FILE_HIDDEN                              *
*                         mask[3] = FILE_READONLY                            *
*                         mask[4] = FILE_SYSTEM                              *
*                                                                            *
*                    A negative value at a given index indicates that        *
*                    the attribute bit of the file is not set.  A positive   *
*                    number indicates that the attribute should be set.      *
*                    A value of 0 indicates a "Don't Care" setting.          *
*                                                                            *
*           options  - The search/output options.  The following options     *
*                       may be ORed together when calling this function:     *
*                                                                            *
*                    FIRST_TIME  - Indicates this is initial call.  This     *
*                                   should always be used.                   *
*                    RECURSE     - Indicates that function should search     *
*                                   all child subdirectories recursively.    *
*                    DO_DIRS     - Indicates that directories should be      *
*                                   included in the search.                  *
*                    DO_FILES    - Indicates that files should be included   *
*                                   in the search.                           *
*                    NAME_ONLY   - Indicates that the output should be       *
*                                   restricted to filespecs only.            *
*                    EDITABLE_TIME - Indicates time and date fields should   *
*                                   be output as one timestamp.              *
*                                                                            *
* Used By:  SysFileTree()                                                    *
*                                                                            *
*   FileSpec            Filespecs to search for                              *
*   ldp                 Pointer to local data                                *
*   smask               Mask of attributes to search for                     *
*   dmask               Mask of attributes to set  *                         *
*   options             Search and output format options                     *
*                                                                            *
*****************************************************************************/

static long RecursiveFindFile(char *FileSpec, char *path, RXTREEDATA *ldp,
                        int *smask, int *dmask, unsigned long options)
{
    char  tempfile[_MAX_PATH+1];

    unsigned long FindCount   = 1;
    unsigned long Attribute   = FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | \
                                FILE_SYSTEM | FILE_ARCHIVED;
    unsigned long AttribDir   = FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | \
                                FILE_ARCHIVED | MUST_HAVE_DIRECTORY | FILE_DIRECTORY;
    unsigned long fdBufLen    = sizeof(FILEFINDBUF3);

    FILEFINDBUF3  fd;

    HDIR fHandle = HDIR_CREATE;


    sprintf(tempfile, "%s%s", path, ldp->TargetSpec);

    if ((options & DO_FILES) &&
        !DosFindFirst(tempfile, &fHandle, Attribute, &fd, fdBufLen, &FindCount, FIL_STANDARD)) {
        do {
            if (SameAttr(smask, fd.attrFile)) {
                sprintf(ldp->truefile, "%s%s", path, fd.achName);

                if (FormatFile(ldp, smask, dmask, options, &fd)) {
                    DosFindClose(fHandle);
                    return INVALID_ROUTINE;
                }
            }

        FindCount = 1;

        } while (!DosFindNext(fHandle, &fd, fdBufLen, &FindCount));

        DosFindClose(fHandle);
    }

    fHandle = HDIR_CREATE;
    FindCount = 1;

    if ((options & DO_DIRS)  &&
        !DosFindFirst(tempfile, &fHandle, AttribDir, &fd, fdBufLen, &FindCount, FIL_STANDARD)) {
        do {
            if (!strcmp(fd.achName, ".") || !strcmp(fd.achName, "..")) continue;
            if (SameAttr(smask, fd.attrFile)) {
                sprintf(ldp->truefile, "%s%s", path, fd.achName);
                if (FormatFile(ldp, smask, dmask, options, &fd)) {
                    DosFindClose(fHandle);
                    return INVALID_ROUTINE;
                }
            }
            FindCount = 1;
        } while (!DosFindNext(fHandle, &fd, fdBufLen, &FindCount));

        DosFindClose(fHandle);
    }

    fHandle = HDIR_CREATE;

    FindCount = 1;

    if (options&RECURSE) {
        sprintf(tempfile, "%s*", path);

        if (!DosFindFirst(tempfile, &fHandle, AttribDir, &fd, fdBufLen, &FindCount, FIL_STANDARD)) {
            do {
                if (!strcmp(fd.achName, ".") || !strcmp(fd.achName, "..")) continue;
                sprintf(tempfile, "%s%s\\", path, fd.achName);
                if (RecursiveFindFile(ldp->TargetSpec, tempfile, ldp, smask, dmask, options)) {
                    DosFindClose(fHandle);
                    return INVALID_ROUTINE;
                }
                FindCount = 1;
            } while (!DosFindNext(fHandle, &fd, fdBufLen, &FindCount));
            DosFindClose(fHandle);
        }

    }
    return VALID_ROUTINE;
}


/*****************************************************************
* Function:  getpath(string, path, filename)                     *
*                                                                *
* Purpose:  This function gets the PATH and FILENAME of the file *
*           target contained in STRING.  The path will end with  *
*           the '\' char if a path is supplied.                  *
*                                                                *
* Used By:  RecursiveFindFile()                                  *
*****************************************************************/

static void getpath(char *string, char *path, char *filename)
{
    int    len;                          /* length of filespec         */
    int    LastSlashPos;                 /* position of last slash     */
    char   szBuff[_MAX_PATH];            /* used to save current dir   */
    char   drv[3];                       /* used to change dir         */
    int    i=0;

    while (string[i] == ' ') i++;        /* skip leading blanks        */

    if (i) {
        len = strlen(string);            /* Get length of full file    */
        if ((string[i] == '\\' || string[i] == '/') ||  /* if first after blank is \ */
            (string[i] == '.' &&
              ((i<len && (string[i+1] == '\\' || string[i+1] == '/')) ||  /* or .\ */
              (i+1<len && string[i+1] == '.' && (string[i+2] == '\\' || string[i+2] == '/')))) ||  /* or ..\ */
              (i<len && string[i+1] == ':'))  /* z: */
                  string = &string[i];
    }

    if (!strcmp(string, ".")) strcpy(string, "*.*"); /* period case?               */
                                                     /* make it a *.* request      */
    else if (!strcmp(string, ".."))                  /* double period case?        */
              strcpy(string, "..\\*.*");             /* make it a ..\*.* request   */

    len = strlen(string);                /* Get length of full file    */
                                         /* spec                       */
    LastSlashPos = len;                  /* Get max pos of last '\'    */

    /* Step back through string until at begin or at '\' char        */

    while (string[LastSlashPos] != '\\' && string[LastSlashPos] != '/' && LastSlashPos >= 0)
      --LastSlashPos;
    if (LastSlashPos < 0) {              /* no backslash, may be drive */
        if (string[1] == ':') {
            len = _MAX_PATH;             /* set max length             */
                                         /* Save the current drive     */
                                         /* and path                   */

            //GetCurrentDirectory(sizeof(szBuff), szBuff);
            getcwd(szBuff, sizeof(szBuff));


            /* just copy the drive letter and the colon, omit the rest */
            /* (necessary i.g. if "I:*" is used */
            memcpy(drv, string, 2);
            drv[2] = '\0';


            chdir(drv);
            //SetCurrentDirectory(drv);        /* change to specified drive  */
                                             /* Get current directory      */
            //GetCurrentDirectory(len, path);
            getcwd(path, len);

            chdir(szBuff);
            //SetCurrentDirectory(szBuff);     /* go back to where we were   */
                                             /* need a trailing slash?     */
            if (path[strlen(path) - 1] != '\\')
                strcat(path, "\\");          /* add a trailing slash       */
                LastSlashPos = 1;            /* make drive the path        */

        } else {
                                             /* Get current directory      */
            //GetCurrentDirectory(_MAX_PATH, path);
            getcwd(path, _MAX_PATH);
                                            /* need a trailing slash?     */
            if (path[strlen(path) - 1] != '\\')
                  strcat(path, "\\");       /* add a trailing slash       */
        }

    } else {                                /* have a path                */
        if (string[1] == ':') {             /* have a drive?              */
                                            /* copy over the path         */
            memcpy(path, string, LastSlashPos+1);
            path[LastSlashPos+1] = '\0';    /* make into an ASCII-Z string*/
        } else {
            char fpath[_MAX_PATH];
            char drive[_MAX_DRIVE];
            char dir[_MAX_DIR];
            char fname[_MAX_FNAME];
            char ext[_MAX_EXT];
            char lastc;


            if (LastSlashPos == 0) { /* only one backslash at the beginning */
                _fullpath(fpath, "\\", _MAX_PATH);  /* Get full path        */
                strcat(fpath, &string[1]);
            } else {
                string[LastSlashPos] = '\0';         /* chop off the path          */
                _fullpath(fpath, string, _MAX_PATH); /* Get full path       */
                string[LastSlashPos] = '\\';         /* put the slash back         */
                lastc = fpath[strlen(fpath)-1];
                if (lastc != '\\' && lastc != '/') strcat(fpath, &string[LastSlashPos]);
            }
                _splitpath( fpath, drive, dir, fname, ext );

                strcpy(path, drive);
                strcat(path, dir);

                if (!strlen(path)) {              /* invalid path?              */
                                                  /* copy over the path         */
                    memcpy(path, string, LastSlashPos+1);
                    path[LastSlashPos+1] = '\0';  /* make into an ASCII-Z string*/
                }
                                                  /* need a trailing slash?     */
                if (path[strlen(path) - 1] != '\\')
                    strcat(path, "\\");           /* add a trailing slash       */
        }
    }

    /* Get file name from filespec (just after last '\')             */
    if (string[LastSlashPos+1])          /* have a real name?          */
                                         /* copy it over               */
        strcpy(filename, &string[LastSlashPos+1]);
    else
        strcpy(filename, "*.*");         /* just use wildcards         */
}


/*********************************************************************/
/* Function: ULONG SameAttr(mask, attr)                              */
/*                                                                   */
/* Purpose:  Returns the value TRUE if the attribute is identical to */
/*           that specified by the mask.  If not the same, then      */
/*           returns the value FALSE.                                */
/*                                                                   */
/* Used By:  RecursiveFindFile()                                     */
/*********************************************************************/

static unsigned long SameAttr(int *mask, unsigned long attr)
{

                                       /* if only want directories   */
                                       /* and is not a directory     */
//  if ((options&DO_DIRS) && !(options&DO_FILES) && !(attr&FILE_ATTRIBUTE_DIRECTORY))
//     return FALSE;
                                       /* if only want files and     */
                                       /* is a directory             */
//  if (!(options&DO_DIRS) && (options&DO_FILES) && (attr&FILE_ATTRIBUTE_DIRECTORY))
//     return FALSE;

    if (mask[0] == RXIGNORE) return  TRUE;
    if (mask[0] < 0 && attr&FILE_ARCHIVED) return  FALSE;
    if (mask[0] > 0 && !(attr&FILE_ARCHIVED)) return  FALSE;
    if (mask[1] < 0 && attr&FILE_DIRECTORY) return  FALSE;
    if (mask[1] > 0 && !(attr&FILE_DIRECTORY)) return  FALSE;
    if (mask[2] < 0 && attr&FILE_HIDDEN) return  FALSE;
    if (mask[2] > 0 && !(attr&FILE_HIDDEN)) return  FALSE;
    if (mask[3] < 0 && attr&FILE_READONLY) return  FALSE;
    if (mask[3] > 0 && !(attr&FILE_READONLY)) return  FALSE;
    if (mask[4] < 0 && attr&FILE_SYSTEM) return  FALSE;
    if (mask[4] > 0 && !(attr&FILE_SYSTEM)) return  FALSE;

    return  TRUE;
}


/*********************************************************************/
/* Function: ULONG NewAttr(mask, attr)                              */
/*                                                                   */
/* Purpose:  Returns the new file attribute, given the mask of       */
/*           attributes to be cleared/set and the current attribute  */
/*           settings.                                               */
/*                                                                   */
/* Used By:  RecursiveFindFile()                                     */
/*********************************************************************/

static unsigned long NewAttr(int *mask, unsigned long attr)
{
    if (mask[0] == RXIGNORE) return  attr;
    if (mask[0] < 0) attr &= ~FILE_ARCHIVED;  /* Clear  */
    if (mask[0] > 0) attr |= FILE_ARCHIVED;   /* Set    */
    if (mask[1] < 0) attr &= ~FILE_DIRECTORY; /* Clear  */
    if (mask[1] > 0) attr |= FILE_DIRECTORY;  /* Set    */
    if (mask[2] < 0) attr &= ~FILE_HIDDEN;    /* Clear  */
    if (mask[2] > 0) attr |= FILE_HIDDEN;     /* Set    */
    if (mask[3] < 0) attr &= ~FILE_READONLY;  /* Clear  */
    if (mask[3] > 0) attr |= FILE_READONLY;   /* Set    */
    if (mask[4] < 0) attr &= ~FILE_SYSTEM;    /* Clear  */
    if (mask[4] > 0) attr |= FILE_SYSTEM;     /* Set    */

    return  attr;
}


/********************************************************************
* Function:  SetFileMode(file, attributes)                          *
*                                                                   *
* Purpose:   Change file attribute bits                             *
*            without PM.                                            *
*                                                                   *
* RC:        0    -  File attributes successfully changed           *
*            1    -  Unable to change attributes                    *
*********************************************************************/

static int SetFileMode(char *file, unsigned long attr )
{
    int rc;
    FILESTATUS3   status;

    /* get the file status        */
    rc = DosQueryPathInfo(file, FIL_STANDARD, (char *)&status,
                           sizeof(status));

    if (!rc) {                                  /* worked?                    */
        status.attrFile = (unsigned short)attr; /* set new attributes         */
                                                /* set the file info          */
        rc = DosSetPathInfo(file, FIL_STANDARD, (void *)&status,
                             sizeof(FILESTATUS3), 0);
    }
    return rc;                             /* give back success flag     */
}


/*********************************************************************/
/* Function: ULONG FormatFile(                                       */
/*                                                                   */
/* Purpose:  Returns the new file attribute, given the mask of       */
/*           attributes to be cleared/set and the current attribute  */
/*           settings.                                               */
/*                                                                   */
/*********************************************************************/

unsigned long FormatFile(RXTREEDATA *ldp, int *smask, int *dmask,
                         unsigned long options, FILEFINDBUF3 *finfo)
{
    unsigned long nattrib;
    unsigned short rc;

    nattrib = NewAttr((INT *)dmask, finfo->attrFile);

    if (nattrib != finfo->attrFile)
        if (SetFileMode(ldp->truefile, nattrib&~FILE_DIRECTORY)) nattrib = finfo->attrFile;

    if (options&NAME_ONLY) strcpy(ldp->Temp, ldp->truefile);
    else {
        if (options&LONG_TIME)
            sprintf(ldp->Temp, "%4d-%02d-%02d %02d:%02d:%02d  %10lu  ",
                    finfo->fdateLastWrite.year+1980,
                    finfo->fdateLastWrite.month,
                    finfo->fdateLastWrite.day,
                    finfo->ftimeLastWrite.hours,
                    finfo->ftimeLastWrite.minutes,
                    2*finfo->ftimeLastWrite.twosecs,
                    finfo->cbFile);
        else {
            if (options&EDITABLE_TIME)
                sprintf(ldp->Temp, "%02d/%02d/%02d/%02d/%02d  %10lu  ",
                    (finfo->fdateLastWrite.year+80)%100,
                    finfo->fdateLastWrite.month,
                    finfo->fdateLastWrite.day,
                    finfo->ftimeLastWrite.hours,
                    finfo->ftimeLastWrite.minutes,
                    finfo->cbFile);
            else sprintf(ldp->Temp, "%2d/%02d/%02d  %2d:%02d%c  %10lu  ",
                        finfo->fdateLastWrite.month,
                        finfo->fdateLastWrite.day,
                        (finfo->fdateLastWrite.year+80)%100,
                        (finfo->ftimeLastWrite.hours < 13?
                        finfo->ftimeLastWrite.hours:
                        (finfo->ftimeLastWrite.hours-(SHORT)12)),
                        finfo->ftimeLastWrite.minutes,
                        ((finfo->ftimeLastWrite.hours < 12 ||
                        finfo->ftimeLastWrite.hours == 24)?'a':'p'),
                        finfo->cbFile);
        }     sprintf(ldp->Temp, "%s%c%c%c%c%c  %s", ldp->Temp,
        ((nattrib&FILE_ARCHIVED)?'A':'-'),
        ((nattrib&FILE_DIRECTORY)?'D':'-'),
        ((nattrib&FILE_HIDDEN)?'H':'-'),
        ((nattrib&FILE_READONLY)?'R':'-'),
        ((nattrib&FILE_SYSTEM)?'S':'-'),
        ldp->truefile);
    }

    ldp->vlen = strlen(ldp->Temp);
    ldp->count++;
    ltoa(ldp->count, ldp->varname+ldp->stemlen, 10);
    ldp->shvb.shvnext = NULL;
    ldp->shvb.shvname.strptr = ldp->varname;
    ldp->shvb.shvname.strlength = strlen(ldp->varname);
    ldp->shvb.shvvalue.strptr = ldp->Temp;
    ldp->shvb.shvvalue.strlength = ldp->vlen;
    ldp->shvb.shvnamelen = ldp->shvb.shvname.strlength;
    ldp->shvb.shvvaluelen = ldp->vlen;
    ldp->shvb.shvcode = RXSHV_SET;
    ldp->shvb.shvret = 0;
    rc = RexxVariablePool(&ldp->shvb);

    if (rc & (RXSHV_BADN | RXSHV_MEMFL)) return INVALID_ROUTINE;

    return 0;
}

