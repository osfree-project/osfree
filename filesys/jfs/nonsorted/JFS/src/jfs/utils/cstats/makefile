# $Id: makefile,v 1.1.1.1 2003/05/21 13:41:12 pasha Exp $ 

# 	SCCSID = @(#)1.1  9/15/99 09:40:30 src/jfs/utils/cstats/makefile, jfsutil, w45.fs32, fixbld
#----------------------------------------------------------------------------
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
# MAKEFILE for the utils\cstats directory
#
# COMPONENT_NAME: cstats
#
# FUNCTIONS: Report cache statistics
#
# ORIGINS: Linked to os2
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
SUBSYS_RELATIVE = utils\cstats
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

ROOT            = $(MAKEDIR:\src\jfs\utils\cstats=)
!if "$(ROOT)" == "$(MAKEDIR)"
ROOT            = $(MAKEDIR:\SRC\JFS\UTILS\CSTATS=)
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

CMP_HDR_PREFIX = /I. \
		/I$(ROOT)\src\$(SUBSYS)\common\include		\
		/I$(ROOT)\rel\os2c\include\base\cps	\
		/I$(COMPILER_HDR1)
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
PUBLIC2 = os2\16bit

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
DBG_CFLAGS =  /Ti+ /D_JFS_DEBUG
DBG_LFLAGS = /de:none
!      endif
!   else
DBG_CFLAGS=
DBG_LFLAGS =
!   endif
!endif
DEFINES = /DM_I386 /D_JFS_OS2 /D_JFS_UTILITY /DINCL_NOPMAPI /DONE_FILESET_PER_AGGR /D_JFS_CACHESTATS
CFLAGS= /C /Ss /W3 /G5 /Q /Rn $(DEFINES) /DTRACE $(DBG_CFLAGS)
LFLAGS= /nod /map /exepack:2 /align:2 $(DBG_LFLAGS)
AFLAGS= -Mx -t -p -W1 -D_JFS_OS2 -D_JFS_UTILITY
#--------------------------------------------------------------------------
# If the Inference rules defined in header.mak are not sufficient, then
# define the inference rules necessary at this point.
# The following inference rules are just an example.
# You can delete this section if you don't need to add more inference rules.
#-------------------------------------------------------------------------

#.asm{$(O)}.obj:

.asm.obj:
        masm $(AFLAGS) $(AINC) $*.asm,$*.obj;

.c{$(O)}.obj:
        $(CC) $(CFLAGS) $(CMP_HDR_PREFIX) /Fo$@ $<


#------------------------------------------------------------------------
#Component defines start here.
#------------------------------------------------------------------------

LIBS    = cpprss36.lib os2386.lib

OBJS  = $(O)\cstats.obj

#--------------------------------------------------------------------------
# The all target is used to specify a list of targets that need to be built
# Please don't delete "dep" from the target list.  It is used to generate the
# depend.mak file.
# Please look at the example for more details.
#-------------------------------------------------------------------------

all: $(OS2)\cstats.exe

$(OS2)\cstats.exe: makefile $(OBJS) $(O)\$(@B).def
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

$(O)\cstats.def: makefile
         rem creating def file<<$@
NAME $(@B)
DESCRIPTION 'CSTATS'
PROTMODE
CODE PRELOAD
DATA PRELOAD
STACKSIZE 65536
<<keep


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
