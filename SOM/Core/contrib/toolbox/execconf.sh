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

ORIGINAL_CMD=$@

if test "$OUTDIR" = "$HOSTDIR"
then
	if test -f $HOSTDIR_TOOLS/executor.sh
	then
		exec $HOSTDIR_TOOLS/executor.sh $ORIGINAL_CMD
	fi
	echo $HOSTDIR_TOOLS/executor.sh missing 1>&2
	exit 1
else
	LASTARG=
	for d in $@
	do
		LASTARG="$d"
	done

	TARGET="$LASTARG"

	e=`basename "$TARGET"`

	for f in "$e"  							\
			"../$PLATFORM_PROTO/$e"			\
			"../unix/$e"					\
			"../common/$e"
	do
		echo looking in $f...
		if test -f "$f"
		then
			if cp "$f" "$TARGET"
			then
				exit 0
			fi
		fi
	done

	echo needing to provide "$e" to "$TARGET" 1>&2

	exit 1
fi
