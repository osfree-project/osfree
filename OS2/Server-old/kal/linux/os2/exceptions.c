/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      exceptions.c                                                  */
/*                                                                            */
/* Description: This file includes the code to support exceptions.            */
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
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* OS/2 APIs for exceptions                                                   */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosSetExceptionHandler                                                     */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosUnsetExceptionHandler                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosUnsetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosRaiseException                                                          */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosRaiseException(PEXCEPTIONREPORTRECORD pexcept)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSendSignalException                                                     */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSendSignalException(PID pid, ULONG exception)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosUnwindException                                                         */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosUnwindException(PEXCEPTIONREGISTRATIONRECORD phandler,
                                   PVOID pTargetIP,
                                   PEXCEPTIONREPORTRECORD pERepRec)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetSignalExceptionFocus                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetSignalExceptionFocus(BOOL32 flag, PULONG pulTimes)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosEnterMustComplete                                                       */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosEnterMustComplete(PULONG pulNesting)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosExitMustComplete                                                        */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosExitMustComplete(PULONG pulNesting)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosAcknowledgeSignalException                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosAcknowledgeSignalException(ULONG ulSignalNum)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryThreadContext                                                      */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryThreadContext(TID tid, ULONG level, PCONTEXTRECORD pcxt)
{
    return ERROR_ACCESS_DENIED;
}

