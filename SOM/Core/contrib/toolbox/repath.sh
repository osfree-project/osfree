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

ROOT="$1"
NEWPATH="$2"

case "$NEWPATH" in
/* )
	ROOT=/
	;;
* )
	;;
esac

NEWPATH_R=
while test "$NEWPATH" != "."
do
	if test "$NEWPATH" = "/"
	then
		break
	fi
	NEWPATH_R="`basename $NEWPATH` $NEWPATH_R "
	NEWPATH=`dirname $NEWPATH`
done

for d in $NEWPATH_R
do
	case "$d" in
	. )
		;;
	.. )
		case "$ROOT" in
		. )
			ROOT=".."
			;;
		* )
			ROOT=`dirname "$ROOT"`
			;;
		esac
		;;
	* )
		case "$ROOT" in
		*/ )
			ROOT="$ROOT$d"
			;;
		* )
			ROOT="$ROOT/$d"
			;;
		esac
		;;
	esac
done

NEWPATH_R=
while test "$ROOT" != "."
do
	if test "$ROOT" = "/"
	then
		break
	fi
	NEWPATH_R="`basename $ROOT` $NEWPATH_R "
	ROOT=`dirname $ROOT`
done

NEWPATH=

for d in $NEWPATH_R
do
	if test "$d" != "."
	then
		if test "$NEWPATH" = ""
		then
			NEWPATH="$d"
		else
			NEWPATH="$NEWPATH/$d"
		fi
	fi
done

echo $NEWPATH
