
/*
 *@@sourcefile wphandle.h:
 *      header file for wphandle.c, which contains the logic for
 *      dealing with those annoying WPS object handles in OS2SYS.INI.
 *
 *      This code is mostly written by Henk Kelder and published
 *      with his kind permission.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_WINSHELLDATA
 *@@include #define INCL_WINWORKPLACE
 *@@include #include <os2.h>
 *@@include #include "helpers\wphandle.h"
 */

/*      This file Copyright (C) 1997-2001 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef WPHANDLE_HEADER_INCLUDED
    #define WPHANDLE_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Errors
     *
     ********************************************************************/

    #define ERROR_WPH_FIRST                         41000

    // #define ERROR_WPH_CRASHED                       (ERROR_WPH_FIRST +   0)
                // now using ERROR_PROTECTION_VIOLATION instead
                // V0.9.19 (2002-07-01) [umoeller]
    #define ERROR_WPH_NO_BASECLASS_DATA             (ERROR_WPH_FIRST +   1)
    #define ERROR_WPH_NO_ACTIVEHANDLES_DATA         (ERROR_WPH_FIRST +   2)
    #define ERROR_WPH_INCOMPLETE_BASECLASS_DATA     (ERROR_WPH_FIRST +   3)
    #define ERROR_WPH_NO_HANDLES_DATA               (ERROR_WPH_FIRST +   4)
    #define ERROR_WPH_CORRUPT_HANDLES_DATA          (ERROR_WPH_FIRST +   5)
                // cannot determine format (invalid keywords)
    #define ERROR_WPH_INVALID_PARENT_HANDLE         (ERROR_WPH_FIRST +   6)
    #define ERROR_WPH_CANNOT_FIND_HANDLE            (ERROR_WPH_FIRST +   7)
    #define ERROR_WPH_DRIV_TREEINSERT_FAILED        (ERROR_WPH_FIRST +   8)
    #define ERROR_WPH_NODE_TREEINSERT_FAILED        (ERROR_WPH_FIRST +   9)
    #define ERROR_WPH_NODE_BEFORE_DRIV              (ERROR_WPH_FIRST +  10)
    #define ERROR_WPH_NO_MATCHING_DRIVE_BLOCK       (ERROR_WPH_FIRST +  11)
    #define ERROR_WPH_NO_MATCHING_ROOT_DIR          (ERROR_WPH_FIRST +  12)
    #define ERROR_WPH_NOT_FILESYSTEM_HANDLE         (ERROR_WPH_FIRST +  13)
    #define ERROR_WPH_PRFQUERYPROFILESIZE_BLOCK     (ERROR_WPH_FIRST +  14)
                // PrfQueryProfileSize failed on BLOCK (fatal)
                // V0.9.19 (2002-07-01) [umoeller]
    #define ERROR_WPH_PRFQUERYPROFILEDATA_BLOCK     (ERROR_WPH_FIRST +  15)
                // PrfQueryProfileData failed on BLOCK (fatal)
                // V0.9.19 (2002-07-01) [umoeller]

    #define ERROR_WPH_LAST                          (ERROR_WPH_FIRST +  15)

    /* ******************************************************************
     *
     *   Definitions
     *
     ********************************************************************/

    #pragma pack(1)

    /*
     *@@ NODE:
     *      file or directory node in the BLOCKS
     *      in OS2SYS.INI. See wphandle.c.
     *
     *@@added V0.9.16 (2001-10-02) [umoeller]
     */

    typedef struct _NODE
    {
        CHAR    achName[4];         // = 'NODE'
        USHORT  usUsage;            // always == 1
        USHORT  usHandle;           // object handle of this NODE
        USHORT  usParentHandle;     // object handle of parent NODE
        BYTE    achFiller[20];      // filler
        USHORT  usNameSize;         // size of non-qualified filename
        CHAR    szName[1];          // variable length: non-qualified filename
                                    // (zero-terminated)
    } NODE, *PNODE;

    /*
     *@@ DRIVE:
     *      drive node in the BLOCKS
     *      in OS2SYS.INI. See wphandle.c.
     *
     *@@added V0.9.16 (2001-10-02) [umoeller]
     */

    typedef struct _DRIVE
    {
        CHAR    achName[4];  // = 'DRIV'
        USHORT  usUnknown1[4];
        ULONG   ulSerialNr;
        USHORT  usUnknown2[2];
        CHAR    szName[1];
    } DRIVE, *PDRIVE;

    #pragma pack()

    /* ******************************************************************
     *
     *   Private declarations
     *
     ********************************************************************/

    #ifdef INCLUDE_WPHANDLE_PRIVATE

        /*
         *@@ DRIVETREENODE:
         *
         *@@added V0.9.16 (2001-10-19) [umoeller]
         */

        typedef struct _DRIVETREENODE
        {
            TREE        Tree;       // ulKey points to the DRIVE.szName
                                    // (null terminated)
            PDRIVE      pDriv;      // actual DRIVE node

            TREE        *ChildrenTree;  // NODETREENODE's, if any
            LONG        cChildren;

        } DRIVETREENODE, *PDRIVETREENODE;

        /*
         *@@ NODETREENODE:
         *
         *@@added V0.9.16 (2001-10-19) [umoeller]
         */

        typedef struct _NODETREENODE
        {
            TREE        Tree;       // ulKey points to the NODE.szName
                                    // (null terminated)
            PNODE       pNode;      // actual NODE node

            TREE        *ChildrenTree;  // NODETREENODE's, if any
            LONG        cChildren;

        } NODETREENODE, *PNODETREENODE;

        /*
         *@@ WPHANDLESBUF:
         *      structure created by wphLoadHandles.
         *
         *      The composed BLOCKs in the handles buffer make up a tree of
         *      DRIVE and NODE structures (see wphandle.h). Each NODE stands
         *      for either a directory or a file. (We don't care about the
         *      DRIVE structures because the root directory gets a NODE also.)
         *      Each NODE contains the non-qualified file name, an fshandle,
         *      and the fshandle of its parent NODE.
         *
         *@@added V0.9.16 (2001-10-02) [umoeller]
         */

        typedef struct _WPHANDLESBUF
        {
            PBYTE       pbData;         // ptr to all handles (buffers from OS2SYS.INI)
            ULONG       cbData;         // byte count of *p

            USHORT      usHiwordAbstract,   // hiword for WPAbstract handles
                        usHiwordFileSystem; // hiword for WPFileSystem handles

            BOOL        fCacheValid;            // TRUE after wphRebuildNodeHashTable()
            PNODETREENODE NodeHashTable[65536]; // hash table with all nodes sorted by handle;
                                                // if item is NULL, no handle is assigned
            TREE        *DrivesTree;          // DRIVETREENODE structs really
            LONG        cDrives;

            LINKLIST    llDuplicateHandles;     // linked list of NODETREENODE's that
                                                // have an fshandle that was already
                                                // occupied (i.e. same fshandle for two
                                                // NODEs)
            LINKLIST    llDuplicateShortNames;

        } HANDLESBUF, *PHANDLESBUF;

    #endif

    /* ******************************************************************
     *
     *   Load handles functions
     *
     ********************************************************************/

    typedef unsigned long HHANDLES;

    APIRET wphQueryActiveHandles(HINI hiniSystem,
                                 PSZ *ppszActiveHandles);

    APIRET wphQueryBaseClassesHiwords(HINI hiniUser,
                                      PUSHORT pusHiwordAbstract,
                                      PUSHORT pusHiwordFileSystem);

    APIRET wphRebuildNodeHashTable(HHANDLES hHandles,
                                   BOOL fQuitOnErrors);

    APIRET wphLoadHandles(HINI hiniUser,
                          HINI hiniSystem,
                          const char *pcszActiveHandles,
                          HHANDLES *phHandles);

    APIRET wphFreeHandles(HHANDLES *phHandles);

    APIRET wphQueryHandleFromPath(HINI hiniUser,
                                  HINI hiniSystem,
                                  const char *pcszName,
                                  HOBJECT *phobj);

    APIRET wphComposePath(HHANDLES hHandles,
                          USHORT usHandle,
                          PSZ pszFilename,
                          ULONG cbFilename,
                          PNODE *ppNode);

    APIRET wphQueryPathFromHandle(HINI hiniUser,
                                  HINI hiniSystem,
                                  HOBJECT hObject,
                                  PSZ pszFilename,
                                  ULONG cbFilename);

    PCSZ wphDescribeError(APIRET arc);

#endif

#if __cplusplus
}
#endif

