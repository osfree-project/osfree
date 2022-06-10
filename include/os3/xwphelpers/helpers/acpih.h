
/*
 *@@sourcefile acpih.h:
 *      header file for acpih.c.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers/acpih.h"
 */

/*      Copyright (C) 2006-2014 Paul Ratcliffe.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

// @@changed V1.0.9 (2012-02-20) [slevine]: sync with current ACPI toolkit, code by David Azarewicz

#ifndef ACPIH_HEADER_INCLUDED
    #define ACPIH_HEADER_INCLUDED

    #pragma pack(4)
    #include <acpi.h>
    #include <acpiapi.h>
    #pragma pack()
    /*
     * Power state values
     */

    // Extracted from acpi.h actype.h etc.
    typedef APIRET APIENTRY ACPISTARTAPI(ACPI_API_HANDLE *);
    typedef ACPISTARTAPI *PACPISTARTAPI;

    typedef APIRET APIENTRY ACPIENDAPI(ACPI_API_HANDLE *);
    typedef ACPIENDAPI *PACPIENDAPI;

    typedef APIRET APIENTRY ACPIGOTOSLEEP(ACPI_API_HANDLE *, UCHAR);
    typedef ACPIGOTOSLEEP *PACPIGOTOSLEEP;

    // @@added V1.0.9 (2012-12-10) [slevine]: sync with current ACPI toolkit
    typedef ACPI_STATUS APIENTRY ACPITKPREPARETOSLEEP(UINT8);
    typedef ACPITKPREPARETOSLEEP *PACPITKPREPARETOSLEEP;

    //@changed V1.0.10 (2014-08-30) [dazarewicz]: change prototype
    typedef APIRET APIENTRY ACPITKGETOBJECTINFOALLOC(ACPI_HANDLE, ACPI_DEVICE_INFO **);
    typedef APIRET APIENTRY ACPITKGETHANDLE(ACPI_HANDLE, ACPI_STRING, ACPI_HANDLE *);
    typedef APIRET APIENTRY ACPITKOSFREE(PVOID);
    typedef APIRET APIENTRY ACPITKWALKNAMESPACE(ACPI_OBJECT_TYPE, ACPI_HANDLE, UINT32,ACPI_WALK_CALLBACK, PVOID, void **);
    typedef APIRET APIENTRY ACPITKEVALUATEOBJECT(ACPI_HANDLE, ACPI_STRING, ACPI_OBJECT_LIST *, ACPI_BUFFER *);

    APIRET APIENTRY acpihOpen(ACPI_API_HANDLE *phACPI);
    typedef APIRET APIENTRY ACPIHOPEN(ACPI_API_HANDLE *);
    typedef ACPIHOPEN *PACPIHOPEN;

    VOID APIENTRY acpihClose(ACPI_API_HANDLE *phACPI);

    //@added V1.0.10 (2014-08-30) [dazarewicz]
    VOID acpihPrepareToSleep(UCHAR ucState);
    APIRET APIENTRY acpihGoToSleep(ACPI_API_HANDLE *phACPI, UCHAR ucState);

    APIRET APIENTRY acpihGetPowerStatus(PAPM, PBOOL);
    typedef APIRET APIENTRY ACPIHGETPOWERSTATUS(PAPM, PBOOL);
    typedef ACPIHGETPOWERSTATUS *PACPIHGETPOWERSTATUS;

    BOOL acpihHasBattery(VOID);

    #define ORD_ACPISTARTAPI    16
    #define ORD_ACPIENDAPI      17
    #define ORD_ACPIGOTOSLEEP   19
    #define ORD_ACPITKGETOBJECTINFOALLOC 85
    #define ORD_ACPITKGETHANDLE 65
    #define ORD_ACPITKOSFREE 66
    #define ORD_ACPITKWALKNAMESPACE 56
    #define ORD_ACPITKEVALUATEOBJECT 50
    // @@added V1.0.9 (2012-12-10) [slevine]: sync with current ACPI toolkit
    #define ORD_ACPITKPREPARETOSLEEP 89
#endif

#if __cplusplus
}
#endif

