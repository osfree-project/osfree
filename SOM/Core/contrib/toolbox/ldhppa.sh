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

if test "$LD" = ""
then
	LD=ld
fi

is_member()
{
	is_member_1=$1
	shift
	for is_member_i in $@
	do
		if test "$is_member_i" = "$is_member_1"
		then
			return 0
		fi
	done
	return 1
}

is_ld()
{
	for is_ld_i in $@
	do
		case "$is_ld_i" in
		$LD )
			return 0
			;;
		$CC )
			return 1;
			;;
		$CXX )
			return 1;
			;;
		* )
			;;
		esac
	done
	return 1
}

# echo $ORIGINAL_CMD=$ORIGINAL_CMD

SEARCH_PATHS="`echo $SHLB_REF_STRONG $SHLB_REF_WEAK | sed y/:/\\ /`"
STRIP_PATHS=

# echo SEARCH_PATHS=$SEARCH_PATHS

$ORIGINAL_CMD

RC=$?

if test "$RC" != "0"
then
	exit $RC
fi

PREV_FLAG=
OUTPUT_FILE=

for d in $ORIGINAL_CMD
do
	case "$d" in
	-o )
		PREV_FLAG=$d
		;;
	* )
		case "$PREV_FLAG" in
		-o )
			OUTPUT_FILE="$d"
			;;
		* )
			;;
		esac
		PREV_FLAG=
		;;
	esac
done

# echo OUTPUT_FILE=$OUTPUT_FILE

if test -f "$OUTPUT_FILE"
then
	for d in `chatr "$OUTPUT_FILE" | grep dynamic`
	do
		if test -f "$d"
		then
# echo POSSIBLE $d
			e=`dirname $d`
			if is_member $e $SEARCH_PATHS
			then
				STRIP_PATHS="$STRIP_PATHS $d"
			fi			
		fi
	done
else
	exit 0
fi

# echo STRIP_PATHS=$STRIP_PATHS

if test "$STRIP_PATHS" = ""
then
	exit 0
fi

rm $OUTPUT_FILE

NEW_CMD="$ORIGINAL_CMD"

for d in $STRIP_PATHS
do
	e=`basename $d`
	if is_ld $ORIGINAL_CMD
	then
		NEW_CMD="$NEW_CMD +cdp $d:$e"
	else
		NEW_CMD="$NEW_CMD -Wl,+cdp,$d:$e"
	fi
done

#echo CC=$CC
#echo CXX=$CXX
#echo LD=$LD
#echo NEW_CMD=$NEW_CMD

$NEW_CMD

RC=$?

# echo NEW_CMD returned $RC

if test "$RC" != "0"
then
	echo NEW_CMD returned $RC
	exit $RC
fi

# chatr $OUTPUT_FILE

# exit 1
