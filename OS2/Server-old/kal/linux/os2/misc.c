/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      misc.c                                                        */
/*                                                                            */
/* Description: This file includes the code to support miscellaneous funcs.   */
/*                                                                            */
/* Copyright (C) IBM Corporation 2003. All Rights Reserved.                   */
/* Copyright (C) W. David Ashley 2004, 2005. All Rights Reserved.             */
/*                                                                            */
/* Author(s):                                                                 */
/*      W. David Ashley  <dashley@us.ibm.com>                                 */
/*                                                                            */
/* This software is subject to the terms of the Common Public License v1.0.   */
/* You must accept the terms of this license to use this software.            */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* #ifdef HAVE_CONFIG_H

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* OS/2 APIs for miscellaneous                                                */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosGetMessage                                                              */
/*    Notes:                                                                  */
/*       - the pszFile must have been generated using the Linux gencat        */
/*         utility                                                            */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosGetMessage(PCHAR* pTable, ULONG cTable, PCHAR pBuf,
                              ULONG cbBuf, ULONG msgnumber, PCSZ pszFile,
                              PULONG pcbMsg)
{
    SAVEENV;
    nl_catd catDescr;
    char * catPtr;
    char * subPtr;
    char escape[3];
    int i;
    PSZ pszTmpMsg, pszNewName;
    ULONG length;

    /* check args */
    if (cTable > 9) {
        RESTOREENV_RETURN(ERROR_MR_INV_IVCOUNT);
    }

    /* convert the file name to Linux format */
    pszNewName = alloca(strlen(pszFile) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_OPEN_FAILED);
    }
    strcpy(pszNewName, pszFile);
    DosNameConversion(pszNewName, "\\", "/", FALSE);

    /* open the catalog */
    catDescr = catopen(pszNewName, NL_CAT_LOCALE);
    if (catDescr == (nl_catd)-1) {
        RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
    }

    /* get the message */
    catPtr = catgets(catDescr, 1, (int)msgnumber,
                     "No text available for message number.");
    if (strlen(catPtr) + 1 > cbBuf) {
        RESTOREENV_RETURN(ERROR_MR_MSG_TOO_LONG);
    }

    /* allocate temp storage for the message */
    pszTmpMsg = alloca(cbBuf);
    if (pszTmpMsg == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }

    /* copy the message */
    strcpy(pBuf, catPtr);

    /* close the catalog */
    catclose(catDescr);

    /* now substitute all the strings */
    if (cTable > 0) {
        for (i = 0; i < cTable; i++) {
            strcpy(pszTmpMsg, pBuf);
            sprintf(escape, "%%%d", i + 1);
            while ((subPtr = strstr(pszTmpMsg, escape)) != NULL) {
                if (strlen(pBuf) + strlen(pTable[i]) - 1 > cbBuf) {
                    RESTOREENV_RETURN(ERROR_MR_MSG_TOO_LONG);
                }
                length = (ULONG)subPtr - (ULONG)pszTmpMsg;
                strncpy(pBuf, pszTmpMsg, length);
                strcpy(pBuf + length, pTable[i]);
                strcat(pBuf, subPtr + strlen(escape));
            }
        }
    }

    /* now fix up everthing */
    *pcbMsg = strlen(pBuf);
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosErrClass                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosErrClass(ULONG code, PULONG pClass, PULONG pAction,
                            PULONG pLocus)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosInsertMessage                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosInsertMessage(PCHAR* pTable, ULONG cTable, PCSZ pszMsg,
                                 ULONG cbMsg, PCHAR pBuf, ULONG cbBuf,
                                 PULONG pcbMsg)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosPutMessage                                                              */
/*    Notes:                                                                  */
/*       - this function only outputs messages to stdout, stderr and the      */
/*         Linux system log.                                                  */
/*       - the message is NOT formated as per the OS/2 specification.         */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosPutMessage(HFILE hfile, ULONG cbMsg, PCHAR pBuf)
{
    SAVEENV;
    char *newBuf = NULL;

    newBuf = alloca(strlen(pBuf) + 2);
    strcpy(newBuf, pBuf);
    strcat(newBuf, "\n");
    switch (hfile) {
    case 1:
        fputs(newBuf, stdout);
        break;
    case 2:
        fputs(newBuf, stderr);
        break;
    default:
        syslog(LOG_USER | LOG_INFO, newBuf);
        break;
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQuerySysInfo                                                            */
/*    Notes:                                                                  */
/*       - this function returns appropriate values for Linux.                */
/*       - some values are returned with static values.                       */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQuerySysInfo(ULONG iStart, ULONG iLast, PVOID pBuf,
                                ULONG cbBuf)
{
    SAVEENV;
    ULONG i;
    PULONG sysval;

    /* check argument */
    if ((iLast - iStart + 1) * sizeof(ULONG) < cbBuf) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }


    sysval = (PULONG)pBuf;
    for (i = iStart; i <= iLast; i++) {
        switch (i) {
        case QSV_MAX_PATH_LENGTH:
            *sysval = (ULONG)pathconf(".", _PC_PATH_MAX);
            sysval++;
            break;
        case QSV_MAX_TEXT_SESSIONS:
            *sysval = 1024;
            sysval++;
            break;
        case QSV_MAX_PM_SESSIONS:
            *sysval = 0;
            sysval++;
            break;
        case QSV_MAX_VDM_SESSIONS:
            *sysval = 0;
            sysval++;
            break;
        case QSV_BOOT_DRIVE:            /* 1=A, 2=B, etc.                     */
            *sysval = 3;
            sysval++;
            break;
        case QSV_DYN_PRI_VARIATION:     /* 0=Absolute, 1=Dynamic              */
            *sysval = 1;
            sysval++;
            break;
        case QSV_MAX_WAIT:              /* seconds                            */
            *sysval = 1; /* this value is unknown */
            sysval++;
            break;
        case QSV_MIN_SLICE:             /* milli seconds                      */
            *sysval = 10; /* POSIX claims this is 10ms */
            sysval++;
            break;
        case QSV_MAX_SLICE:             /* milli seconds                      */
            *sysval = 10; /* this value is unknown */
            sysval++;
            break;
        case QSV_PAGE_SIZE:
            *sysval = (ULONG)sysconf(_SC_PAGESIZE);
            sysval++;
            break;
        case QSV_VERSION_MAJOR:
            *sysval = 100;
            sysval++;
            break;
        case QSV_VERSION_MINOR:
            *sysval = 0;
            sysval++;
            break;
        case QSV_VERSION_REVISION:      /* Revision letter                    */
            *sysval = 0;
            sysval++;
            break;
        case QSV_MS_COUNT:              /* Free running millisecond counter   */
            *sysval = 0; /* fix this! */
            sysval++;
            break;
        case QSV_TIME_LOW:              /* Low dword of time in seconds       */
            *sysval = (ULONG)time(NULL);;
            sysval++;
            break;
        case QSV_TIME_HIGH:             /* High dword of time in seconds      */
            *sysval = 0;
            sysval++;
            break;
        case QSV_TOTPHYSMEM:            /* Physical memory on system          */
        case QSV_TOTRESMEM:             /* Resident memory on system          */
        case QSV_TOTAVAILMEM:           /* Available memory for all processes */
        case QSV_MAXPRMEM:              /* Avail private mem for calling proc */
        case QSV_MAXSHMEM:              /* Avail shared mem for calling proc  */
            *sysval = (ULONG)(sysconf(_SC_PAGESIZE) * sysconf(_SC_PHYS_PAGES));
            sysval++;
            break;
        case QSV_TIMER_INTERVAL:        /* Timer interval in tenths of ms     */
            *sysval = 100; /* POSIX claims this is 10ms */
            sysval++;
            break;
        case QSV_MAX_COMP_LENGTH:       /* max len of one component in a name */
            *sysval = (ULONG)pathconf(".", _PC_NAME_MAX);;
            sysval++;
            break;
        case QSV_FOREGROUND_FS_SESSION: /* Session ID of current fgnd FS session*/
            *sysval = 0;
            sysval++;
            break;
        case QSV_FOREGROUND_PROCESS:    /* Process ID of current fgnd process */
            *sysval = 0;
            sysval++;
            break;
        default:
            RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosScanEnv                                                                 */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosScanEnv(PCSZ pszName, PSZ *ppszValue)
{
    SAVEENV;

    *ppszValue = getenv(pszName);
    if (*ppszValue == NULL) {
        RESTOREENV_RETURN(ERROR_ENVVAR_NOT_FOUND);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSearchPath                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSearchPath(ULONG flag, PCSZ pszPathOrName, PCSZ pszFilename,
                              PBYTE pBuf, ULONG cbBuf)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryMessageCP                                                          */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryMessageCP(PCHAR pb, ULONG cb, PCSZ pszFilename,
                                  PULONG cbBuf)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryRASInfo                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryRASInfo(ULONG Index, PPVOID Addr)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetExtLIBPATH                                                           */
/*    Notes:                                                                  */
/*       - this function modifies the Linux LD_LIBRARY_PATH                   */
/*       - the flags argument is ignored                                      */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetExtLIBPATH( PCSZ pszExtLIBPATH, ULONG flags)
{
    SAVEENV;
    int rc;
    static char * libstr = "LD_LIBRARY_PATH";
    char * newPath;

    if (pszExtLIBPATH == NULL) {
        rc = putenv(libstr);
    }
    else {
        /* Note! This is a memory leak, but it cannot be helped. If the var */
        /* newPath is in automatic storage it will disappear from the env!  */
        newPath = malloc(strlen(libstr) + strlen(pszExtLIBPATH) + 2);
        strcpy(newPath, libstr);
        strcat(newPath, "=");
        strcat(newPath, pszExtLIBPATH);
        rc = putenv(newPath);
    }
    if (rc == -1) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryExtLIBPATH                                                         */
/*    Notes:                                                                  */
/*       - this function returns the Linux LD_LIBRARY_PATH                    */
/*       - the flags argument is ignored                                      */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryExtLIBPATH( PCSZ pszExtLIBPATH, ULONG flags)
{
    SAVEENV;
    char * libpath;

    libpath = getenv("LD_LIBRARY_PATH");
    if (libpath) {
        strcpy((PSZ)pszExtLIBPATH, libpath);
        RESTOREENV_RETURN(NO_ERROR);
    }
    else {
        pszExtLIBPATH = NULL;
        RESTOREENV_RETURN(ERROR_ENVVAR_NOT_FOUND);
    }
}

