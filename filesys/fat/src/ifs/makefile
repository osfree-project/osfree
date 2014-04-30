#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = fat32.ifs
DESC = FAT32 filesystem driver
#defines object file names in format objname.$(O)
srcfiles = $(p)fat32$(e) $(p)ifsmount$(e) $(p)ifsfind$(e) $(p)ifsdir$(e) $(p)ifsattr$(e) &
     $(p)ifsfile$(e) $(p)ifslog$(e) $(p)ifscache$(e) $(p)ifsswap$(e) $(p)ifsmem$(e) &
     $(p)ifsea$(e) $(p)ifslang$(e)
# defines additional options for C compiler
ADD_COPT = -2 -ml -ecw -s -zdp -zgf -zls -zp=1 -zu -ze -zq -od -of+ -d__16BITS__ -d__WATCOM -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)libc.lib
NOLIB   = 1
DLL     = 1
DLLOPT  = initglobal
EXPORTS = &
    FS_ALLOCATEPAGESPACE, &
    FS_ATTACH, &
#      Not yet implemented
    FS_CANCELLOCKREQUEST, &
    FS_CHDIR, &
    FS_CHGFILEPTR, &
    FS_CLOSE, &
    FS_COMMIT, &
    FS_COPY, &
    FS_DELETE, &
    FS_DOPAGEIO, &
    FS_EXIT, &
    FS_FILEATTRIBUTE, &
    FS_FILEINFO, &
#      Not yet implemented
    FS_FILEIO, &
#      Not yet implemented
    FS_FILELOCKS, &
    FS_FINDCLOSE, &
    FS_FINDFIRST, &
    FS_FINDFROMNAME, &
    FS_FINDNEXT, &
#      Obsolete in OS/2 WARP
    FS_FINDNOTIFYCLOSE, &
#      Obsolete in OS/2 WARP
    FS_FINDNOTIFYFIRST, &
#      Obsolete in OS/2 WARP
    FS_FINDNOTIFYNEXT, &
    FS_FLUSHBUF, &
    FS_FSCTL, &
    FS_FSINFO, &
    FS_INIT, &
    FS_IOCTL, &
    FS_MKDIR, &
    FS_MOUNT, &
    FS_MOVE, &
    FS_NEWSIZE, &
    FS_NMPIPE, &
    FS_OPENCREATE, &
    FS_OPENPAGEFILE, &
    FS_PATHINFO, &
    FS_PROCESSNAME, &
    FS_READ, &
    FS_RMDIR, &
    FS_SETSWAP, &
    FS_SHUTDOWN, &
#      N/A : local FSD
#   FS_VERIFYUNCNAME, &
    FS_WRITE, &
#
    FS_ATTRIBUTE, &
    FS_NAME
#    FS_MPSAFEFLAGS2
IMPORTS = &
    FSH_SEGALLOC FSHELPER.1, &
    FSH_SEGFREE FSHELPER.2, &
    FSH_SEGREALLOC FSHELPER.3, &
    FSH_FORCENOSWAP FSHELPER.4, &
    FSH_INTERR FSHELPER.5, &
    FSH_SEMREQUEST FSHELPER.6, &
    FSH_SEMCLEAR FSHELPER.7, &
    FSH_PROBEBUF FSHELPER.8, &
    FSH_GETPRIORITY FSHELPER.9, &
    FSH_IOSEMCLEAR FSHELPER.10, &
    FSH_FLUSHBUF FSHELPER.11, &
    FSH_CRITERROR FSHELPER.12, &
    FSH_DEVIOCTL FSHELPER.13, &
    FSH_GETVOLPARM FSHELPER.14, &
    FSH_FINDDUPHVPB FSHELPER.15, &
    FSH_DOVOLIO FSHELPER.16, &
    FSH_ADDSHARE FSHELPER.17, &
    FSH_REMOVESHARE FSHELPER.18, &
    FSH_GETOVERLAPBUF FSHELPER.19, &
    FSH_ISCURDIRPREFIX FSHELPER.20, &
    FSH_LOADCHAR FSHELPER.21, &
    FSH_PREVCHAR FSHELPER.22, &
    FSH_STORECHAR FSHELPER.23, &
    FSH_SEMSET FSHELPER.24, &
    FSH_SEMSETWAIT FSHELPER.25, &
    FSH_SEMWAIT FSHELPER.26, &
    FSH_WILDMATCH FSHELPER.27, &
    FSH_YIELD FSHELPER.28, &
    FSH_DOVOLIO2 FSHELPER.29, &
    FSH_FINDCHAR FSHELPER.30, &
#    MFSH_SETBOOTDRIVE FSHELPER.31
    FSH_CANONICALIZE FSHELPER.32, &
#    MFSH_DOVOLIO FSHELPER.33
#    MFSH_INTERR  FSHELPER.34
#    MFSH_SEGALLOC FSHELPER.35
#    MFSH_SEGFREE FSHELPER.36
#    MFSH_SEGREALLOC FSHELPER.37
#    MFSH_CALLRM FSHELPER.38
#    MFSH_LOCK FSHELPER.39
#    MFSH_PHYSTOVIRT FSHELPER.40
#    MFSH_UNLOCK FSHELPER.41
#    MFSH_UNPHYSTOVIRT FSHELPER.42
#    MFSH_VIRT2PHYS FSHELPER.43
    FSH_QSYSINFO FSHELPER.44, &
    FSH_NAMEFROMSFN FSHELPER.45, &
    FSH_UPPERCASE FSHELPER.46, &
    FSH_CHECKEANAME FSHELPER.47, &
    FSH_CALLDRIVER FSHELPER.48, &
    FSH_SETVOLUME FSHELPER.49, &
    FSH_STACKSPACE FSHELPER.50, &
    FSH_REGISTERPERFCTRS FSHELPER.51, &
    FSH_IOBOOST FSHELPER.52, &
    FSH_QUERYSERVERTHREAD FSHELPER.53, &
    FSH_QUERYOPLOCK FSHELPER.54, &
    FSH_EXTENDTIMESLICE FSHELPER.55
#    MFSH_SYSCTL FSHELPER.56

DEST    = os2$(SEP)boot

!include $(%ROOT)/mk/appsos2v1.mk
