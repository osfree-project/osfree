# $Id: makefile,v 1.3 2004/03/21 03:09:03 bird Exp $

# 	SCCSID = @(#)1.3  7/31/98 08:21:13 src/jfs/utils/os2stats/makefile, jfsutil, w45.fs32, 990417.1
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
OBJS =	stats.obj

!ifndef VACPP
VACPP = \IBMCPP
!endif

HEADERS = /I..\libfs /I$(VACPP)\include /I$(VACPP)\include\os2 /I..\..\common\include /I..\..\common\include\os2

CFLAGS = /Ti+ /G4 /C /Ss /D_JFS_OS2 /D_JFS_UTILITY

.SUFFIXES:	.obj .h .c

.c.obj:
	icc.exe $(CFLAGS) $(HEADERS) %s

all: stats.exe

stats.exe:	$(OBJS) stats.def
	icc.exe @<<
	/Festats.exe
	/Fm /Ti+ /Gm+
	stats.def
	$(OBJS)
<<

clean:
	if exist *.obj	del *.obj
	if exist *.map	del *.map
	if exist *.exe	del *.exe
	if exist *.dll	del *.dll
	if exist *.lst	del *.lst
	if exist *.sym	del *.sym
