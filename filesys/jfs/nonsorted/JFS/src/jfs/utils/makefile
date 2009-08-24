# $Id: makefile,v 1.1.1.1 2003/05/21 13:38:50 pasha Exp $ 

#       SCCSID = @(#)1.20  1/13/99 18:14:42 src/jfs/utils/makefile, jfsutil, w45.fs32, 990417.1
#----------------------------------------------------------------------------
# MAKEFILE for the utils\ujfs   directory
#
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
#
#                    HISTORY
#+++ PS add any utils to compile
#
# COMPONENT_NAME: UJFS
#
#----------------------------------------------------------------------------
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
SUBSYS_RELATIVE = utils
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

ROOT            = $(MAKEDIR:\src\jfs\utils=)
!if "$(ROOT)" == "$(MAKEDIR)"
ROOT            = $(MAKEDIR:\SRC\JFS\UTILS=)
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
VACPP_VER = v36

#CMP_HDR_PREFIX = <specify non-default path(s) for front of INCLUDE path>

#CMP_HDR_PREFIX = /I. /I$(ROOT)\src\$(SUBSYS)\utils\libfs        \
#                /I$(ROOT)\src\$(SUBSYS)\common\include          \
#                /I$(ROOT)\src\$(SUBSYS)\common\include\os2      \
#                /I$(COMPILER_HDR1)
#CMP_HDR_SUFFIX = <specify non-default path(s) for back INCLUDE path>
#CMP_HDR_SUFFIX=
#CMP_LIB_PREFIX = <specify non-default path(s) for front of LIB path>
CMP_LIB_PREFIX= $(COMPILER_LIB_PATH)
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
PUBLIC2 = cps
#PUBLIC2 = os2\16bit

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
!         error Unsupported build type!  We only differentiate between retail and debug
!      else
LFLAGS= /map /nop /DE:NONE /nopackd /NOE /NOFREE /nofix /exepack:2 /align:2
!      endif
!   else
LFLAGS= /map /nop /nopackd /NOE /NOFREE /nofix /exepack:2 /align:2
!   endif
!endif
# +++
# DEFINES = /DM_I386 /D_JFS_OS2 /D_JFS_UTILITY /DINCL_NOPMAPI /DONE_FILESET_PER_AGGR

#LFLAGS= /map /nop /DE:NONE /nopackd /NOE /NOFREE
#--------------------------------------------------------------------------
# If the Inference rules defined in header.mak are not sufficient, then
# define the inference rules necessary at this point.
# The following inference rules are just an example.
# You can delete this section if you don't need to add more inference rules.
#-------------------------------------------------------------------------


#------------------------------------------------------------------------
#Component defines start here.
#------------------------------------------------------------------------

LIBS    =  libconv.lib+cpprss36.lib+os2386.lib

OBJS = $(OBASE)\utils\libfs\ujfslib.lib $(OBASE)\utils\format\format.lib $(OBASE)\utils\chkdsk\chkdsk.lib

#OBJS  = $(O)\..\format\initmap.obj  $(O)\..\format\inodemap.obj  $(O)\..\format\inodes.obj  \
#        $(O)\..\format\xmkfs.obj


#--------------------------------------------------------------------------
# The all target is used to specify a list of targets that need to be built
# Please don't delete "dep" from the target list.  It is used to generate the
# depend.mak file.
# Please look at the example for more details.
#-------------------------------------------------------------------------

all: libfs logview cstats clrbblks logredo logform logdump cachejfs xchkdmp xchklog chklog xpeek validfs extendfs defragfs format chkdsk os2stats chkexe $(DLL)\ujfs.dll

chkexe:
      cd chkexe
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

extendfs:
      cd extendfs
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

defragfs:
      cd defrag
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

cstats:
      cd cstats
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

os2stats:
      cd os2stats
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

logredo:
      cd logredo
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

logview:
      cd logview
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

clrbblks:
      cd clrbblks
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

logdump:
      cd logdump
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

logform:
      cd logform
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

cachejfs:
      cd cachejfs
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

xchkdmp:
      cd xchkdmp
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

validfs:
      cd validfs
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

xchklog:
      cd xchklog
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

chklog:
      cd chklog
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

xpeek:
      cd xpeek
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      cd ..\

libfs:
      cd libfs
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      CD ..\

chkexe:
      cd chkexe
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      CD ..\

format:
      cd format
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      CD ..\

chkdsk:
      cd chkdsk
      $(MAKE) -$(MAKEFLAGS) $(OPT)
      CD ..\

$(DLL)\ujfs.dll: makefile $(OBJS) $(O)\$(@B).def $(O)\format\format.lib \
                 $(O)\chkdsk\chkdsk.lib $(O)\libfs\ujfslib.lib
        $(CREATE_PATH)
        $(LINK) $(LFLAGS)  @<<$(O)\$(@B).lnk
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

$(O)\ujfs.def: makefile
         rem creating def file<<$@
LIBRARY UJFS INITINSTANCE
DESCRIPTION 'UJFS'
PROTMODE
CODE PRELOAD
DATA PRELOAD
EXPORTS
  chkdsk32
  FORMAT
  CHKDSK
STACKSIZE 16384
<<keep
#***************************************************************************
# Include Footer.mak at this point.
#***************************************************************************




!include $(ROOT)\footer.mak
