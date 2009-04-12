/* $Id: fsckmsgl.h,v 1.2 2004/03/21 02:43:26 pasha Exp $ */

/* static char *SCCSID = "@(#)1.23.1.2  12/2/99 11:13:51 src/jfs/utils/chkdsk/fsckmsgl.h, jfschk, w45.fs32, fixbld";*/
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
 *   MODULE_NAME:		fsckmsgl.h
 *
 *   COMPONENT_NAME: 	jfschk
 *
 */
#ifndef H_FSCKMSGL
#define H_FSCKMSGL

#define LOG10000 10000 /* fsck_MSGOK
                               Operation was successful.
#define LOG10001 10001 /* fsck_ALLFXD
                               All observed inconsistencies have been repaired.
#define LOG10002 10002 /* fsck_RIBADDATFMT
                               Invalid data format detected in root directory.
#define LOG10003 10003 /* fsck_BADBLKCTTTL
                               A combination of Minimum Free Blocks and Total Usable Blocks 
                               which is invalid for the filesystem size was detected in the 
                               superblock (%s).
#define LOG10004 10004 /* fsck_BADBLKNO
                               Invalid block number(s) (%s) detected for file system object %s%s%s.
#define LOG10005 10005 /* fsck_BADBSBLCHN
                               File system object %s%s%s has a corrupt backward sibling chain.
#define LOG10006 10006 /* fsck_BADFSBLCHN
                               File system object %s%s%s has a corrupt forward sibling chain.
#define LOG10007 10007 /* fsck_BADINOTYP
                               Inode %s%s has unrecognized type.
#define LOG10008 10008 /* fsck_BADINODXDFLDO
                               File system object %s%s%s has invalid descriptor (%s).
#define LOG10009 10009 /* fsck_BADINOLKCT
                               Inode %s%s has incorrect link count.
#define LOG10010 10010 /* fsck_BADINOREF
                               Directory inode %s%s refers to a nonexistent inode %s%s 
                               (entry %s).
#define LOG10011 10011 /* fsck_ERRONLOG
                               Error (%s,%s) writing to the chkdsk service log 
                               (%s,%s,%s,%s).  Continuing.
#define LOG10012 10012 /* fsck_BOOTSECFXD
                               The boot sector has been refreshed.
#define LOG10013 10013 /* fsck_BADKEYS
                               File system object %s%s%s has corrupt data (%s).
#define LOG10014 10014 /* fsck_BADSBOTHR
                               Invalid data (%s) detected in the superblock (%s).
#define LOG10015 10015 /* fsck_BADSBAGSIZ
                               Invalid allocation group size in the superblock (%s).
#define LOG10016 10016 /* fsck_BADSBBLSIZ
                               Invalid filesystem block size in the superblock (%s).
#define LOG10017 10017 /* fsck_BADSBFSSIZ
                               Invalid filesystem size in the superblock (%s).
#define LOG10018 10018 /* fsck_BDSBBTHCRRPT
                               Superblock is corrupt and cannot be repaired since both primary
                               and secondary copies are corrupt.  CHKDSK CANNOT CONTINUE.
#define LOG10019 10019 /* fsck_BDSBNWRTACC
                               Primary superblock is corrupt and cannot be repaired without 
                               write access. Continuing. 
#define LOG10020 10020 /* fsck_CHKLOGUSAGE
                               ***** CHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10021 10021 /* fsck_BADSBMGC
                               Invalid magic number in the superblock (%s).
#define LOG10022 10022 /* fsck_BADSBVRSN
                               Invalid version number in the superblock (%s).
#define LOG10023 10023 /* fsck_BMAPCBMXB
                               Incorrect maxbud AG detected in Block Map Control Page.
#define LOG10024 10024 /* fsck_BMAPCNF
                               Incorrect number of free blocks detected in Block Map Control 
                               Page.
#define LOG10025 10025 /* fsck_BLSIZLTLVBLSIZ
                               In superblock (%s) filesystem block size smaller than 
                               volume block size.
#define LOG10026 10026 /* fsck_RIBADFMT
                               Invalid format detected in Root directory.
#define LOG10027 10027 /* fsck_DEFAULTVOL
                               Device parm defaulting to current volume: %1
#define LOG10028 10028 /* fsck_CNTWRTBSMBR
                               Unable to write to boot sector Master Boot Record.  Continuing.
#define LOG10029 10029 /* fsck_INCONSIST2NDRY
                               Secondary file/directory allocation structure (%s) is 
                               not a correct redundant copy of primary structure.
#define LOG10030 10030 /* fsck_INCONSIST2NDRY1
                               Unable to replicate primary file/directory allocation structure
                               (%s) to secondary.  FUTURE RECOVERY CAPABILITY IS COMPROMISED.
#define LOG10031 10031 /* fsck_CHKLOGNEW
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10032 10032 /* fsck_LSFNCNTCRE
                               MINOR: Cannot create directory lost+found in root directory.  
                               mkdir lost+found in the root directory then run chkdsk with the
                               F parameter to reconnect lost files and/or directories.
#define LOG10033 10033 /* fsck_INOCANTNAME
                               Fileset object %s%s%s:  No paths found.
#define LOG10034 10034 /* fsck_INOBADREF
                               The path(s) refer to an unallocated file.
#define LOG10035 10035 /* fsck_CHKLOGOLD
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10036 10036 /* fsck_WILLRMVBADREF
                               The path(s) refer to an unallocated file. Will remove.
#define LOG10037 10037 /* fsck_CANTREPAIRAIS
                               Unable to repair primary inode allocation structure (%s). Continuing.
#define LOG10038 10038 /* fsck_RIBADTREE
                               Root directory has a corrupt tree.
#define LOG10039 10039 /* fsck_RICRETREE
                               Initialized tree created for root directory.
#define LOG10040 10040 /* fsck_CNTRESUPP
                               Unable to read primary superblock.
#define LOG10041 10041 /* fsck_CHKLOGINVALRCD
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10042 10042 /* fsck_SESSSTART
                               CHKDSK processing started: %s
#define LOG10043 10043 /* fsck_SEPARATOR
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10044 10044 /* fsck_CNTWRTSUPP
                               Unable to write primary superblock.
#define LOG10045 10045 /* fsck_ERRONAGG
                               Fatal error (%s,%s) accessing the filesystem
                               (%s,%s,%s,%s).
#define LOG10046 10046 /* fsck_DIRWHDLKS
                               Multiple parent directories for directory %s%s.
#define LOG10047 10047 /* fsck_LOGSPECINVALID
                               External journal log not supported.
#define LOG10048 10048 /* fsck_DUPBLKREF
                               Duplicate reference to %s block(s) beginning at offset %s found
                               in file system object %s%s%s.
#define LOG10049 10049 /* fsck_BADMETAINOF
                               File set metadata inode %s%s is corrupt.
#define LOG10050 10050 /* fsck_ERRONWSP
                               Fatal error (%s,%s) accessing the workspace
                               (%s,%s,%s,%s).
#define LOG10051 10051 /* fsck_EXHDYNSTG
                               Insufficient dynamic storage available for required workspace
                               (%s,%s). CHKDSK CANNOT CONTINUE
#define LOG10052 10052 /* fsck_DEVGETCHARRC
                               DosDevIOCtl(...DSK_GETDEVICEPARAMS...) returned rc = %s
#define LOG10053 10053 /* fsck_FSMNTD
                               Filesystem is currently mounted.
#define LOG10054 10054 /* fsck_BADINOOTHR
                               Invalid data (%s) detected in file system object %s%s%s.
#define LOG10055 10055 /* fsck_FSSMMRY1
                               Block size in bytes:  %s
#define LOG10056 10056 /* fsck_FSSMMRY2
                               Filesystem size in blocks:  %s
#define LOG10057 10057 /* fsck_FSSMMRY3
                               Filesystem Summary:
#define LOG10058 10058 /* fsck_FSSMMRY4
                               Blocks in use for inodes:  %s
#define LOG10059 10059 /* fsck_FSSMMRY5
                               Inode count:  %s
#define LOG10060 10060 /* fsck_FSSMMRY6
                               File count:  %s
#define LOG10061 10061 /* fsck_FSSMMRY9
                               Directory count:  %s
#define LOG10062 10062 /* fsck_DUPBLKREFS
                               Inode %s%s has references to cross linked blocks.
#define LOG10063 10063 /* fsck_FSSMMRY7
                               Block count:  %s
#define LOG10064 10064 /* fsck_FSSMMRY8
                               Free block count:  %s
#define LOG10065 10065 /* fsck_ILLINOREF
                               Directory inode %s%s entry %s refers to an illegal inode %s%s.
#define LOG10066 10066 /* fsck_XCHKLOGUSAGE
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10067 10067 /* fsck_INOCNTGETPATH
                               Unable to get path for link from directory %s%s to fileset object %s%s%s.
#define LOG10068 10068 /* fsck_INOEA
                               Format error in Extended Attributes Space or descriptor.
#define LOG10069 10069 /* fsck_BADAGFELIST
                               Discrepancies detected in the Free Inode Extent List 
                               for Allocation Group %s%s. (%s)
#define LOG10070 10070 /* fsck_MNCNTRCNCTINOF
                               MINOR: Unable to reconnect file inode %s%s.  Continuing.
#define LOG10071 10071 /* fsck_INCINOREF
                               Directory %s%s entry \"..\" refers to an incorrect parent 
                               directory (%s%s).
#define LOG10072 10072 /* fsck_INCINOREFCRCT
                               Directory inode %s%s entry \"..\" reference to incorrect inode
                               corrected.
#define LOG10073 10073 /* fsck_INOCLRD
                               Storage allocated to inode %s%s has been cleared.
#define LOG10074 10074 /* fsck_INOEACLRD
                               Storage allocated to extended attributes for inode %s%s has been
                               cleared.
#define LOG10075 10075 /* fsck_INOINLSFNF
                               File inode %s%s has been reconnected to /lost+found/.
#define LOG10076 10076 /* fsck_INOLKCTFXD
                               Link count for inode %s%s has been adjusted/corrected.
#define LOG10077 10077 /* fsck_DEVLOCKRC
                               DosDevIOCtl(...DSK_LOCKDRIVE...) returned rc = %s
#define LOG10078 10078 /* fsck_INONOPATHS
                               No paths were found for inode %s%s.
#define LOG10079 10079 /* fsck_INOPATHOK
                               File system object %s%s%s is linked as: %s
#define LOG10080 10080 /* fsck_INOREFRMV
                               Directory inode %s%s entry reference to inode %s%s removed.
#define LOG10081 10081 /* fsck_BADSBFWSL
                               Invalid fwsp length detected in the superblock (%s).
#define LOG10082 10082 /* fsck_BADSBFWSA
                               Invalid fwsp address detected in the superblock (%s).
#define LOG10083 10083 /* fsck_DEVOPENRDRC
                               DosOpen(...OPEN_SHARE_DENYNONE|OPEN_ACCESS_READONLY...) 
                               returned rc = %s
#define LOG10084 10084 /* fsck_CANTCHKEA
                               MINOR: Insufficient dynamic storage to validate extended 
                               attributes format.
#define LOG10085 10085 /* fsck_LOGREDOFAIL
                               logredo failed (rc=%s).  chkdsk continuing.
#define LOG10086 10086 /* fsck_LOGREDORC
                               logredo returned rc = %s
#define LOG10087 10087 /* fsck_LSFNNOTDIR
                               Unable to create a lost+found directory in root because root 
                               already contains a non-directory object named lost+found.  
                               Rename the existing lost+found object in root, mkdir lost+found
                               in the root directory, then run chkdsk with the F parameter to 
                               reconnect lost files and/or directories.
#define LOG10088 10088 /* fsck_MNTFSYS2
                               Checking a mounted filesystem does not produce dependable 
                               results.
#define LOG10089 10089 /* fsck_MODIFIED
                               **** Filesystem was modified. ****
#define LOG10090 10090 /* fsck_DEVREDETERMRC
                               ujfs_redeterminemedia() returned rc = %s
#define LOG10091 10091 /* fsck_MRKSBDONE
                               Superblock marked.
#define LOG10092 10092 /* fsck_MSSNGBLKS
                               %s blocks are missing.
#define LOG10093 10093 /* fsck_CNTWRTBS
                               Unable to write to boot sector.  Continuing.
#define LOG10094 10094 /* fsck_LSFNNOTFOUND
                               No \\lost+found directory found in the filesystem.
#define LOG10095 10095 /* fsck_BADLINKCTS
                               Incorrect link counts detected in the aggregate.
#define LOG10096 10096 /* fsck_BADIAMAGNBI
                               The Inode Allocation Map control information has 
                               an incorrect number of backed inodes value for AG %s%s.
#define LOG10097 10097 /* fsck_BADIAMAGNFI
                               The Inode Allocation Map control information has 
                               an incorrect number of free inodes value for AG %s%s.
#define LOG10098 10098 /* fsck_URCVREAD
                               Unrecoverable error reading %s from %s. 
                               CHKDSK CANNOT CONTINUE.
#define LOG10099 10099 /* fsck_PHASE0
                               **Phase 0 - Replay Journal Log
#define LOG10100 10100 /* fsck_PHASE1
                               **Phase 1 - Check Blocks, Files/Directories, and 
                               Directory Entries
#define LOG10101 10101 /* fsck_PHASE2
                               **Phase 2 - Count links
#define LOG10102 10102 /* fsck_PHASE3
                               **Phase 3 - Duplicate Block Rescan and Directory 
                               Connectedness
#define LOG10103 10103 /* fsck_PHASE4
                               **Phase 4 - Report Problems
#define LOG10104 10104 /* fsck_PHASE5
                               **Phase 5 - Check Connectivity
#define LOG10105 10105 /* fsck_PHASE6
                               **Phase 6 - Perform Corrections
#define LOG10106 10106 /* fsck_PHASE7R
                               **Phase 7 - Rebuild File/Directory Allocation Maps
#define LOG10107 10107 /* fsck_PHASE8R
                               **Phase 8 - Rebuild Disk Allocation Maps
#define LOG10108 10108 /* fsck_PHASE9
                               **Phase 9 - Reformat File System Log
#define LOG10109 10109 /* fsck_WILLRMVBADENTRY
                               Directory has entry for unallocated file %s%s. Will remove.
#define LOG10110 10110 /* fsck_ERRONAITRD
                               FATAL ERROR (%s,%s) ON READ Aggregate Inode Table (%s) 
                               first extent.
#define LOG10111 10111 /* fsck_WILLCLEAREA
                               Format error in Extended Attributes Space or descriptor. 
                               Will clear.
#define LOG10112 10112 /* fsck_BADIAMIAGPXDL
                               The extent descriptor for inodes %s through %s is invalid.
                               Inode Allocation Map %s%s, 
                               Inode Allocation Group %s, 
                               Extent Index %s
#define LOG10113 10113 /* fsck_EAFORMATBAD
                               Extended attributes for file set object %s%s%s have an invalid format.
#define LOG10114 10114 /* fsck_REPAIRSINPROGRESS    
                               Superblock marked dirty because repairs are about to be written.
#define LOG10115 10115 /* fsck_PRMMUTEXCLLEVELS
                               Mutually exclusive /F parameters specified.
#define LOG10116 10116 /* fsck_PRMUSAGE
                               Usage: chkdsk [/f[:0|:1|:2|:3]] [/c] [/v] [/o] Device
#define LOG10117 10117 /* fsck_PRMUNRECOPTION
                               Unrecognized CHKDSK F parameter value detected:   %s
#define LOG10118 10118 /* fsck_PRMUNSUPPENUM
                               Unsupported CHKDSK parameter:   %s
#define LOG10119 10119 /* fsck_LOGFORMATFAIL
                               logformat failed (rc=%s).  chkdsk continuing.
#define LOG10120 10120 /* fsck_LOGFORMATRC
                               logformat returned rc = %s
#define LOG10121 10121 /* fsck_CNTRDDEVCHAR
                               Unable to read device characteristics.  Boot sector cannot be 
                               refreshed.  Continuing.
#define LOG10122 10122 /* fsck_SBBADP
                               Primary superblock is corrupt.
#define LOG10123 10123 /* fsck_BADIAG
                               Inode Allocation Group %s%s is inconsistent.
#define LOG10124 10124 /* fsck_BADIAMIAGPXDU
                               CHKDSK cannot repair an allocation error for files and/or 
                               directories %s through %s.
#define LOG10125 10125 /* fsck_RINOTDIR
                               Root inode is not a directory.
#define LOG10126 10126 /* fsck_RIUNALLOC
                               Root inode is not allocated.
#define LOG10127 10127 /* fsck_ROOTALLOC
                               Root inode allocated.
#define LOG10128 10128 /* fsck_ROOTNOWDIR
                               Root inode changed to directory.
#define LOG10129 10129 /* fsck_DEVUNLOCKRC
                               DosDevIOCtl(...DSK_UNLOCKDRIVE...) returned rc = %s
#define LOG10130 10130 /* fsck_SBOKP
                               Primary superblock is valid.
#define LOG10131 10131 /* fsck_SESSEND
                               CHKDSK processing terminated:  %s   with return code: %s.
#define LOG10132 10132 /* fsck_DEVCLOSERC
                               DosClose returned rc = %s
#define LOG10133 10133 /* fsck_SESSPRMDFLT
                               Using default parameter:  /F:0
#define LOG10134 10134 /* fsck_DEVOPENRDWRRC
                               DosOpen(...OPEN_SHARE_DENYREADWRITE|OPEN_ACCESS_READWRITE...)
                               returned rc = %s
#define LOG10135 10135 /* fsck_UALINOREF
                               One or more directory entry found for unallocated inode %s%s.
#define LOG10136 10136 /* fsck_CANTRECOVERINOS
                               CHKDSK cannot recover files and/or directories %s through %s.
                               CHKDSK CANNOT CONTINUE.
#define LOG10137 10137 /* fsck_URCVWRT
                               Unrecoverable error writing %s to %s. 
                               CHKDSK CANNOT CONTINUE.
#define LOG10138 10138 /* fsck_WILLFIXRIBADDATFMT
                               The root directory has an invalid data format.  Will correct.
#define LOG10139 10139 /* fsck_WILLFIXRIBADFMT
                               The root directory has an invalid format.  Will correct.
#define LOG10140 10140 /* fsck_WILLRELEASEINOS
                               CHKDSK cannot recover files and/or directories %s through %s.
                               Will release.
#define LOG10141 10141 /* fsck_BADINOCLAIMSDUPSF
                               File claims cross linked block(s).
#define LOG10142 10142 /* fsck_BADINODATAFORMAT
                               CHKDSK cannot repair the data format error(s) in this file.
#define LOG10143 10143 /* fsck_BADINOFORMAT
                               CHKDSK cannot repair the format error(s) in this file.
#define LOG10144 10144 /* fsck_CANTREPAIRINO
                               CHKDSK cannot repair %s%s%s.
#define LOG10145 10145 /* fsck_DRIVEID
                               The current hard disk drive is:  %s
#define LOG10146 10146 /* fsck_DRIVETYPE
                               The type of file system for the disk is JFS.
#define LOG10147 10147 /* fsck_WILLRELEASEINO
                               CHKDSK cannot repair %s%s%s.  Will release.
#define LOG10148 10148 /* fsck_SBOKS
                               Secondary superblock is valid.
#define LOG10149 10149 /* fsck_WRSUP
                               Unable to open for write access.  Proceeding in read-only mode.
#define LOG10150 10150 /* fsck_WILLFIXDIRWHDLKS
                               Multiple parent directories for directory %s%s.  Will correct.
#define LOG10151 10151 /* fsck_WILLFIXINCREF
                               Directory %s%s entry \"..\" refers to an incorrect parent
                               directory (%s%s). Will correct.
#define LOG10152 10152 /* fsck_XTRABLKS
                               %s unexpected blocks detected.
#define LOG10153 10153 /* fsck_RODIRSWHLKS
                               Directories with illegal hard links have been detected.
#define LOG10154 10154 /* fsck_ROINCINOREFS
                               Directory entries (entries \"..\") referring to incorrect
                               parent directories have been detected.
#define LOG10155 10155 /* fsck_ROUALINOREFS
                               Directory entries for unallocated files have been detected.
#define LOG10156 10156 /* fsck_CNTWRTSUPS
                               Unable to write secondary superblock.
#define LOG10157 10157 /* fsck_WILLFIXLINKCTS
                               Incorrect link counts have been detected. Will correct.
#define LOG10158 10158 /* fsck_INOINLINECONFLICT
                               File set object %s%s%s has inline data conflict (type %s).
#define LOG10159 10159 /* fsck_DUPBLKMDREFS
                               Duplicate block references have been detected in Metadata. 
                               CHKDSK CANNOT CONTINUE.
#define LOG10160 10160 /* fsck_DUPBLKMDREF
                               Multiple metadata references to %s blocks beginning at 
                               offset %s have been detected.
#define LOG10161 10161 /* fsck_BADDINOFREELIST1
                               Directory inode %s%s has a node with empty freelist and 
                               nonzero freecount.
#define LOG10162 10162 /* fsck_CANTREADEAITEXT1
                               Unable to read entire first extent of AIT (%s).
#define LOG10163 10163 /* fsck_CANTREADAITEXT1
                               Unable to read first extent of AIT (%s).
#define LOG10164 10164 /* fsck_URCVUNLCK
                               Unrecoverable error during UNLOCK processing.
#define LOG10165 10165 /* fsck_URCVCLOSE
                               Unrecoverable error during CLOSE processing.
#define LOG10166 10166 /* fsck_RICRRCTDREF
                               The root directory reference (entry \"..\") has been corrected.
#define LOG10167 10167 /* fsck_INOPATHCRCT
                               %s appears to be the correct path for directory %s%s.
#define LOG10168 10168 /* fsck_RODIRWHLKS
                               Directory inode %s%s has illegal hard links.
#define LOG10169 10169 /* fsck_ROINCINOREF
                               Directory inode %s%s refers (entry \"..\") to an incorrect
                               inode (%s%s).
#define LOG10170 10170 /* fsck_ROUALINOREF
                               One or more directory entry found for unallocated inode %s%s.
#define LOG10171 10171 /* fsck_ERRORSDETECTED
                               ERRORS HAVE BEEN DETECTED.  Run chkdsk with the F parameter 
                               to repair.
#define LOG10172 10172 /* fsck_RIINCINOREF
                               The root directory refers (entry \"..\") to an incorrect inode.
#define LOG10173 10173 /* fsck_INOSINLSFND
                               %s directory reconnected to /lost+found/.
#define LOG10174 10174 /* fsck_MNCNTRCNCTINOSD
                               MINOR: Unable to reconnect %s directory.  Continuing.
#define LOG10175 10175 /* fsck_ROUNCONNIO
                               Inode %s%s is not connected to the root directory tree.
#define LOG10176 10176 /* fsck_XCHKLOGNEW
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10177 10177 /* fsck_XCHKLOGOLD
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10178 10178 /* fsck_XCHKLOGOPNFAIL
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10179 10179 /* fsck_BADINOSTAMP
                               Invalid stamp detected in file system object %s%s%s.
#define LOG10180 10180 /* fsck_ROUNCONNIOS
                               Files and/or directories not connected to the root directory 
                               tree have been detected.
#define LOG10181 10181 /* fsck_WILLFIXROUALINOREFS
                               Directory entries for unallocated files have been detected.
                               Will remove.
#define LOG10182 10182 /* fsck_WILLFIXROUNCONNIOS
                               Files and/or directories not connected to the root directory
                               tree have been detected.  Will reconnect.
#define LOG10183 10183 /* fsck_WILLFIXRODIRSWHLKS
                               Directories with illegal hard links have been detected.
                               Will correct.
#define LOG10184 10184 /* fsck_WILLFIXROINCINOREFS
                               Directories (entries \"..\") referring to incorrect parent
                               directories have been detected.  Will correct.
#define LOG10185 10185 /* fsck_XCHKDMPOPNFAIL
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10186 10186 /* fsck_AGGCLN
                               Filesystem is clean.
#define LOG10187 10187 /* fsck_AGGCLNNOTDRTY
                               Filesystem is clean but is marked dirty.  Run chkdsk with the
                               F parameter to fix.
#define LOG10188 10188 /* fsck_AGGDRTY
                               Filesystem is dirty.
#define LOG10189 10189 /* fsck_AGGDRTYNOTCLN
                               Filesystem is dirty but is marked clean.  In its present state,
                               the results of accessing %s (except by this utility) are 
                               undefined.
#define LOG10190 10190 /* fsck_AGGMRKDCLN
                               Filesystem has been marked clean.
#define LOG10191 10191 /* fsck_AGGMRKDDRTY
                               Filesystem has been marked dirty because it contains critical
                               errors.  Filesystem may be unrecoverable.
#define LOG10192 10192 /* fsck_BADINOMTNODE
                               File system object %s%s%s has an illegal empty node.
#define LOG10193 10193 /* fsck_BADINOODDINTRNEXT
                               File system object %s%s%s -- extent at offset %s has an invalid size (%s).
#define LOG10194 10194 /* fsck_BADINOFRONTGAP
                               Dense file (inode %s%s) begins with an unallocated section.
#define LOG10195 10195 /* fsck_BADINOINTERNGAP
                               Dense file (inode %s%s) has an unallocated section after 
                               offset %s.
#define LOG10196 10196 /* fsck_BADAGFELIST1
                               The Free Inode Extent List is inconsistent for Allocation
                               Group %s%s.
#define LOG10197 10197 /* fsck_BADAGFILIST
                               Discrepancies detected in the Free Inode List for Allocation
                               Group %s%s. (%s) 
#define LOG10198 10198 /* fsck_BADAGFILIST1
                               The Free Inode List is inconsistent for Allocation Group %s%s.
#define LOG10199 10199 /* fsck_BADIAGAG
                               Inode Allocation Group %s%s has an invalid Allocation 
                               Group (%s).
#define LOG10200 10200 /* fsck_BADDIRENTRY
                               Directory has an entry for an unallocated file %s%s.
#define LOG10201 10201 /* fsck_BADDINONODESIZ
                               Directory inode %s%s has a node with incorrect size.
#define LOG10202 10202 /* fsck_BADDINOODDNODESIZ
                               Directory inode %s%s has a node with invalid size.
#define LOG10203 10203 /* fsck_BADDINOFREELIST4
                               Directory inode %s%s has a node with an invalid freelist.
#define LOG10204 10204 /* fsck_BADDINOFREELIST2
                               Directory inode %s%s has a node with an incorrect freecount.
#define LOG10205 10205 /* fsck_BADDINOFREELIST3
                               Directory inode %s%s has a node with an incorrect freelist.
#define LOG10206 10206 /* fsck_BADIAGAGSTRT
                               Inode Allocation Group %s%s has an invalid AG Start.
#define LOG10207 10207 /* fsck_BADIAGFIES
                               Inode Allocation Group %s%s has an invalid Free Extent Summary.
#define LOG10208 10208 /* fsck_BADIAGFIS
                               Inode Allocation Group %s%s has an invalid Free Inode Summary.
#define LOG10209 10209 /* fsck_BADIAGFLIST
                               Discrepancies detected in the Free IAG List. (%s,%s)
#define LOG10210 10210 /* fsck_BADIAGFL1
                               The Free Inode Allocation Group List is inconsistent (%s).
#define LOG10211 10211 /* fsck_BADIAGIAGNUM
                               Inode Allocation Group %s has an incorrect IAG number value (%s).
#define LOG10212 10212 /* fsck_BADIAGNFEXT
                               Inode Allocation Group %s%s has an inconsistent count
                               for number of free extents.
#define LOG10213 10213 /* fsck_BADIAGNFINO
                               Inode Allocation Group %s%s has an inconsistent count
                               for number of free inodes.
#define LOG10214 10214 /* fsck_BADIAGPMAP
                               Discrepancies detected in the pmap for Inode Allocation
                               Group %s%s.
#define LOG10215 10215 /* fsck_BADIAM
                               Errors detected in the Fileset File/Directory Allocation Map.
#define LOG10216 10216 /* fsck_BADIAMBPIE
                               The Inode Allocation Map blocks per inode extent is incorrect (%s).
#define LOG10217 10217 /* fsck_BADIAMCTL
                               Errors detected in the Fileset File/Directory Allocation 
                               Map control information.
#define LOG10218 10218 /* fsck_BADIAML2BPIE
                               The Inode Allocation Map log2(blocks per inode extent)
                               is incorrect (%s).
#define LOG10219 10219 /* fsck_BADIAMNBI
                               The Inode Allocation Map has an incorrect number of
                               backed inodes value (%s).
#define LOG10220 10220 /* fsck_BADIAMNFI
                               The Inode Allocation Map has an incorrect number of
                               free inodes value (%s).
#define LOG10221 10221 /* fsck_BADIAMNXTIAG
                               The Inode Allocation Map has an incorrect next IAG value (%s).
#define LOG10222 10222 /* fsck_XCHKDMPUSAGE
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10223 10223 /* fsck_BADIAGAGCRCTD
                               Correcting Inode Allocation Group %s%s invalid Allocation Group (%s).
#define LOG10224 10224 /* fsck_BADIAGAGSTRTCRCTD
                               Correcting Inode Allocation Group %s%s invalid AG Start.
#define LOG10225 10225 /* fsck_BADBMAPSLFV
                               Inconsistencies detected in leaf values (%s).
#define LOG10226 10226 /* fsck_BADBMAPSLNV
                               Inconsistencies detected in internal values (%s).
#define LOG10227 10227 /* fsck_BADBMAPSOTHER
                               Incorrect data detected in pages (%s).
#define LOG10228 10228 /* fsck_BADDMAPPMAPS
                               Descrepancies detected between observed block allocations and
                               pmaps.
#define LOG10229 10229 /* fsck_BADBLKALLOC
                               Incorrect data detected in disk allocation structures.
#define LOG10230 10230 /* fsck_BADBLKALLOCCTL
                               Incorrect data detected in disk allocation control structures.
#define LOG10231 10231 /* fsck_BMAPBADHT
                               Incorrect height detected in in page %s,%s.
#define LOG10232 10232 /* fsck_BMAPBADL2NLF
                               Incorrect l2 number of leafs detected in in page %s,%s.
#define LOG10233 10233 /* fsck_BMAPBADLFI
                               Incorrect leaf index detected in in page %s,%s.
#define LOG10234 10234 /* fsck_BMAPBADLFV
                               Incorrect leaf (%s) value detected in %s page %s.
#define LOG10235 10235 /* fsck_BMAPBADLNV
                               Incorrect internal (%s) value detected in %s page %s.
#define LOG10236 10236 /* fsck_BMAPBADBMN
                               Incorrect budmin detected in in page %s,%s.
#define LOG10237 10237 /* fsck_BMAPBADNLF
                               Incorrect number of leafs detected in in page %s,%s.
#define LOG10238 10238 /* fsck_BMAPCAGNF
                               Incorrect number of free blocks in AG %s detected in 
                               Block Map Control Page.
#define LOG10239 10239 /* fsck_BMAPCASB
                               Incorrect aggregate size detected in Block Map Control Page.
#define LOG10240 10240 /* fsck_BMAPCBPAG
                               Incorrect blocks per AG detected in Block Map Control Page.
#define LOG10241 10241 /* fsck_BMAPCDMCLAG
                               Incorrect dmap control level detected in Block Map Control Page.
#define LOG10242 10242 /* fsck_BMAPCDMCLH
                               Incorrect dmap control height detected in Block Map Control Page.
#define LOG10243 10243 /* fsck_BMAPCDMCLW
                               Incorrect dmap control width detected in Block Map Control Page.
#define LOG10244 10244 /* fsck_BMAPCL2BPAG
                               Incorrect l2 blocks per AG detected in Block Map Control Page.
#define LOG10245 10245 /* fsck_BMAPCL2BPP
                               Incorrect l2 blocks per page detected in Block Map Control Page.
#define LOG10246 10246 /* fsck_BMAPCMAAG
                               Incorrect maximum active AGs detected in Block Map Control Page.
#define LOG10247 10247 /* fsck_BMAPCMXLVL
                               Incorrect maximum level detected in Block Map Control Page.
#define LOG10248 10248 /* fsck_BMAPCNAG
                               Incorrect number of AGs detected in Block Map Control Page.
#define LOG10249 10249 /* fsck_BMAPCPAG
                               Invalid preferred AG detected in Block Map Control Page.
#define LOG10250 10250 /* fsck_XCHKLOGSBOK
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10251 10251 /* fsck_DMAPBADNBLK
                               Incorrect number of blocks detected in dmap %s.
#define LOG10252 10252 /* fsck_DMAPBADNFREE
                               Incorrect number free detected in dmap %s.
#define LOG10253 10253 /* fsck_DMAPBADSTRT
                               Incorrect start detected in dmap %s.
#define LOG10254 10254 /* fsck_PMAPSBOFF
                               %s consecutive blocks observed available but pmap 
                               (%s, %s, %s) indicates they are allocated.
#define LOG10255 10255 /* fsck_PMAPSBON
                               %s conseutive blocks observed allocated but pmap 
                               (%s, %s, %s) 
                               indicates they are available.
#define LOG10256 10256 /* fsck_BADBMAPCAGFCL
                               Discrepancies detected in the Block Map Control Page AG free
                               count list.
#define LOG10257 10257 /* fsck_BADBMAPCOTH
                               Incorrect data detected in the Block Map Control Page.
#define LOG10258 10258 /* fsck_BMAPCDMCSTI
                               Incorrect dmap control start index detected in Block Map 
                               Control Page.
#define LOG10259 10259 /* fsck_BADSBFWSL1
                               Incorrect fwsp length detected in the superblock (%s).
#define LOG10260 10260 /* fsck_BADSBFJLA
                               Incorrect jlog address detected in the superblock (%s).
#define LOG10261 10261 /* fsck_BADSBFJLL
                               Incorrect jlog length detected in the superblock (%s).
#define LOG10262 10262 /* fsck_HEARTBEAT0
                                ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10263 10263 /* fsck_HEARTBEAT1
                                ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10264 10264 /* fsck_HEARTBEAT2
                                ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10265 10265 /* fsck_HEARTBEAT3
                                ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10266 10266 /* fsck_HEARTBEAT4
                                ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10267 10267 /* fsck_HEARTBEAT5
                                ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10268 10268 /* fsck_HEARTBEAT6
                               ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10269 10269 /* fsck_HEARTBEAT7
                               ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10270 10270 /* fsck_HEARTBEAT8
                               ***** HEARTBEAT MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10271 10271 /* fsck_BADINOCLAIMSDUPSD
                               Directory claims cross linked block(s).
#define LOG10272 10272 /* fsck_BADINOCLAIMSDUPSO
                               File system object claims cross linked block(s).
#define LOG10273 10273 /* fsck_SPARSEFILSYS
                                File system is formatted for sparse files.
#define LOG10274 10274 /* fsck_INCDASDUSEDCRCT
                                   ,"CHKDSK  Directory inode %s%s incorrect DASD used value corrected.\n\r"
#define LOG10275 10275 /* fsck_INOSINLSFNDS
                               %s directories reconnected to /lost+found/.
#define LOG10276 10276 /* fsck_INOSINLSFNF
                               %s file reconnected to /lost+found/.
#define LOG10277 10277 /* fsck_INOSINLSFNFS
                               %s files reconnected to /lost+found/.
#define LOG10278 10278 /* fsck_BADINODATAFORMATD
                                CHKDSK cannot repair the data format error(s) in this directory.
#define LOG10279 10279 /* fsck_BADINOFORMATD
                                CHKDSK cannot repair the format error(s) in this directory.
#define LOG10280 10280 /* fsck_BADINODATAFORMATO
                                CHKDSK cannot repair the data format error(s) in this file system object.
#define LOG10281 10281 /* fsck_BADINOFORMATO
                                CHKDSK cannot repair the format error(s) in this file system object.
#define LOG10282 10282 /* fsck_PHASE7V
                               **Phase 7 - Verify File/Directory Allocation Maps
#define LOG10283 10283 /* fsck_PHASE8V
                               **Phase 8 - Verify Disk Allocation Maps
#define LOG10284 10284 /* fsck_PARMOMITLOGREDO
                               CHKDSK parameter detected:  Omit logredo()
#define LOG10285 10285 /* avail_285
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10286 10286 /* avail_286
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10287 10287 /* avail_287
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10288 10288 /* avail_288
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10289 10289 /* avail_289
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10290 10290 /* fsck_BADMETAINOP
                                Primary metadata inode %s%s is corrupt.
#define LOG10291 10291 /* fsck_BADMETAINOS
                                Secondary metadata inode %s%s is corrupt.
#define LOG10292 10292 /* fsck_SBBADS
                               Secondary superblock is corrupt.
#define LOG10293 10293 /* fsck_CANTREADAITS
                               Unable to read the Secondary File/Directory Allocation Table.
#define LOG10294 10294 /* fsck_BADIAMA
                               Errors detected in the File System File/Directory Allocation Map.
#define LOG10295 10295 /* fsck_BADIAMCTLA
                               Errors detected in the File System File/Directory Allocation 
                               Map control information.
#define LOG10296 10296 /* fsck_ERRORSINAITS
                               Errors detected in the Secondary File/Directory Allocation Table.
#define LOG10297 10297 /* fsck_CNTRESUPS
                               Unable to read secondary superblock.
#define LOG10298 10298 /* avail_298
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10299 10299 /* avail_299
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10300 10300 /* avail_300
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10301 10301 /* fsck_MNCNTRCNCTINOSDS
                               MINOR: Unable to reconnect %s directories.  Continuing.
#define LOG10302 10302 /* fsck_MNCNTRCNCTINOSF
                               MINOR: Unable to reconnect %s file.  Continuing.
#define LOG10303 10303 /* fsck_MNCNTRCNCTINOSFS
                               MINOR: Unable to reconnect %s files.  Continuing.
#define LOG10304 10304 /* avail_304
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10305 10305 /* avail_305
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10306 10306 /* avail_306
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10307 10307 /* avail_307
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10308 10308 /* avail_308
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10309 10309 /* fsck_INOINLSFND
                               Directory inode %s%s has been reconnected to /lost+found/.
#define LOG10310 10310 /* fsck_MNCNTRCNCTINOD
                               MINOR: Unable to reconnect directory inode %s%s.  Continuing.
#define LOG10311 10311 /* avail_311
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10312 10312 /* fsck_CNTRESUPB
                               Unable to read either superblock.
#define LOG10313 10313 /* fsck_STDSUMMARY1
                               %s kilobytes total disk space.
#define LOG10314 10314 /* fsck_STDSUMMARY2
                               %s kilobytes in %s directories.
#define LOG10315 10315 /* fsck_STDSUMMARY3
                               %s kilobytes in %s user files.
#define LOG10316 10316 /* fsck_STDSUMMARY4
                               %s kilobytes in extended attributes
#define LOG10317 10317 /* fsck_STDSUMMARY5
                               %s kilobytes reserved for system use.
#define LOG10318 10318 /* fsck_STDSUMMARY6
                               %s kilobytes are available for use.
#define LOG10319 10319 /* fsck_XCHKDMPMTORBADREAD
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10320 10320 /* fsck_XCHKDMPBADFORMAT
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10321 10321 /* fsck_CHKLOGSBOK
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10322 10322 /* fsck_XCHKLOGBADFNAME
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10323 10323 /* fsck_XCHKDMPBADFNAME
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10324 10324 /* fsck_CANTREADAITP
                               Unable to read the Primary File/Directory Allocation Table.
#define LOG10325 10325 /* fsck_ERRORSINAITP
                               Errors detected in the Primary File/Directory Allocation Table.
#define LOG10326 10326 /* fsck_CANTCONTINUE
                               CHKDSK CANNOT CONTINUE.
#define LOG10327 10327 /* fsck_CANTINITSVCLOG
                               ********** CHKDSK CANNOT INITIALIZE THIS SERVICE LOG.  DISREGARD RESIDUAL LOG MESSAGES WHICH MAY BE APPENDED. **********
#define LOG10328 10328 /* fsck_CHKLOGNOVOL
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10329 10329 /* fsck_XCHKLOGNOVOL
                               ***** CHKLOG/XCHKLOG MESSAGE ***** SHOULD NOT BE LOGGED *****
#define LOG10330 10330 /* fsck_PARMFIXLVL
                               CHKDSK parameter detected:  FixLevel: %s
#define LOG10331 10331 /* fsck_PARMIFDRTY
                               CHKDSK parameter detected:  IfDirty
#define LOG10332 10332 /* fsck_PARMVERBOSE
                               CHKDSK parameter detected:  VerboseMessaging
#define LOG10333 10333 /* fsck_PARMAUTOCHK
                               CHKDSK parameter detected:  AutoCheck mode
#define LOG10334 10334 /* fsck_PARMPMCHK
                               CHKDSK parameter detected:  PMchkdsk mode
#define LOG10335 10335 /* fsck_PARMDEBUG
                               CHKDSK parameter detected:  Debug mode
#define LOG10336 10336 /* fsck_CHKDSKSYSALLOC
                               CHKDSK.SYS returned %s bytes of high memory at %s
#define LOG10337 10337 /* fsck_CHKDSKSYSOPENRC
                               DosOpen(CHKDSK.SYS,...) returned rc = %s
#define LOG10338 10338 /* fsck_CHKDSKSYSCALLRC
                               DosDevIOCtl( ... ) to call CHKDSK.SYS returned 
                               rc = %s, data.rc = %s
#define LOG10339 10339 /* fsck_DEVBEGINFMTRC
                               ujfs_beginformat() returned rc = %s
#define LOG10340 10340 /* fsck_PARMCLRBDBLKLST
                               CHKDSK parameter detected:  Clear LVM Bad Block List
#define LOG10341 10341 /* fsck_CHKDSKFSSYNCHRC
                               (JFSCTL_CHKDSK, ... ) to synch file system returned rc = %s
#define LOG10342 10342 /* fsck_LVMFOUNDBDBLKS
                                warning: The LVM has detected bad blocks in the partition.  Run 
                               CHKDSK /B to transfer entries from the LVM bad block table to the 
                               JFS bad block table.
#define LOG10343 10343 /* fsck_LVMGETBBLKINFORC
                                LVM GetBadBlockInformation returned rc = %s
#define LOG10344 10344 /* fsck_LVMGETTBLSIZERC
                                LVM GetTableSize rc = %s
#define LOG10345 10345 /* fsck_LVMFSNOWAVAIL
                                THE FILE SYSTEM IS NOW AVAILABLE.
#define LOG10346 10346 /* fsck_LVMTRNSBBLKSTOJFS
                                Transferring entries from the LVM Bad Block Table for this file 
                                system to the JFS Bad Block Table for this file system.
#define LOG10347 10347 /* fsck_INTERNALERROR
                                INTERNAL ERROR (%s,%s,%s,%s). CHKDSK CANNOT CONTINUE.
#define LOG10348 10348 /* fsck_CLRBBACTIVITY
                                CLRBBLKS:  relocated %s extents, for total %s relocated blocks.
#define LOG10349 10349 /* fsck_CLRBBLVMLISTDATA
                                CLRBBLKS: LVM List #%s  num entries = %s  num active entries = %s
#define LOG10350 10350 /* fsck_CLRBBLVMNUMLISTS
                                CLRBBLKS: number of LVM Lists = %s
#define LOG10351 10351 /* fsck_CLRBBRANGE
                                CLRBBLKS: Attempted to relocate bad blocks in range %s to %s.
#define LOG10352 10352 /* fsck_CLRBBRESULTS
                                LVM reports %s bad blocks.  Of these, %s have been transferred to 
                                the JFS Bad Block List.
#define LOG10353 10353 /* fsck_CLRBBLKSRC
                                CLRBBLKS rc = %s
#define LOG10354 10354 /* fsck_DOSEXECPGMRC
                                DosExecPgm(...clrbblks...) rc = %s
#define LOG10355 10355 /* fsck_DEVOPENRDWRSRC
                                DosOpen(...OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE...) returned rc = %s
#define LOG10356 10356 /* fsck_CLRBBOPENFAILED
                                Device unavailable or locked by another process.  CHKDSK CANNOT CONTINUE.
#define LOG10357 10357 /* fsck_BADINONODESELF
                                File system object %s%s%s has a node with an invalid self field.
#define LOG10358 10358 /* fsck_INOPATHBAD
                               File system object %s%s%s is illegally linked as: %s
#define LOG10359 10359 /* fsck_CANTRECONINSUFSTG
                                Insufficient storage (%s) available to continue reconnecting lost files 
                                and/or directories. Continuing. 
#define LOG10360 10360 /* fsck_INOACL
                                Format error in Access Control List space or descriptor.
#define LOG10361 10361 /* fsck_WILLCLEARACL
                                Format error in Access Control List space or descriptor. Will clear.
#define LOG10362 10362 /* fsck_INOACLCLRD
                                Storage allocated to Access Control List for inode %s%s has been cleared.
#define LOG10363 10363 /* fsck_STDSUMMARY4A
                               %s kilobytes in access control lists
#define LOG10364 10364 /* fsck_EXHFILSYSSTG
                                File system is full or too fragmented to allocated requested %s blocks.
#define LOG10365 10365 /* fsck_INOMINOR
                                Minor format error detected.
#define LOG10366 10366 /* fsck_INOMINORFXD
                                Minor format error in inode %s%s has been corrected.
#define LOG10367 10367 /* fsck_WILLFIXINOMINOR
                                Minor format error detected. Will fix.
#define LOG10368 10368 /* fsck_BADINODXDFLDL
                                File system object %s%s%s has invalid length in descriptor (%s).
#define LOG10369 10369 /* fsck_BADINODXDFLDO
                                File system object %s%s%s has invalid offset in descriptor (%s).
#define LOG10370 10370 /* avail_370
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10371 10371 /* avail_371
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10372 10372 /* avail_372
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10373 10373 /* avail_373
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10374 10374 /* avail_374
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10375 10375 /* avail_375
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10376 10376 /* avail_376
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10377 10377 /* avail_377
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10378 10378 /* avail_378
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10379 10379 /* avail_379
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10380 10380 /* avail_380
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10381 10381 /* avail_381
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10382 10382 /* avail_382
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10383 10383 /* avail_383
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10384 10384 /* avail_384
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10385 10385 /* fsck_ACL, fsck_aggregate, fsck_aggr_inode
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10386 10386 /* fsck_directory, fsck_dirpfx
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10387 10387 /* fsck_dmap
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10388 10388 /* fsck_EA
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10389 10389 /* fsck_file, fsck_fileset, fsck_fileset_inode
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10390 10390 /* fsck_inoext, fsck_inopfx, fsck_metaIAG
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10391 10391 /* fsck_L0
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10392 10392 /* fsck_L1
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10393 10393 /* fsck_L2
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10394 10394 /* fsck_metadata
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10395 10395 /* fsck_objcontents
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10396 10396 /* fsck_primary
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10397 10397 /* fsck_secondary
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10398 10398 /* fsck_symbolic_link
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10399 10399 /* fsck_dotext
                               ***** MESSAGE INSERT ***** SHOULD NOT BE LOGGED *****
#define LOG10400 10400 /* lrdo_ALREADYREDONE
                               LOGREDO:  Log already redone! 
#define LOG10401 10401 /* lrdo_LOGEND
                               LOGREDO:  End of log found at logend = %s
#define LOG10402 10402 /* lrdo_REXTNDBEGIN
                                LOGREDO:  Beginning to recover File System Extend processing.
#define LOG10403 10403 /* lrdo_REXTNDDONE
                                LOGREDO:  Done recovering File System Extend processing.
#define LOG10404 10404 /* lrdo_REXTNDTOPRE
                                LOGREDO:  File System Extend recovered to prior size.
#define LOG10405 10405 /* lrdo_REXTNDTOPOST
                                LOGREDO:  File System Extend recovered and completed.
#define LOG10406 10406 /* lrdo_REXTNDFAIL
                                LOGREDO:  Recovery of File System Extend failed.  Error = %s
#define LOG10407 10407 /* lrdo_LOGSUPBADMGC
                                LOGREDO:  Log superblock contains invalid magic number.
#define LOG10408 10408 /* lrdo_LOGSUPBADVER
                                LOGREDO:  Log superblock contains invalid version number.
#define LOG10409 10409 /* lrdo_LOGENDBAD1
                                LOGREDO:  Unable to find valid end of log.
#define LOG10410 10410 /* lrdo_INITFAILED
                                LOGREDO:  Initialization failed.  rc = %s   error num = %s
#define LOG10411 10411 /* lrdo_LOGSUPBADBLKSZ
                                LOGREDO:  Log superblock contains invalid block size.
#define LOG10412 10412 /* lrdo_LOGSUPBADL2BLKSZ
                                LOGREDO:  Log superblock contains invalid log2(block size).
#define LOG10413 10413 /* lrdo_LOGSUPBADLOGSZ
                                LOGREDO:  Log superblock contains invalid log size.
#define LOG10414 10414 /* lrdo_LOGENDBAD2
                                LOGREDO:  Log end is not in valid range.
#define LOG10415 10415 /* lrdo_RPTSYNCNUM
                                LOGREDO:  Synch point record number:  %s
#define LOG10416 10416 /* lrdo_RPTSYNCADDR
                                :LOGREDO:  Synch point record address:  %s
#define LOG10417 10417 /* lrdo_RPTNUMLOGREC
                                LOGREDO:  Number of log records:    %s
#define LOG10418 10418 /* lrdo_RPTNUMDOBLK
                                LOGREDO:  Number of Do blocks:    %s
#define LOG10419 10419 /* lrdo_RPTNUMNODOBLK
                                LOGREDO:  Number of NoDo blocks:    %s
#define LOG10420 10420 /* lrdo_SYNCRECORD
                                LOGREDO:  Log record for Sync Point at:    %s 
#define LOG10421 10421 /* lrdo_MOUNTRECORD
                                LOGREDO:  Log record for Volume Mount at:    %s
#define LOG10422 10422 /* fsck_ALLOCHIGHMEM
                                Calling CHKDSK.SYS to allocate high memory....
#define LOG10423 10423 /* fsck_ALLOCHIGHMEMRSLT
                                CHKDSK.SYS returned  addr= %s  length= %s
#define LOG10424 10424 /* lrdo_ALLOC4BMAP
                                LOGREDO:  Allocating for BMap:  %s bytes
#define LOG10425 10425 /* lrdo_ALLOC4IMAP
                                LOGREDO:  Allocating for IMap:  %s bytes
#define LOG10426 10426 /* lrdo_ALLOC4DOBLK
                                LOGREDO:  Allocating for DoBLk:  %s bytes
#define LOG10427 10427 /* lrdo_ALLOC4NOREDOFL
                                LOGREDO:  Allocating for NoReDoFile:  %s bytes
#define LOG10428 10428 /* lrdo_ALLOC4REDOPG
                                LOGREDO:  Allocating for ReDoPage:  %s bytes
#define LOG10429 10429 /* lrdo_ALLOC4NODOFL
                                LOGREDO:  Allocating for NoDoFile:  %s bytes
#define LOG10430 10430 /* lrdo_ALLOC4EXTDTPG
                                LOGREDO:  Allocating for ExtendDtPg:  %s bytes
#define LOG10431 10431 /* lrdo_ALLOC4BMAPFAIL
                                LOGREDO:   Failed trying to allocate for BMap:  %s bytes
#define LOG10432 10432 /* lrdo_ALLOC4IMAPFAIL
                                LOGREDO:  Failed trying to allocate for IMap:  %s bytes
#define LOG10433 10433 /* lrdo_ALLOC4DOBLKFAIL
                                LOGREDO:  Failed trying to allocate for DoBLk:  %s bytes
#define LOG10434 10434 /* lrdo_ALLOC4NOREDOFLFAIL
                                LOGREDO:  Failed trying to allocate for NoReDoFile:  %s bytes
#define LOG10435 10435 /* lrdo_ALLOC4REDOPGFAIL
                                LOGREDO:  Failed trying to allocate for ReDoPage:  %s bytes
#define LOG10436 10436 /* lrdo_ALLOC4NODOFLFAIL
                                LOGREDO:  Failed trying to allocate for NoDoFile:  %s bytes
#define LOG10437 10437 /* lrdo_ALLOC4EXTDTPGFAIL
                                LOGREDO:  Failed trying to allocate for ExtendDtPg:  %s bytes
#define LOG10438 10438 /* lrdo_USINGBMAPALLOC4IMAP
                                LOGREDO:  Working IMap will occupy storage originally allocated for BMap.
#define LOG10439 10439 /* lrdo_USINGBMAPALLOC4NRFL
                                LOGREDO:  Some storage allocated for BMap now used for NoRedoFile records.
#define LOG10440 10440 /* lrdo_USINGBMAPALLOC4DOBLK
                                LOGREDO:  Some storage allocated for BMap now used for DoBLock records.
#define LOG10441 10441 /* lrdo_USINGBMAPALLOC4RDPG
                                LOGREDO:  Some storage allocated for BMap now used for RedoPage records.
#define LOG10442 10442 /* lrdo_USINGBMAPALLOC4NDFL
                                LOGREDO:  Some storage allocated for BMap now used for NoDoFile records.
#define LOG10443 10443 /* lrdo_USINGBMAPALLOC4EDPG
                                LOGREDO:  Some storage allocated for BMap now used for ExtendDtPage records.
#define LOG10444 10444 /* lrdo_CANTREADLOGSUP
                                LOGREDO:  Unable to read Journal Log superblock.
#define LOG10445 10445 /* lrdo_CANTUPDLOGSUP
                                LOGREDO:  Unable to update Journal Log superblock.
#define LOG10446 10446 /* lrdo_NEXTADDRINVALID
                                LOGREDO:  Address of next log record is invalid.
#define LOG10447 10447 /* lrdo_NEXTADDROUTRANGE
                                LOGREDO:  Address (%s) of next log record is out of range.
#define LOG10448 10448 /* lrdo_NEXTADDRSAME
                                LOGREDO:  Address (%s) of next log record is the same as address of current log record.
#define LOG10449 10449 /* lrdo_LOGWRAPPED
                                LOGREDO:  The Journal Log has wrapped around and written over itself.
#define LOG10450 10450 /* lrdo_BADCOMMIT
                                LOGREDO:  Invalid Commit record at %s.
#define LOG10451 10451 /* lrdo_BADMOUNT
                                LOGREDO:  Invalid Mount record at %s.
#define LOG10452 10452 /* lrdo_DEVOPNREADERROR
                                LOGREDO: Device open/read error
#define LOG10453 10453 /* lrdo_BADREDOPG
                                LOGREDO:  Invalid RedoPage record at %s.
#define LOG10454 10454 /* lrdo_BADNOREDOPG
                                LOGREDO:  Invalid NoRedoPage record at %s.
#define LOG10455 10455 /* lrdo_BADNOREDOINOEXT
                                LOGREDO:  Invalid NoRedoInoExt record at %s.
#define LOG10456 10456 /* lrdo_BADUPDATEMAP
                                LOGREDO:  Invalid UpdateMap record at %s.
#define LOG10457 10457 /* lrdo_UNKNOWNTYPE
                                LOGREDO:  Unrecognized log record type at %s.
#define LOG10458 10458 /* lrdo_ERRORNEEDREFORMAT
                                LOGREDO:  Errors encountered imply the log is corrupt.  Reformat needed.
#define LOG10459 10459 /* lrdo_ERRORCANTCONTIN
                                LOGREDO:  Errors encountered prevent logredo() from continuing. 
#define LOG10460 10460 /* lrdo_ERRORCANTUPDMAPS
                                LOGREDO: Unable to update map(s).
#define LOG10461 10461 /* lrdo_ERRORCANTUPDFSSUPER
                                LOGREDO:  Unable to update file system superblock.
#define LOG10462 10462 /* lrdo_CANTWRITELOGSUPER
                                LOGREDO:  Unable to write to log superblock.
#define LOG10463 10463 /* lrdo_CANTREADFSSUPER
                                LOGREDO:  Unable to read file system superblock.
#define LOG10464 10464 /* lrdo_FSSUPERBADMAGIC
                                LOGREDO:  File system superblock has invalid magic number.
#define LOG10465 10465 /* lrdo_FSSUPERBADVERS
                                File system superblock has invalid version number.
#define LOG10466 10466 /* lrdo_FSSUPERBADLOGLOC
                                File system superblock has invalid log location.
#define LOG10467 10467 /* lrdo_FSSUPERBADLOGSER
                                LOGREDO:  File system superblock has invalid log serial number.
#define LOG10468 10468 /* lrdo_INITMAPSFAIL
                                LOGREDO:  Unable to initialize map(s).
#define LOG10469 10469 /* lrdo_READFSSUPERFAIL
                                Attempt to read file system superblock failed.
#define LOG10470 10470 /* lrdo_WRITEFSSUPERFAIL
                                Attempt to write file system superblock failed.
#define LOG10471 10471 /* lrdo_READFSPRIMSBFAIL
                                LOGREDO:  Attempt to read file system primary superblock failed.
#define LOG10472 10472 /* lrdo_READFS2NDSBFAIL
                                LOGREDO:  Attempt to read file system secondary superblock failed.
#define LOG10473 10473 /* lrdo_WRITEFSPRIMSBFAIL
                                LOGREDO:  Attempt to write file system primary superblock failed.
#define LOG10474 10474 /* lrdo_WRITEFS2NDSBFAIL
                                LOGREDO:  Attempt to write file system secondary superblock failed.
#define LOG10475 10475 /* lrdo_BUFFLUSHFAIL
                                LOGREDO:  Attempt to flush buffers failed.
#define LOG10476 10476 /* lrdo_NOTAFSDEVNUM
                                LOGREDO: Not a valid file system device number.
#define LOG10477 10477 /* lrdo_LOGNOTINLINE1
                                LOGREDO:  Journal log must be inline. *
#define LOG10478 10478 /* lrdo_NOTAFSDEV
                                LOGREDO:  Device is not a file system device.
#define LOG10479 10479 /* lrdo_LOGNOTINLINE2
                                LOGREDO:  Journal log is not inline.
#define LOG10480 10480 /* lrdo_LOGOPEN
                                LOGREDO:  Journal log is now open.  (major = %s, minor = %s)
#define LOG10481 10481 /* lrdo_ERRORONVOL
                                LOGREDO:  Error in volume %s.
#define LOG10482 10482 /* lrdo_OPENFAILED
                                LOGREDO:  Device open failed.
#define LOG10483 10483 /* lrdo_CANTINITMAPS
                                LOGREDO:  Failed attempting to initialize map(s).
#define LOG10484 10484 /* lrdo_BADDISKBLKNUM
                                LOGREDO:  Invalid volume block number (%s).
#define LOG10485 10485 /* lrdo_BADINODENUM
                                LOGREDO:  Invalid inode number (%s).
#define LOG10486 10486 /* lrdo_CANTREADBLK
                                LOGREDO:  Failed attempt to read block %s.
#define LOG10487 10487 /* lrdo_IOERRREADINGBLK
                                LOGREDO:  I/O error attempting to read block %s.
#define LOG10488 10488 /* lrdo_BADUPDMAPREC
                                LOGREDO:  Invalid UpdateMap record (nxd = %s).
#define LOG10489 10489 /* lrdo_BADLOGSER
                                LOGREDO:  Invalid log serial number.
#define LOG10490 10490 /* LRDO_FINDLOGENDFAIL
                                LOGREDO:  Failed attempt to find the current end of the Journal Log.
#define LOG10491 10491 /* lrdo_LOGREADFAIL
                                LOGREDO:  Failed attempting to read the Journal Log (logaddr = %s).
#define LOG10492 10492 /* lrdo_UNRECOGTYPE
                                LOGREDO:  Unrecognized log record type (logaddr = %s).
#define LOG10493 10493 /* lrdo_IOERRONLOG
                                LOGREDO:  I/O error on Journal Log (logaddr = %s).
#define LOG10494 10494 /* lrdo_LOGWRAP
                                LOGREDO:  The Journal Log has wrapped. 
#define LOG10495 10495 /* lrdo_EXTFSREADFSSUPERFAIL
                                LOGREDO:  RecovExtFS: Attempt to read file system superblock failed.
#define LOG10496 10496 /* lrdo_EXTFSREADBLKMAPINOFAIL
                                LOGREDO:  RecovExtFS: Attempt to read block map inode failed.
#define LOG10497 10497 /* lrdo_EXTFSREADBLKFAIL1
                                LOGREDO:  RecovExtFS: Read aggregate block %s failed (1).
#define LOG10498 10498 /* lrdo_EXTFSREADBLKFAIL2
                                LOGREDO:  RecovExtFS: Read aggregate block %s failed (2).
#define LOG10499 10499 /* lrdo_EXTFSWRITEBLKFAIL1
                                :LOGREDO:  RecovExtFS: Write aggregate block %s failed (1).

#define LOG10500 10500 /* lrdo_EXTFSWRITEBLKFAIL2
                                LOGREDO:  RecovExtFS: Write aggregate block %s failed (2).
#define LOG10501 10501 /* lrdo_EXTFSWRITEBLKFAIL3
                                LOGREDO:  RecovExtFS: Write aggregate block %s failed (3).
#define LOG10502 10502 /* lrdo_EXTFSREADBLKFAIL3
                                LOGREDO:  RecovExtFS: Read aggregate block %s failed (3).
#define LOG10503 10503 /* lrdo_EXTFSREADBLKFAIL4
                                LOGREDO:  RecovExtFS: Read aggregate block %s failed (4).
#define LOG10504 10504 /* lrdo_EXTFSINITLOGREDOFAIL
                                LOGREDO:  RecovExtFS: Error in initLogredo().
#define LOG10505 10505 /* lrdo_EXTFSREADBLKFAIL5
                                LOGREDO:  RecovExtFS: Read aggregate block %s failed (5).
#define LOG10506 10506 /* lrdo_EXTFSWRITEBLKFAIL4
                                LOGREDO:  RecovExtFS: Write aggregate block %s failed (4).
#define LOG10507 10507 /* lrdo_EXTFSWRITEFSSUPERFAIL
                                LOGREDO:  RecovExtFS: Attempt to write file system superblock failed.
#define LOG10508 10508 /* lrdo_EXTFSREADLOGSUPFAIL
                                LOGREDO:  RecovExtFS: Attempt to read Journal Log superblock failed.
#define LOG10509 10509 /* lrdo_EXTFSWRITELOGSUPFAIL
                                LOGREDO:  RecovExtFS: Attempt to write Journal Log superblock failed. 
#define LOG10510 10510 /* lrdo_READBMAPINOFAIL
                                LOGREDO:   Read Block Map inode failed.
#define LOG10511 10511 /* lrdo_READBMAPFAIL
                                LOGREDO:   Read Block Map failed.
#define LOG10512 10512 /* lrdo_READIMAPINOFAIL
                                LOGREDO:   Read Inode Alloc Map inode failed.
#define LOG10513 10513 /* lrdo_READIMAPFAIL
                                LOGREDO:   Read Inode Alloc Map failed.
#define LOG10514 10514 /* lrdo_RBMPREADXTFAIL
                                LOGREDO:   Read Block Map leftmost leaf failed.
#define LOG10515 10515 /* lrdo_RBMPREADDATFAIL
                                LOGREDO:   Read Block Map data extents failed.
#define LOG10516 10516 /* lrdo_RBMPREADNXTLFFAIL
                                LOGREDO:   Read Block Map next leaf failed.
#define LOG10517 10517 /* lrdo_RIMPREADXTFAIL
                                LOGREDO:   Read Inode Alloc Map leftmost leaf failed.
#define LOG10518 10518 /* lrdo_RIMPREADCTLFAIL
                                LOGREDO:   Read Inode Alloc Map control page failed.
#define LOG10519 10519 /* lrdo_RIMPREADNXTLFFAIL
                                LOGREDO:   Read Inode Alloc Map next leaf failed.
#define LOG10520 10520 /* lrdo_RIMPREADDATFAIL
                                LOGREDO:   Read Inode Alloc Map data extents failed.
#define LOG10521 10521 /* lrdo_UMPREADIMAPINOFAIL
                                LOGREDO:   Read Inode Alloc Map inode failed in UpdateMaps().
#define LOG10522 10522 /* lrdo_UMPWRITEIMAPCTLFAIL
                                LOGREDO:  Write Inode Alloc Map control page failed in UpdateMaps().
#define LOG10523 10523 /* lrdo_UMPREADBMAPINOFAIL
                                LOGREDO:  Read Block Map inode failed in UpdateMaps().
#define LOG10524 10524 /* lrdo_UMPWRITEBMAPCTLFAIL
                                LOGREDO:  Write Block Map control page failed in UpdateMaps().
#define LOG10525 10525 /* lrdo_WRIMPNOTRBLDGIMAP
                                LOGREDO:  Not rebuilding the Inode Alloc Map because chkdsk will be doing it.
#define LOG10526 10526 /* lrdo_WRIMPRXTFAIL
                                LOGREDO:  Write Inode Alloc Map, rXtree() failed.
#define LOG10527 10527 /* lrdo_WRIMPBLKWRITEFAIL
                                LOGREDO:  Write block (blk=%s) failed when updating the Inode Alloc Mapl.
#define LOG10528 10528 /* lrdo_WRIMPREADLFFAIL
                                LOGREDO:  Read next leaf (addr=%s) failed when updating the Inode Alloc Map.
#define LOG10529 10529 /* lrdo_WRIMPBADNPAGES
                                LOGREDO:  Incorrect npages detected when updating the Inode Alloc Map.
#define LOG10530 10530 /* lrdo_WRIMPDONE
                                LOGREDO:  Done updating the Inode Allocation Map.
#define LOG10531 10531 /* lrdo_WRIMPSTART
                                LOGREDO:  Beginning to update the Inode Allocation Map.
#define LOG10532 10532 /* lrdo_WRBMPDONE
                                LOGREDO:  Done updating the Block Map.
#define LOG10533 10533 /* lrdo_WRBMPSTART
                                LOGREDO:  Beginning to update the Block Map.
#define LOG10534 10534 /* lrdo_WRBMPNOTRBLDGBMAP
                                LOGREDO:  Not rebuilding the Block Map because chkdsk will be doing it.
#define LOG10535 10535 /* lrdo_RBLDGIMAPERROR1
                                LOGREDO:  Error for pmap and inoext when rebuilding the Inode Allocation Map (1).
#define LOG10536 10536 /* lrdo_WRBMPBADMAPSIZE
                                LOGREDO:  Inconsistent map size in Block Map.
#define LOG10537 10537 /* lrdo_WRBMPBADLFIDX0
                                LOGREDO:  Incorrect leaf index detected (k=%s, j=%s, idx=%s) while writing Block Map.
#define LOG10538 10538 /* lrdo_RBLDGBMAPERROR
                                LOGREDO:  Error rebuilding DMap page (k=%s, j=%s, i=%s).
#define LOG10539 10539 /* lrdo_WRBMPRXTFAIL
                                LOGREDO:  rXtree() failed when called while writing Block Map.
#define LOG10540 10540 /* lrdo_WRBMPBLKWRITEFAIL
                                LOGREDO:  Write block (at %s) failed while writing Block Map.
#define LOG10541 10541 /* lrdo_WRBMPREADLFFAIL
                                LOGREDO:  Read next leaf (at %s) failed while writing Block Map.
#define LOG10542 10542 /* lrdo_WRBMPBADTOTPG
                                LOGREDO:  Read next leaf (at %s) failed while writing Block Map.
#define LOG10543 10543 /* lrdo_UPDMPBADLFIDX
                                LOGREDO:  Invalid leaf index detected while updating dmap page.
#define LOG10544 10544 /* lrdo_RXTREADLFFAIL
                                LOGREDO:  rXtree() Read first leaf failed.
#define LOG10545 10545 /* lrdo_BRDBADBLOCK
                                LOGREDO:  bread() Invalid block number specified (%s).
#define LOG10546 10546 /* lrdo_BRDREADBLKFAIL
                                LOGREDO:  bread() Read block (%s) failed.
#define LOG10547 10547 /* lrdo_RBLDGIMAPERROR2
                                LOGREDO:  Error for pmap and inoext when rebuilding the Inode Allocation Map (2).
#define LOG10548 10548 /* lrdo_DAFTUPDPGFAILED
                                LOGREDO:   doAfter: updatePage failed.  (logaddr = %s, rc = %s)
#define LOG10549 10549 /* lrdo_DAFTMRKBMPFAILED
                                LOGREDO:   doAfter: markBmap failed.  (logaddr = %s, rc = %s)
#define LOG10550 10550 /* lrdo_DEDPBREADFAILED
                                LOGREDO:   doExtDtPg: bread failed.  (offset = %s, rc = %s)
#define LOG10551 10551 /* lrdo_DNRPFNDDTRTPGREDOFAIL
                                LOGREDO:   doNoRedoPage: type dtpage: findPageRedo failed.  (rc = %s)
#define LOG10552 10552 /* lrdo_DNRPFNDDTPGPGREDOFAIL
                                LOGREDO:   doNoRedoPage: type dtroot: findPageRedo failed.  (rc = %s)
#define LOG10553 10553 /* lrdo_DNRPFNDXTRTPGREDOFAIL
                                LOGREDO:   doNoRedoPage: type xtroot: findPageRedo failed.  (rc = %s)
#define LOG10554 10554 /* lrdo_DNRPFNDXTPGPGREDOFAIL
                                LOGREDO:   doNoRedoPage: type xtpage: findPageRedo failed.  (rc = %s)
#define LOG10555 10555 /* lrdo_DNRPUNKNOWNTYPE
                                LOGREDO:   doNoRedoPage: Unknown NoRedoPage record type.
#define LOG10556 10556 /* lrdo_DNRIFNDNOREDORECFAIL
                                LOGREDO:   doNoRedoInoExt: findPageRedo failed.  (rc = %s)
#define LOG10557 10557 /* lrdo_DUMPUNKNOWNTYPE
                                LOGREDO:   doUpdateMap: Unknown updateMap record type.
#define LOG10558 10558 /* lrdo_DPRFBADSTBLENTRY
                                LOGREDO:   dtpg_resetFreeList: Invalid stbl entry.
#define LOG10559 10559 /* lrdo_DPRFBADSLOTNXTIDX
                                LOGREDO:   dtpg_resetFreeList: Invalid slot next index.
#define LOG10560 10560 /* lrdo_DRRFBADSTBLENTRY
                                LOGREDO:   dtrt_resetFreeList: Invalid stbl entry.
#define LOG10561 10561 /* lrdo_DRRFBADSLOTNXTIDX
                                LOGREDO:   dtrt_resetFreeList: Invalid slot next index.
#define LOG10562 10562 /* lrdo_MBMPBLKOUTRANGE
                                LOGREDO:   markBmap: Block number(s) out of range (start=%s, length=%s).
#define LOG10563 10563 /* lrdo_UPPGFNDPGREDOFAIL
                                LOGREDO:   updatePage: findPageRedo failed.  (rc = %s)
#define LOG10564 10564 /* lrdo_UPPGBADINODESEGOFFSET
                                LOGREDO:   updatePage: type=INODE.  Invalid segment offset. (offset = %s)
#define LOG10565 10565 /* lrdo_UPPGBREADFAIL1
                                LOGREDO:   updatePage: bread failed (1).  (rc = %s)
#define LOG10566 10566 /* lrdo_UPPGBREADFAIL2
                                LOGREDO:   updatePage: bread failed (2).  (rc = %s)
#define LOG10567 10567 /* lrdo_UPPGBREADFAIL3
                                LOGREDO:   updatePage: bread failed (3).  (rc = %s)
#define LOG10568 10568 /* lrdo_UPPGBREADFAIL4
                                LOGREDO:   updatePage: bread failed (4).  (rc = %s)
#define LOG10569 10569 /* lrdo_UPPGMIMPFAIL
                                LOGREDO:   updatePage: markImap failed.  (rc = %s)
#define LOG10570 10570 /* lrdo_UPPGMBMPFAIL
                                LOGREDO:   updatePage: markBmap failed.  (rc = %s)
#define LOG10571 10571 /* lrdo_UPPGDTRTRFLFAIL
                                LOGREDO:   updatePage: dtrt_resetFreeList failed.  (rc = %s)
#define LOG10572 10572 /* lrdo_UPPGDTPGRFLFAIL
                                LOGREDO:   updatePage: dtpg_resetFreeList failed.  (rc = %s)
#define LOG10573 10573 /* lrdo_UPPGSEDPFAIL
                                LOGREDO:   updatePage: saveExtDtPg failed.  (rc = %s)
#define LOG10574 10574 /* lrdo_FEOLPGV1FAIL
                                LOGREDO:   fineEndOfLog: pageVal[1] failed (rc = %s).
#define LOG10575 10575 /* lrdo_FEOLPGV2FAIL
                                LOGREDO:   fineEndOfLog: pageVal[2] failed (rc = %s).
#define LOG10576 10576 /* lrdo_FEOLPGV3FAIL
                                LOGREDO:   fineEndOfLog: pageVal[3] failed (rc = %s).
#define LOG10577 10577 /* lrdo_FEOLPGV4FAIL
                                LOGREDO:   fineEndOfLog: pageVal[4] failed (rc = %s).
#define LOG10578 10578 /* lrdo_FEOLPGV4AFAIL
                                LOGREDO:   fineEndOfLog: pageVal[4(a)] failed (rc = %s).
#define LOG10579 10579 /* lrdo_PVGETPGFAIL
                                LOGREDO:   pageVal: getLogpage failed (pno = %s, rc = %s).
#define LOG10580 10580 /* lrdo_SLPWRITEFAIL
                                LOGREDO:   setLogpage: Write log page failed (pno = %s, rc = %s).
#define LOG10581 10581 /* lrdo_LRLOGWRAP
                                LOGREDO:   logRead: Log wrapped over itself (lognumread = %s).
#define LOG10582 10582 /* lrdo_LRREADFAIL
                                LOGREDO:   logRead: Read log page failed (pno = %s, rc = %s).
#define LOG10583 10583 /* lrdo_LRMWFAIL1
                                LOGREDO:   logRead: MoveWords[1] failed (rc = %s).
#define LOG10584 10584 /* lrdo_LRMWFAIL2
                                LOGREDO:   logRead: MoveWords[2] failed (rc = %s).
#define LOG10585 10585 /* lrdo_MWREADFAIL
                                LOGREDO:   moveWords: getLogpage failed (pno = %s, rc = %s).
#define LOG10586 10586 /* fsck_DASDLIMITSPRIMED
                                   DASD Limit has been primed for directories on the volume. 
#define LOG10587 10587 /* fsck_DASDUSEDPRIMED
                                   DASD Used has been primed for directories on the volume. 
#define LOG10588 10588 /* avail_for_debug_only_588
                                LOGREDO:logRead: Log data must not exceed LOGPSIZE (pno = %s).
#define LOG10589 10589 /* avail_for_debug_only_589
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10590 10590 /* avail_for_debug_only_590
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10591 10591 /* avail_for_debug_only_591
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10592 10592 /* avail_for_debug_only_592
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10593 10593 /* avail_for_debug_only_593
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10594 10594 /* avail_for_debug_only_594
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10595 10595 /* avail_for_debug_only_595
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10596 10596 /* avail_for_debug_only_596
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10597 10597 /* avail_for_debug_only_597
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10598 10598 /* avail_for_debug_only_598
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****
#define LOG10599 10599 /* avail_for_debug_only_599
                                ***** UNDEFINED ***** SHOULD NOT BE LOGGED *****

#endif
