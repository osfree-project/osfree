/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      modulemgr.c                                                   */
/*                                                                            */
/* Description: This file includes the code to support modules.               */
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
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* OS/2 APIs for modules                                                      */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosLoadModule                                                              */
/*    Notes:                                                                  */
/*       - only a limited number of error codes are returned                  */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosLoadModule(PSZ pszName, ULONG cbName, PCSZ pszModname,
                              PHMODULE phmod)
{
    SAVEENV;

    *phmod = (HMODULE)dlopen(pszModname, RTLD_NOW | RTLD_GLOBAL);
    if (*phmod == 0) {
        if (pszName != NULL) {
            strncpy(pszName, dlerror(), cbName - 1);
        }
        RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosFreeModule                                                              */
/*    Notes:                                                                  */
/*       - only a limited number of error codes are returned                  */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFreeModule(HMODULE hmod)
{
    SAVEENV;
    int rc;

    rc = dlclose((void *)hmod);
    if (rc) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryProcAddr                                                           */
/*    Notes:                                                                  */
/*       - ordinal is ignored                                                 */
/*       - pszName must be non-null                                           */
/*       - only a limited number of error codes are returned                  */
/*       - external symbols (not just procedure names) are returned           */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryProcAddr(HMODULE hmod, ULONG ordinal, PCSZ pszName,
                                 PFN* ppfn)
{
    SAVEENV;
    char * rc;

    /* check param */
    if (pszName == NULL) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    *ppfn = dlsym((void *)hmod, pszName);
    rc = dlerror();
    if (rc) {
        RESTOREENV_RETURN(ERROR_INVALID_NAME);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryModuleHandle                                                       */
/*    Notes:                                                                  */
/*       - unsupported                                                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryModuleHandle(PCSZ pszModname, PHMODULE phmod)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryModuleName                                                         */
/*    Notes:                                                                  */
/*       - unsupported                                                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryModuleName(HMODULE hmod, ULONG cbName, PCHAR pch)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryProcType                                                           */
/*    Notes:                                                                  */
/*       - see notes for DosQueryProcAddr function                            */
/*       - always claims a procedure is 32 bit                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryProcType(HMODULE hmod, ULONG ordinal, PCSZ pszName,
                                 PULONG pulproctype)
{
    SAVEENV;
    APIRET rc;
    PFN pfn;

    rc = DosQueryProcAddr(hmod, ordinal, pszName, &pfn);
    if (rc == NO_ERROR) {
        *pulproctype = PT_32BIT;
    }

    RESTOREENV_RETURN(rc);
}

