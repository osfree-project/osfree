#
# A Makefile for minifsd
# (c) osFree project,
# author, date
#

PROJ     = mfs
DESC     = A mini file system driver
srcfiles = $(p)mfs$(e) $(p)fs_stubs$(e) $(p)fs_all$(e)
# linking dll file
DLL     = 1
EXPORTS = &
#  Optional
#   FS_ALLOCATEPAGESPACE,
    FS_ATTACH, &
# Optional
#   FS_CANCELLOCKREQUEST
    FS_CHDIR, &
    FS_CHGFILEPTR, &
    FS_CLOSE, &
    FS_COMMIT, &
    FS_COPY, &
    FS_DELETE, &
# Optional
#   FS_DOPAGEIO,
    FS_EXIT, &
    FS_FILEATTRIBUTE, &
    FS_FILEINFO, &
# Optional
    FS_FILEIO, &
# Optional
#   FS_FILELOCKS, &
    FS_FINDCLOSE, &
    FS_FINDFIRST, &
    FS_FINDFROMNAME, &
    FS_FINDNEXT, &
# Obsolete in OS/2 WARP
    FS_FINDNOTIFYCLOSE, &
# Obsolete in OS/2 WARP
    FS_FINDNOTIFYFIRST, &
# Obsolete in OS/2 WARP
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
# Optional
#   FS_OPENPAGEFILE,
    FS_PATHINFO, &
    FS_PROCESSNAME, &
    FS_READ, &
    FS_RMDIR, &
    FS_SETSWAP, &
    FS_SHUTDOWN, &
# N/A : local FSD
#   FS_VERIFYUNCNAME,
    FS_WRITE, &
#
# MINI FSD entry points
#
    MFS_CHGFILEPTR, &
    MFS_CLOSE, &
    MFS_INIT, &
    MFS_OPEN, &
    MFS_READ, &
    MFS_TERM

#OPTIONS = heapsize=32K libfile libentry
#OPTIONS =
DEST = os2

!include $(%ROOT)/mk/appsos2v1.mk
