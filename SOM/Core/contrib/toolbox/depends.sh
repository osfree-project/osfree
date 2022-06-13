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

TARGETS=$@

umask 077

if test "$TMP" = ""
then
	MYTMPDIR="/tmp/depends.$$"
else
	MYTMPDIR="$TMP/depends.$$"
fi

mkdir "$MYTMPDIR"

RC="$?"

if test "$RC" != "0"
then
	exit $RC
fi

MYMAKEFILE="make$$.mak"
MYORDERLST="order$$.lst"

if test "$MAKE" = ""
then
	echo MAKE variable not setup 1>&2
	exit 1
fi

getvalue()
{
	while read GV_NAME GV_VALUE
	do
		if test "$GV_NAME" = "$2"
		then
			echo "$GV_VALUE"
		fi
	done <$1
}

gen_makefile()
{
	echo "all: depends-all"
	echo
	echo

	ALL_DEPENDS=
	while read FILELIST
	do
		NAME=`getvalue $FILELIST NAME`
		DEPENDS=`getvalue $FILELIST DEPENDS`
		PROVIDES=`getvalue $FILELIST PROVIDES`
		echo "$PROVIDES: $DEPENDS"
		echo "	echo $NAME >>$MYORDERLST" 
		echo
		ALL_DEPENDS="$ALL_DEPENDS $NAME"
	done

	echo "depends-all: $ALL_DEPENDS"
	echo
}

gen_makefile >$MYTMPDIR/$MYMAKEFILE

(
	cd $MYTMPDIR
	if test "$?" = "0"
	then
	    $MAKE -f $MYMAKEFILE $TARGETS >/dev/null
	fi
)

RC=$?

if test "$RC" = "0"
then
	cat $MYTMPDIR/$MYORDERLST
fi

rm $MYTMPDIR/$MYORDERLST $MYTMPDIR/$MYMAKEFILE
rmdir $MYTMPDIR

exit $RC
