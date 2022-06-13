#!/bin/sh -e
#
#  Copyright 2011, Roger Brown
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
#  $Id$

if test "$SOMBASE" = ""
then
	echo SOMBASE needs to be set
	exit 1
fi

test -w "$SOMBASE/include"

case `basename "$0"` in
somcorba | somstars )
	EMITTER_LIST=h
	;;
somxh )
	EMITTER_LIST=xh
	;;
* )
	EMITTER_LIST="h xh"
	;;
esac

cd "$SOMBASE/include"

find . -type f -name \*.idl | while read N
do
	echo "$N"
	for d in $EMITTER_LIST
	do
		case `basename "$N"` in
		somdtype.idl )
			"$SOMBASE/bin/sc" -s$d -DEMIT_SOMDTYPES -I. "$N" -d .
			;;
		* )
			"$SOMBASE/bin/sc" -s$d -I. "$N" -d .
			;;
		esac
	done
done

for d in $EMITTER_LIST
do
	case $d in
	xh )
		"$SOMBASE/bin/sc" -sapi -I. ./somcls.idl -d .
		;;
	* )
		;;
	esac	
done

