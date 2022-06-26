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

!include ..\..\config.in
!include ..\..\make\setup.in

# ***************************************************************************
# *
# *     2) include mode-specific compiler inference rules
# *
# ***************************************************************************

!include $(MAKE_INCLUDE_DIR)\compile_dll_mt.in
#       this defines the proper OUTPUTDIR

# ***************************************************************************
# *
# *     3) list objects to be built
# *
# ***************************************************************************

!include ..\..\make\objects.in

OBJS = $(XFLDR_OBJS_CLASSES)

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

# we do not link in here, so define ALLTARGET manually

ALLTARGET = $(OBJS)

# ***************************************************************************
# *
# *     6) define main target
# *
# ***************************************************************************

!include $(MAKE_INCLUDE_DIR)\targets.in


