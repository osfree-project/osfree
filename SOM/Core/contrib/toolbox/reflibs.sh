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
LHS_CMD=
RHS_CMD=
LAST_ARG=
DEFAULT_LIST="/usr/lib /lib"
DIR_LIST=

first()
{
	echo "$1"
}

findlib()
{
	fl=$1
	shift
	for fp in $@
	do
		if test -f "$fp/$fl"
		then
			echo "$fp/$fl"
			return 0
		fi
	done
	return 1
}

arglist()
{
	fl=$1
	shift
	for fp in $@
	do
		if test "$fp" != ""
		then
			echo "$fl$fp"
		fi
	done
	return 1
}

for d in $@
do
	if test "$d" != ""
	then
		bn=`basename "$d"`

		case "$LAST_ARG" in
		-o )
			RHS_CMD="$RHS_CMD $d "		
			;;
		* )
			case "$bn" in
			lib*.a )
				fn=`findlib $bn $DIR_LIST $DEFAULT_LIST`
				if test "$fn" != "$d"
				then
					dn=`dirname $d`

					fn=`findlib $bn $DIR_LIST $dn $DEFAULT_LIST`
					if test "$fn" = "$d"
					then
						DIR_LIST="$DIR_LIST $dn"
					else
						fn=`findlib $bn $dn $DIR_LIST $DEFAULT_LIST`
						if test "$fn" = "$d"
						then
							DIR_LIST="$dn $DIR_LIST"
						else
							echo cannot find $d in $DIR_LIST properly
							exit 1
						fi
					fi
				fi
				ln=`echo $bn | sed s/lib// | sed y/./\ /`
				ln=`first $ln`
				RHS_CMD="$RHS_CMD -l$ln "
				;;
			-L* )
				RHS_CMD="$RHS_CMD "
				DIR_LIST="$DIR_LIST `echo $d | sed s/-L/\ /"
				echo DIR_LIST=$DIR_LIST
				;;
			-* )
				RHS_CMD="$RHS_CMD $d "		
				;;
			* )
				if test "$RHS_CMD" = ""
				then
					LHS_CMD="$LHS_CMD $d "		
				else
					RHS_CMD="$RHS_CMD $d "		
				fi
				;;
			esac
			;;
		esac
	fi

	LAST_ARG="$d"
done



# echo ORIGINAL_CMD=$ORIGINAL_CMD
# echo DIR_LIST=$DIR_LIST
# echo LHS_CMD=$LHS_CMD
# echo RHS_CMD=$RHS_CMD

NEW_CMD="$LHS_CMD `arglist -L $DIR_LIST` $RHS_CMD"

echo $NEW_CMD

exec $NEW_CMD
