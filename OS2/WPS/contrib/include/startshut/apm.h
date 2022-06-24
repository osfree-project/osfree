
/*
 *@@sourcefile apm.h:
 *      header file for apm.c.
 *      apm.c contains the XWorkplace APM and ACPI interfaces for
 *      automatically turning the computer off after
 *      shutdown has completed. This file declares
 *      everything necessary for this.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\apmh.h"
 *@@include #include "startshut\apm.h"
 */

/*
 *      Massive thanks go out to ARAKAWA Atsushi (arakaw@ibm.net)
 *      for filling this in, and to Roman Stangl (rstangl@vnet.ibm.com)
 *      for finding out all the APM stuff.
 *
 *      Copyright (C) 1997-2003 Ulrich M”ller, ARAKAWA Atsushi.
 *      Copyright (C) 2006-2007 Paul Ratcliffe.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef APM_HEADER_INCLUDED
    #define APM_HEADER_INCLUDED

    #define APM_UNKNOWN             0x00
    #define APM_OK                  0x01
    #define APM_IGNORE              0x02
    #define APM_CANCEL              0x04
    #define APM_DOSSHUTDOWN_0       0x08
    #define APM_DOSSHUTDOWN_1       0x10

    PSZ apmQueryVersion(VOID);

    BOOL apmPowerOffSupported(VOID);

    ULONG apmPreparePowerOff(PSZ pszError);

    VOID apmDoPowerOff(VOID);

    PSZ acpiQueryVersion(VOID);

    BOOL acpiPowerOffSupported(VOID);

    ULONG acpiPreparePowerOff(PSZ pszError);

    VOID acpiDoPowerOff(VOID);

#endif
