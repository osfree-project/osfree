/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      utility.c                                                     */
/*                                                                            */
/* Description: This file includes the utility functions to support the       */
/*              OS2Linux package.                                             */
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
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_DOSDATETIME
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* Utility functions                                                          */
/*    Note that all of these function have a "Dos" prefix in their name but   */
/*    they do NOT conflict with any OS/2 functions. None of these functions   */
/*    are a part of the original OS/2 API set. They support the OS2Linux      */
/*    library itself. The prototypes for these functions are in os2linux.h.   */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* Local definitions                                                          */
/*----------------------------------------------------------------------------*/

static CHAR ver[16];


/*============================================================================*/
/* Public APIs - these are available to the OS/2 application                  */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosGetLibVersion                                                           */
/*    Notes:                                                                  */
/*       - returns the version of the OS2Linux library                        */
/*----------------------------------------------------------------------------*/

VOID APIENTRY DosGetLibVersion(PSZ * pszVer)
{
    SAVEENV;

    sprintf(ver, "%s", VERSION);
     *pszVer = ver;
     RESTOREENV;
}


/*============================================================================*/
/* Private APIs - these are not available to the OS/2 application             */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosMakeArgv                                                                */
/*    Notes:                                                                  */
/*       - the memory returned by this function MUST be freed!                */
/*----------------------------------------------------------------------------*/

int APIENTRY DosMakeArgv(const char *s, const char *delimiters, char ***argvp)
{
    SAVEENV;
    int i;
    int numtokens;
    const char *snew;
    char *t;
    char *lasts;

    /* check args */
    if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
        errno = EINVAL;
        RESTOREENV_RETURN(-1);
    }
    *argvp = NULL;
    snew = s + strspn(s, delimiters); /* snew is real start of string */
    if ((t = malloc(strlen(snew) + 1)) == NULL) {
        RESTOREENV_RETURN(-1);
    }
    strcpy(t, snew);
    numtokens = 0;

    /* count the number of tokens in s */
    if (strtok_r(t, delimiters, &lasts) != NULL) {
       for (numtokens = 1; strtok_r(NULL, delimiters, &lasts) != NULL; numtokens++) ;
    }

    /* create argument array for ptrs to the tokens */
    if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
       free(t);
       RESTOREENV_RETURN(-1);
    }

    /* insert pointers to tokens into the argument array */
    if (numtokens == 0) {
       free(t);
    }
    else {
       strcpy(t, snew);
       **argvp = strtok_r(t, delimiters, &lasts);
       for (i = 1; i < numtokens; i++)
           *((*argvp) + i) = strtok_r(NULL, delimiters, &lasts);
     }

     /* put in final NULL pointer */
     *((*argvp) + numtokens) = NULL;
     RESTOREENV_RETURN(numtokens);
}


/*----------------------------------------------------------------------------*/
/* DosFreeMakeArgv                                                            */
/*    Notes:                                                                  */
/*       - free the memory returned by DosMakeArgv                            */
/*----------------------------------------------------------------------------*/

void APIENTRY DosFreeMakeArgv(char **argv)
{
    SAVEENV;

    if (argv == NULL) {
        return;
    }
    if (*argv != NULL) {
        free(*argv);
    }
    free(argv);

    RESTOREENV;
}


/*----------------------------------------------------------------------------*/
/* DosNameConversion                                                          */
/*    Notes:                                                                  */
/*       - substitute characters in a path/name                               */
/*       - the default action is to convert '\\' characters to '/' characters */
/*----------------------------------------------------------------------------*/

void APIENTRY DosNameConversion(PSZ pszName, PUCHAR pSeparator, PUCHAR pRepl,
                                BOOL uc)
{
    CHAR separator;
    CHAR replacement;
    PSZ tempName;

    if (pszName == NULL) {
        return;
    }

    /* set the delimiter to convert */
    if (pSeparator == NULL) {
        separator = '\\';
    }
    else {
        separator = *pSeparator;
    }

    /* set the replacement character */
    if (pRepl == NULL) {
        replacement = '/';
    }
    else {
        replacement = *pRepl;
    }

    /* now convert the characters */
    tempName = pszName;
    while (*tempName != '\0') {
        if (*tempName == separator) {
            *tempName = replacement;
        }
        tempName++;
    }

    /* covert the name to upper case if necessary */
    if (uc) {
        tempName = pszName;
        while (*tempName) {
            *tempName = (CHAR)toupper(*tempName);
            tempName++;
        }
    }

}


/*----------------------------------------------------------------------------*/
/* DosReqestSysMutexSem                                                       */
/*    Notes:                                                                  */
/*       - request the system-wide mutex semaphore                            */
/*----------------------------------------------------------------------------*/

VOID APIENTRY DosRequestSysMutexSem(void)
{
    int rc;

    while ((rc = sem_wait(DosLinuxStruct.SysMutexSem)) != 0) {
        if (errno == EINTR) {
            continue;
        }
        else if (errno) {
            DosLogError("DosRequestSysMutexSem", "sem_wait", errno);
            exit(12);
        }
    }
    return;
}


/*----------------------------------------------------------------------------*/
/* DosReleaseSysMutexSem                                                      */
/*    Notes:                                                                  */
/*       - release the system-wide mutex semaphore                            */
/*----------------------------------------------------------------------------*/

VOID APIENTRY DosReleaseSysMutexSem(void)
{
    sem_post(DosLinuxStruct.SysMutexSem);
    return;
}


/*----------------------------------------------------------------------------*/
/* DosLogError                                                                */
/*    Notes:                                                                  */
/*       - put a message in the Linux system log.                             */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosLogError(PSZ pszOS2FuncName, PSZ pszCFuncName, int errnum)
{
    SAVEENV;

    syslog(LOG_USER | LOG_ERR, "OS2Linux: %s in %s returned errno code %d.\n",
           pszCFuncName, pszOS2FuncName, errnum);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosFtok                                                                    */
/*    Notes:                                                                  */
/*       - custom OS2Linux ftok function                                      */
/*----------------------------------------------------------------------------*/

key_t APIENTRY DosFtok(PSZ pszName)
{
    SAVEENV;
    key_t mykey = -1; /* default is an error indicator */
    PSZ fsName;
    static char tempdir[] = "/tmp";
    struct stat statbuf;
    int rc;

    /* append Linux temporary directory to the front of the name */
    fsName = alloca(strlen(tempdir) + strlen(pszName) + 1);
    strcpy(fsName, tempdir);
    strcat(fsName, pszName);

    /* make sure the file exists */
    rc = stat(fsName, &statbuf);
    if (rc) {
        switch (errno) {
        case ENOENT:
        case ENOTDIR:
            /* file not found, so create it */
            rc = open(fsName, O_CREAT | O_EXCL, S_IRWXU | S_IRWXG);
            if (rc == -1) {
                RESTOREENV_RETURN(mykey);
            }
            close(rc);
            break;
        default:
            RESTOREENV_RETURN(mykey);
        }
    }

    /* now that we know the file exists we can ftok it */
    mykey = ftok(fsName, 'O');

    RESTOREENV_RETURN(mykey);
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_0                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 0                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_0(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[0] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[0]->hSem);
    if (DosLinuxStruct.timerslot[0]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[0]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_1                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 1                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_1(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[1] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[1]->hSem);
    if (DosLinuxStruct.timerslot[1]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[1]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_2                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 2                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_2(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[2] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[2]->hSem);
    if (DosLinuxStruct.timerslot[2]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[2]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_3                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 3                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_3(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[3] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[3]->hSem);
    if (DosLinuxStruct.timerslot[3]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[3]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_4                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 4                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_4(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[4] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[4]->hSem);
    if (DosLinuxStruct.timerslot[4]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[4]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_5                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 5                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_5(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[5] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[5]->hSem);
    if (DosLinuxStruct.timerslot[5]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[5]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_6                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 6                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_6(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[6] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[6]->hSem);
    if (DosLinuxStruct.timerslot[6]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[6]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_7                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 7                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_7(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[7] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[7]->hSem);
    if (DosLinuxStruct.timerslot[7]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[7]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_8                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 8                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_8(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[8] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[8]->hSem);
    if (DosLinuxStruct.timerslot[8]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[8]);
    }

    RESTOREENV;
    return;
}


/*----------------------------------------------------------------------------*/
/* DosAsyncTimerHandler_9                                                     */
/*    Notes:                                                                  */
/*       - handles timer slot 9                                               */
/*----------------------------------------------------------------------------*/

void DosAsyncTimerHandler_9(int signo, siginfo_t *info, void *context)
{
    SAVEENV;

    /* check that the timer actually expired */
    if (info->si_code != SI_TIMER) {
        return;
    }

    /* check that the timerslot is valid */
    if (DosLinuxStruct.timerslot[9] == NULL) {
        return;
    }

    DosPostEventSem(DosLinuxStruct.timerslot[9]->hSem);
    if (DosLinuxStruct.timerslot[9]->oneshot) {
        DosStopTimer((HTIMER)DosLinuxStruct.timerslot[9]);
    }

    RESTOREENV;
    return;
}

