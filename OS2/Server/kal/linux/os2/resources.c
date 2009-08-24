/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      resources.c                                                   */
/*                                                                            */
/* Description: This file includes the code to resources.                     */
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
#define INCL_DOSRESOURCES
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* OS/2 APIs for resources                                                    */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosGetResource                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported by user mode Linux and always       */
/*         returns ERROR_ACCESS_DENIED                                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosGetResource(HMODULE hmod, ULONG idType, ULONG idName,
                               PPVOID ppb)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosFreeResource                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported by user mode Linux and always       */
/*         returns ERROR_ACCESS_DENIED                                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFreeResource(PVOID pb)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryResourceSize                                                       */
/*    Notes:                                                                  */
/*       - this function is not supported by user mode Linux and always       */
/*         returns ERROR_ACCESS_DENIED                                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryResourceSize(HMODULE hmod, ULONG idt, ULONG idn,
                                     PULONG pulsize)
{
    return ERROR_ACCESS_DENIED;
}


