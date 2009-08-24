# $Id: makefile,v 1.1.1.1 2003/05/21 13:38:40 pasha Exp $

#       SCCSID = @(#)1.57  7/22/99 13:11:12 src/jfs/ifs/makefile, sysjfs, w45.fs32, fixbld
#----------------------------------------------------------------------------
# MAKEFILE for the ifs directory
#
# COMPONENT_NAME: sysjfs
#
#   Copyright (c) International Business Machines  Corp., 2000
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#----------------------------------------------------------------------------
#
#  Change History :
#
#
#----------------------------------------------------------------------------
# SUBSYS is a macro to define the name of the subsystem that this makefile is a
# part of.   It is the name of the subdirectory that is above all of the
# individual components.
#
# SUBSYS_RELATIVE is a macro to define the name of this component.
# SRC_RELATIVE is the full path for the current directory from the
# ROOT.
#
# This uses the substitution feature which takes the current directory of
# this makefile (MAKEDIR) and removes the relative part of the directory to
# get the TOP level directory.
#
# The other macros are used to build directories in other levels of the tree.
#------------------------------------------------------------------------------

SUBSYS          = jfs
SUBSYS_RELATIVE = ifs
SRC_RELATIVE    = src\$(SUBSYS)\$(SUBSYS_RELATIVE)
SRC_NL_RELATIVE = mri\$(NLV)\$(SUBSYS)\$(SUBSYS_RELATIVE)


#------------------------------------------------------------------------------
#  Use built in macro "MAKEDIR" to establish "ROOT".  Due to case sensitivity
#
#  of nmake macro substitution, try all lowercase then all uppercase.  Fail if
#  current working directory path is mixed case.  This is a current nmake
#  limitation on macro substitutions but should not be a problem for most
#  people.  We are searching for a fix to this problem.
#------------------------------------------------------------------------------
# \src\mm\misc\mmpmcrt is an example of the path for a specific component
# in the tree structure. Please change the path as applied to your component.
# Refer to Nmake documentation, if you have any questions about MAKEDIR
#------------------------------------------------------------------------------

ROOT            = $(MAKEDIR:src\jfs\ifs=)
!if "$(ROOT)" == "$(MAKEDIR)"
ROOT            = $(MAKEDIR:SRC\JFS\IFS=)
!endif
!if "$(ROOT)" == "$(MAKEDIR)"
!error Problem setting "ROOT"! Make sure current working directory is either all\
uppercase or all lowercase!
!endif

#-----------------------------------------------------------------
# Set up component/subsystem specific macros for use in header.mak
# This allows you establish a search order for header and lib paths.  A
# prefix would be a path that you would like to add to the beginning of
# the standard path which is set in header.mak.  A suffix tags onto the
# end of the standard path.
#-----------------------------------------------------------------
# If using Visual Age C++ you need to specify the version required
VACPP_VER =v36

#CMP_HDR_PREFIX = <specify non-default path(s) for front of INCLUDE path>

#CMP_HDR_PREFIX =
#CMP_HDR_SUFFIX = <specify non-default path(s) for back INCLUDE path>
#CMP_HDR_SUFFIX=
#CMP_LIB_PREFIX = <specify non-default path(s) for front of LIB path>
#CMP_LIB_PREFIX= $(SUBSYS_LIB)
#CMP_LIB_SUFFIX = <specify non-default path(s) for back LIB path>
#CMP_LIB_SUFFIX= $(SUBSYS_LIB)

#-----------------------------------------------------------------------
#INSTPATH = <specify the path for the files to be put in release tree>
#Instpath should be defined to get to the root of the install tree.
#For Example, suppose you want to place binaries in
#   \wpos\rel\386\retail\os2\toolkt\version1
# \wpos\rel\386\retail would be represented by $(R)
# Instpath = \os2\toolkt\version1
# Since REL=$(R)+$(INSTPATH), REL will become
# \wpos\rel\386\retail\os2\toolkt\version1
#-----------------------------------------------------------------------


# Uncomment to use ibmc libs.  With this out, it uses the
# default metaware libs location.
USE_IBMC_LIB = 1

#-----------------------------------------------------------------------------
# PUBLIC can be one of "ibmmk", "cps","os2".  Please define the below macros
# if you want to include the public files in a different order.  These macros
# would be used if you have a preference as to which directory gets searched
# first.  For example, you would like to search the OS2 header or include
# files before you search the ibmmk header or include files.  In this case,
# you would specify PUBLIC1 = OS2 and PUBLIC2= IBMMK and PUBLIC3 = CPS.
# These macros are then used in header.mak to set the path order.
#------------------------------------------------------------------------------

PUBLIC1 = os2
PUBLIC2 = os2\16bit
PUBLIC3 = os2\inc

#------------------------------------------------------------------------------
# HOST_PROC and HOST_OS will be set in the environment by the build lab.
# Set them in your private builds.
# Please don't set these variables in the public build.
#-----------------------------------------------------------------------------

HOST_PROC=386
HOST_OS=OS2

#------------------------------------------------------------------------------
# header.mak is the public makefile that sets up all paths and options.  It
# should not be altered.  Additions or changes to header.mak for a component
# can be overwritten in this makefile.  Additions or changes to header.mak
# for an entire subsystem can be overwritten in subsys.mak
#-----------------------------------------------------------------------------

!include $(ROOT)\header.mak

#-------------------------------------------------------------------------
# subsys.mak is used as a way for each subsystem to override any settings
# that were set in header.mak.  It is owned by development.
#-------------------------------------------------------------------------

!include $(ROOT)\src\$(SUBSYS)\common\subsys.mak

#----------------------------------------------------------------
# Set up the list of suffixes, if the suffixes in header.mak are not
# sufficient.  Be sure to include the entire list.
#-----------------------------------------------------------------

#.suffixes:

#------------------------------------------------------------------------
# Please define all compiler, assembler, and linker flags here.
# Use if's to define the flags for each type of build.
#------------------------------------------------------------------------
!ifdef BLD_TYPE
!   if "$(BLD_TYPE)"!="retail"
!      if "$(BLD_TYPE)"!="debug"
!         if "$(BLD_TYPE)"!="dekko"
!            if "$(BLD_TYPE)"!="icatgam"
!               error Unsupported build type!  We only differentiate between retail and debug
!            else
DBG_C16FLAGS= /Zi
DBG_CFLAGS= /Ti+ /O- /Os- -D_FS_OP_TRACE -D_JFS_DEBUG -DMMIOPH
DBG_AFLAGS= -Zi
DBG_LFLAGS= /Debug /NOEXEPACK
!            endif
!         else
DBG_C16FLAGS=
DBG_CFLAGS= /O+ /Os- -DMMIOPH
DBG_AFLAGS=
DBG_LFLAGS= /nod
!         endif
!      else
DBG_C16FLAGS=
DBG_CFLAGS= -D_FS_OP_TRACE -D_JFS_DEBUG -DMMIOPH
DBG_AFLAGS= $(DBG_CFLAGS)
DBG_LFLAGS= /nod
!      endif
!   else
DBG_C16FLAGS=
DBG_CFLAGS= /O+ /Os-
DBG_AFLAGS=
DBG_LFLAGS= /nod
!   endif
!endif

ADEFINES = -D_JFS_OS2
CDEFINES = -D_JFS_OS2 -D_JFS_LAZYCOMMIT -D_JFS_CACHESTATS -D_JFS_STATISTICS
AFLAGS= -Mx -t -p -W1 $(ADEFINES) $(DBG_AFLAGS)
CFLAGS= /Mt /Rn /C /Ss /Sp1 /W2 /Q /Gr /qtune=blend $(CDEFINES) $(DBG_CFLAGS)
LFLAGS= /map /nop /nopackd /nofix /exepack:2 /align:2 $(DBG_LFLAGS)
C16FLAGS= /G2s /Zlp /PLM /W3 /c -nologo $(DBG_C16FLAGS)

#--------------------------------------------------------------------------
# If the Inference rules defined in header.mak are not sufficient, then
# define the inference rules necessary at this point.
# The following inference rules are just an example.
# You can delete this section if you don't need to add more inference rules.
#-------------------------------------------------------------------------

.asm{$(O)}.obj:
        $(AS) $(AFLAGS) $(AINC) $<

#        $(AS) $(AFLAGS) $(AINC) $<,$@;

.asm.lst:
        $(AS) -l -n $(AFLAGS) $(AINC) $<,nul,$@;

.c{$(O)}.obj:
        $(CC) $(CFLAGS) /Fo$@ /Fa$(O)\$(@B).lst $<

.c.lst:
        $(CC) $(CFLAGS) /Fa$@ $<

#------------------------------------------------------------------------
#Component defines start here.
#------------------------------------------------------------------------


LIB386  = os2386.lib
LIBP    = os2386p.lib
LIBDDE  = cpprns36.lib
KEELIB  = $(R_OS2_LIB)\kee.lib
FSHELP  = $(R_OS2_LIB)\fshelper.lib
LIBS    = $(LIB386) $(LIBP) $(LIBDDE) $(FSHELP) $(KEELIB) \
          doscalls.lib
#          $(R_OS2_LIB)\doscalls.lib - this is a part of the LIB env var. (DDK)

RTRACE  = $(OS2)\system\trace

OBJS  = $(O)\fs_ioctl.obj   $(O)\thunk.obj        \
        $(O)\sel2flat.obj   $(O)\flat2sel.obj     \
        $(O)\devhelp.obj    $(O)\debug.obj        $(O)\misc.obj         \
        $(O)\selector.obj   $(O)\dprintf.obj      $(O)\jfs_init1.obj    \
        $(O)\tmconv.obj     $(O)\jfs_strat2.obj   $(O)\jfs_chkdsk.obj   \
        $(O)\jfs_ifs.obj    $(O)\jfs_debug.obj    $(O)\jfs_lock.obj     \
        $(O)\jfs_mount.obj  $(O)\jfs_umount.obj   $(O)\jfs_init_os2.obj \
        $(O)\jfs_extent.obj $(O)\jfs_inode.obj    $(O)\jfs_bufmgr.obj   \
        $(O)\jfs_logmgr.obj $(O)\jfs_cachemgr.obj $(O)\jfs_hold.obj     \
        $(O)\jfs_dmap.obj   $(O)\jfs_dtree.obj    $(O)\jfs_imap.obj     \
        $(O)\jfs_rele.obj   $(O)\jfs_sync.obj     $(O)\jfs_txnmgr.obj   \
        $(O)\jfs_util.obj   $(O)\jfs_xtree.obj    $(O)\jfs_ea.obj       \
        $(O)\jfs_finfo.obj  $(O)\jfs_mkdir.obj    $(O)\jfs_dnlc.obj     \
        $(O)\jfs_lookup.obj $(O)\jfs_getattr.obj  $(O)\jfs_create.obj   \
        $(O)\jfs_open.obj   $(O)\jfs_close.obj    $(O)\jfs_readdir.obj  \
        $(O)\jfs_remove.obj $(O)\jfs_rename.obj   $(O)\jfs_rmdir.obj    \
        $(O)\jfs_rdwr.obj   $(O)\jfs_fsync.obj    $(O)\jfs_ftruncate.obj \
        $(O)\jfs_dio.obj    $(O)\jfs_unicode.obj  $(O)\jfs_pageio.obj   \
        $(O)\jfs_rawio.obj  $(O)\jfs_filelock.obj $(O)\jfs_defragfs.obj \
        $(O)\jfs_extendfs.obj $(O)\jfs_acl.obj    $(O)\jfs_clrbblks.obj \
        $(O)\jfs_dasdlim.obj


#--------------------------------------------------------------------------
# The all target is used to specify a list of targets that need to be built
# Please don't delete "dep" from the target list.  It is used to generate the
# depend.mak file.
# Please look at the example for more details.
#-------------------------------------------------------------------------

all: dep $(OS2)\jfs.ifs # $(RTRACE)\jfs.tdf

#16 bit C code
$(O)\fs_ioctl.obj:
        $(CC16) $(C16FLAGS) /I $(R_OS2_H16) /I $(MSC16INC) /Fo$@ $(@B).c

#32 bit IFS
$(OS2)\jfs.ifs: $(OBJS) $(O)\$(@B).def makefile
        $(CREATE_PATH)
        -4$(LINK) $(LFLAGS) @<<$(O)\$(@B).lnk
$(OBJS: =+^
)
$@
$(O)\$(@B).map
$(LIBS)
$(O)\$(@B).def
; $@ end of generated file (see makefile)
<<keep
        cd $(@D)
        mapsym $(O)\$(@B).map
        cd $(MAKEDIR)

$(O)\jfs.def: makefile
         rem creating def file<<$@
LIBRARY jfs
PROTMODE
DATA PRELOAD SINGLE SHARED MOVABLE
CODE PRELOAD MOVABLE
EXPORTS
        FS_ATTRIBUTE
        FS_NAME
        FS_MPSAFEFLAGS2
        FS32_ATTRIBUTE

        FS_ALLOCATEPAGESPACE = fs_allocatepagespace16
        FS_ATTACH = fs_attach16
        FS_CANCELLOCKREQUEST = fs_cancellockrequest16
        FS_CANCELLOCKREQUESTL = fs_cancellockrequestl16
        FS_CHDIR = fs_chdir16
        FS_CHGFILEPTR = fs_chgfileptr16
        FS_CLOSE = fs_close16
        FS_COMMIT = fs_commit16
        FS_COPY = fs_copy16
        FS_DELETE = fs_delete16
        FS_DOPAGEIO = fs_dopageio16
        FS_EXIT = fs_exit16
        FS_FILEATTRIBUTE = fs_fileattribute16
        FS_FILEINFO = fs_fileinfo16
        FS_FILEIO = fs_fileio16
        FS_FILELOCKS = fs_filelocks16
        FS_FILELOCKSL = fs_filelocksl16
        FS_FINDCLOSE = fs_findclose16
        FS_FINDFIRST = fs_findfirst16
        FS_FINDFROMNAME = fs_findfromname16
        FS_FINDNEXT = fs_findnext16
        FS_FINDNOTIFYCLOSE = fs_findnotifyclose16
        FS_FINDNOTIFYFIRST = fs_findnotifyfirst16
        FS_FINDNOTIFYNEXT = fs_findnotifynext16
        FS_FLUSHBUF = fs_flushbuf16
        FS_FSCTL = fs_fsctl16
        FS_FSINFO = fs_fsinfo16
        FS_INIT = fs_init16
        FS_IOCTL
        FS_MKDIR = fs_mkdir16
        FS_MOUNT = fs_mount16
        FS_MOVE = fs_move16
        FS_NEWSIZE = fs_newsize16
        FS_NEWSIZEL = fs_newsizel16
        FS_NMPIPE = fs_nmpipe16
        FS_OPENCREATE = fs_opencreate16
        FS_OPENPAGEFILE = fs_openpagefile16
        FS_PATHINFO = fs_pathinfo16
        FS_PROCESSNAME = fs_processname16
        FS_READ = fs_read16
        FS_RMDIR = fs_rmdir16
        FS_SETSWAP = fs_setswap16
        FS_SHUTDOWN = fs_shutdown16
        FS_WRITE = fs_write16
        FS32_READ
        FS32_WRITE
        FS32_CHGFILEPTR
        FS32_CHGFILEPTRL
        FS32_READFILEATCACHE
        FS32_RETURNFILECACHE
IMPORTS
        DosQuerySysInfo = DOSCALLS.348
        TCBUser_ID = DOSCALLS.470
<<keep

#       @@@@@@@@@@@@@@@@@@@  Creating Trace Files  @@@@@@@@@@@@@@@@@@@
#
#       Trcust takes 3 inputs and creates 2 outputs.
#       The inputs are:
#                      jfs.tsf
#                      jfs.map
#                      jfs.ifs
#
#       The outputs are:
#                      jfs.tdf
#                      trc012F.tff

$(RTRACE)\jfs.tdf: jfs.tsf $(OS2)\jfs.ifs
        if not exist $(RTRACE) makepath $(RTRACE)
        cd $(O)
        trcust $(ROOT)\$(SRC_RELATIVE)\jfs.tsf /m=jfs.map /l=$(OS2)\jfs.ifs > jfs.trc
        -copy jfs.tdf $(RTRACE)
        -copy trc012F.tff $(RTRACE)
        @del jfs.tdf
        @del trc012F.tff
        cd $(MAKEDIR)


#***************************************************************************
# Include Footer.mak at this point.
#***************************************************************************




#****************************************************************************
#  Dependency generation and Checking
#****************************************************************************

$(O)\depend.mak:  $(O)\dephold
        -$(CREATE_PATH)
        -$(TOUCH) $(O)\depchk
        -echo generate depend.mak
        includes -e -i -l -S$$(O) -P$$(ROOT)=$(ROOT) -C=hh -C=xh -C=api *.c >$@ 2>$(O)\depend.err
       -del $(O)\depchk

$(O)\dephold:
        -$(TOUCH) $@


!include $(ROOT)\footer.mak
