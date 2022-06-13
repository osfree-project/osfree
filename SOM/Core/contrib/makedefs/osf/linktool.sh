#!/bin/sh
#
#  Copyright 2008, Roger Brown
#
#  This file is part of Roger Brown's Toolkit.
#
#  This program is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
# 
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#
# $Id$
#

MYMAKE=$INTDIR/clean.linktool.mak

if test "$MAKEDEFS" = ""
then
	echo $0: MAKEDEFS not set 1>&2
	exit 1
fi

if test "$MAKE" = ""
then
	echo $0: MAKE not set 1>&2
	exit 1
fi

../../toolbox/linktool.sh \
	-I $MAKEDEFS \
	-s ../../makedefs/$PLATFORM_PROTO/linktool.cf \
	$@ >$MYMAKE

if test "$?" != "0"
then
	exit 1
fi

MAKEFILE=$MYMAKE $MAKE -f $MYMAKE INTDIR=$INTDIR BUILDTYPE=$BUILDTYPE

