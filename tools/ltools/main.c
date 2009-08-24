/* LTOOLS
   Programs to read, write, delete or change Linux extended 2 filesystems under DOS

   Main Program  main.c

   Copyright (C) 1996-2005 Werner Zimmermann(Werner.Zimmermann|AT|fht-esslingen.de)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   For further information please see file README.TXT.

   ---History, Changes:-----------------------------------------------------------
   V1.0: Written by Jason Hunter and Dave Lutz.
   V2.0: Rewritten by Werner Zimmermann, FHTE Esslingen, Germany, Oct. 1996
   -new user interface, new command line options
   -new drive detection code
   -debugged buffering, memory allocation to handle big directories
   -error handling
   V2.0a:Integrated floppy disk support. Full support for (short) symlinks.
   W.Zimmermann, Nov. 9, 96
   V2.0b:Completely rewrote memory allocation stuff in dir.c,
   improved harddisk partition detection.
   W.Zimmermann, Nov. 16,96
   V2.0c:Now also single files can be ldir'ed.
   (Limited) support for wildcards, see README.TXT.
   W.Zimmermann, Nov. 21,96
   V2.0d:lread can now copy multiple files.
   W.Zimmermann, Dec. 6,96
   V2.0e: Initialized inode_num in search_dir to fix bug when doing
   multiple read of files.
   W.Zimmermann, Dec. 11,96
   V2.1: Dynamic buffer allocation in function readdisk in readdisk.c,
   removed READ_BUF_SIZE.
   W.Zimmermann, Jan. 24,97
   V2.2: SCSI support has now also been tested. Some comments modified.
   Now under Windows95 also long filenames for lread are supported
   (unfortunately this does not seem to work with DJPP's gcc)
   W.Zimmermann, May 10, 97
   V2.3: Fixed partition table read. Could not handle multiple linux
   partitions in DOS extended partitions. Bug found by Jakob
   Saternus. Thanks, Jakob!
   Included better Windows 95 detection, now can distinguish between
   pure DOS7 and Win95. Thanks to Santiago Garcia Mantinan!
   W.Zimmermann, June 6, 97
   V2.4: Now works under Windows NT (must be compiled for NT, see makefile.dos).
   Some minor bugfixes. This release was inspired by and contains code
   supplied by Christophe Grenier. Thanks, Christophe!
   W.Zimmermann, Sept. 1, 98
   V2.5: Bugfixes in argument parsing in main.c and in readdisk.c. Again
   in cooperation with Christophe Grenier.
   W.Zimmermann, Sept. 1, 98
   V2.6: Minor modification in comp_name(). Again in cooperation with Christophe Grenier.
   W.Zimmermann, Oct. 12, 98
   V2.7: Bugfixes in path handling code
   comp_name():lread now allow to read path with . and ..
   eatpath():lread won't anymore display / as a file
   main():correct the overflow of 'source' by eatpath()
   Christophe GRENIER, Nov. 8, 98
   Old bug not corrected: symlinks pointing to directory won't work.
   V2.70:Command line argument parsing changed, experimental write support,
   changing UID,GID,file mode works, deleting works too, copying not
   yet implemented
   New environment variable LDRIVE=/dev/... to set default drive.
   New environment variable LDIR=<LinuxPath> to set a base directory.
   W.Zimmermann with preworks by F.Miller, Jan. 12, 1999
   V2.8: Now copying Dos-Files to Linux works, but yet is not completely
   tested. The code still contains a lot of debugging stuff.
   'Support' for compiling lread/ldir under Linux discontinued.
   W.Zimmermann, Jan. 15, 1999
   V2.9: Write buffer/cache implemented to speed up lwrite.
   W.Zimmermann, Jan. 19, 1999
   V3.0: Some beautifying, no real modifications.
   W.Zimmermann, Feb. 1, 1999
   V3.1: Implemented read buffer/cache to speed up lread.
   Compilation with Gnu's gcc is no longer supported (did not work
   under V3.0 anyhow).
   ldir now tries to find out, if it's running under Windows NT and
   calls ldirNT automatically.
   W.Zimmermann, Feb. 15, 1999
   V3.2: Identical with V3.1, but LREADhttp, lread's http based graphical
   user interface included in the package.
   W.Zimmermann, Mar. 15, 1999
   V3.3: Integrated support for UNIX and QDOS operating systems (LINUX x86,
   68K, Sun Solaris, QDOS). This support was hacked by Robert Zidlicky.
   If you have problems with running LDREAD under UNIX systems, please
   directly contact Richard under <rdzidlic@geocities.com> or
   <rdzidlic@cip.informatik.uni-erlangen.de>
   readdisk.c now is a wrapper for all operating system specific func-
   tions. The original 'readdisk' is renamed to DOSdisk.h and inclu-
   ded by readdisk.c. UNIX/QDOS's harddisk access is in UNIXdisk.h, which
   alternatively is included by readdisk.c. Nevertheless there still
   is some operating system specific stuff in the other files, mainly
   to convert data structures to and from big endian, as some UNIX systems
   runs on big endian machines. dos_long_create and lock_drive have been
   moved to DOSdisk.h.
   Richard also fixed a bug in inode.c.
   W.Zimmermann, Mar. 27, 1999
   V3.4: All error output now goes to STDERR, all 'normal' output goes to
   STDOUT. Normally both are connected to 'stdout'. New command line
   parameter '-x' redirects this output to a file 'zzz.zwz'. This modi-
   fication was necessary for LREADgui.java. LREADsrv.c was modified
   accordingly (LREADsrv.c V1.2).
   W.Zimmermann, Apr. 15, 1999
   V3.5: Minor #include modification in main.c for compatibility with
   older Borland compilers. Major modification in dir.c for improved
   symlink handling. This code was provided by Ben Van Hof.
   Package renamed to LTOOLS.
   W.Zimmermann, Mai 28, 1999
   V3.6: Changed calculation of 'start' block in DOSdisk.h to make LTOOLS
   work with big SCSI disks. Debugging was done by Milton Nusbaum.
   W.Zimmermann, June 30, 1999
   V3.7: Increased partition count limit from 9 to 99
   W. Zimmermann, Aug 1, 1999
   V3.8: LDIR now shows total and free disk space. Bug in source and
   destination filename handling with LWRITE fixed.
   W. Zimmermann, Aug. 14, 1999
   V3.9: Bugfix for the fix of V3.7.
   W. Zimmermann, Aug. 26, 1999
   V4.0: Implemented Linux file and directory renaming
   W. Zimmermann, Nov. 27, 1999
   V4.1: Implemented Linux directory creation and creation of symbolic links.
   Bug fix for handling disk block sizes >1024 bytes.
   W. Zimmermann, Dec. 5, 1999
   V4.2: Implemented deleting directories, if they are empty.
   Implemented deling symlinks.
   W. Zimmermann, Dec. 15,1999
   V4.3: Adaption to new kernels, where the 16bit name_len field has been split
   up into a 8bit field name_len and a 8bit field file_type. Fix provided by
   Ricky Gonzales.
   W. Zimmermann, Feb. 8, 2000
   V4.4: Some minor corrections to make lread more compatible to CLint.
   W.Zimmermann, Feb. 15, 2000
   V4.5: Modifications to make the LTOOLS compatible with Microsoft Visual C++.
   Visual C++ executables will only work under Windows NT/2000, not under Windows 9x!
   Debugging can now be initiated from the command line with 'ldir -deb=X', where is
   is a value from 0 to 63, defining the debug level (see ldir.h).
   Additionally a new Java Swing based graphical user interface LTOOLgui is provided.
   W.Zimmermann, Mar. 26, 2000
   V4.6: Make ldir work with big harddisks under Windows NT and DOS/Windows 9x. Under
   DOS/Windows 9x use Int 13 Bios extensions. Major revision of function biosdisk/
   ExtBiosdisk in DOSdisk.h.
   W.Zimmermann, April 12, 2000
   V4.6: Changed readdisk and writedisk to work with lba and offset instead of loc, because
   we could not handle partitions >4GB.
   W.Zimmermann, April 25, 2000
   V4.8: Fixed a memory allocation problem during lread in main.c
   W.Zimmermann, Mai 23, 2000
   V4.9: Completely removed CHS harddisk adressing in examine_drive().
   W.Zimmermann, June 1, 2000
   V4.9.1: Included a setup utility for the Windows version, which creates a program group
   with some icons.
   W.Zimmermann, Aug 17, 2000
   V5.0: New command line switch '-sdev=X:Y', where X is a (decimal) disk number, Y is a decimal
   partition number.
   W.Zimmermann, Sept 1, 2000
   V5.1: Made blocks_so_far long instead of int in inode.c to overcome a buffer overflow with reading
   files >32MB from Linux.
   W.Zimmermann, Sept 17, 2000
   V5.2: Deal with Linux extended partitions (partition type 85). Does not work correctly, if a disk does
   contain DOS/Windows extended partitions as well as Linux extended partitions at the same time.
   Automatically calling ldirNT from ldir, ldir from ldirNT depending on OS type.
   Changed NBLOCK from statically defined to dynamically defined based on BLOCK_SIZE. Changed variable definition
   in main.c from block_list[NBLOCK] to block_listt[64];
   Changed calculation of 'Disk space' to adapt to different block sizes.
   Introduced some of the ext2 revision 2 new features.
   W.Zimmermann, Nov. 12, 2000
   V5.3: Skipping directories when doing LREAD with wildcards.
   W.Zimmermann, July 17, 2001
   V5.4: Update LTOOLGUI to V2.3, no change for LREAD.
   W.Zimmermann, Oct 15, 2001
   V5.5: Update LTOOLGUI to V2.4, output uid and gid as unsigned instead of signed values in ldir.
   Extended uid and gid to be 32bit instead of 16bit values.
   W.Zimmermann, Oct 15, 2001
   V5.6: Update LTOOLGUI to V2.5 for alphabetic sorting of files and directories.
   W.Zimmermann, Mar 1, 2002
   V5.7: New command line option "-old" to use CHS instead of LBA information from the partition table.
   Normally "-old" should be used in combination with "-noext".
   W.Zimmermann, Jan. 1, 2003
   V5.8: Renamed "-old" option as "-chs" (more meaningful for user).
   Eliminated requirement for "-READ" or "-WRITE" option to precede any other options.
   Rewrote dos_long_creat() to support third-party LFN services and pre-7.0 DOS versions.
   Added "-s:X:Y" syntax as alternative to "-s=X:Y", so LDRIVE environment variable can avoid "=" character.
   Joe da Silva, Mar. 15, 2003
   approved by W.Zimmermann, Mar. 17, 2003
   V5.9: Fix some typos.
   W. Zimmermann, Apr. 4, 2003
   V6.0: Redesign of the Java-GUI LTOOLgui, introduction of the C#-GUI. Both developed by Joerg Holzapfel. Thanks!
   No change in the command line tools.
   W. Zimmermann, Jul. 18, 2003
   V6.1: Changed LDIR.BAT to call ldirDOS.exe or ldirNT.exe based on environment variable %OS%. Additionally, LDIR.BAT
   now uses an environmental variable LOPTIONS to set default options for the LTOOLS. 
   In ldirDOS.exe now check for BIOS INT 13h extensions under DOS too (previous versions: Under Windows only).
   W. Zimmermann, Aug. 10, 2003
   V6.2: ReiserFS read only support experimental. Not supported under DOS.
   Uses rfstool.exe developed by Gerson Kurz. Great tool, thanks Gerson.
   W. Zimmermann, Aug. 17, 2003
   V6.3: Modifications in rfs.c to adapt it to RFSTOOL 0.14
   W. Zimmermann, Sep. 26, 2003
   V6.4: Modified block allocation strategy in group.c and inode.c for 'ldir -WRITE', to solve problems with blocksizes
   2048 and 4096. Solution and debugging provided by Yuzuru Ueda and Reiner Beninga. Thanks, guys!
   Modified UNIXdisk.h to make 'ldir -part' work under Linux too. Rewrote examine_drive().
   Additional command line option '-lba' (complement to '-chs', forces to use LBA instead of CHS disk access.
   W. Zimmermann, Nov. 6, 2003
   V6.5: Initalized driveAdrPacket.reserved in DOSdisk.h, suggested by Yuzuru Ueda to fix possible disk parameter read problem,
   noticed with a SCSI disk under DOS.
   W. Zimmermann, Nov. 28, 2003
   V6.6: Copyright update for year 2004
   W. Zimmermann, Dec. 20, 2003
   V6.7: Show modification time instead of creation time in list_dir(). 
   Bugfix for popen() in rfs.c when compiling with Visual C++ from Visual Studio .NET.
   W. Zimmermann, Mar. 1, 2004
   V6.8: Start integration of ReiserFS RFSTOOL write support with RFSTOOL 0.14-1.0.
   W. Zimmermann, May. 11, 2004
   V6.9: Fixed a minor bug in rfs.c when scanning directory and file names with blanks.
   Major update for RFSTOOL with new partition scan method for Windows and Linux.
   W. Zimmermann, Aug 15, 2004
   V6.10: Several modifications to adapt for Solaris x86 10
   W. Zimmermann, July 15, 2005
   V6.11: Detect partition type of LVM partitions
          Copyright updates for 2006. Modification in LTOOLSgui and LTOOLSnet.
          Timezone adjustmants thanks to Rick Jones.
   W. Zimmermann, Feb. 28, 2006
   V6.12: Integrated support for image files. Thanks to C. Schaubschlaeger
   W. Zimmermann, April 25, 2006
*/

#define VERSION  "V6.12"

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef UNIX
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <math.h>
#ifndef __OS2__
  #ifndef __NT__
    #ifdef _MSC_VER 
      #include <io.h>
    #else
      #include <dir.h>
    #endif
  #else
    #include <io.h>
    #include "porting.h"
  #endif
#else
#include <io.h>
#include "porting.h"
#endif
#endif

#include "ext2.h"
#include "proto.h"
#include "ldir.h"

#ifndef UNIX
#ifndef __OS2__
#include <dos.h>
#include <conio.h>
#include <io.h>
#ifndef __MSDOS__
#include "windows.h"
#endif
#endif
#endif

extern unsigned short NBLOCK;
extern unsigned short BLOCK_SIZE;
extern void *MALLOC(size_t size);
extern void FREE(void *block);
extern super_block sb;
extern group_desc *gt;

unsigned int disk_no = 0;
unsigned int part_no = 0;
char *disk_name;
#ifdef UNIX
int disk_fd;
#else 
#ifdef __OS2__
int disk_fd;
#endif
#endif

#ifdef NEWOUT
	FILE *STDERR;
	FILE *STDOUT;
#else
	#ifdef UNIX
		#define STDOUT stdout
		#define STDERR stdout
	#else
		FILE *STDERR = stdout;
		FILE *STDOUT = stdout;
	#endif
#endif

char ext2_disk[256] = LINUX_DISK;
char ext2_image_file_name[256] = "";

enum
{   LDIR, LREAD, LWRITE, LTEST
}
modus = LDIR;

int isOldStyleLba = 0;

enum
{   CHANGE, DEL, COPY, REN
}
u_modus = CHANGE;
/*CHANGE        =change file access rights, GID or UID
   DEL           =delete file
   COPY          =copy file from DOS to Linux
   REN		 =rename Linux file
 */

enum
{  NORMAL, MKDIR, LINK
}
submodus=NORMAL;

enum
{   NO, YES
}
quiet = NO;

#ifndef min
#define min(a,b)  ((a<=b)?a:b)
#endif

int isNT = 0;
int isWin = 0;
int isUseBiosExtensions=0;
int isNoUseBiosExtensions=0;
int isDebug = 0;
int DebugOut(int level, const char *fmt,...)
{   va_list argptr;
    int i=0;

    if (level & isDebug)
    {	va_start(argptr, fmt);
	i=vprintf(fmt, argptr);
    	va_end(argptr);
    }
    return i;
}

char source[MAX_PATH_LENGTH] = "/", destin[MAX_PATH_LENGTH] = "STDOUT";
char Fmode[32] = "", Uid[32] = "", Gid[32] = "";

int main(int argc, char **argv)
{   inode *in, newIn;
    int fd, i;
    long bytes;
    unsigned long inode_num, directory_inode_num;
    unsigned long block_num, x, y, totalBlocks, blockList[64], blocksInBuf, *sind = NULL, *dind = NULL, *tind = NULL,
     		sindBlock = 0, dindBlock = 0, tindBlock = 0, indBlocks = 0;
    char inode_name[MAX_PATH_LENGTH], sourcebak[MAX_PATH_LENGTH], 
    		destin_mode = NORM, *temp, ans;
    unsigned short fmode=0;
    FILE *file;
    char sourceNameFound = 0;
    char destinNameFound = 0;
    struct dir newDirEntry;
    int i_mode;
#ifdef UNIX
    static int blocks_per_block;
#else
#ifndef __NT__
#ifndef __OS2__
    union REGS inregs, outregs;
    struct SREGS segregs;
#endif
#endif
#endif

#ifdef DEBUG_MAIN	//debug level 
    isDebug=isDebug+1;
#endif
#ifdef DEBUG_READDISK
    isDebug=isDebug+2;
#endif
#ifdef DEBUG_SUPER
    isDebug=isDebug+4;
#endif
#ifdef DEBUG_DIR
    isDebug=isDebug+8;
#endif
#ifdef DEBUG_GROUP
    isDebug=isDebug+16;
#endif
#ifdef DEBUG_INODE
    isDebug=isDebug+32;
#endif

#ifdef NEWOUT
    STDOUT = stdout; //fdopen(1, "wt+");
    STDERR = STDOUT;
//  printf("---Using NEWOUT---\n");
#endif

    tzset();

#ifndef UNIX
#ifndef __OS2__

//try to find out if we are under Windows NT
    temp = getenv("PROCESSOR_ARCHITECTURE");
    if (temp!=NULL)
    	if (!strcmp(temp, "x86"))				//this is Windows NT, this variable is not defined under DOS or Windows 9x
    	    isNT=1;

#if !defined(__NT__)
    //inregs.x.ax = 0x1687;
    inregs.x.ax = 0x160A;					//check, if we are running under Windows
    int86x(0x2F, &inregs, &outregs, &segregs);
    if (outregs.x.ax == 0)
    	isWin=1;

    if (isNT)
    {   _splitpath(argv[0], sourcebak, inode_name, NULL, NULL);	//change ldir to ldirNT
	_makepath(sourcebak, sourcebak, inode_name, "ldirNT", NULL);
	for (x = 1; x < argc; x++)
        {   strcat(sourcebak, " ");
	    strcat(sourcebak, argv[x]);
	}
        DebugOut(1,"Command line: %s",sourcebak);
        system("copy ldirNT.exe ldir.exe");
	return system(sourcebak);					//under Windows NT  ldirNT.exe is called instead of ldir.exe
    }
#else
    _splitpath(argv[0], NULL, NULL, inode_name, NULL);
    strlwr(inode_name);
    if ((!strcmp(inode_name,"ldirnt")) && (isNT==0))
    {   _splitpath(argv[0], sourcebak, inode_name, NULL, NULL);	//change ldirNT to ldir
	_makepath(sourcebak, sourcebak, inode_name, "ldir", NULL);
	for (x = 1; x < argc; x++)
        {   strcat(sourcebak, " ");
	    strcat(sourcebak, argv[x]);
	}
	system("copy ldirDOS.exe ldir.exe");
	return system(sourcebak);			
    }
#endif

#endif
#else
    test_endianness();
#endif

    disk_name = getenv("LDRIVE");				//Get default drive from environment variable LDRIVE, if set
    if (disk_name != NULL)
	strncpy(ext2_disk, disk_name, sizeof(ext2_disk));	//option overrides variable LINUX_DISK in ldir.h
    //and is overridden by command line param '-s'
    temp = getenv("LDIR");					//Get default base directory from environment variable LDIR, if set
    if (temp != NULL)
    {   if (temp[0] == '/')
        {   strncpy(source, temp, sizeof(source));
	    if (source[strlen(source) - 1] != '/')
		strcat(source, "/");
	} else
        {   fprintf(STDERR, "The LinuxPath base directory, which you specified with 'lcd LinuxPathBaseDirectory', must begin with '/'\n");
	    return (-1);
	}
    }
/*
    if ((argc > 1) && strstr(argv[1], "-READ"))			// See if its ldir, lread or lwrite
    {   modus = LREAD;
	x = 2;
    } else if ((argc > 1) && strstr(argv[1], "-WRITE"))
    {   modus = LWRITE;
	x = 2;
    } else
    {   modus = LDIR;
	x = 1;
    }
*/
    modus = LDIR;						// Default to LDIR mode
    for (x = 1; x < argc; x++)					// Parse command line
    {   // See if help is requested
	if ((strncmp(argv[x], "-h", 2) == 0) || (strncmp(argv[x], "-?", 2) == 0))
        {   fprintf(STDOUT, "\n******************************************************************************\n");
	    fprintf(STDOUT, "LTOOLS version %s (C) 1996-2006 Werner.Zimmermann|AT|fht-esslingen.de\n", VERSION);
	    fprintf(STDOUT, "See http://www.fht-esslingen.de/~zimmerma/software/ltools.html for updates.\n");
            fprintf(STDOUT, "Build %s\n", __DATE__);
#ifdef UNIX
	    fprintf(STDOUT, "UNIX/QDOS port by Richard Zidlicky\n");
#endif

	    fprintf(STDOUT, "******************************************************************************\n");
	    fprintf(STDOUT, "List, read, write or modify files on Linux Extended 2 filesystems\n\n");
	    fprintf(STDOUT, "usage: ldir    [-s/dev/hd..] /LinuxPath/LinuxDir\n");
	    fprintf(STDOUT, "       -----------> list Linux directories\n\n");
	    fprintf(STDOUT, "  or   lread   [-s/dev/hd..] /LinuxPath/LinuxFile [DosFile]\n");
	    fprintf(STDOUT, "       -----------> copy a file from Linux to local system\n\n");
	    fprintf(STDOUT, "  or   lwrite  [-s/dev/hd..] [-f.. -u.. -g..] DosFile /LinuxPath/LinuxFile\n");
	    fprintf(STDOUT, "       -----------> copy a file from local system to Linux\n\n");
	    fprintf(STDOUT, "  or   lchange [-s/dev/hd..] [-fMODE] [-uUID] [-gGID] /LinuxPath/LinuxFile\n");
	    fprintf(STDOUT, "       -----------> change access rights, UID or GID\n\n");
	    fprintf(STDOUT, "  or   ldel    [-s/dev/hd..] /LinuxPath/LinuxFile\n");
	    fprintf(STDOUT, "       -----------> delete a Linux file, directory (if empty) or symlink\n\n");

	    fprintf(STDOUT,"---Press Enter key to continue---\n\n"); getchar();
	    fprintf(STDOUT, "  or   lren    [-s/dev/hd..]  [-f.. -u.. -g..] /LinuxPath/oldName  newName\n");
	    fprintf(STDOUT, "       -----------> rename a Linux file, directory or symlink\n\n");
	    fprintf(STDOUT, "  or   lmkdir  [-s/dev/hd..]  [-f.. -u.. -g..] newLinuxDirectory\n");
	    fprintf(STDOUT, "       -----------> create a new Linux directory\n\n");
	    fprintf(STDOUT, "  or   lln     [-s/dev/hd..]  [-f.. -u.. -g..] LinuxLinkTarget  LinuxLinkSource\n");
	    fprintf(STDOUT, "       -----------> create a symbolic link\n\n");
#ifndef UNIX
	    fprintf(STDOUT, "  or   lcd     /LinuxBaseDirectory/\n");
	    fprintf(STDOUT, "       -----------> set the Linux base directory (Note: Must begin with '/')\n\n");
	    fprintf(STDOUT, "  or   ldrive  /dev/...\n");
	    fprintf(STDOUT, "       -----------> set the Linux default drive\n\n");
#else
	    fprintf(STDOUT, "  or   . lcd    /LinuxBaseDirectory/\n");
	    fprintf(STDOUT, "       -----------> set the Linux base directory (Note: Must begin with '/')\n\n");
	    fprintf(STDOUT, "  or   . ldrive /dev/...\n");
	    fprintf(STDOUT, "       -----------> set the Linux default drive\n\n");
#endif
	    fprintf(STDOUT, "  or   ldir    -h\n");
	    fprintf(STDOUT, "       -----------> get version and this help message\n\n");

	    fprintf(STDOUT,"---Press Enter key to continue---\n\n"); getchar();
	    fprintf(STDOUT,"Note: ReiserFS support is read only, so lwrite, lchange, ... will not work\n");
	    fprintf(STDOUT,"      ReiserFS is not supported under DOS.\n\n");
	    
	    fprintf(STDOUT, "Additional options:\n");
	    fprintf(STDOUT, " -q     ... quiet, no warnings when writing to Linux \n");
	    fprintf(STDOUT, " -x     ... all output goes into file zzz.zwz\n");
	    fprintf(STDOUT, " -deb=X ... debugging, where X is the debugging level (0..63, see ldir.h)\n");
	    fprintf(STDOUT, " -s=X:Y ... specify disk number X and partition number Y directly,\n");
	    fprintf(STDOUT, "            eg. -s=128:1, instead of -s/dev/hda1 (only DOS/Windows)\n");
	    fprintf(STDOUT, " -s:X:Y ... alternative syntax instead of -s=X:Y\n");
	    fprintf(STDOUT, " -ext   ... force use of support for large harddisks (only DOS/Windows 9x)\n");
	    fprintf(STDOUT, " -noext ... turn off support for large harddisks (only DOS/Windows 9x)\n");
	    fprintf(STDOUT, " -lba   ... use new style LBA disk access (only DOS/Windows 9x)\n");  
	    fprintf(STDOUT, " -chs   ... use old style CHS based disk access (only DOS/Windows 9x)\n");  
	    fprintf(STDOUT, "            (Normally used together with '-noext')\n");

#ifndef UNIX
	    fprintf(STDOUT,"---Press Enter key to continue---\n\n"); getchar();
            fprintf(STDOUT, "You can set your default Linux drive via     'set LDRIVE=/dev/hd..'\n\n");
	    fprintf(STDOUT, "You can set your default Linux directory via 'set LDIR=/LinuxDirectory/'\n\n");
	    fprintf(STDOUT, "To find out which partitions you have, use\n       ldir -part\n");
	    fprintf(STDOUT, "If you have problems (especially SCSI drives!), read the README file\n");
	    fprintf(STDOUT, "Please note: In a Windows 9x DOS box LWRITE may spin off your floppy disk drive.\n");
#else
	    fprintf(STDOUT, "You can set your default Linux drive via     'export LDRIVE=/dev/hd..'\n\n");
	    fprintf(STDOUT, "You can set your default Linux directory via 'export LDIR=/LinuxDirectory/'\n\n");
#endif
	    fprintf(STDOUT, "You can specify a disk image with '-sMyImageFile' instead of a real harddisk.\n");
	    fprintf(STDOUT, "(does not work under DOS)\n");
	    return (-1);
	}
	else if (strncmp(argv[x], "-v", 2) == 0)
        {   fprintf(STDOUT, "\n******************************************************************************\n");
	    fprintf(STDOUT, "LTOOLS version %s (C) 1996-2006 Werner.Zimmermann|AT|fht-esslingen.de\n", VERSION);
            fprintf(STDOUT, "Support for Linux Extended 2 and 3 filesystems under DOS, Windows and Linux\n");
	    fprintf(STDOUT, "See http://www.fht-esslingen.de/~zimmerma/software/ltools.html for updates.\n");
#ifdef __BORLANDC__
    	    fprintf(STDOUT, "Compiled with Borland C %X.%X on %s ", __BORLANDC__ / 256, __BORLANDC__ % 256, __DATE__);
#else
#ifdef _MSC_VER
    	    fprintf(STDOUT, "Compiled with Microsoft C %i.%i on %s ", _MSC_VER / 100, _MSC_VER % 100, __DATE__);
#else
	#ifdef __GNUC__
   	    fprintf(STDOUT, "Compiled with GNU GCC on %s ", __DATE__);
	#else
   	    fprintf(STDOUT, "Compiled with unknown compiler on %s ", __DATE__);
	#endif
#endif
#endif
#ifdef __NT__
#ifndef USEDLL
    	    fprintf(STDOUT, "as 32bit EXE\n");
#else
    	    fprintf(STDOUT, "as 32bit DLL\n");
#endif
#else
#ifndef __OS2__
#ifndef UNIX
    	    fprintf(STDOUT, "as 16bit binary\n");
#else
	    fprintf(STDOUT, "\n");
#endif
#endif
#endif
#ifdef UNIX
	    fprintf(STDOUT, "UNIX/QDOS port by Richard Zidlicky\n");
#endif
	    fprintf(STDOUT, "******************************************************************************\n");

            rfsVersion();

	    return (-1);
	}
	// See if LREAD mode is specified (override LDIR default)
	else if (strncmp(argv[x], "-READ", 5) == 0)
	{   modus = LREAD;
	    continue;
	}
	// See if LWRITE mode is specified (override LDIR default)
	else if (strncmp(argv[x], "-WRITE", 6) == 0)
	{   modus = LWRITE;
	    continue;
	}
	// See if disk drive is specified
	else if (strncmp(argv[x], "-s", 2) == 0)
        {   strncpy(ext2_disk, &argv[x][2], sizeof(ext2_disk));		// strip -s when copying
	    continue;
	}
	// See if it is '-q', i.e. quiet operation without warnings for WRITE operation
	else if (strncmp(argv[x], "-q", 2) == 0)
        {   quiet = YES;
	    continue;
	}
	// See if it is '-x', i.e. quiet operation without warnings for WRITE operation and output to file
	else if (strncmp(argv[x], "-x", 2) == 0)
        {   quiet = YES;
#ifdef NEWOUT        
	    STDOUT = freopen("zzz.zwz", "w+", STDOUT);
#else
	    STDOUT = fopen("zzz.zwz", "w+");
#endif
	    STDERR = STDOUT;
	    continue;
	}
	// See if it is '-deb', i.e. debugging
	else if (strncmp(argv[x], "-deb=", 4) == 0)
        {  temp=argv[x]+4;
	   if ((*temp=='=') || (*temp==':'))
           	isDebug=atoi(temp+1);
           else
           	isDebug=7;				//default debug level
           continue;
	}
	// See if it is '-ext', i.e. use Bios extensions
	else if (strncmp(argv[x], "-ext=", 4) == 0)
        {  isUseBiosExtensions=1;
           continue;
	}
	// See if it is '-noext', i.e. do not use Bios extensions
	else if (strncmp(argv[x], "-noext=", 4) == 0)
        {  isNoUseBiosExtensions=1;
           continue;
	}
	// See if it is '-chs', i.e. use old style CHS translation
	else if (strncmp(argv[x], "-chs=", 4) == 0)
        {  isOldStyleLba=1;
           continue;
	}
	// See if it is '-lba', i.e. use new style LBA translation
	else if (strncmp(argv[x], "-lba=", 4) == 0)
        {  isOldStyleLba=0;
           continue;
	}
	// See if we want to display the partition table of all drives (undocumented)
	else if (strncmp(argv[x], "-part", 5) == 0)
        {   modus = LTEST;
	    continue;
//	    break;
	}
	// See if it is -f
	else if (strncmp(argv[x], "-f", 2) == 0)
        {   if (modus == LWRITE)
            {	strncpy(Fmode, &argv[x][2], sizeof(Fmode));	// strip -f when copying
		if (strlen(Fmode) == 1)			// convert octal values and set access rights
		    fmode = (((int) Fmode[0] - '0') & 0x7) << 6;
		else if (strlen(Fmode) == 2)
		    fmode = (((int) Fmode[1] - '0') & 0x7) <<3 | (((int) Fmode[0] - '0') & 0x7) << 6;
		else if (strlen(Fmode) == 3)
		    fmode = (((int) Fmode[2] - '0') & 0x7) | (((int) Fmode[1] - '0') & 0x7) << 3 | (((int) Fmode[0] - '0') & 0x7) << 6;
		else
		{   fprintf(STDERR, "\nAccess rights must be between 1 and three octal digits\n");
		    return -1;
		}
		continue;
	    } else
            {   fprintf(STDERR, "\n'-f' not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// See if it is -u
	else if (strncmp(argv[x], "-u", 2) == 0)
        {   if (modus == LWRITE)
            {   strncpy(Uid, &argv[x][2], sizeof(Uid));	// strip -u when copying
		continue;
	    } else
            {   fprintf(STDERR, "\n'-u' not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// See if it is -g
	else if (strncmp(argv[x], "-g", 2) == 0)
        {   if (modus == LWRITE)
            {   strncpy(Gid, &argv[x][2], sizeof(Gid));	// strip -g when copying
		continue;
	    } else
            {   fprintf(STDERR, "\n'-g' not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// See if it is -del
	else if (strncmp(argv[x], "-del", 4) == 0)
        {   if (modus == LWRITE)
            {   u_modus = DEL;
		continue;
	    } else
            {   fprintf(STDERR, "\n'-del'not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// See if it is -copy
	else if (strncmp(argv[x], "-copy", 4) == 0)
        {   if (modus == LWRITE)
            {   u_modus = COPY;
	    } else
            {   fprintf(STDERR, "\n'-copy' not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// See if it is -ren
	else if (strncmp(argv[x], "-ren", 4) == 0)
        {   if (modus == LWRITE)
            {   u_modus = REN;
	    } else
            {   fprintf(STDERR, "\n'-ren' not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// See if it is -mkdir
	else if (strncmp(argv[x], "-mkdir", 4) == 0)
        {   if (modus == LWRITE)
            {   u_modus = COPY;
                submodus= MKDIR;
	    } else
            {   fprintf(STDERR, "\n'-mkdir' not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// See if it is -link
	else if (strncmp(argv[x], "-link", 4) == 0)
        {   if (modus == LWRITE)
            {   u_modus = COPY;
                submodus= LINK;
	    } else
            {   fprintf(STDERR, "\n'-link' not allowed here, please try 'ldir -h'\n");
		return (-1);
	    }
	}
	// if it is none of these it must be a file/directory name
	else
        {   if (!sourceNameFound)
	    {							// must be source file name
		if ((modus == LWRITE) && (u_modus == COPY))
                {   strncpy(destin, source, sizeof(destin));
		    strncpy(source, argv[x], sizeof(source));
		} else
                {   if (argv[x][0] == '/')
			strncpy(source, argv[x], sizeof(source));
		    else
			strncat(source, argv[x], sizeof(source) - strlen(source));
		}
		sourceNameFound = 1;
	    } else
	    {							// must be destination file name
		if ((modus == LWRITE) && (u_modus == COPY))
                {   if (argv[x][0] == '/')
			strncpy(destin, argv[x], sizeof(destin));
		    else
			strncat(destin, argv[x], sizeof(destin) - strlen(source));
		    destinNameFound=1;
		} else
                {   strncpy(destin, argv[x], sizeof(destin));
		    if (!strcmp(destin, "."))			//copy destin to same filename as source?
                    {   strcpy(destin, source);
			destin_mode = SELF;
		    }
		    destinNameFound = 1;
		}
	    }
	    continue;
	}
    }

    DebugOut(1,"Command line: ");
    for (x=0;x<argc;x++)
    	DebugOut(1,"%s ",argv[x]);
    DebugOut(1,"\n");
    DebugOut(1,"Mode: %u   Source:%s---Destin:%s---\n",modus,source,destin);

#ifdef __NT__
#ifndef _MSC_VER
    DebugOut(1,"Running ldirNT under %s %d.%d\n", isNT?"Windows NT ":"OS ",_osmajor, _osminor);
#endif
#else
#ifdef UNIX
    DebugOut(1,"Running ldir under Unix\n");
#else
#ifndef __OS2__
    DebugOut(1,"Running ldir under %s V%d.%d \n", isWin ? "Windows ":"DOS ",_osmajor, _osminor);
#else
    DebugOut(1,"Running ldir under OS/2\n");
#endif
#endif
#endif

    if (modus==LTEST)
    {
	fprintf(STDOUT, "\n******************************************************************************\n");
	fprintf(STDOUT, "LTOOLS version %s (C) 1996-2006 Werner.Zimmermann|AT|fht-esslingen.de\n", VERSION);
#ifdef __BORLANDC__
    	fprintf(STDOUT, "Compiled with Borland C %X.%X on %s ", __BORLANDC__ / 256, __BORLANDC__ % 256, __DATE__);
#else
#ifdef _MSC_VER
    	fprintf(STDOUT, "Compiled with Microsoft C %i.%i on %s ", _MSC_VER / 100, _MSC_VER % 100, __DATE__);
#else
    	fprintf(STDOUT, "Compiled with unknown compiler on %s ", __DATE__);
#endif
#endif
#ifdef __NT__
    	fprintf(STDOUT, "as 32bit binary\n");
#else
#ifndef UNIX
#ifndef __OS2__
    	fprintf(STDOUT, "as 16bit binary\n");
#else
    	fprintf(STDOUT, "as 32bit binary\n");
#endif
#endif
#endif
#ifdef UNIX
	fprintf(STDOUT, "UNIX/QDOS port by Richard Zidlicky\n");
#endif
#ifdef __NT__
#ifndef _MSC_VER
    	fprintf(STDOUT,"Running ldirNT under %s %d.%d\n", isNT?"Windows NT ":"OS ",_osmajor, _osminor);
#endif
#else
#ifdef __OS2__
    	fprintf(STDOUT,"Running ldir under OS/2\n");
#else
#ifdef UNIX
    	fprintf(STDOUT,"Running ldir under Unix\n");
#else
    	fprintf(STDOUT,"Running ldir under %s V%d.%d \n", isWin ? "Windows ":"DOS ",_osmajor, _osminor);
#endif
#endif
#endif
	fprintf(STDOUT, "******************************************************************************\n");
    }

//  printf("modus=%d u_modus=%d submodus=%d disk=%s=%s source=%s destin=%s\n",modus,u_modus,submodus,ext2_disk,drive_name,source,destin);

    if ((modus==LWRITE) && ((u_modus==DEL) || (u_modus==COPY) || (u_modus==REN)) && (quiet == NO))
    {   fprintf(STDERR, "\nLWRITE is experimental. It may destroy your files or even your disk!\n");
	fprintf(STDERR, "USE AT YOUR OWN RISK! LWRITE will set your filesystem to 'not clear'\n");
	fprintf(STDERR, "On most installations, this will cause an automatic run of fsck when\n");
	fprintf(STDERR, "you boot up Linux. If not, run fsck (e2fsck) manually!\n\n");
	fprintf(STDERR, "Do you really want to continue [Y/N] ?  ");
	scanf("%c", &ans);
	if ((char) toupper(ans) != 'Y')
	    return (-1);
	fprintf(STDERR, "\n");
    }
    // With LREAD or LWRITE at least a source file name must be given
    if (((modus == LREAD) || (modus == LWRITE)) && (!sourceNameFound))
    {   fprintf(STDERR, "You must specify a source filename (help: lread -h  or lwrite -h)\n");
	return (-1);
    }
    if ((modus == LWRITE) && (!sourceNameFound || !destinNameFound) && ((u_modus==COPY) || (u_modus==REN)))
    {   fprintf(STDERR, "You must specify a destination filename (help: lwrite -h)\n");
	return (-1);
    }
#ifdef UNIX
    disk_name = ext2_disk;
#endif

    if ((temp=strstr(ext2_disk, "/dev/hd"))!=NULL)			// figure out disk number for DOS
    {	disk_no = temp[7] -'a' + 128;
    } else if ((temp=strstr(ext2_disk, "/dev/sd"))!=NULL)
    {	disk_no = temp[7] -'a' + 128;
    } else if ((temp=strstr(ext2_disk, "/dev/rd"))!=NULL)
    {	disk_no = temp[7] -'a' + 256;
    } else if (strstr(ext2_disk, "/dev/fd0"))
    { disk_no = 0;
    } else if (strstr(ext2_disk, "/dev/fd1"))
    {	disk_no = 1;
    } else if (strncmp(ext2_disk, "=", 1)==0 || strncmp(ext2_disk, ":",1)==0 )	//directly specify disk and partition number
    {	char *p;
        disk_no = atoi(ext2_disk+1);
    	p = (char*) strstr(ext2_disk+1,":");
    	if (p!=NULL)
    	    part_no=atoi(p+1);
    } else								//Assume it is an image disk
    {   disk_no = 2;
	part_no = 1;
	strcpy(ext2_image_file_name, ext2_disk);
//	disk_no = 128;
//	fprintf(STDERR, "Unknown Drive Specification: %s   %i\n", ext2_disk, disk_no);
    }

    if ((disk_no >= 128) && (part_no ==0))			//do it only for harddisks ???
    {   strcpy(sourcebak, ext2_disk);				//figure out partition number
	if (strstr(ext2_disk, "dev/hd"))
        {   temp = (char*) strtok(sourcebak, "/dev/hd");
	} else if (strstr(ext2_disk, "dev/sd"))
        {   temp = (char*) strtok(sourcebak, "dev/sd");
//	    fprintf(STDERR, "SCSI drive support is experimental, see the README file\n");
	} else if (strstr(ext2_disk, "dev/rd"))
        {   temp = (char*) strtok(sourcebak, "dev/rd");
	}
	part_no = atoi(&temp[1]);				//convert char to int

	if ((part_no < 1) || (part_no > 99))			//if ((part_no < 1) || (part_no > 99))
        {   part_no = 0;
	}
    } else if (part_no==0)
    {	part_no = 1;						//on floppy disks we only have one partition
    }
#ifndef UNIX
    _fmode = O_BINARY;						// we want all files binary for this
#endif

    DebugOut(1,"disk_name='%s'  ext2_disk='%s'  disk_no:part_no=%d:%d\n",disk_name, ext2_disk,disk_no, part_no);

    if (modus != LTEST)
    {   if (disk_no >= 256)
        {   disk_no = disk_no - 128;
    	    if ((i=rfsMain(argc, argv))==0)
    	        return 0;
   	    fprintf(STDERR, "ERROR: ReiserFS Error %d \n", i);
    	    return i;
        }
    	if (examine_drive())
        {   exit(-1);
	}
    } else
    {
#ifdef UNIX
//	fprintf(STDERR, "Sorry, 'ldir -part' only works under DOS/Windows - use 'fdisk' instead\n");
//	return(0);
#endif
        for (disk_no = 128; disk_no <= 135; disk_no++)
        {   part_no = 0;
	    examine_drive();
	    if (disk_name!=NULL)
	    	if (strcmp(disk_name, LINUX_DISK)!=0)
		    break;
	}
	rfsPart();
	return (0);
    }

    if ((i=load_super())!=0)					// Couldn't get superblock
    {   if (i==-1)
    	{
    	    if ((i=rfsMain(argc, argv))==0)
    	        return 0;
   	    fprintf(STDERR, "ERROR: ReiserFS Error %d \n", i);
    	    return i;
	}
	fprintf(STDERR, "ERROR: Error %d Loading Superblock\n", i);
    	return i;
    }
#ifdef UNIX
    blocks_per_block = BLOCK_SIZE / 4;
#endif
    if (load_groups() == -1)					// Couldn't read groups
    {   fprintf(STDERR, "Error Reading Groups\n");
	return (-1);
    }
    DebugOut(16,"Printing Group--------------------------\n");
    if (isDebug & 16) print_groups();


    if ((modus == LWRITE) && (u_modus == COPY))
    {   strcpy(sourcebak, destin);
	if (eatpath(destin, inode_name, INIT))			//Does the file exist?
        {   fprintf(STDERR, "Linux-file '%s' exists - can't overwrite - please delete first\n", sourcebak);
	    return (-1);
	}
	strcpy(destin, sourcebak);
	for (x = strlen(destin) - 1; x != 0; x--)		//Extract the path
        {   if (destin[x] == '/')
		break;
	}
	destin[x + 1] = '\0';

	modus = LDIR;
	if ((inode_num = eatpath(destin, inode_name, INIT)) == 0)	//Reload the associated directory Inode
        {   fprintf(STDERR, "Invalid Linux path! LWRITE can only copy files to existing directories\n");
	    return (-1);
	}
	modus = LWRITE;
	strcpy(destin, sourcebak);				//Extract the filename
	temp = (char *) strrchr(destin, '/');
	strcpy(destin, (char *) &temp[1]);
    } else
    {   strcpy(sourcebak, source);
	inode_num = eatpath(source, inode_name, INIT);
    }
    if ((inode_num == 0) && !((modus == LWRITE) && (u_modus == COPY)))	// File or Directory not found
    {   fprintf(STDERR, "Linux Path Invalid\n");
	return (-1);
    }

    DebugOut(1,"1: inode_num=%ld\n", inode_num);
    DebugOut(1,"argc:%i---x:%li---Modus:%i---Source:%s---Destination:%s---\n", argc, x, modus, sourcebak, destin);


    switch (modus)						// See if a dir is requested
    {   case LDIR:
            {   search_dir(NULL, NULL, inode_name, KILL_BUF);	//close buffer
		if ((in = load_inode(inode_num)) == NULL)	//load directory inode
                {   fprintf(STDERR, "Inode Error\n");
		    return (-1);
		}
		list_dir(in, sourcebak);			//print out directory
		if (quiet == NO)
                {
                    fprintf(STDOUT, "Disk space: %6.3f MB of %6.3f MB free\n",
                    	    ((float) sb.s_free_blocks_count) * BLOCK_SIZE / (1024.0*1024.0),
			    ((float) sb.s_blocks_count)      * BLOCK_SIZE / (1024.0*1024.0));
		}
		return (0);
	    }

	case LWRITE:
            {   switch (u_modus)				//check LWRITE submode
                {   case CHANGE:
                        {
#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(1);			//lock the drive
#endif
#endif
			    while (inode_num)
                            {

                            	in = load_inode(inode_num);
				if (in == NULL)
                                {   fprintf(STDERR, "Inode Error\n");
				    return (-1);
				}
                                if (strlen(Fmode) != 0)
                                    in->i_mode = ( in->i_mode & 0xF000 ) | fmode;
				if (strlen(Uid)!=0)
				{   in->i_uid = atol(Uid) & 0xFFFF;
				    in->i_uid_high = atol(Uid) >> 16;
				}
				if (strlen(Gid)!=0)
				{   in->i_gid = atol(Gid) & 0xFFFF;
				    in->i_gid_high = atol(Gid) >> 16;
				}

				store_inode(inode_num, in);
				inode_num = search_dir(NULL, NULL, inode_name, KEEP_BUF);
			    }
			    search_dir(NULL, NULL, inode_name, KILL_BUF);
#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(0);			//unlock the drive
#endif
#endif
			    return (0);
			}

		    case REN:
			{
#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(1);			//lock the drive
#endif
#endif

			    search_dir(NULL, NULL, inode_name, KILL_BUF);	//Free buffer
			    if ((in = load_inode(inode_num)) == NULL)	//Load Directory inode
                            {   fprintf(STDERR, "Could not load directory inode\n");
				return (-1);
			    }

			    if (modify_dir(in,destin,&newDirEntry,2,&i_mode) != 0) //Check if the new name already exists
                            {   fprintf(STDERR, "'%s' already exists - cannot rename '%s'\n",destin, source);
				return (-1);
			    }

			    strcpy(newDirEntry.name, "");
			    newDirEntry.inode_num = 0;
			    if ((inode_num = modify_dir(in, inode_name, &newDirEntry, 0, &i_mode)) == 0)	//Delete entry in directory table
                            {   fprintf(STDERR, "Could not delete file entry in directory\n");
				return (-1);
			    }
			    newDirEntry.inode_num = inode_num;
			    strcpy(newDirEntry.name, destin);
			    modify_dir(in, "", &newDirEntry, 1, &i_mode);//Insert entry in directory table

			    in = load_inode(inode_num);
                            if (strlen(Fmode) != 0)
                               	in->i_mode = ( in->i_mode & 0xF000 ) | fmode;
			    if (strlen(Uid) != 0)
			    {   in->i_uid = atol(Uid) & 0xFFFF;
				in->i_uid_high = atol(Uid) >> 16;
			    }
			    if (strlen(Gid) != 0)
			    {   in->i_gid = atol(Gid) & 0xFFFF;
				in->i_gid_high = atol(Gid) >> 16;
			    }
			    store_inode(inode_num, in);

#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(0);				//unlock the drive
#endif
#endif
			    return (0);
			}

		    case DEL:
                        {
/* This is incomplete:
   -When the filename contains wildcards, this only works with the first file.
 */
#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(1);			//lock the drive
#endif
#endif

			    //search_dir(NULL, NULL, inode_name, KILL_BUF);	//Free buffer

			    directory_inode_num=inode_num;

			    if ((in = load_inode(directory_inode_num)) == NULL)	//Load Directory inode
                            {   fprintf(STDERR, "Could not load directory inode\n");
				return (-1);
			    }

		    	    strcpy(newDirEntry.name, "");
			    newDirEntry.inode_num = 0;

			    inode_num = modify_dir(in,inode_name,&newDirEntry, 2, &i_mode); //test only to get i_mode

			    if (S_ISDIR(i_mode))				//It's a directory, ...
			    {	strcpy(destin,source);				//... check if it is empty
				strcat(destin,"/*");
				eatpath(destin, inode_name, INIT);
				search_dir(NULL, NULL, inode_name, KEEP_BUF);
				if ((inode_num = search_dir(NULL, NULL, inode_name, KEEP_BUF))!=0)
				{   fprintf(STDERR,"Cannot delete directory - is not empty\n");
				    return -1;
				}

				inode_num = eatpath(source, inode_name, INIT);		//Get directory again
				search_dir(NULL, NULL, inode_name, KILL_BUF);		//Free buffer
				if ((in = load_inode(directory_inode_num)) == NULL)	//Load directory inode
				{   fprintf(STDERR, "Could not load directory inode\n");
    				    return (-1);
				}
			    }

			    if ((inode_num = modify_dir(in, inode_name, &newDirEntry, 0, &i_mode)) == 0)		//Delete entry in directory table
                            {   fprintf(STDERR, "Could not delete file entry in directory\n");
				return (-1);
			    }
			    if ((in = load_inode(inode_num)) == NULL)	//Load file inode
                            {   fprintf(STDERR, "Could not load file inode\n");
				return (-1);
			    }

			    x=0;
			    block_num = block_list(in);		//get first block number
			    while (block_num  !=  0)
                            {   modifyBlockBitmap(block_num, 0);//clear blocks in block bitmap

				DebugOut(1,"Deleting %4ld.block: blocknum=%ld\n", x++, block_num);

				sb.s_free_blocks_count++;	//adapt super block's free block count
				gt[(block_num - sb.s_first_data_block) / sb.s_blocks_per_group].bg_free_blocks_count++;
				block_num = block_list(NULL);	//get next block number
			    }
			    modifyBlockBitmap((int) NULL, 4);	//write bitmap to disk

			    modifyInodeBitmap(inode_num, 0);	//clear inode in inode bitmap
			    modifyInodeBitmap((int) NULL, 4);	//write bitmap to disk
			    sb.s_free_inodes_count++;		//adapt superblock's free inode count
			    gt[(inode_num - 1) / sb.s_inodes_per_group].bg_free_inodes_count++;

			    if (S_ISDIR(in->i_mode))		//Deleting an empty directory
			    {   gt[(inode_num - 1) / sb.s_inodes_per_group].bg_used_dirs_count--;
				store_groups();
			    	memset(in, 0, sizeof(inode));	//clear file inode
			    	store_inode(inode_num, in);	//write inode to disk
				in = load_inode(directory_inode_num);
				in->i_links_count--;		//now change the link count in the parent directory
				store_inode(directory_inode_num,in);
			    } else				//Deleting a normal file
			    {
			    	store_groups();
			    	memset(in, 0, sizeof(inode));	//clear file inode
			    	store_inode(inode_num, in);	//write inode to disk
			    }
			    /*for debugging: */ sb.s_state = 0;
			    //mark file system 'not clean'

			    if (write_super() == -1)		//write superblock to disk
                            {   fprintf(STDERR, "Error Writing Superblock\n");
				return (-1);
			    }

#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(0);			//unlock the drive
#endif
#endif
			    return (0);
			}
		    case COPY:
                        {   search_dir(NULL, NULL, inode_name, KILL_BUF);	//Free buffer

			    if (!((sb.s_free_inodes_count != 0) && (sb.s_free_blocks_count != 0)))
                            {   fprintf(STDERR, "Disk full no free inodes or blocks\n");
				return (-1);
			    }
			    if ((temp = MALLOC(NBLOCK * BLOCK_SIZE)) == NULL)	//allocate temp. buffer
                            {   fprintf(STDERR, "Memory allocation error in lwrite\n");
				return (-1);
			    }

			    DebugOut(1,"\ninode_num=%ld\n", inode_num);

			    directory_inode_num=inode_num;
			    if ((in = load_inode(inode_num)) == NULL)	//Load Directory inode
                            {   fprintf(STDERR, "Could not load directory inode\n");
				return (-1);
			    }

#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(1);			//lock the drive
#endif
#endif

			    if (submodus==MKDIR)
			    {	in->i_links_count++;
			        store_inode(inode_num, in);
			        newDirEntry.file_type=EXT2_FT_DIR;
			    } else if (submodus==LINK)
			    {	newDirEntry.file_type=EXT2_FT_SYMLINK;
			    } else
			    {	newDirEntry.file_type=EXT2_FT_REG_FILE;
			    }


			    //Looking for a free inode
			    for (inode_num = 1; inode_num <= sb.s_inodes_count; inode_num++)
                            {   if (modifyInodeBitmap(inode_num, 2) == 0)
				    break;
			    }

			    DebugOut(1,"Found free inode=%ld\n", inode_num);

			    if (submodus==MKDIR)				//Create the directory template file
			    {	strcpy(source,"ldirtmpl");
				if ((temp = MALLOC(BLOCK_SIZE))==NULL)
				{   fprintf(STDERR,"Memory problem in main.c - exiting\n");
				    return(-1);
				}
				memset(temp,0,BLOCK_SIZE);
				*(long*)(temp)=inode_num;		//Entry '.' 	inode_num
				*(short*)(temp+4)=12;			//		rec_len
				*(char*)(temp+6)=1;			//		name_len
				*(char*)(temp+8)='.';			//		name
				*(long*)(temp+12)=directory_inode_num;	//Entry '..' 	inode_num
				*(short*)(temp+12+4)=BLOCK_SIZE-12;	//		rec_len
				*(char*)(temp+12+6)=2;			//		name_len
				*(char*)(temp+12+8)='.';		//		name
				*(char*)(temp+12+9)='.';

			    	if ((sb.s_rev_level!=EXT2_GOOD_OLD_REV)
			    		&& (sb.s_feature_incompat && EXT2_FEATURE_INCOMPAT_FILETYPE))
				{	*(temp+7)=EXT2_FT_DIR;		//		file_type
					*(temp+12+7)=EXT2_FT_DIR;
				}

				if ((file = fopen(source,"wb+"))==NULL)
				{   fprintf(STDERR,"Cannot create directory template\n");
				    return (-1);
				}
				fwrite(temp,sizeof(char),BLOCK_SIZE,file);
				fclose(file);
			    }

			    if ((submodus!=LINK)&&((file = fopen(source, "rb")) == NULL))
                            {   fprintf(STDERR, "Can't open DOS-file '%s'", source);
				return (-1);
			    }

			    newDirEntry.inode_num = inode_num;
			    strcpy(newDirEntry.name, destin);

			    modify_dir(in, "", &newDirEntry, 1, &i_mode);	//Insert entry in directory table

			    modifyInodeBitmap(inode_num, 1);	//set inode in inode bitmap
			    modifyInodeBitmap((int) NULL, 4);	//write bitmap to disk

			    block_num = sb.s_first_data_block;
			    memset(&newIn, 0, sizeof(inode));	//initialize new inode
			    totalBlocks = bytes = blocksInBuf = 0;

			    if (submodus==LINK)
			    { 	goto onlyLink;
			    }

			    do
			    {					//read DOS file into temp buffer
				bytes = fread(temp, 1, NBLOCK * BLOCK_SIZE, file);

			    	if (submodus==MKDIR)
			    	{   memcpy(temp,&inode_num,4);			//inode number of '.'
			    	    memcpy(temp+12,&directory_inode_num,4);	//inode number of '..'
			    	}


				DebugOut(1,"reading %lu byte from DOS\n", bytes);

				if (bytes == 0)
				    break;			//EOF

				blocksInBuf = bytes / BLOCK_SIZE;
				if (bytes % BLOCK_SIZE)
				    blocksInBuf++;

				//Allocate the blocks
				for (x = 0; x < blocksInBuf; x++)
				{				//triple indirection handling
				    if ((totalBlocks - 12 - (BLOCK_SIZE / 4)) == ((BLOCK_SIZE / 4L) * (BLOCK_SIZE / 4L)))
                                    {   if ((tind = (unsigned long *) MALLOC(BLOCK_SIZE)) == NULL)
                                        {   fprintf(STDERR, "Memory allocation error during file copy\n");
					    blocksInBuf = x;
					    break;
					}
					//Looking for a free block
					for (; block_num < sb.s_blocks_count; block_num++)	//continue at the last block_num
                                        {   if (modifyBlockBitmap(block_num, 2) == 0)
						break;
					}
					if (block_num >= sb.s_blocks_count)
                                        {   fprintf(STDERR, "Disk full no more blocks free - could not completely copy file\n");
					    blocksInBuf = x;
					    break;
					}
					modifyBlockBitmap(block_num, 1);	//set blocks in block bitmap
					sb.s_free_blocks_count--;	//adapt super block's free block count
					gt[(block_num - sb.s_first_data_block) / sb.s_blocks_per_group].bg_free_blocks_count--;

					tindBlock = block_num;

					DebugOut(1,"totalBlocks=%ld  Triple indirection block=%ld\n", totalBlocks, tindBlock);

					newIn.i_block[14] = tindBlock;

					memset((byte *) tind, 0, BLOCK_SIZE);
					indBlocks++;
				    }
				    //double indirection handling
				    if ((totalBlocks - 12 - (BLOCK_SIZE / 4)) % ((BLOCK_SIZE / 4L) * (BLOCK_SIZE / 4L)) == 0)
                                    {   if (dind != NULL)
                                        {

					    DebugOut(1,"writing double indirection block\n");

#ifdef UNIX
					    {			// fix byteorder, reverse done in inode.c
						int i;
						_u32 *p = dind;
						for (i = 0; i < blocks_per_block; i++, p++)
						    *p = cpu_to_le32(*p);
					    }
#endif
					    writedisk((byte *) dind, dindBlock * (BLOCK_SIZE / DISK_BLOCK_SIZE),
					    		             (unsigned short) ((dindBlock * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE);
					} else
                                        {   if ((dind = (unsigned long *) MALLOC(BLOCK_SIZE)) == NULL)
                                            {   fprintf(STDERR, "Memory allocation error during file copy\n");
						blocksInBuf = x;
						break;
					    }
					}
					//Looking for a free block
					for (; block_num < sb.s_blocks_count; block_num++)	//continue at the last block_num
                                        {   if (modifyBlockBitmap(block_num, 2) == 0)
						break;
					}
					if (block_num >= sb.s_blocks_count)
                                        {   fprintf(STDERR, "Disk full no more blocks free - could not completely copy file\n");
					    blocksInBuf = x;
					    break;
					}
					modifyBlockBitmap(block_num, 1);	//set blocks in block bitmap
					sb.s_free_blocks_count--;	//adapt super block's free block count
					gt[(block_num - sb.s_first_data_block) / sb.s_blocks_per_group].bg_free_blocks_count--;

					dindBlock = block_num;

					DebugOut(1,"totalBlocks=%ld  Double indirection block=%ld\n", totalBlocks, dindBlock);

					if (totalBlocks == 12 + BLOCK_SIZE / 4)
					    newIn.i_block[13] = dindBlock;
					else
					    tind[((totalBlocks - 12) / (BLOCK_SIZE / 4) - 1) % (BLOCK_SIZE / 4)] = dindBlock;

					memset((byte *) dind, 0, BLOCK_SIZE);
					indBlocks++;
				    }
				    //single indirection handling
				    if ((totalBlocks - 12) % (BLOCK_SIZE / 4) == 0)
                                    {   if (sind != NULL)	//write previous sind-block
                                        {

					    DebugOut(1,"writing single indirection block\n");

#ifdef UNIX
					    {			// fix byteorder, reverse done in inode.c
						int i;
						_u32 *p = sind;
						for (i = 0; i < blocks_per_block; i++, p++)
						    *p = cpu_to_le32(*p);
					    }
#endif
					    writedisk((byte *) sind, sindBlock * (BLOCK_SIZE / DISK_BLOCK_SIZE),
					    			    (unsigned short) ((sindBlock * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE);
					} else
                                        {   if ((sind = (unsigned long *) MALLOC(BLOCK_SIZE)) == NULL)
                                            {   fprintf(STDERR, "Memory allocation error during file copy\n");
						blocksInBuf = x;
						break;
					    }
					}
					//Looking for a free block
					for (; block_num < sb.s_blocks_count; block_num++)	//continue at the last block_num
                                        {   if (modifyBlockBitmap(block_num, 2) == 0)
						break;
					}
					if (block_num >= sb.s_blocks_count)
                                        {   fprintf(STDERR, "Disk full no more blocks free - could not completely copy file\n");
					    blocksInBuf = x;
					    break;
					}
					modifyBlockBitmap(block_num, 1);	//set blocks in block bitmap
					sb.s_free_blocks_count--;	//adapt super block's free block count
					gt[(block_num - sb.s_first_data_block) / sb.s_blocks_per_group].bg_free_blocks_count--;

					sindBlock = block_num;

					DebugOut(1,"totalBlocks=%ld  Single Indirection block=%ld\n", totalBlocks, sindBlock);

					if (totalBlocks == 12)
					    newIn.i_block[12] = sindBlock;
					else
					    dind[((totalBlocks - 12) / (BLOCK_SIZE / 4) - 1) % (BLOCK_SIZE / 4)] = sindBlock;

					memset((byte *) sind, 0, BLOCK_SIZE);
					indBlocks++;
				    }
				    //Looking for a free block
				    for (; block_num < sb.s_blocks_count; block_num++)		//continue at the last block_num
                                    {   if (modifyBlockBitmap(block_num, 2) == 0)
					    break;
				    }
				    if (block_num >= sb.s_blocks_count)
                                    {   fprintf(STDERR, "Disk full no more blocks free - could not completely copy file\n");
					blocksInBuf = x;
					break;
				    }
				    modifyBlockBitmap(block_num, 1);	//set blocks in block bitmap
				    sb.s_free_blocks_count--;	//adapt free block counts
				    gt[(block_num - sb.s_first_data_block) / sb.s_blocks_per_group].bg_free_blocks_count--;

				    if (totalBlocks < 12)
                                    {   newIn.i_block[totalBlocks] = block_num;		//pointers to blocks (no indirection)
					totalBlocks++;
				    } else
					//pointers to blocks (indirection)
                                    {   sind[(totalBlocks - 12) % (BLOCK_SIZE / 4)] = block_num;
					totalBlocks++;
				    }
				    blockList[x] = block_num;
				}

/*##############################################################################
   Note: what if we find no more free blocks -> break statements in the above block allocation routines
   we have to make sure, that we do not write too many blocks
   ##############################################################################
 */

				//now write the blocks to the Linux disk
				for (x = 0; x < blocksInBuf; x++)
                                {   if (disk_no < 128)		//with diskettes we write block per block
                                    {   writedisk((byte *) & temp[x * BLOCK_SIZE], blockList[x] * (BLOCK_SIZE / DISK_BLOCK_SIZE),
                                     						  (unsigned short) ((blockList[x] * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE);	//write to Linux-disk
				    } else
					/*with harddisk we do a multiblock write for blocks in sequence, this assumes a disk in LBA mode!!!
					   else we may get write errors #4 !!! */
                                    {   for (y = x + 1; y < blocksInBuf; y++)
                                        {   if (blockList[y] != blockList[y - 1] + 1)
						break;
					}

					DebugOut(1,"x=%ld Sequence of %ld blocks from block=%ld to %ld\n", x, y - x, blockList[x], blockList[y - 1]);

					writedisk((byte *) & temp[x * BLOCK_SIZE], blockList[x] * (BLOCK_SIZE / DISK_BLOCK_SIZE),
										  (unsigned short) ((blockList[x] * BLOCK_SIZE) % DISK_BLOCK_SIZE), (y - x) * BLOCK_SIZE);	//write to Linux-disk
					x = y - 1;
				    }
				}

				newIn.i_size = newIn.i_size + bytes;	//file size

			    }
			    while (bytes == NBLOCK * BLOCK_SIZE);
			    modifyBlockBitmap((int) NULL, 4);	//write bitmap to disk

			    if (sind != NULL)			//write sind-block
                            {

				DebugOut(1,"writing single indirection block\n");

#ifdef UNIX
				{				// fix byteorder, reverse done in inode.c
				    int i;
				    _u32 *p = sind;
				    for (i = 0; i < blocks_per_block; i++, p++)
					*p = cpu_to_le32(*p);
				}
#endif
				writedisk((byte *) sind, sindBlock * (BLOCK_SIZE / DISK_BLOCK_SIZE),
							(unsigned short) ((sindBlock * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE);
				FREE(sind);
			    }
			    if (dind != NULL)			//write dind-block
                            {

				DebugOut(1,"writing double indirection block\n");

#ifdef UNIX
				{				// fix byteorder, reverse done in inode.c
				    int i;
				    _u32 *p = dind;
				    for (i = 0; i < blocks_per_block; i++, p++)
					*p = cpu_to_le32(*p);
				}
#endif
				writedisk((byte *) dind, dindBlock * (BLOCK_SIZE / DISK_BLOCK_SIZE),
							(unsigned short) ((dindBlock * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE);
				FREE(dind);
			    }
			    if (tind != NULL)			//write tind-block
                            {

				DebugOut(1,"writing triple indirection block\n");

#ifdef UNIX
				{				// fix byteorder, reverse done in inode.c
				    int i;
				    _u32 *p = tind;
				    for (i = 0; i < blocks_per_block; i++, p++)
					*p = cpu_to_le32(*p);
				}
#endif
				writedisk((byte *) tind, tindBlock * (BLOCK_SIZE / DISK_BLOCK_SIZE),
							(unsigned short) ((tindBlock * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE);
				FREE(tind);
			    }
onlyLink:		    sb.s_free_inodes_count--;		//adapt superblock's free inode count
			    gt[(inode_num - 1) / sb.s_inodes_per_group].bg_free_inodes_count--;

			    if (submodus==MKDIR)
			    {	gt[(inode_num - 1) / sb.s_inodes_per_group].bg_used_dirs_count++;
			    	store_groups();
			        newIn.i_mode = 0x41FF;		// File mode -> Directory
			    	newIn.i_links_count = 2;	// Links count
			    	newIn.i_size = BLOCK_SIZE;	// Directory size
			    	newIn.i_blocks = (totalBlocks + indBlocks) * BLOCK_SIZE / DISK_BLOCK_SIZE;	// Blocks count
			    } else if (submodus==LINK)
			    {	store_groups();
			    	totalBlocks = 0;
			        indBlocks = 0;
			        newIn.i_mode = 0xA1FF;
			        newIn.i_links_count = 1;
			        newIn.i_version = 1;
			        newIn.i_flags = 0;
			        if (strlen(source)>15*4)
			            fprintf(STDERR,"WARNING: Symbolic link target name truncated\n");
			        newIn.i_size = min(strlen(source),15*4);
				strncpy((char*) &newIn.i_block[0],source,15*4);	 //check length!!!
			    } else
			    {	store_groups();
				newIn.i_mode = 0x81FF;		// File mode -> File
			    	newIn.i_links_count = 1;	// Links count
			    	newIn.i_blocks = (totalBlocks + indBlocks) * BLOCK_SIZE / DISK_BLOCK_SIZE;	// Blocks count
			    }

			    newIn.i_uid = 0;			// Owner Uid
			    newIn.i_uid_high = 0;
			    newIn.i_gid = 0;			// Group Id
			    newIn.i_gid_high = 0;
			    newIn.i_ctime = time(NULL);		// File creation time
			    newIn.i_mtime = newIn.i_ctime;	// File modification time
			    
			    if (strlen(Fmode) != 0)
			     	newIn.i_mode = ( newIn.i_mode & 0xF000 ) | fmode;
			    if (strlen(Uid) != 0)
			    {   newIn.i_uid = atol(Uid) & 0xFFFF;
				newIn.i_uid_high = atol(Uid) >> 16;
		  	    }
			    if (strlen(Gid) != 0)
			    {   newIn.i_gid = atol(Gid) & 0xFFFF;
				newIn.i_gid_high = atol(Gid) >> 16;
		  	    }

			    DebugOut(1,"writing inode\n");

			    store_inode(inode_num, &newIn);	//write inode to disk

			    /*for debugging: */ sb.s_state = 0;
			    //mark file system 'not clean'

			    DebugOut(1,"writing superblock\n");

			    if (write_super() == -1)		//write superblock to disk
                            {   fprintf(STDERR, "Error Writing Superblock\n");
				exit(-1);
			    }
#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			    lockDrive(0);			//unlock the drive
#endif
#endif
			    FREE(temp);
			    return (0);
			}
		}
		break;
	    }

	case LREAD:
            {   if ((temp = MALLOC(NBLOCK * BLOCK_SIZE)) == NULL)
                {   fprintf(STDERR, "Memory allocation error in lread\n");
		    return (-1);
		}
		while (inode_num)
                {   if ((in = load_inode(inode_num)) == NULL)
                    {   fprintf(STDERR, "Inode Error\n");
			return (-1);
		    }
		    if (!S_ISFIL(in->i_mode))			//Skip everything, which is not a file
		    {	printf("Skip %20s --- is not a file\n",inode_name);
		    	goto cont;
		    }
		    if (destin_mode == SELF)
			strncpy(destin, inode_name, sizeof(destin));
		    if (strcmp(destin, "STDOUT"))		//See if output to a file
                    {   if ((quiet == NO) && ((file = fopen(destin, "rb")) != NULL))
                        {   fclose(file);
			    fprintf(STDERR, "File '%s' exists, overwrite? [Y/N]  ", destin);
			    scanf("%c", &ans);
			    if ((char) toupper(ans) != 'Y')
                            {   fprintf(STDERR, "Could not copy file '%s' to '%s'\n", source, destin);
				return (-1);
			    }
			}
#ifndef UNIX
#if !defined(__NT__) && !defined(__OS2__)
			fd = dos_long_creat(destin);
#else
			fd = creat(destin, S_IREAD | S_IWRITE | O_BINARY);
#endif
#else
			fd = creat(destin, 0666);
#endif
			if (fd == -1)
                        {   fprintf(STDERR, "Problem opening second argument file (DOS file) %s %s\n", sourcebak, destin);
			    return (-1);
			}
		    } else
			//we want output to stdout
                    {   fd = 1;
		    }

		    if ((bytes = read_inode(in, temp, NBLOCK * BLOCK_SIZE, KEEP_BUF)) == -1)	// read first blocks
                    {   fprintf(STDERR, "Error Reading Inode\n");
			return (0);
		    }
		    do
                    {   if ((x = write(fd, temp, bytes)) != bytes)	// write to disk
                        {   fprintf(STDERR, "Error Writing DOS File %s  written %lu Byte  should write %lu Byte - Probably disk full\n", destin, x, bytes);
			    return (-1);
			}
			bytes = read_inode(NULL, temp, NBLOCK * BLOCK_SIZE, KEEP_BUF);
			if (bytes == -1)			// problem reading
                        {   fprintf(STDERR, "Error Reading 2\n");
			    return (-1);
			}
		    }
		    while (bytes != 0);
		    if (bytes)
			fprintf(STDERR, "*** Problem encountered reading file %s***\n", sourcebak);
		    if (fd > 2)
			close(fd);
cont:		    read_inode(NULL, NULL, 0, KILL_BUF);	//release the buffer
		    strcpy(source, sourcebak);
		    inode_num = search_dir(NULL, NULL, inode_name, KEEP_BUF);
		}
		search_dir(NULL, NULL, inode_name, KILL_BUF);
		FREE(temp);
		break;
	    }
	case LTEST:
	    {							//we never should come here
	    }

    }
    return (0);
}

//***** ldirNT.dll interface ***************
//----- work in progress--------------------
#ifdef USEDLL
__declspec(dllexport) void WINAPI ldirNT(int argc, char* argv[])
{   main(argc, argv);
}
#endif
