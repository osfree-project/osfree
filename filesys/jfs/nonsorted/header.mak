# $Id: header.mak,v 1.2 2002/04/28 21:28:22 bird Exp $
#
# Header.mak
#
# This file will (try) setup the build environment. I have no idea what this
# file look like in the real build environment at IBM. But, I have made an
# attempt to create a configuration file which sets ut the build environment
# need to build JFS.
#
# We assumes that setenv.cmd is executed and environment variables are set.
#
#
# Wild guess how this works in the real build environment:
#   Each compiler is invoked thru an .cmd file which sets correct variables?
#   In which other way that this could they have 'cl' and 'icc' use different
#   runtime include files?
#

#
# Global environment changes
#
!if [SET INCLUDE=.;$(ROOT)\src\$(SUBSYS)\common\include;$(INCLUDE)]
!endif

#
# VAC 3.6.5 for OS/2
#
# CINCLUDE    = -I.;$(ROOT)\src\$(SUBSYS)\common\include;$(CXXMAIN)\include;$(TKMAIN)\h;$(DDKBASE)\h
CC          = icc
COMPILER_HDR1 = $(CXXMAIN)\include
COMPILER_LIB_PATH = $(CXXMAIN)\lib


#
# MS C v6.0a
#
C16INCLUDE  = -I. -I$(ROOT)\src\$(SUBSYS)\common\include -I$(DDKBASE)\h -I$(MSCMAIN)\include
CC16        = cl $(C16INCLUDE)


#
# Assembler - MASM 5.10A
#
AINC        = -I. -I$(ROOT)\src\$(SUBSYS)\common\include -I$(DDKBASE)\inc
AS          = masm $(AINC)


#
# Linker - ilink from VAC 3.6.5
#
LINK        = ilink /nofree /linenumbers


#
# Librarian - ilib from VAC 3.6.5
#
LIBUTIL     = ilib


#
# Import Librarian - implib from VAC 3.6.5
#
IMPLIB      = implib


#
# Object directory
#
OBASE       = $(ROOT)\obj\$(SUBSYS)
O           = $(OBASE)\$(SUBSYS_RELATIVE)


#
# Div stubs
#
R_OS2_LIB   = $(ROOT)\lib


#
# Output paths. (OS2 system tree root dir is '$(ROOT)\bin')
#
OS2         = $(ROOT)\bin\os2
DLL         = $(OS2)\dll


#
# Create-path tool.
#
CREATE_PATH = $(ROOT)\createpath.cmd $(@D)


#
# Create object path.
#
!if [$(ROOT)\createpath.cmd $(O)]
!endif

