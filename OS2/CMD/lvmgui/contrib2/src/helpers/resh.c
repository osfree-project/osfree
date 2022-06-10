
/*
 *@@sourcefile resh.c:
 *      resh.c contains Resource helper functions.
 *
 *      Function prefixes:
 *      --  resh*   Resource helper functions
 *
 *      These functions are forward-declared in resh.h, which
 *      must be #include'd first.
 *
 *@@header "helpers\resh.h"
 *@@added V0.9.4 (2000-07-27) [umoeller]
 */

/*
 *      Copyright (C) 2000 Christoph Schulte M”nting.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *      GNU General Public License for more details.
 */

// OS2 includes

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSMODULEMGR
#define INCL_DOSRESOURCES
#define INCL_DOSERRORS
#include <os2.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/resh.h"

/*
 *@@category: Helpers\Control program helpers\Resources
 */

/*
 *@@ reshWriteResourceToFile:
 *      get the indicated resource from the current module (usually the
 *      executable) and write it to a file. If pulBytesWritten is not NULL,
 *      the number of written bytes is stored to that location.
 *
 *      Returns:
 *      -- NO_ERROR: no error, resource successfully read and written.
 *      -- ERROR_INVALID_DATA: no. of bytes written != no. of bytes in
 *                             resource. Shouldn't happen.
 *
 *      plus the error codes from DosQueryResourceSize, DosOpen,
 *      DosGetResource, DosWrite.
 *
 *      <B>Warning:</B> If the given file exists, it will be
 *      overwritten without asking for confirmation!
 *
 *@@added V0.9.4 [csm]
 *@@changed V0.9.4 (2000-07-24) [umoeller]: added hmod, changed return values
 */

APIRET reshWriteResourceToFile(HMODULE hmod,        // in: module handle or NULLHANDLE for current EXE
                               ULONG  ulTypeID,        // in:  type of the resource
                               ULONG  ulNameID,        // in:  ID of the resource
                               const char *pcszFilename, // in:  name of file to write to
                               PULONG pulBytesWritten) // out: number of bytes written
{
    ULONG   arc = 0;
    ULONG ulResourceSize;
    ULONG ulBytesWritten = 0;

    arc = DosQueryResourceSize(hmod,
                               ulTypeID,
                               ulNameID,
                               &ulResourceSize);
    if (arc == NO_ERROR)
    {
        PVOID pvResourceData;

        HFILE hFile;
        ULONG ulAction = 0;

        // Open file for writing, replace if exists
        arc = DosOpen((PSZ)pcszFilename,
                      &hFile,
                      &ulAction,                           // action taken
                      ulResourceSize,                      // primary allocation size
                      FILE_ARCHIVED | FILE_NORMAL,         // file attribute
                      OPEN_ACTION_CREATE_IF_NEW
                        | OPEN_ACTION_REPLACE_IF_EXISTS,   // open flags
                      OPEN_FLAGS_NOINHERIT
                        | OPEN_FLAGS_SEQUENTIAL            // sequential, not random access
                        | OPEN_SHARE_DENYREADWRITE         // deny r/w share
                        | OPEN_ACCESS_WRITEONLY,           // write mode
                      NULL);                               // no EAs

        // If successful: get resource, write it and close file
        if (arc == NO_ERROR)
        {
            arc = DosGetResource(hmod,
                                 ulTypeID,
                                 ulNameID,
                                 &pvResourceData);
            if (arc == NO_ERROR)
            {
                arc = DosWrite(hFile,
                               pvResourceData,
                               ulResourceSize,
                               &ulBytesWritten);
                if (arc == NO_ERROR)
                    if (ulResourceSize != ulBytesWritten)
                        arc = ERROR_INVALID_DATA;

                DosFreeResource(pvResourceData);
            }

            DosClose(hFile);
        }
    }

    if (pulBytesWritten)
        *pulBytesWritten = ulBytesWritten;

    return arc;
}

