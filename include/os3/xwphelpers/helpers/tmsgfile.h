
/*
 *@@sourcefile tmsgfile.h:
 *      header file for tmsgfile.c. See notes there.
 *
 *      This file is entirely new with V0.9.0.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\tree.h"
 *@@include #include "helpers\xstring.h"
 *@@include #include "helpers\tmsgfile.h"
 */

#if __cplusplus
extern "C" {
#endif

#ifndef TMSGFILE_HEADER_INCLUDED
    #define TMSGFILE_HEADER_INCLUDED

    #ifndef XSTRING_HEADER_INCLUDED
        #error tmsgfile.h requires xstring.h to be included first.
    #endif

    #ifndef XWPTREE_INCLUDED
        #error tmsgfile.h requires tree.h to be included first.
    #endif

    /*
     *@@ TMFMSGFILE:
     *      representation of a text message file opened by
     *      tmfOpenMessageFile.
     *
     *@@added V0.9.16 (2001-10-08) [umoeller]
     */

    typedef struct _TMFMSGFILE
    {
        PSZ     pszFilename;            // copy of .TMF file name

        TREE    *IDsTreeRoot;           // root of tree with MSGENTRY's (a TREE* really)
        ULONG   cIDs;                   // count of entries in the tree

        CHAR    szTimestamp[30];
    } TMFMSGFILE, *PTMFMSGFILE;

    APIRET tmfOpenMessageFile(const char *pcszMessageFile,
                              PTMFMSGFILE *ppMsgFile);

    APIRET tmfCloseMessageFile(PTMFMSGFILE *ppMsgFile);

    APIRET tmfGetMessage(PTMFMSGFILE pMsgFile,
                         PCSZ pcszMessageName,
                         PXSTRING pstr,
                         PCSZ *pTable,
                         ULONG cTableEntries);

#endif // TMSGFILE_HEADER_INCLUDED

#if __cplusplus
}
#endif

