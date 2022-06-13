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

EXPORT_IN=../../$1/unix/$2.exp

if test "$INTDIR" = ""
then
	INTDIR=$BUILDTYPE
	if test "$INTDIR" = ""
	then
		INTDIR=.
	fi
fi

EXPORT_IN_C=$INTDIR/$2.exp.c

if cp $EXPORT_IN $EXPORT_IN_C
then
	EXPORT_OUT=$INTDIR/$2.exp
	EXPORT_TMP=$EXPORT_OUT.tmp
	if $CC -E $CFLAGS $PLATFORM_CFLAGS $EXPORT_IN_C | grep -v "#" >$EXPORT_TMP
	then
		if test -f $EXPORT_OUT
		then
			rm $EXPORT_OUT
		fi
		while read N
		do
			if test "$N" != ""
			then
				echo $N >>$EXPORT_OUT
			fi
		done <$EXPORT_TMP
		echo -bE:$EXPORT_OUT
		rm $EXPORT_TMP
		EXPORT_MAK=$INTDIR/clean.$2.exp.mak
		echo "clean:" >$EXPORT_MAK
		echo "	if test -f \"$EXPORT_OUT\"; then rm \"$EXPORT_OUT\"; fi" >>$EXPORT_MAK
	fi
	rm $EXPORT_IN_C
fi
