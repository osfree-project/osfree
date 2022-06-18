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

TRYCC_FLAGS=$@
LAUNDRYLIST=

trycc_clean()
{
   for d in $LAUNDRYLIST
   do
       if test -f $d
	   then
	        rm $d
	   fi
   done
}

if test "$CC" = ""
then
     echo 1>&2 $0: CC not set
     exit 1
fi

if test "$INTDIR" = ""
then
     echo 1>&2 $0: INTDIR not set
     exit 1
fi

if test -d $INTDIR
then
	TRYSTUB=$INTDIR/tryC$$
	LAUNDRYLIST="$TRYSTUB.c"
	if cat >$TRYSTUB.c
	then
		LAUNDRYLIST="$LAUNDRYLIST $TRYSTUB.o"

		if $CC -c $TRYSTUB.c -o $TRYSTUB.o $TRYCC_FLAGS 2>/dev/null
		then
			LAUNDRYLIST="$LAUNDRYLIST $TRYSTUB$EXESUFFIX"

			if $CC $TRYSTUB.o -o $TRYSTUB$EXESUFFIX $TRYCC_FLAGS 2>/dev/null
			then
				trycc_clean

				exit 0
			fi
		fi
	fi		
else
	echo 1>&2 $0: INTDIR $INTDIR does not exist
fi

trycc_clean

exit 1
