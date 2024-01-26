/***************************************************************************\
* level.h -- a syslevel files API header                 940422
*
* Copyright (c) 1994 Martin Lafaix. All Rights Reserved.
\***************************************************************************/

/***************************************************************************/
/* functions prototypes                                                    */
/***************************************************************************/
APIRET LvlOpenLevelFile(PSZ pszName, PHFILE phFile,
                        ULONG ulOpenMode, PSZ pszCID);
APIRET LvlQueryLevelFile(PSZ pszName, PSZ pszCID,
                         PVOID pBuffer, ULONG ulBufSize);
APIRET LvlQueryLevelFileData(HFILE hFile, ULONG ulWhat,
                             PVOID pBuffer, ULONG ulBufSize, PULONG pulSize);
APIRET LvlWriteLevelFileData(HFILE hFile, ULONG ulWhat,
                             PVOID pBuffer, ULONG ulBufSize, PULONG pulSize);

#define LvlCloseLevelFile DosClose

/***************************************************************************/
/* ulOpenMode values                                                       */
/***************************************************************************/
#define OLF_OPEN          0x00000001L              /* scan from current dir*/
#define OLF_SCANDISKS     0x00000002L              /* scan all disks       */
#define OLF_CHECKID       0x00000004L              /* check component ID   */
#define OLF_CREATE        0x00000008L              /* create syslevel file */

/***************************************************************************/
/* ulWhat values                                                           */
/***************************************************************************/
#define QLD_MAJORVERSION  1
#define QLD_MINORVERSION  2
#define QLD_REVISION      3
#define QLD_KIND          4
#define QLD_CURRENTCSD    5
#define QLD_PREVIOUSCSD   6
#define QLD_TITLE         7
#define QLD_ID            8
#define QLD_TYPE          9

/***************************************************************************/
/* kind values                                                             */
/***************************************************************************/
#define SLK_BASE          0                        /* 'base' component     */
#define SLK_EXTENSION     2                        /* system extension     */
#define SLK_STANDARD      15                       /* 'normal' component   */
