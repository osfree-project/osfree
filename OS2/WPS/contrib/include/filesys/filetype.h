
/*
 *@@sourcefile filesys.h:
 *      header file for filetype.c (extended file types implementation).
 *
 *      This file is ALL new with V0.9.0.
 *
 *@@include #include <os2.h>
 *@@include #include <wpdataf.h>                    // WPDataFile
 *@@include #include "helpers\linklist.h"
 *@@include #include "shared\notebook.h"            // for notebook callbacks
 *@@include #include "filesys\filetype.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef FILETYPE_HEADER_INCLUDED
    #define FILETYPE_HEADER_INCLUDED

    #define MAX_ASSOCS_PER_OBJECT       20

    #ifdef SOM_WPFileSystem_h

        ULONG ftypRegisterInstanceTypesAndFilters(M_WPFileSystem *pClassObject);

        PCSZ ftypFindClassFromInstanceType(PCSZ pcszType);

        PCSZ ftypFindClassFromInstanceFilter(PCSZ pcszObjectTitle,
                                             ULONG ulTitleLen);

    #endif

    /* ******************************************************************
     *
     *   XFldDataFile extended associations
     *
     ********************************************************************/

    APIRET ftypRenameFileType(PCSZ pcszOld,
                              PCSZ pcszNew);

    ULONG ftypAssocObjectDeleted(WPObject *somSelf);

#ifndef __NOTURBOFOLDERS__
    #ifdef SOM_XFldDataFile_h
        WPObject* ftypQueryAssociatedProgram(WPDataFile *somSelf,
                                             PULONG pulView,
                                             BOOL fUsePlainTextAsDefault);

        BOOL ftypModifyDataFileOpenSubmenu(WPDataFile *somSelf,
                                           HWND hwndOpenSubmenu);
    #endif

    /* ******************************************************************
     *
     *   Import/Export facility
     *
     ********************************************************************/

    #ifdef XSTRING_HEADER_INCLUDED
    APIRET ftypImportTypes(PCSZ pcszFilename,
                           PXSTRING pstrError);
    #endif

    APIRET ftypExportTypes(PCSZ pcszFileName);

    /* ******************************************************************
     *
     *   Notebook callbacks (notebook.c) for XFldWPS "File types" page
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED

        extern MPARAM *G_pampFileTypesPage;
        extern ULONG G_cFileTypesPage;

        VOID XWPENTRY ftypFileTypesInitPage(PNOTEBOOKPAGE pnbp,
                                            ULONG flFlags);

        MRESULT XWPENTRY ftypFileTypesItemChanged(PNOTEBOOKPAGE pnbp,
                                         ULONG ulItemID,
                                         USHORT usNotifyCode,
                                         ULONG ulExtra);

        MRESULT EXPENTRY fnwpImportWPSFilters(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);

        /* ******************************************************************
         *
         *   XFldDataFile notebook callbacks (notebook.c)
         *
         ********************************************************************/

        extern MPARAM *G_pampDatafileTypesPage;
        extern ULONG G_cDatafileTypesPage;

        VOID XWPENTRY ftypDatafileTypesInitPage(PNOTEBOOKPAGE pnbp,
                                                ULONG flFlags);

        MRESULT XWPENTRY ftypDatafileTypesItemChanged(PNOTEBOOKPAGE pnbp,
                                             ULONG ulItemID,
                                             USHORT usNotifyCode,
                                             ULONG ulExtra);

        /* ******************************************************************
         *
         *   XWPProgram/XWPProgramFile notebook callbacks (notebook.c)
         *
         ********************************************************************/

        ULONG ftypInsertAssociationsPage(WPObject *somSelf,
                                         HWND hwndNotebook);

        VOID XWPENTRY ftypAssociationsInitPage(PNOTEBOOKPAGE pnbp,
                                               ULONG flFlags);

        MRESULT XWPENTRY ftypAssociationsItemChanged(PNOTEBOOKPAGE pnbp,
                                            ULONG ulItemID,
                                            USHORT usNotifyCode,
                                            ULONG ulExtra);
    #endif

    /* ******************************************************************
     *
     *   Implementation definitions
     *
     ********************************************************************/

    /*
     *@@ FNFOREACHAUTOMATICTYPE:
     *      callback for ftypForEachAutoType.
     *
     *      If this returns FALSE, processing is
     *      aborted.
     *
     *@@added V0.9.20 (2002-07-25) [umoeller]
     */

    typedef BOOL _Optlink FNFOREACHAUTOMATICTYPE(PCSZ pcszType,
                                                 ULONG ulTypeLen,
                                                 PVOID pvUser);
    typedef FNFOREACHAUTOMATICTYPE *PFNFOREACHAUTOMATICTYPE;

    ULONG ftypForEachAutoType(PCSZ pcszObjectTitle,
                              PFNFOREACHAUTOMATICTYPE pfnftypForEachAutoType,
                              PVOID pvUser);

    #ifdef FILETYPE_PRIVATE

        BOOL ftypAppendSingleTypeUnique(PLINKLIST pll,
                                        PCSZ pcszNewType,
                                        ULONG ulNewTypeLen);

        ULONG ftypAppendTypesFromString(PCSZ pcszTypes,
                                        CHAR cSeparator,
                                        PLINKLIST pllTypes);

        VOID ftypClearTypesList(HWND hwndCnr,
                                PLINKLIST pllFileTypes);

        // forward decl here
        typedef struct _FILETYPELISTITEM *PFILETYPELISTITEM;

        /*
         * FILETYPERECORD:
         *      extended record core structure for
         *      "File types" container (Tree view).
         *
         *@@changed V0.9.9 (2001-03-27) [umoeller]: now using CHECKBOXRECORDCORE
         */

        typedef struct _FILETYPERECORD
        {
            CHECKBOXRECORDCORE  recc;               // extended record core for checkboxes;
                                                    // see comctl.c
            PFILETYPELISTITEM   pliFileType;        // added V0.9.9 (2001-02-06) [umoeller]
        } FILETYPERECORD, *PFILETYPERECORD;

        /*
         * FILETYPELISTITEM:
         *      list item structure for building an internal
         *      linked list of all file types (linklist.c).
         */

        typedef struct _FILETYPELISTITEM
        {
            PFILETYPERECORD     precc;
            PSZ                 pszFileType;        // copy of file type in INI (malloc)
            BOOL                fProcessed;
            BOOL                fCircular;          // security; prevent circular references
        } FILETYPELISTITEM;
    #endif

#endif

#endif


