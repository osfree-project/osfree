#
# See make\readme.txt about an introduction to the make system introduced
# in the CVS trunk on 2003-01-28.
#
#       Copyright (C) 1998-2003 Ulrich M”ller.
#       This file is part of the XWorkplace source package.
#       XWorkplace is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published
#       by the Free Software Foundation, in version 2 as it comes in the
#       "COPYING" file of the XWorkplace main distribution.
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#

# ***************************************************************************
# *
# *     1) include generic setup definitions (compiler options etc.)
# *
# ***************************************************************************

!include $(PROJECT_BASE_DIR)\config.in
!include $(PROJECT_BASE_DIR)\make\setup.in

# ***************************************************************************
# *
# *     2) include mode-specific compiler inference rules
# *
# ***************************************************************************

!ifndef CC_HELPERS
CC_HELPERS = error!
!endif

!ifndef NOINCLUDEDEPEND
!ifndef OUTPUTDIR_HELPERS
!error OUTPUTDIR_HELPERS is not defined.
!endif
!endif

CC = $(CC_HELPERS) /DBZ_NO_STDIO
OUTPUTDIR = $(OUTPUTDIR_HELPERS)

!include $(MAKE_INCLUDE_DIR)\_sub_compile.in

# ***************************************************************************
# *
# *     3) list objects to be built
# *
# ***************************************************************************

OBJS = $(OUTPUTDIR)\blocksort.obj\
$(OUTPUTDIR)\bzlib.obj\
$(OUTPUTDIR)\bzlib2.obj\
$(OUTPUTDIR)\compress.obj\
$(OUTPUTDIR)\crctable.obj\
$(OUTPUTDIR)\decompress.obj\
$(OUTPUTDIR)\huffman.obj\
$(OUTPUTDIR)\randtable.obj

# ***************************************************************************
# *
# *     4) define specific stuff for linker include
# *
# ***************************************************************************

# ***************************************************************************
# *
# *     5) link executable
# *
# ***************************************************************************

ALLTARGET = $(OUTPUTDIR)\libbz2.lib

# ***************************************************************************
# *
# *     6) define main target
# *
# ***************************************************************************

!include $(MAKE_INCLUDE_DIR)\targets.in

$(OUTPUTDIR)\libbz2.lib: $(OBJS)
    - del $@
    ilib /nol /nob $@ @<<$(TEMP)\ilib.lnk
+$(OBJS: =&^
);
<<


