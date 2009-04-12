/* $Id: fsckmsge.h,v 1.1.1.1 2003/05/21 13:39:54 pasha Exp $ */

/* static char *SCCSID = "@(#)1.26.1.2  12/2/99 11:13:48 src/jfs/utils/chkdsk/fsckmsge.h, jfschk, w45.fs32, fixbld";*/
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *  
 *   MODULE_NAME:		fsckmsge.h
 *
 *   COMPONENT_NAME: 	jfschk
 *
 */
#ifndef H_FSCKMSGT
#define H_FSCKMSGT

#ifndef _JFS_XCHKDMP
#include "xfsck.h" 
#endif

/*
 * The following array contains the text for JFS/OS2 chkdsk messages
 *
 * NOTE: the message contained in position i in this array 
 *
 *	o is processed according to contents of position i in the 
 *	  message protocol array (fsckmsgp.h)
 *
 *	o is translated to the local language in position i+50 in the 
 *        JFS message file (source: jfs.txt  binary: jfs.msg, jfsh.msg) if and 
 *	only if it also appears in jfs.txt.
 *
 */

char *MsgText[]={
/*  0*/
    "Operation was successful.\n\r"

/*  1*/
   ,"CHKDSK  All observed inconsistencies have been repaired.\n\r"
/*  2*/
   ,"CHKDSK  Invalid data format detected in root directory.\n\r"
/*  3*/
   ,"CHKDSK  A combination of Minimum Free Blocks and Total Usable Blocks which is inval\
id for the filesystem size was detected in the superblock (%s).\n\r"
/*  4*/
   ,"CHKDSK  Invalid block number(s) (%s) detected for file system object %s%s%s.\n\r"
/*  5*/ 
   ,"CHKDSK  File system object %s%s%s has a corrupt backward sibling chain.\n\r"
/*  6*/
   ,"CHKDSK  File system object %s%s%s has a corrupt forward sibling chain.\n\r"
/*  7*/
   ,"CHKDSK  Inode %s%s has unrecognized type.\n\r"
/*  8*/
   ,"CHKDSK  File system object %s%s%s has invalid descriptor (%s).\n\r"
/*  9*/
   ,"CHKDSK  Inode %s%s has incorrect link count.\n\r"
/* 10*/
   ,"CHKDSK  Directory inode %s%s refers to a nonexistent inode %s%s (entry %s).\n\r"
/* 11*/
   ,"CHKDSK  Error (%s,%s) writing to the chkdsk service log (%s,%s,%s,%s).  \
Continuing.\n\r"
/* 12*/
   ,"CHKDSK  The boot sector has been refreshed.\n\r"
/* 13*/
   ,"CHKDSK  File system object %s%s%s has corrupt data (%s).\n\r"
/* 14*/
   ,"CHKDSK  Invalid data (%s) detected in the superblock (%s).\n\r"
/* 15*/
   ,"CHKDSK  Invalid allocation group size in the superblock (%s).\n\r"
/* 16*/
   ,"CHKDSK  Invalid filesystem block size in the superblock (%s).\n\r"
/* 17*/
   ,"CHKDSK  Invalid filesystem size in the superblock (%s).\n\r"
/* 18*/
   ,"CHKDSK  Superblock is corrupt and cannot be repaired since both primary and seconda\
ry copies are corrupt.  CHKDSK CANNOT CONTINUE.\n\r"
/* 19*/
   ,"CHKDSK  Primary superblock is corrupt and cannot be repaired without write access. \
Continuing. \n\r"
/* 20*/
   ,"Usage: chklog [-L[:N|:P]] Device\n\r"
/* 21 */
   ,"CHKDSK  Invalid magic number in the superblock (%s).\n\r"
/* 22 */
   ,"CHKDSK  Invalid version number in the superblock (%s).\n\r"
/* 23*/
   ,"CHKDSK  Incorrect maxbud AG detected in Block Map Control Page.\n\r"
/* 24*/
   ,"CHKDSK  Incorrect number of free blocks detected in Block Map Control Page.\n\r"
/* 25*/
   ,"CHKDSK  In superblock (%s) filesystem block size smaller than volume block size.\n\r"
/* 26*/
   ,"CHKDSK  Invalid format detected in Root directory.\n\r"
/* 27*/
   ,"CHKDSK  Device parm defaulting to current volume: %s\n\r"
/* 28*/
   ,"CHKDSK  Unable to write to boot sector Master Boot Record.  Continuing.\n\r"
/* 29*/
   ,"CHKDSK  Secondary file/directory allocation structure (%s) is not a \
correct redundant copy of primary structure.\n\r"
/* 30*/
   ,"CHKDSK  Unable to replicate primary file/directory allocation structure \
(%s) to secondary.  FUTURE RECOVERY CAPABILITY IS COMPROMISED.\n\r"
/* 31*/
   ,"chkdsk service log selected:  MOST RECENT\n\r"
/* 32*/
   ,"CHKDSK  MINOR: Cannot create directory lost+found in root directory.  mkdir \
lost+found in the root directory then run chkdsk with the F parameter to reconnect\
lost files and/or directories.\n\r"
/* 33*/
   ,"CHKDSK  Fileset object %s%s%s:  No paths found.\n\r"
/* 34*/
   ,"CHKDSK  The path(s) refer to an unallocated file.\n\r "
/* 35*/
   ,"chkdsk service log selected:  PREVIOUS\n\r"
/* 36*/
   ,"CHKDSK  The path(s) refer to an unallocated file. Will remove.\n\r "
/* 37*/
   ,"CHKDSK  Unable to repair primary inode allocation structure (%s). Continuing.\n\r"
/* 38*/
   ,"CHKDSK  Root directory has a corrupt tree.\n\r"
/* 39*/
   ,"CHKDSK  Initialized tree created for root directory.\n\r"
/* 40*/
   ,"CHKDSK  Unable to read primary superblock.\n\r"

/* 41 */
   ,"chkdsk service log: INVALID LOG RECORD: SKIPPING ONE OR MORE LOG RECORDS.\n\r"
/* 42 */
   ,"CHKDSK  processing started: %s\n\r"
/* 43*/
   ,"\n********************************************************** \n\n\r"
/* 44*/
   ,"CHKDSK  Unable to write primary superblock.\n\r"
/* 45*/
   ,"CHKDSK  Fatal error (%s,%s) accessing the filesystem (%s,%s,%s,%s).\n\r"
/* 46*/
   ,"CHKDSK  Multiple parent directories for directory %s%s.\n\r"
/* 47*/
   ,"CHKDSK  External journal log not supported.\n\r"
/* 48*/
   ,"CHKDSK  Duplicate reference to %s block(s) beginning at offset %s found in \
file system object %s%s%s.\n\r"
/* 49*/
   ,"CHKDSK  File set metadata inode %s%s is corrupt.\n\r"
/* 50*/
   ,"CHKDSK  Fatal error (%s,%s) accessing the workspace (%s,%s,%s,%s).\n\r"
/* 51*/
   ,"CHKDSK  Insufficient dynamic storage available for required workspace \
(%s,%s). CHKDSK CANNOT CONTINUE\n\r"
/* 52*/
   ,"CHKDSK  DosDevIOCtl(...DSK_GETDEVICEPARAMS...) returned rc = %s\n\r"
/* 53*/
   ,"CHKDSK  Filesystem is currently mounted.\n\r"
/* 54*/
   ,"CHKDSK  Invalid data (%s) detected in file system object %s%s%s.\n\r"
/* 55*/
   ,"CHKDSK  Block size in bytes:  %s\n\r"
/* 56*/
   ,"CHKDSK  Filesystem size in blocks:  %s\n\r"
/* 57*/
   ,"CHKDSK  Filesystem Summary:\n\r"
/* 58*/
   ,"CHKDSK    Blocks in use for inodes:  %s\n\r"
/* 59*/
   ,"CHKDSK    Inode count:  %s\n\r"
/* 60*/
   ,"CHKDSK    File count:  %s\n\r"

/* 61 */
   ,"CHKDSK    Directory count:  %s\n\r"
/* 62 */
   ,"CHKDSK  Inode %s%s has references to cross linked blocks.\n\r"
/* 63*/
   ,"CHKDSK    Block count:  %s\n\r"
/* 64*/
   ,"CHKDSK    Free block count:  %s\n\r"
/* 65*/
   ,"CHKDSK  Directory inode %s%s entry %s refers to an illegal inode %s%s.\n\r"
/* 66*/
   ,"Usage: xchklog [-L[:N|:P]] [-F<filename>] Device\n\r"
/* 67*/
   ,"CHKDSK  Unable to get path for link from directory %s%s to fileset object %s%s%s.\n\r"
/* 68*/
   ,"CHKDSK  Format error in Extended Attributes Space or descriptor.\n\r"
/* 69*/
   ,"CHKDSK  Discrepancies detected in the Free Inode Extent List for Allocation \
Group %s%s. (%s)\n\r"
/* 70*/
   ,"CHKDSK  MINOR: Unable to reconnect file inode %s%s.  Continuing.\n\r"
/* 71*/
   ,"CHKDSK  Directory %s%s entry \"..\" refers to an incorrect parent directory (%s%s).\n\r"
/* 72*/
   ,"CHKDSK  Directory inode %s%s entry \"..\" reference to incorrect inode \
corrected.\n\r"
/* 73*/
   ,"CHKDSK  Storage allocated to inode %s%s has been cleared.\n\r"
/* 74*/
   ,"CHKDSK  Storage allocated to extended attributes for inode \
%s%s has been cleared.\n\r"
/* 75*/
   ,"CHKDSK  File inode %s%s has been reconnected to /lost+found/.\n\r"
/* 76*/
   ,"CHKDSK  Link count for inode %s%s has been adjusted/corrected.\n\r"
/* 77*/
   ,"CHKDSK  DosDevIOCtl(...DSK_LOCKDRIVE...) returned rc = %s\n\r"
/* 78*/
   ,"CHKDSK  No paths were found for inode %s%s.\n\r"
/* 79*/
   ,"CHKDSK  File system object %s%s%s is linked as: %s\n\r"
/* 80*/
   ,"CHKDSK  Directory inode %s%s entry reference to inode %s%s removed.\n\r"

/* 81 */
   ,"CHKDSK  Invalid fwsp length detected in the superblock (%s).\n\r"
/* 82 */
   ,"CHKDSK  Invalid fwsp address detected in the superblock (%s).\n\r"
/* 83*/
   ,"CHKDSK  DosOpen(...OPEN_SHARE_DENYNONE|OPEN_ACCESS_READONLY...) \
returned rc = %s\n\r"
/* 84*/
   ,"CHKDSK  MINOR: Insufficient dynamic storage to validate extended attributes \
format.\n\r"
/* 85*/
   ,"CHKDSK  logredo failed (rc=%s).  chkdsk continuing.\n\r"
/* 86*/
   ,"CHKDSK  logredo returned rc = %s\n\r"
/* 87*/
   ,"CHKDSK  Unable to create a lost+found directory in root  because root already \
contains a non-directory object named lost+found.  Rename the existing lost+found \
object in root, mkdir lost+found in the root directory, then run chkdsk with the \
F parameter to reconnect lost files and/or directories.\n\r"
/* 88*/
   ,"CHKDSK  Checking a mounted filesystem does not produce dependable results.\n\r"
/* 89*/
   ,"CHKDSK **** Filesystem was modified. ****\n\r"
/* 90*/
   ,"CHKDSK  ujfs_redeterminemedia() returned rc = %s\n\r"
/* 91*/
   ,"CHKDSK  Superblock marked.\n\r"
/* 92*/
   ,"CHKDSK  %s blocks are missing.\n\r"
/* 93*/
   ,"CHKDSK  Unable to write to boot sector.  Continuing.\n\r"
/* 94*/
   ,"CHKDSK  No \\lost+found directory found in the filesystem.\n\r"
/* 95*/
   ,"CHKDSK  Incorrect link counts detected in the aggregate.\n\r"
/* 96*/
   ,"CHKDSK  The Inode Allocation Map control information has an incorrect number of \
backed inodes value for AG %s%s.\n\r"
/* 97*/
   ,"CHKDSK  The Inode Allocation Map control information has an incorrect number of \
free inodes value for AG %s%s.\n\r"
/* 98*/
   ,"CHKDSK  Unrecoverable error reading %s from %s. CHKDSK CANNOT CONTINUE.\n\r"
/* 99*/
   ,"CHKDSK **Phase 0 - Replay Journal Log\n\r"
/*100*/
   ,"CHKDSK **Phase 1 - Check Blocks, Files/Directories, and  Directory Entries\n\r"

/*101 */
   ,"CHKDSK **Phase 2 - Count links\n\r"
/*102 */
   ,"CHKDSK **Phase 3 - Duplicate Block Rescan and Directory Connectedness\n\r"
/*103*/
   ,"CHKDSK **Phase 4 - Report Problems\n\r"
/*104*/
   ,"CHKDSK **Phase 5 - Check Connectivity\n\r"
/*105*/
   ,"CHKDSK **Phase 6 - Perform Corrections\n\r"
/*106*/
   ,"CHKDSK **Phase 7 - Rebuild File/Directory Allocation Maps\n\r"
/*107*/
   ,"CHKDSK **Phase 8 - Rebuild Disk Allocation Maps\n\r"
/*108*/
   ,"CHKDSK **Phase 9 - Reformat File System Log\n\r"
/*109*/
   ,"CHKDSK  Directory has entry for unallocated file %s%s. Will remove.\n\r"
/*110*/
   ,"CHKDSK  FATAL ERROR (%s,%s) ON READ Aggregate Inode Table (%s) first extent.\n\r"
/*111*/
   ,"CHKDSK  Format error in Extended Attributes Space or descriptor. Will clear.\n\r"
/*112*/
   ,"CHKDSK  The extent descriptor for inodes %s through %s is invalid.  (Inode Allocation Map %s%s,\
Inode Allocation Group %s, Extent Index %s).\n\r"
/*113*/
   ,"CHKDSK  Extended attributes for file set object %s%s%s have an invalid format.\n\r"
/*114*/
   ,"CHKDSK  Superblock marked dirty because repairs are about to be written.\n\r"
/*115*/
   ,"CHKDSK Mutually exclusive /F parameters specified.\n\r"
/*116*/
   ,"CHKDSK Usage: chkdsk [/f[:0|:1|:2|:3]] [/c] [/v] [/o] Device\n\r"
/*117*/
   ,"CHKDSK Unrecognized CHKDSK F parameter value detected:   %s\n\r"
/*118*/
   ,"CHKDSK Unsupported CHKDSK parameter:   %s\n\r"
/*119*/
   ,"CHKDSK  logformat failed (rc=%s).  chkdsk continuing.\n\r"
/*120*/
   ,"CHKDSK  logformat returned rc = %s\n\r"

/*121 */
   ,"CHKDSK  Unable to read device characteristics.  Boot sector cannot be refreshed.  \
Continuing.\n\r"
/*122 */
   ,"CHKDSK  Primary superblock is corrupt.\n\r"
/*123 */
   ,"CHKDSK  Inode Allocation Group %s%s is inconsistent.\n\r"
/*124 */
   ,"CHKDSK  cannot repair an allocation error for files and/or directories %s through %s.\n\r"
/*125*/
   ,"CHKDSK  Root inode is not a directory.\n\r"
/*126*/
   ,"CHKDSK  Root inode is not allocated.\n\r"
/*127*/
   ,"CHKDSK  Root inode allocated.\n\r"
/*128*/
   ,"CHKDSK  Root inode changed to directory.\n\r"
/*129*/
   ,"CHKDSK  DosDevIOCtl(...DSK_UNLOCKDRIVE...) returned rc = %s\n\r"
/*130*/
   ,"CHKDSK  Primary superblock is valid.\n\r"
/*131*/
   ,"CHKDSK  processing terminated:  %s   with return code: %s.\n\r"
/*132*/
   ,"CHKDSK  DosClose returned rc = %s\n\r"
/*133*/
   ,"CHKDSK  Using default parameter:  /F:0\n\r"
/*134*/
   ,"CHKDSK  DosOpen(...OPEN_SHARE_DENYREADWRITE|OPEN_ACCESS_READWRITE...) \
returned rc = %s\n\r"
/*135*/
   ,"CHKDSK  One or more directory entry found for unallocated inode %s%s.\n\r"
/*136*/
   ,"CHKDSK  cannot recover files and/or directories %s through %s.  \
CHKDSK CANNOT CONTINUE.\n\r"
/*137*/
   ,"CHKDSK  Unrecoverable error writing %s to %s. CHKDSK CANNOT CONTINUE.\n\r"
/*138*/
   ,"CHKDSK  The root directory has an invalid data format.  Will correct.\n\r"
/*139*/
   ,"CHKDSK  The root directory has an invalid format.  Will correct.\n\r"
/*140*/
   ,"CHKDSK  cannot recover files and/or directories %s through %s.  Will release.\n\r"

/*141 */
   ,"CHKDSK  File claims cross linked block(s).\n\r"
/*142 */
   ,"CHKDSK  cannot repair the data format error(s) in this file.\n\r"
/*143*/
   ,"CHKDSK  cannot repair the format error(s) in this file.\n\r"
/*144*/
   ,"CHKDSK  cannot repair %s%s%s.\n\r"
/*145*/
   ,"CHKDSK  The current hard disk drive is:  %s\n\r"
/*146*/
   ,"CHKDSK  The type of file system for the disk is JFS.\n\r"
/*147*/
   ,"CHKDSK  cannot repair %s%s%s.  Will release.\n\r"
/*148*/
   ,"CHKDSK  Secondary superblock is valid.\n\r"
/*149*/
   ,"CHKDSK  Unable to open for write access.  Proceeding in read-only mode.\n\r"
/*150 */
   ,"CHKDSK  Multiple parent directories for directory %s%s.  Will correct.\n\r"
/*151 */
   ,"CHKDSK  Directory %s%s entry \"..\" refers to an incorrect parent directory \
(%s%s). Will correct.\n\r"
/*152*/
   ,"CHKDSK  %s unexpected blocks detected.\n\r"
/*153*/
   ,"CHKDSK  Directories with illegal hard links have been detected.\n\r"
/*154*/
   ,"CHKDSK  Directory entries (entries \"..\") referring to incorrect parent \
directories have been detected.\n\r"
/*155*/
   ,"CHKDSK  Directory entries for unallocated files have been detected.\n\r"
/*156*/
   ,"CHKDSK  Unable to write secondary superblock.\n\r"
/*157*/
   ,"CHKDSK  Incorrect link counts have been detected. Will correct.\n\r"
/*158*/
   ,"CHKDSK  File set object %s%s%s has inline data conflict (type %s).\n\r"
/*159*/
   ,"CHKDSK  Duplicate block references have been detected in Metadata. CHKDSK CANNOT CONTINUE.\n\r"
/*160*/
   ,"CHKDSK  Multiple metadata references to %s blocks beginning at \
offset %s have been detected.\n\r"
/*161*/
   ,"CHKDSK  Directory inode %s%s has a node with empty freelist and nonzero freecount.\n\r"
/*162*/
   ,"CHKDSK  Unable to read entire first extent of AIT (%s).\n\r"
/*163*/
   ,"CHKDSK  Unable to read first extent of AIT (%s).\n\r"
/*164*/
   ,"CHKDSK  Unrecoverable error during UNLOCK processing.\n\r"
/*165*/
   ,"CHKDSK  Unrecoverable error during CLOSE processing.\n\r"
/*166*/
   ,"CHKDSK  The root directory reference (entry \"..\") has been corrected.\n\r"
/*167*/
   ,"CHKDSK  %s appears to be the correct path for directory %s%s.\n\r"
/*168*/
   ,"CHKDSK  Directory inode %s%s has illegal hard links.\n\r"
/*169*/
   ,"CHKDSK  Directory inode %s%s refers (entry \"..\") to an incorrect inode (%s%s).\n\r"
/*170*/
   ,"CHKDSK  One or more directory entry found for unallocated inode %s%s.\n\r"
/*171*/
   ,"CHKDSK  ERRORS HAVE BEEN DETECTED.  Run chkdsk with the F parameter to repair.\n\r"
/*172*/
   ,"CHKDSK  The root directory refers (entry \"..\") to an incorrect inode.\n\r"
/*173*/   
   ,"CHKDSK  %s directory reconnected to /lost+found/.\n\r"
/*174*/
   ,"CHKDSK  MINOR: Unable to reconnect %s directory.  Continuing.\n\r"
/*175*/
   ,"CHKDSK  Inode %s%s is not connected to the root directory tree.\n\r"
/*176*/
   ,"XCHKLOG  Most recent chkdsk service log extracted into:  %s\n\r"
/*177*/
   ,"XCHKLOG  Prior chkdsk service log extracted into:  %s\n\r"
/*178*/
   ,"XCHKLOG  Can't open output file:  %s\n\r"
/*179*/
   ,"CHKDSK  Invalid stamp detected in file system object %s%s%s.\n\r"
/*180*/
   ,"CHKDSK  Files and/or directories not connected to the root directory tree \
have been detected.\n\r"
/*181*/
   ,"CHKDSK  Directory entries for unallocated files have been detected.  \
Will remove.\n\r"
/*182*/
   ,"CHKDSK  Files and/or directories not connected to the root directory tree \
have been detected.  Will reconnect.\n\r"
/*183*/
   ,"CHKDSK  Directories with illegal hard links have been detected.  Will correct.\n\r"
/*184*/
   ,"CHKDSK  Directories (entries \"..\") referring to incorrect parent directories \
have been detected.  Will correct.\n\r"
/*185*/
   ,"XCHKDMP  Can't open input file:  %s\n\r"
/*186*/
   ,"CHKDSK  Filesystem is clean.\n\r"
/*187*/
   ,"CHKDSK  Filesystem is clean but is marked dirty.  Run chkdsk with the F \
parameter to fix.\n\r"
/*188*/
   ,"CHKDSK  Filesystem is dirty.\n\r"
/*189*/
   ,"CHKDSK  Filesystem is dirty but is marked clean.  In its present state, the \
results of accessing %s (except by this utility) are undefined.\n\r"
/*190*/
   ,"CHKDSK  Filesystem has been marked clean.\n\r"
/*191*/
   ,"CHKDSK  Filesystem has been marked dirty because it contains critical errors.\
Filesystem may be unrecoverable.\n\r"
/*192*/
   ,"CHKDSK  File system object %s%s%s has an illegal empty node.\n\r"
/*193*/
   ,"CHKDSK File system object %s%s%s -- extent at offset %s has an invalid size (%s).\n\r"
/*194*/
   ,"CHKDSK  Dense file (inode %s%s) begins with an unallocated section.\n\r"
/*195*/
   ,"CHKDSK  Dense file (inode %s%s) has an unallocated section after offset %s.\n\r"
/*196*/
   ,"CHKDSK  The  Free Inode Extent List is inconsistent for Allocation \
Group %s%s.\n\r"
/*197*/
   ,"CHKDSK  Discrepancies detected in the Free Inode List for Allocation \
Group %s%s. (%s) \n\r"
/*198*/
   ,"CHKDSK  The Free Inode List is inconsistent for Allocation Group %s%s.\n\r"
/*199*/
   ,"CHKDSK  Inode Allocation Group %s%s has an invalid Allocation Group (%s).\n\r"
/*200*/
   ,"CHKDSK  Directory has an entry for an unallocated file %s%s.\n\r"
/*201*/
   ,"CHKDSK  Directory inode %s%s has a node with incorrect size.\n\r"
/*202*/
   ,"CHKDSK  Directory inode %s%s has a node with invalid size.\n\r"
/*203*/
   ,"CHKDSK  Directory inode %s%s has a node with an invalid freelist.\n\r"
/*204*/
   ,"CHKDSK  Directory inode %s%s has a node with an incorrect freecount.\n\r"
/*205*/
   ,"CHKDSK  Directory inode %s%s has a node with an incorrect freelist.\n\r"
/*206*/
   ,"CHKDSK  Inode Allocation Group %s%s has an invalid AG Start.\n\r"
/*207*/
   ,"CHKDSK  Inode Allocation Group %s%s has an invalid Free Extent Summary.\n\r"
/*208*/
   ,"CHKDSK  Inode Allocation Group %s%s has an invalid Free Inode Summary.\n\r"
/*209*/
   ,"CHKDSK  Discrepancies detected in the Free IAG List. (%s,%s)\n\r"
/*210*/
   ,"CHKDSK  The Free Inode Allocation Group List is inconsistent (%s).\n\r"
/*211*/
   ,"CHKDSK  Inode Allocation Group %s%s has an incorrect IAG number value.\n\r"
/*212*/
   ,"CHKDSK  Inode Allocation Group %s%s has an inconsistent count for \
number of free extents.\n\r"
/*213*/
   ,"CHKDSK  Inode Allocation Group %s%s has an inconsistent count for \
number of free inodes.\n\r"
/*214*/
   ,"CHKDSK  Discrepancies detected in the pmap for Inode Allocation Group %s%s.\n\r"
/*215*/
   ,"CHKDSK  Errors detected in the Fileset File/Directory Allocation Map.\n\r"
/*216*/
   ,"CHKDSK  The Inode Allocation Map blocks per inode extent is incorrect (%s).\n\r"
/*217*/
   ,"CHKDSK  Errors detected in the Fileset File/Directory Allocation Map control information.\n\r"
/*218*/
   ,"CHKDSK  The Inode Allocation Map log2(blocks per inode extent) is incorrect (%s).\n\r"
/*219*/
   ,"CHKDSK  The Inode Allocation Map has an incorrect number of backed inodes value (%s).\n\r"
/*220*/
   ,"CHKDSK  The Inode Allocation Map has an incorrect number of free inodes value (%s).\n\r"
/*221*/
   ,"CHKDSK  The Inode Allocation Map has an incorrect next IAG value (%s).\n\r"
/*222*/
   ,"Usage: xchkdmp [-L[:N|:P]] [-F<filename>]\n\r"
/*223*/
   ,"CHKDSK  Correcting Inode Allocation Group %s%s invalid Allocation Group (%s).\n\r"
/*224*/
   ,"CHKDSK  Correcting Inode Allocation Group %s%s invalid AG Start.\n\r"
/*225*/
   ,"CHKDSK  Inconsistencies detected in leaf values (%s).\n\r"
/*226*/
   ,"CHKDSK  Inconsistencies detected in internal values (%s).\n\r"
/*227*/
   ,"CHKDSK  Incorrect data detected in pages (%s).\n\r"
/*228*/
   ,"CHKDSK  Descrepancies detected between observed block allocations and pmaps.\n\r"
/*229*/
   ,"CHKDSK  Incorrect data detected in disk allocation structures.\n\r"
/*230*/
   ,"CHKDSK  Incorrect data detected in disk allocation control structures.\n\r"
/*231*/
   ,"CHKDSK  Incorrect height detected in in page %s,%s.\n\r"
/*232*/
   ,"CHKDSK  Incorrect l2 number of leafs detected in in page %s,%s.\n\r"
/*233*/
   ,"CHKDSK  Incorrect leaf index detected in in page %s,%s.\n\r"
/*234*/
   ,"CHKDSK  Incorrect leaf (%s) value detected in %s page %s.\n\r"
/*235*/
   ,"CHKDSK  Incorrect internal (%s) value detected in %s page %s.\n\r"
/*236*/
   ,"CHKDSK  Incorrect budmin detected in in page %s,%s.\n\r"
/*237*/
   ,"CHKDSK  Incorrect number of leafs detected in page %s,%s.\n\r"
/*238*/
   ,"CHKDSK  Incorrect number of free blocks in AG %s detected in Block Map Control \
Page.\n\r"
/*239*/
   ,"CHKDSK  Incorrect aggregate size detected in Block Map Control Page.\n\r"
/*240*/
   ,"CHKDSK  Incorrect blocks per AG detected in Block Map Control Page.\n\r"
/*241*/
   ,"CHKDSK  Incorrect dmap control level detected in Block Map Control Page.\n\r"
/*242*/
   ,"CHKDSK  Incorrect dmap control height detected in Block Map Control Page.\n\r"
/*243*/
   ,"CHKDSK  Incorrect dmap control width detected in Block Map Control Page.\n\r"
/*244*/
   ,"CHKDSK  Incorrect l2 blocks per AG detected in Block Map Control Page.\n\r"
/*245*/
   ,"CHKDSK  Incorrect l2 blocks per page detected in Block Map Control Page.\n\r"
/*246*/
   ,"CHKDSK  Incorrect maximum active AGs detected in Block Map Control Page.\n\r"
/*247*/
   ,"CHKDSK  Incorrect maximum level detected in Block Map Control Page.\n\r"
/*248*/
   ,"CHKDSK  Incorrect number of AGs detected in Block Map Control Page.\n\r"
/*249*/
   ,"CHKDSK  Invalid preferred AG detected in Block Map Control Page.\n\r"
/*250*/
   ,"XCHKLOG  %s superblock is valid.\n\r"
/*251*/
   ,"CHKDSK  Incorrect number of blocks detected in dmap %s.\n\r"
/*252*/
   ,"CHKDSK  Incorrect number free detected in dmap %s.\n\r"
/*253*/
   ,"CHKDSK  Incorrect start detected in dmap %s.\n\r"
/*254*/
   ,"CHKDSK  %s consecutive blocks observed available but pmap (%s, %s, %s) \
indicates they are allocated.\n\r"
/*255*/
   ,"CHKDSK  %s consecutive blocks observed allocated but pmap (%s, %s, %s) \
indicates they are available.\n\r"
/*256*/
   ,"CHKDSK  Discrepancies detected in the Block Map Control Page AG free count list.\n\r"
/*257*/
   ,"CHKDSK  Incorrect data detected in the Block Map Control Page.\n\r"
/*258*/
   ,"CHKDSK  Incorrect dmap control start index detected in Block Map Control Page.\n\r"
/*259*/
   ,"CHKDSK  Incorrect fwsp length detected in the superblock (%s).\n\r"
/*260*/
   ,"CHKDSK  Incorrect jlog address detected in the superblock (%s).\n\r"
/*261*/
   ,"CHKDSK  Incorrect jlog length detected in the superblock (%s).\n\r"
/*262*/
   ,"    |........\r"
/*263*/
   ,"    .|.......\r"
/*264*/
   ,"    ..|......\r"
/*265*/
   ,"    ...|.....\r"
/*266*/
   ,"    ....|....\r"
/*267*/
   ,"    .....|...\r"
/*268*/
   ,"    ......|..\r"
/*269*/
   ,"    .......|.\r"
/*270*/
   ,"    ........|\r"
/*271*/
   ,"CHKDSK  Directory claims cross linked block(s).\n\r"
/*272*/
   ,"CHKDSK  File system object claims cross linked block(s).\n\r"
/*273*/
   ,"CHKDSK   File system is formatted for sparse files.\n\r"
/*274*/
   ,"CHKDSK  Directory inode %s%s incorrect DASD used value corrected.\n\r"
/*275*/
   ,"CHKDSK  %s directories reconnected to /lost+found/.\n\r"
/*276*/
   ,"CHKDSK  %s file reconnected to /lost+found/.\n\r"
/*277*/
   ,"CHKDSK  %s files reconnected to /lost+found/.\n\r"
/*278*/
   ,"CHKDSK cannot repair the data format error(s) in this directory.\n\r"
/*279*/
   ,"CHKDSK cannot repair the format error(s) in this directory.\n\r"
/*280*/
   ,"CHKDSK cannot repair the data format error(s) in this file system object.\n\r"
/*281*/
   ,"CHKDSK cannot repair the format error(s) in this file system object.\n\r"
/*282*/
   ,"CHKDSK **Phase 7 - Verify File/Directory Allocation Maps\n\r"
/*283*/
   ,"CHKDSK **Phase 8 - Verify Disk Allocation Maps\n\r"
/*284*/
   ,"CHKDSK parameter detected:  Omit logredo()\n\r"
/*285*/
   ,"CHKDSK   *undefined* \n\r"
/*286*/
   ,"CHKDSK   *undefined* \n\r"
/*287*/
   ,"CHKDSK   *undefined* \n\r"
/*288*/
   ,"CHKDSK   *undefined* \n\r"
/*289*/
   ,"CHKDSK   *undefined* \n\r"
/*290*/
   ,"CHKDSK  Primary metadata inode %s%s is corrupt.\n\r"
/*291*/
   ,"CHKDSK  Secondary metadata inode %s%s is corrupt.\n\r"
/*292*/
   ,"CHKDSK  Secondary superblock is corrupt.\n\r"
/*293*/
   ,"CHKDSK  Unable to read the Secondary File/Directory Allocation Table.\n\r"
/*294*/
   ,"CHKDSK  Errors detected in the File System File/Directory Allocation Map.\n\r"
/*295*/
   ,"CHKDSK  Errors detected in the File System File/Directory Allocation Map control information.\n\r"
/*296*/
   ,"CHKDSK  Errors detected in the Secondary File/Directory Allocation Table.\n\r"
/*297*/
   ,"CHKDSK  Unable to read secondary superblock.\n\r"
/*298*/
   ,"CHKDSK   *undefined* \n\r"
/*299*/
   ,"CHKDSK   *undefined* \n\r"
/*300*/
   ,"CHKDSK   *undefined* \n\r"
/*301*/
   ,"CHKDSK  MINOR: Unable to reconnect %s directories.  Continuing.\n\r"
/*302*/
   ,"CHKDSK  MINOR: Unable to reconnect %s file.  Continuing.\n\r"
/*303*/
   ,"CHKDSK  MINOR: Unable to reconnect %s files.  Continuing.\n\r"
/*304*/
   ,"CHKDSK   *undefined* \n\r"
/*305*/
   ,"CHKDSK   *undefined* \n\r"
/*306*/
   ,"CHKDSK   *undefined* \n\r"
/*307*/
   ,"CHKDSK   *undefined* \n\r"
/*308*/
   ,"CHKDSK   *undefined* \n\r"
/*309*/
   ,"CHKDSK  Directory inode %s%s has been reconnected to /lost+found/.\n\r"
/*310*/
   ,"CHKDSK  MINOR: Unable to reconnect directory inode %s%s.  Continuing.\n\r"
/*311*/
   ,"CHKDSK   *undefined* \n\r"
/*312*/
   ,"CHKDSK  Unable to read either superblock.\n\r"
/*313*/
   ,"%s kilobytes total disk space.\n\r"
/*314*/
   ,"%s kilobytes in %s directories.\n\r"
/*315*/
   ,"%s kilobytes in %s user files.\n\r"
/*316*/
   ,"%s kilobytes in extended attributes\n\r"
/*317*/
   ,"%s kilobytes reserved for system use.\n\r"
/*318*/
   ,"%s kilobytes are available for use.\n\r"
/*319*/
   ,"XCHKDMP Can't read input file:  %s\n\r"
/*320*/
   ,"XCHKDMP Input file (%s) not recognized as an extracted JFS/Warp chkdsk service log.\n\r"
/*321*/
   ,"XCHKLOG %s superblock is valid.\n\r"
/*322*/
   ,"XCHKLOG  Output file path and name must be less than 128 characters.\n\r"
/*323*/
   ,"XCHKDMP  Input file path and name must be less than 128 characters.\n\r"
/*324*/
   ,"CHKDSK  Unable to read the Primary File/Directory Allocation Table.\n\r"
/*325*/
  ,"CHKDSK  Errors detected in the Primary File/Directory Allocation Table.\n\r"
/*326*/
   ,"CHKDSK CANNOT CONTINUE.\n\r"
/*327*/
   ,"********** CHKDSK CANNOT INITIALIZE THIS SERVICE LOG.  DISREGARD RESIDUAL LOG \
MESSAGES WHICH MAY BE APPENDED. **********\n\r"
/*328*/
   ,"CHKLOG  Required parameter missing:  device specification\n\r"
/*329*/
   ,"XCHKLOG  Required parameter missing:  device specification\n\r"
/*330*/
   ,"CHKDSK parameter detected:  FixLevel: %s\n\r"
/*331*/
   ,"CHKDSK parameter detected:  IfDirty\n\r"
/*332*/
   ,"CHKDSK parameter detected:  VerboseMessaging\n\r"
/*333*/
   ,"CHKDSK parameter detected:  AutoCheck mode\n\r"
/*334*/
   ,"CHKDSK parameter detected:  PMchkdsk mode\n\r"
/*335*/
   ,"CHKDSK parameter detected:  Debug mode\n\r"
/*336*/
   ,"CHKDSK  CHKDSK.SYS returned %s bytes of high memory at %s\n\r"
/*337*/
   ,"CHKDSK  DosOpen(CHKDSK.SYS,...) returned rc = %s\n\r"
/*338*/
   ,"CHKDSK  DosDevIOCtl( ... ) to call CHKDSK.SYS returned rc = %s, data.rc = %s\n\r"
/*339*/
   ,"CHKDSK  ujfs_beginformat() returned rc = %s\n\r"
/*340*/
   ,"CHKDSK parameter detected:  Clear LVM Bad Block List\n\r"
/*341*/
   ,"CHKDSK (JFSCTL_CHKDSK, ... ) returned rc = %s\n\r"
/*342*/
   ,"CHKDSK   CHKDSK warning: The LVM has detected bad blocks in the partition.  \
Run CHKDSK /B to transfer entries from the LVM bad block table to the JFS bad \
block table.\n\r"
/*343*/
   ,"CHKDSK   LVM GetBadBlockInformation returned rc = %s\n\r"
/*344*/
   ,"CHKDSK   LVM GetTableSize rc = %s\n\r"
/*345*/
   ,"CHKDSK  THE FILE SYSTEM IS NOW AVAILABLE.\n\r"
/*346*/
   ,"CHKDSK  Transferring entries from the LVM Bad Block Table for this file \
system to the JFS Bad Block Table for this file system.\n\r"
/*347*/
   ,"CHKDSK   INTERNAL ERROR (%s,%s,%s,%s). CHKDSK CANNOT CONTINUE.\n\r"
/*348*/
   ,"CHKDSK   CLRBBLKS:  relocated %s extents, for total %s relocated blocks.\n\r"
/*349*/
   ,"CHKDSK   CLRBBLKS: LVM List #%s  num entries = %s  num active entries = %s\n\r"
/*350*/
   ,"CHKDSK   CLRBBLKS: number of LVM Lists = %s\n\r"
/*351*/
   ,"CHKDSK   CLRBBLKS: Attempted to relocate bad blocks in range %s to %s.\n\r"
/*352*/
   ,"CHKDSK   LVM reports %s bad blocks.  Of these, %s have been transferred to \
the JFS Bad Block List.\n\r"
/*353*/
   ,"CHKDSK   CLRBBLKS rc = %s\n\r"
/*354*/
   ,"CHKDSK   DosExecPgm(...clrbblks...) rc = %s\n\r"
/*355*/
   ,"CHKDSK  DosOpen(...OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE...) returned rc = %s\n\r"
/*356*/
   ,"CHKDSK  Device unavailable or locked by another process.  CHKDSK CANNOT CONTINUE.\n\r"
/*357*/
   ,"CHKDSK  File system object %s%s%s has a node with an invalid self field.\n\r"
/*358*/
   ,"CHKDSK  File system object %s%s%s is illegally linked as: %s\n\r"
/*359*/
   ,"CHKDSK  Insufficient storage (%s) available to continue reconnecting \
lost files and/or directories. Continuing.\n\r"
/*360*/
   ,"CHKDSK  Format error in Access Control List space or descriptor.\n\r"
/*361*/
   ,"CHKDSK  Format error in Access Control List space or descriptor. Will clear.\n\r"
/*362*/
   ,"CHKDSK  Storage allocated to Access Control List for inode \
%s%s has been cleared.\n\r"
/*363*/
   ,"%s kilobytes in access control lists\n\r"
/*364*/
   ,"CHKDSK  File system is full or too fragmented to allocated requested %s blocks.\n\r"
/*365*/
   ,"CHKDSK  Minor format error detected.\n\r"
/*366*/
   ,"CHKDSK   Minor format error in inode %s%s has been corrected. \n\r"
/*367*/
   ,"CHKDSK  Minor format error detected. Will fix.\n\r"
/*368*/
   ,"CHKDSK  File system object %s%s%s has invalid length in descriptor (%s).\n\r"
/*369*/
   ,"CHKDSK  File system object %s%s%s has invalid offset in descriptor (%s).\n\r"
/*370*/
   ,"CHKDSK   *undefined* \n\r"
/*371*/
   ,"CHKDSK   *undefined* \n\r"
/*372*/
   ,"CHKDSK   *undefined* \n\r"
/*373*/
   ,"CHKDSK   *undefined* \n\r"
/*374*/
   ,"CHKDSK   *undefined* \n\r"
/*375*/
   ,"CHKDSK   *undefined* \n\r"
/*376*/
   ,"CHKDSK   *undefined* \n\r"
/*377*/
   ,"CHKDSK   *undefined* \n\r"
/*378*/
   ,"CHKDSK   *undefined* \n\r"
/*379*/
   ,"CHKDSK   *undefined* \n\r"
/*380*/
   ,"CHKDSK   *undefined* \n\r"
/*381*/
   ,"CHKDSK   *undefined* \n\r"
/*382*/
   ,"CHKDSK   *undefined* \n\r"
/*383*/
   ,"CHKDSK   *undefined* \n\r"
/*384*/
   ,"CHKDSK   *undefined* \n\r"
/*385*/
   ,"A" 
/*386*/
   ,"D"      
/*387*/
   ,"DM"         
/*388*/
   ,"E"     
/*389*/
   ,"F" 
/*390*/
   ,"I"         
/*391*/
   ,"L0"      
/*392*/
   ,"L1"      
/*393*/
   ,"L2"      
/*394*/
   ,"M"            
/*395*/
   ,"C"  
/*396*/
   ,"P"        
/*397*/
   ,"S"        
/*398*/
   ,"L"       
/*399*/
   ,".RCN\0"     
/*400*/
   ,"CHKDSK:LOGREDO:  Log already redone! \n\r"
/*401*/
   ,"CHKDSK:LOGREDO:  End of log found at logend = %s\n\r"
/*402*/
   ,"CHKDSK:LOGREDO:  Beginning to recover File System Extend processing.\n\r"
/*403*/
   ,"CHKDSK:LOGREDO:  Done recovering File System Extend processing.\n\r"
/*404*/
   ,"CHKDSK:LOGREDO:  File System Extend recovered to prior size.\n\r"
/*405*/
   ,"CHKDSK:LOGREDO:  File System Extend recovered and completed.\n\r"
/*406*/
   ,"CHKDSK:LOGREDO:  Recovery of File System Extend failed.  Error = %s\n\r"
/*407*/
   ,"CHKDSK:LOGREDO:  Log superblock contains invalid magic number. \n\r"
/*408*/
   ,"CHKDSK:LOGREDO:  Log superblock contains invalid version number. \n\r"
/*409*/
   ,"CHKDSK:LOGREDO:  Unable to find valid end of log.\n\r"
/*410*/
   ,"CHKDSK:LOGREDO:  Initialization failed.  rc = %s   error num = %s\n\r"
/*411*/
   ,"CHKDSK:LOGREDO:  Log superblock contains invalid block size.\n\r"
/*412*/
   ,"CHKDSK:LOGREDO:  Log superblock contains invalid log2(block size).\n\r"
/*413*/
   ,"CHKDSK:LOGREDO:  Log superblock contains invalid log size.\n\r"
/*414*/
   ,"CHKDSK:LOGREDO:  Log end is not in valid range.\n\r"
/*415*/
   ,"CHKDSK:LOGREDO:  Synch point record number:  %s \n\r"
/*416*/
   ,"CHKDSK:LOGREDO:  Synch point record address:  %s \n\r"
/*417*/
   ,"CHKDSK:LOGREDO:  Number of log records:    %s \n\r"
/*418*/
   ,"CHKDSK:LOGREDO:  Number of Do blocks:    %s \n\r"
/*419*/
   ,"CHKDSK:LOGREDO:  Number of NoDo blocks:    %s \n\r"
/*420*/
   ,"CHKDSK:LOGREDO:  Log record for Sync Point at:    %s \n\r"
/*421*/
   ,"CHKDSK:LOGREDO:  Log record for Volume Mount at:    %s \n\r"
/*422*/
   ,"CHKDSK:  Calling CHKDSK.SYS to allocate high memory.... \n\r"
/*423*/
   ,"CHKDSK   CHKDSK.SYS returned  addr= %s  length= %s \n\r"
/*424*/
   ,"CHKDSK:LOGREDO:  Allocating for BMap:  %s bytes \n\r"
/*425*/
   ,"CHKDSK:LOGREDO:  Allocating for IMap:  %s bytes \n\r"
/*426*/
   ,"CHKDSK:LOGREDO:  Allocating for DoBLk:  %s bytes  \n\r"
/*427*/
   ,"CHKDSK:LOGREDO:  Allocating for NoReDoFile:  %s bytes  \n\r"
/*428*/
   ,"CHKDSK:LOGREDO:  Allocating for ReDoPage:  %s bytes  \n\r"
/*429*/
   ,"CHKDSK:LOGREDO:  Allocating for NoDoFile:  %s bytes  \n\r"
/*430*/
   ,"CHKDSK:LOGREDO:  Allocating for ExtendDtPg:  %s bytes  \n\r"
/*431*/
   ,"CHKDSK:LOGREDO:   Failed trying to allocate for BMap:  %s bytes \n\r"
/*432*/
   ,"CHKDSK:LOGREDO:  Failed trying to allocate for IMap:  %s bytes  \n\r"
/*433*/
   ,"CHKDSK:LOGREDO:  Failed trying to allocate for DoBLk:  %s bytes   \n\r"
/*434*/
   ,"CHKDSK:LOGREDO:  Failed trying to allocate for NoReDoFile:  %s bytes \n\r"
/*435*/
   ,"CHKDSK:LOGREDO:  Failed trying to allocate for ReDoPage:  %s bytes \n\r"
/*436*/
   ,"CHKDSK:LOGREDO:  Failed trying to allocate for NoDoFile:  %s bytes \n\r"
/*437*/
   ,"CHKDSK:LOGREDO:  Failed trying to allocate for ExtendDtPg:  %s bytes \n\r"
/*438*/
   ,"CHKDSK:LOGREDO:  Working IMap will occupy storage originally allocated for BMap.\n\r"
/*439*/
   ,"CHKDSK:LOGREDO:  Some storage allocated for BMap now used for NoRedoFile records.\n\r"
/*440*/
   ,"CHKDSK:LOGREDO:  Some storage allocated for BMap now used for DoBLock records.\n\r"
/*441*/
   ,"CHKDSK:LOGREDO:  Some storage allocated for BMap now used for RedoPage records.\n\r"
/*442*/
   ,"CHKDSK:LOGREDO:  Some storage allocated for BMap now used for NoDoFile records.\n\r"
/*443*/
   ,"CHKDSK:LOGREDO:  Some storage allocated for BMap now used for ExtendDtPage records.\n\r"
/*444*/
   ,"CHKDSK:LOGREDO:  Unable to read Journal Log superblock.\n\r"
/*445*/
   ,"CHKDSK:LOGREDO:  Unable to update Journal Log superblock.\n\r"
/*446*/
   ,"CHKDSK:LOGREDO:  Address of next log record is invalid.\n\r"
/*447*/
   ,"CHKDSK:LOGREDO:  Address (%s) of next log record is out of range.\n\r"
/*448*/
   ,"CHKDSK:LOGREDO:  Address (%s) of next log record is the same as address of current log record.\n\r"
/*449*/
   ,"CHKDSK:LOGREDO:  The Journal Log has wrapped around and written over itself.\n\r"
/*450*/
   ,"CHKDSK:LOGREDO:  Invalid Commit record at %s.\n\r"
/*451*/
   ,"CHKDSK:LOGREDO:  Invalid Mount record at %s.\n\r"
/*452*/
   ,"CHKDSK:LOGREDO:  Device open/read error\n\r"
/*453*/
   ,"CHKDSK:LOGREDO:  Invalid RedoPage record at %s. \n\r"
/*454*/
   ,"CHKDSK:LOGREDO:  Invalid NoRedoPage record at %s.\n\r"
/*455*/
   ,"CHKDSK:LOGREDO:  Invalid NoRedoInoExt record at %s.\n\r"
/*456*/
   ,"CHKDSK:LOGREDO:  Invalid UpdateMap record at %s.\n\r"
/*457*/
   ,"CHKDSK:LOGREDO:  Unrecognized log record type at %s.\n\r"
/*458*/
   ,"CHKDSK:LOGREDO:  Errors encountered imply the log is corrupt.  Reformat needed.\n\r"
/*459*/
   ,"CHKDSK:LOGREDO:  Errors encountered prevent logredo() from continuing. \n\r"
/*460*/
   ,"CHKDSK:LOGREDO:  Unable to update map(s).\n\r"
/*461*/
   ,"CHKDSK:LOGREDO:  Unable to update file system superblock.\n\r"
/*462*/
   ,"CHKDSK:LOGREDO:  Unable to write to log superblock.\n\r"
/*463*/
   ,"CHKDSK:LOGREDO:  Unable to read file system superblock. \n\r"
/*464*/
   ,"CHKDSK:LOGREDO:  File system superblock has invalid magic number.\n\r"
/*465*/
   ,"CHKDSK:LOGREDO:  File system superblock has invalid version number.\n\r"
/*466*/
   ,"CHKDSK:LOGREDO:  File system superblock has invalid log location.\n\r"
/*467*/
   ,"CHKDSK:LOGREDO:  File system superblock has invalid log serial number.\n\r"
/*468*/
   ,"CHKDSK:LOGREDO:  Unable to initialize map(s).\n\r"
/*469*/
   ,"CHKDSK:LOGREDO:  Attempt to read file system superblock failed.\n\r"
/*470*/
   ,"CHKDSK:LOGREDO:  Attempt to write file system superblock failed.\n\r"
/*471*/
   ,"CHKDSK:LOGREDO:  Attempt to read file system primary superblock failed.\n\r"
/*472*/
   ,"CHKDSK:LOGREDO:  Attempt to read file system secondary superblock failed.\n\r"
/*473*/
   ,"CHKDSK:LOGREDO:  Attempt to write file system primary superblock failed.\n\r"
/*474*/
   ,"CHKDSK:LOGREDO:  Attempt to write file system secondary superblock failed.\n\r"
/*475*/
   ,"CHKDSK:LOGREDO:  Attempt to flush buffers failed.\n\r"
/*476*/
   ,"CHKDSK:LOGREDO:  Not a valid file system device number.\n\r"
/*477*/
   ,"CHKDSK:LOGREDO:  Journal log must be inline. \n\r"
/*478*/
   ,"CHKDSK:LOGREDO:  Device is not a file system device.\n\r"
/*479*/
   ,"CHKDSK:LOGREDO:  Journal log is not inline.\n\r"
/*480*/
   ,"CHKDSK:LOGREDO:  Journal log is now open.  (major = %s, minor = %s) \n\r"
/*481*/
   ,"CHKDSK:LOGREDO:  Error in volume %s.\n\r"
/*482*/
   ,"CHKDSK:LOGREDO:  Device open failed.\n\r"
/*483*/
   ,"CHKDSK:LOGREDO:  Failed attempting to initialize map(s).\n\r"
/*484*/
   ,"CHKDSK:LOGREDO:  Invalid volume block number (%s).\n\r"
/*485*/
   ,"CHKDSK:LOGREDO:  Invalid inode number (%s).\n\r"
/*486*/
   ,"CHKDSK:LOGREDO:  Failed attempt to read block %s.\n\r"
/*487*/
   ,"CHKDSK:LOGREDO:  I/O error attempting to read block %s.\n\r"
/*488*/
   ,"CHKDSK:LOGREDO:  Invalid UpdateMap record (nxd = %s). \n\r"
/*489*/
   ,"CHKDSK:LOGREDO:  Invalid log serial number.\n\r"
/*490*/
   ,"CHKDSK:LOGREDO:  Failed attempt to find the current end of the Journal Log.\n\r"
/*491*/
   ,"CHKDSK:LOGREDO:  Failed attempting to read the Journal Log (logaddr = %s).\n\r"
/*492*/
   ,"CHKDSK:LOGREDO:  Unrecognized log record type (logaddr = %s).\n\r"
/*493*/
   ,"CHKDSK:LOGREDO:  I/O error on Journal Log (logaddr = %s).\n\r"
/*494*/
   ,"CHKDSK:LOGREDO:  The Journal Log has wrapped. \n\r"
/*495*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Attempt to read file system superblock failed.\n\r"
/*496*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Attempt to read block map inode failed.\n\r"
/*497*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Read aggregate block %s failed (1).\n\r"
/*498*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Read aggregate block %s failed (2).\n\r"
/*499*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Write aggregate block %s failed (1).\n\r"
/*500*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Write aggregate block %s failed (2).\n\r"
/*501*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Write aggregate block %s failed (3).\n\r"
/*502*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Read aggregate block %s failed (3).\n\r"
/*503*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Read aggregate block %s failed (4).\n\r"
/*504*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Error in initLogredo().\n\r"
/*505*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Read aggregate block %s failed (5).\n\r"
/*506*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Write aggregate block %s failed (4).\n\r"
/*507*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Attempt to write file system superblock failed.\n\r"
/*508*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Attempt to read Journal Log superblock failed.\n\r"
/*509*/
   ,"CHKDSK:LOGREDO:  RecovExtFS: Attempt to write Journal Log superblock failed. \n\r"
/*510*/
   ,"CHKDSK:LOGREDO:   Read Block Map inode failed.\n\r"
/*511*/
   ,"CHKDSK:LOGREDO:   Read Block Map failed.\n\r"
/*512*/
   ,"CHKDSK:LOGREDO:   Read Inode Alloc Map inode failed.\n\r"
/*513*/
   ,"CHKDSK:LOGREDO:   Read Inode Alloc Map failed.\n\r"
/*514*/
   ,"CHKDSK:LOGREDO:   Read Block Map leftmost leaf failed.\n\r"
/*515*/
   ,"CHKDSK:LOGREDO:   Read Block Map data extents failed.\n\r"
/*516*/
   ,"CHKDSK:LOGREDO:   Read Block Map next leaf failed.\n\r"
/*517*/
   ,"CHKDSK:LOGREDO:   Read Inode Alloc Map leftmost leaf failed.\n\r"
/*518*/
   ,"CHKDSK:LOGREDO:   Read Inode Alloc Map control page failed.\n\r"
/*519*/
   ,"CHKDSK:LOGREDO:   Read Inode Alloc Map next leaf failed.\n\r"
/*520*/
   ,"CHKDSK:LOGREDO:   Read Inode Alloc Map data extents failed.\n\r"
/*521*/
   ,"CHKDSK:LOGREDO:   Read Inode Alloc Map inode failed in UpdateMaps().\n\r"
/*522*/
   ,"CHKDSK:LOGREDO:  Write Inode Alloc Map control page failed in UpdateMaps().\n\r"
/*523*/
   ,"CHKDSK:LOGREDO:  Read Block Map inode failed in UpdateMaps().\n\r"
/*524*/
   ,"CHKDSK:LOGREDO:  Write Block Map control page failed in UpdateMaps().\n\r"
/*525*/
   ,"CHKDSK:LOGREDO:  Not rebuilding the Inode Alloc Map because chkdsk will be doing it.\n\r"
/*526*/
   ,"CHKDSK:LOGREDO:  Write Inode Alloc Map, rXtree() failed.\n\r"
/*527*/
   ,"CHKDSK:LOGREDO:  Write block (blk=%s) failed when updating the Inode Alloc Mapl.\n\r"
/*528*/
   ,"CHKDSK:LOGREDO:  Read next leaf (addr=%s) failed when updating the Inode Alloc Map.\n\r"
/*529*/
   ,"CHKDSK:LOGREDO:  Incorrect npages detected when updating the Inode Alloc Map.\n\r"
/*530*/
   ,"CHKDSK:LOGREDO:  Done updating the Inode Allocation Map.\n\r"
/*531*/
   ,"CHKDSK:LOGREDO:  Beginning to update the Inode Allocation Map.\n\r"
/*532*/
   ,"CHKDSK:LOGREDO:  Done updating the Block Map.\n\r"
/*533*/
   ,"CHKDSK:LOGREDO:  Beginning to update the Block Map.\n\r"
/*534*/
   ,"CHKDSK:LOGREDO:  Not rebuilding the Block Map because chkdsk will be doing it.\n\r"
/*535*/
   ,"CHKDSK:LOGREDO:  Error for pmap and inoext when rebuilding the Inode Allocation Map (1).\n\r"
/*536*/
   ,"CHKDSK:LOGREDO:  Inconsistent map size in Block Map.\n\r"
/*537*/
   ,"CHKDSK:LOGREDO:  Incorrect leaf index detected (k=%s, j=%s, idx=%s) while writing Block Map.\n\r"
/*538*/
   ,"CHKDSK:LOGREDO:  Error rebuilding DMap page (k=%s, j=%s, i=%s).\n\r"
/*539*/
   ,"CHKDSK:LOGREDO:  rXtree() failed when called while writing Block Map.\n\r"
/*540*/
   ,"CHKDSK:LOGREDO:  Write block (at %s) failed while writing Block Map.\n\r"
/*541*/
   ,"CHKDSK:LOGREDO:  Read next leaf (at %s) failed while writing Block Map.\n\r"
/*542*/
   ,"CHKDSK:LOGREDO:  Incorrect total pages while writing Block Map.\n\r"
/*543*/
   ,"CHKDSK:LOGREDO:  Invalid leaf index detected while updating dmap page.\n\r"
/*544*/
   ,"CHKDSK:LOGREDO:  rXtree() Read first leaf failed.\n\r"
/*545*/
   ,"CHKDSK:LOGREDO:  bread() Invalid block number specified (%s).\n\r"
/*546*/
   ,"CHKDSK:LOGREDO:  bread() Read block (%s) failed.\n\r"
/*547*/
   ,"CHKDSK:LOGREDO:  Error for pmap and inoext when rebuilding the Inode Allocation Map (2).\n\r"
/*548*/
   ,"CHKDSK:LOGREDO:   doAfter: updatePage failed.  (logaddr = %s, rc = %s)\n\r"
/*549*/
   ,"CHKDSK:LOGREDO:   doAfter: markBmap failed.  (logaddr = %s, rc = %s)\n\r"
/*550*/
   ,"CHKDSK:LOGREDO:   doExtDtPg: bread failed.  (offset = %s, rc = %s)\n\r"
/*551*/
   ,"CHKDSK:LOGREDO:   doNoRedoPage: type dtpage: findPageRedo failed.  (rc = %s)\n\r"
/*552*/
   ,"CHKDSK:LOGREDO:   doNoRedoPage: type dtroot: findPageRedo failed.  (rc = %s)\n\r"
/*553*/
   ,"CHKDSK:LOGREDO:   doNoRedoPage: type xtroot: findPageRedo failed.  (rc = %s)\n\r"
/*554*/
   ,"CHKDSK:LOGREDO:   doNoRedoPage: type xtpage: findPageRedo failed.  (rc = %s)\n\r"
/*555*/
   ,"CHKDSK:LOGREDO:   doNoRedoPage: Unknown NoRedoPage record type.\n\r"
/*556*/
   ,"CHKDSK:LOGREDO:   doNoRedoInoExt: findPageRedo failed.  (rc = %s)\n\r"
/*557*/
   ,"CHKDSK:LOGREDO:   doUpdateMap: Unknown updateMap record type.\n\r"
/*558*/
   ,"CHKDSK:LOGREDO:   dtpg_resetFreeList: Invalid stbl entry.\n\r"
/*559*/
   ,"CHKDSK:LOGREDO:   dtpg_resetFreeList: Invalid slot next index.\n\r"
/*560*/
   ,"CHKDSK:LOGREDO:   dtrt_resetFreeList: Invalid stbl entry.\n\r"
/*561*/
   ,"CHKDSK:LOGREDO:   dtrt_resetFreeList: Invalid slot next index.\n\r"
/*562*/
   ,"CHKDSK:LOGREDO:   markBmap: Block number(s) out of range (start=%s, length=%s). \n\r"
/*563*/
   ,"CHKDSK:LOGREDO:   updatePage: findPageRedo failed.  (rc = %s)\n\r"
/*564*/
   ,"CHKDSK:LOGREDO:   updatePage: type=INODE.  Invalid segment offset. (offset = %s)\n\r"
/*565*/
   ,"CHKDSK:LOGREDO:   updatePage: bread failed (1).  (rc = %s)\n\r"
/*566*/
   ,"CHKDSK:LOGREDO:   updatePage: bread failed (2).  (rc = %s)\n\r"
/*567*/
   ,"CHKDSK:LOGREDO:   updatePage: bread failed (3).  (rc = %s)\n\r"
/*568*/
   ,"CHKDSK:LOGREDO:   updatePage: bread failed (4).  (rc = %s)\n\r"
/*569*/
   ,"CHKDSK:LOGREDO:   updatePage: markImap failed.  (rc = %s)\n\r"
/*570*/
   ,"CHKDSK:LOGREDO:   updatePage: markBmap failed.  (rc = %s)\n\r"
/*571*/
   ,"CHKDSK:LOGREDO:   updatePage: dtrt_resetFreeList failed.  (rc = %s)\n\r"
/*572*/
   ,"CHKDSK:LOGREDO:   updatePage: dtpg_resetFreeList failed.  (rc = %s)\n\r"
/*573*/
   ,"CHKDSK:LOGREDO:   updatePage: saveExtDtPg failed.  (rc = %s)\n\r"
/*574*/
   ,"CHKDSK:LOGREDO:   fineEndOfLog: pageVal[1] failed (rc = %s).\n\r"
/*575*/
   ,"CHKDSK:LOGREDO:   fineEndOfLog: pageVal[2] failed (rc = %s).\n\r"
/*576*/
   ,"CHKDSK:LOGREDO:   fineEndOfLog: pageVal[3] failed (rc = %s).\n\r"
/*577*/
   ,"CHKDSK:LOGREDO:   fineEndOfLog: pageVal[4] failed (rc = %s).\n\r"
/*578*/
   ,"CHKDSK:LOGREDO:   fineEndOfLog: pageVal[4(a)] failed (rc = %s).\n\r"
/*579*/
   ,"CHKDSK:LOGREDO:   pageVal: getLogpage failed (pno = %s, rc = %s).\n\r"
/*580*/
   ,"CHKDSK:LOGREDO:   setLogpage: Write log page failed (pno = %s, rc = %s).\n\r"
/*581*/
   ,"CHKDSK:LOGREDO:   logRead: Log wrapped over itself (lognumread = %s).\n\r"
/*582*/
   ,"CHKDSK:LOGREDO:   logRead: Read log page failed (pno = %s, rc = %s).\n\r"
/*583*/
   ,"CHKDSK:LOGREDO:   logRead: MoveWords[1] failed (rc = %s).\n\r"
/*584*/
   ,"CHKDSK:LOGREDO:   logRead: MoveWords[2] failed (rc = %s).\n\r"
/*585*/
   ,"CHKDSK:LOGREDO:   moveWords: getLogpage failed (pno = %s, rc = %s).\n\r"
/*586*/
   ,"CHKDSK  DASD Limit has been primed for directories on the volume.\n\r"    /* @F1*/
/*587*/
   ,"CHKDSK  DASD Used has been primed for directories on the volume.\n\r"    /* @F1*/
/*588*/
   ,"CHKDSK:LOGREDO:   logRead: Log data must not exceed LOGPSIZE (pno = %s).\n\r"      /* @D1 */
/*589*/
   ,"CHKDSK   *undefined* \n\r"
/*590*/
   ,"CHKDSK   *undefined* \n\r"
/*591*/
   ,"CHKDSK   *undefined* \n\r"
/*592*/
   ,"CHKDSK   *undefined* \n\r"
/*593*/
   ,"CHKDSK   *undefined* \n\r"
/*594*/
   ,"CHKDSK   *undefined* \n\r"
/*595*/
   ,"CHKDSK   *undefined* \n\r"
/*596*/
   ,"CHKDSK   *undefined* \n\r"
/*597*/
   ,"CHKDSK   *undefined* \n\r"
/*598*/
   ,"CHKDSK   *undefined* \n\r"
/*599*/
   ,"CHKDSK   *undefined* \n\r"
   };

#endif
