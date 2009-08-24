/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      memmgr.c                                                      */
/*                                                                            */
/* Description: This file includes the code to the OS/2 memory manager.       */
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
#define INCL_DOSMEMMGR
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* OS/2 APIs for memory manager                                               */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosAllocMem                                                                */
/*    Notes:                                                                  */
/*       - flag is completely ignored                                         */
/*       - the system memory page size might not be 4096                      */
/*       - the memory returned is allocated using the malloc function         */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosAllocMem(PPVOID ppb, ULONG cb, ULONG flag)
{
    SAVEENV;
    ULONG pgsize = (ULONG)sysconf(_SC_PAGESIZE);

    if (cb % pgsize) {
        cb = ((cb / pgsize) + 1) * pgsize;
    }
    *ppb = malloc(cb);
    if (*ppb == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosFreeMem                                                                 */
/*    Notes:                                                                  */
/*       - the specified memory is freed using the free function              */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFreeMem(PVOID pb)
{
    SAVEENV;

    free(pb);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSetMem                                                                  */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetMem(CPVOID pb, ULONG cb, ULONG flag)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosGiveSharedMem                                                           */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosGiveSharedMem(CPVOID pb, PID pid, ULONG flag)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosGetSharedMem                                                            */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosGetSharedMem(CPVOID pb, ULONG flag)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosGetNamedSharedMem                                                       */
/*    Notes:                                                                  */
/*       - if flag PAG_WRITE is not set then the memory will be READ ONLY,    */
/*         otherwise the memory will be set to read/write                     */
/*       - pszName must be non-NULL                                           */
/*       - the system memory page size might not be 4096                      */
/*       - the memory returned is allocated using the shmget and shmat        */
/*         functions                                                          */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosGetNamedSharedMem(PPVOID ppb, PCSZ pszName, ULONG flag)
{
    SAVEENV;
    key_t mykey = 0;
    PSZ pszNewName;
    int rc;
    void * shmaddr;
    int shmat_flag = 0;

    if (pszName == NULL) {
        RESTOREENV_RETURN(ERROR_INVALID_NAME);
    }

    /* convert the name to Linux format */
    pszNewName = alloca(strlen(pszName) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszNewName, pszName);
    DosNameConversion(pszNewName, "\\", ".", TRUE);
    if (*pszNewName == '.') {
        *pszNewName = '/';
    }

    /* get a key for the shared memory */
    mykey = DosFtok(pszNewName);
    if (mykey == -1) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* get the shared memory id */
    rc = shmget(mykey, 0, S_IRWXU | S_IRWXG);
    if (rc == -1) {
        switch (errno) {
        case EEXIST:
            RESTOREENV_RETURN(ERROR_ALREADY_EXISTS);
        case ENOENT:
            RESTOREENV_RETURN(ERROR_INVALID_NAME);
        default:
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    /* attach memory and set the address of it */
    if (!(flag & PAG_WRITE)) {
        shmat_flag = SHM_RDONLY;
    }
    shmaddr = shmat(rc, NULL, shmat_flag);
    if (shmaddr == (void *)-1) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    *ppb = shmaddr;

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosAllocSharedMem                                                          */
/*    Notes:                                                                  */
/*       - if flag PAG_WRITE is not set then the memory will be READ ONLY,    */
/*         otherwise the memory will be set to read/write                     */
/*       - pszName must be non-NULL                                           */
/*       - the system memory page size might not be 4096                      */
/*       - the memory returned is allocated using the shmget and shmat        */
/*         functions                                                          */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosAllocSharedMem(PPVOID ppb, PCSZ pszName, ULONG cb,
                                  ULONG flag)
{
    SAVEENV;
    key_t mykey = 0;
    PSZ pszNewName;
    int rc;
    void * shmaddr;
    int shmat_flag = 0;

    if (pszName == NULL) {
        RESTOREENV_RETURN(ERROR_INVALID_NAME);
    }

    /* convert the name to Linux format */
    pszNewName = alloca(strlen(pszName) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszNewName, pszName);
    DosNameConversion(pszNewName, "\\", ".", TRUE);
    if (*pszNewName == '.') {
        *pszNewName = '/';
    }

    /* get a key for the shared memory */
    mykey = DosFtok(pszNewName);
    if (mykey == -1) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* get the shared memory id */
    rc = shmget(mykey, cb, IPC_CREAT | IPC_EXCL | S_IRWXU | S_IRWXG);
    if (rc == -1) {
        switch (errno) {
        case EEXIST:
            RESTOREENV_RETURN(ERROR_ALREADY_EXISTS);
        case ENOENT:
            RESTOREENV_RETURN(ERROR_INVALID_NAME);
        default:
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    /* attach memory and set the address of it */
    if (!(flag & PAG_WRITE)) {
        shmat_flag = SHM_RDONLY;
    }
    shmaddr = shmat(rc, NULL, shmat_flag);
    if (shmaddr == (void *)-1) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    *ppb = shmaddr;

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryMem                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryMem(CPVOID pb, PULONG pcb, PULONG pFlag)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSubAllocMem                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSubAllocMem(PVOID pbBase, PPVOID ppb, ULONG cb)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSubFreeMem                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSubFreeMem(PVOID pbBase, PVOID pb, ULONG cb)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSubSetMem                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSubSetMem(PVOID pbBase, ULONG flag, ULONG cb)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSubUnsetMem                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSubUnsetMem(PVOID pbBase)
{
    return ERROR_ACCESS_DENIED;
}

