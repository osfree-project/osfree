/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      os2.c                                                         */
/*                                                                            */
/* Description: Main source file for the OS.2 Linux module. This file         */
/*              includes the module initialization and cleanup functions.     */
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
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"

#include <l4/sys/syscalls.h>
#include <l4/crtx/ctor.h>
// l4con
#include <l4/l4con/l4contxt.h>
// dope
//#include "dopestd.h"
//#include <dopelib.h>
//#include <vscreen.h>

/*----------------------------------------------------------------------------*/
/* Global variables                                                           */
/*----------------------------------------------------------------------------*/

/* this is as good a place as any to put this */
DOSLINUXSTRUCT DosLinuxStruct;

/*----------------------------------------------------------------------------*/
/* Local Definitions                                                          */
/*----------------------------------------------------------------------------*/

#define SYSMUTEXSEMNAME "/OS2LINUX_MUTEXSEM_SYSLIB"

// _init(void) __attribute__((constructor));
//int _fini(void) __attribute__((destructor));


/*----------------------------------------------------------------------------*/
/* _init - Module initialization routine                                      */
/*----------------------------------------------------------------------------*/
static void __attribute__((constructor))
_init(void)
{
    int rc;

    if (!contxt_init(4096, 1000)) 
    {
    };

    printf("Test of console initialization\n");
#if 0
    /* initialize the DosLinuxStructure to zero */
    memset(&DosLinuxStruct, 0, sizeof(DOSLINUXSTRUCT));

    /* OS/2 processes support */
    DosLinuxStruct.pib.pib_ulpid = getpid();
    DosLinuxStruct.pib.pib_ulppid = getppid();
    DosLinuxStruct.pib.pib_hmte = 0;
    DosLinuxStruct.pib.pib_pchcmd = NULL;
    DosLinuxStruct.pib.pib_pchenv = *__environ;
    DosLinuxStruct.pib.pib_flstatus = 0;
    DosLinuxStruct.pib.pib_ultype = 2; /* ML_WINDOW_PROC_TYPE */

    /* thread specific memory support */
//    rc = pthread_key_create(&DosLinuxStruct.thrdKey, NULL);
//    if (rc) {
//        syslog(LOG_USER | LOG_ERR,
//               "OS2Linux: Error creating thread specific memory key.\n");
//        exit(12);
//    }

    /* thread specific TIB support */
//    rc = pthread_key_create(&DosLinuxStruct.thrdTIBKey, NULL);
//    if (rc) {
//        syslog(LOG_USER | LOG_ERR,
//               "OS2Linux: Error creating thread specific TIB key.\n");
//        exit(12);
//    }

    /* OS/2 process critical section mutex semaphore */
//    rc = pthread_mutex_init(&DosLinuxStruct.critSecMutex, NULL );
//    if (rc) {
//        syslog(LOG_USER | LOG_ERR,
//               "OS2Linux: Error creating thread critical section mutex semaphore.\n"
//               "OS2Linux: The errno was %d.\n", errno);
//        exit(12);
//    }

    /* system-wide mutex semaphore for this library */
    /* do NOT use the OS/2 APIs here! */
//    DosLinuxStruct.SysMutexSem = sem_open(SYSMUTEXSEMNAME, (O_CREAT | O_EXCL),
//                                          (S_IRWXU | S_IRWXG), 0);
//    if (DosLinuxStruct.SysMutexSem == SEM_FAILED ) {
//        if (errno == EEXIST) {
//            DosLinuxStruct.SysMutexSem = sem_open(SYSMUTEXSEMNAME, 0);;
//            if (DosLinuxStruct.SysMutexSem == SEM_FAILED ) {
//                syslog(LOG_USER | LOG_ERR,
//                       "OS2Linux: Error opening the system-wide mutex semaphore.\n"
//                       "OS2Linux: The errno was %d.\n", errno);
//                exit(12);
//            }
//        }
//        else {
//            syslog(LOG_USER | LOG_ERR,
//                   "OS2Linux: Error creating the system-wide mutex semaphore.\n"
//                   "OS2Linux: The errno was %d.\n", errno);
//            exit(12);
//        }
//    }
//    sem_post(DosLinuxStruct.SysMutexSem);

    /* termination code to use for this program */
    DosLinuxStruct.termcode = TC_EXIT; /* fix this! */

    /* support for an array of OS/2 asyncronous timers */
    DosLinuxStruct.asyncTimerHandler[0] = DosAsyncTimerHandler_0;
    DosLinuxStruct.asyncTimerHandler[1] = DosAsyncTimerHandler_1;
    DosLinuxStruct.asyncTimerHandler[2] = DosAsyncTimerHandler_2;
    DosLinuxStruct.asyncTimerHandler[3] = DosAsyncTimerHandler_3;
    DosLinuxStruct.asyncTimerHandler[4] = DosAsyncTimerHandler_4;
    DosLinuxStruct.asyncTimerHandler[5] = DosAsyncTimerHandler_5;
    DosLinuxStruct.asyncTimerHandler[6] = DosAsyncTimerHandler_6;
    DosLinuxStruct.asyncTimerHandler[7] = DosAsyncTimerHandler_7;
    DosLinuxStruct.asyncTimerHandler[8] = DosAsyncTimerHandler_8;
    DosLinuxStruct.asyncTimerHandler[9] = DosAsyncTimerHandler_9;
#endif
    /* establish the program errno for the client program */
    errno = 0;
}

#if 0
/*----------------------------------------------------------------------------*/
/* _fini - Module finialization routine                                       */
/*----------------------------------------------------------------------------*/

int _fini(void)
{
    int i, j;

    /* process the DosExitList of functions in LIFO order */
    for (i = 255; i >= 0; i--) { /* by ordercode */
        for (j = MAX_EXIT_LIST - 1; j >= 0; j--) { /* by list entry */
            if (DosLinuxStruct.exitList[j].ordercode == i &&
             DosLinuxStruct.exitList[j].func != NULL) {
                /* execute the function */
                (DosLinuxStruct.exitList[j].func)(DosLinuxStruct.termcode);
            }
        }
    }
    VioDeInitCurses();

    /* DO THIS LAST! */
    sem_close(DosLinuxStruct.SysMutexSem);

    return 0;
}

#endif