#
# $Header$
#

# 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
# access your Linux ext2fs partitions as normal drive letters.
# Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

MWDD32_VERSION  = \"1.70\"
EXT2OS2_VERSION = \"2.40\"
MWDD32_PKG_VER  = 170
EXT2OS2_PKG_VER = 240

#
# EXT2OS2_BASE : base directory of build tree
#
EXT2OS2_BASE = \32bits\ext2-os2

#
# MINIFSD_BASE : base directory of mini FSD
#
MINIFSD_BASE = $(EXT2OS2_BASE)\minifsd

#
# DDKPATH : base directory of the ddk 2.0
#
DDKPATH = \ddkx86

#
# IFSTKTPATH : base directory of the IFS toolkit (no change required)
#
IFSTKTPATH = $(EXT2OS2_BASE)\ifstkt

#
# MSVCPATH : base directory of MS Visual C++
#
MSVCPATH = \msvc

#
# IBM Visualage C++ install directory
#
VACPATH = \apps\ibmcpp


#
# Linux ext2fs utilities path
#
E2FSPROGS=$(EXT2OS2_BASE)\e2fsprogs-1.10

#
# EMX/GCC path
#
EMX = d:\emx

TEST_DIR=e:\mwfs
