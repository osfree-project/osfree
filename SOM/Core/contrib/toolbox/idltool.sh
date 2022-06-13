#!/bin/sh -e
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

ARG0=$0
CMD=
TARGET=
DIRS=
ARGS=
LAST=

debugEcho()
{
	echo $@  >/dev/null
}

for d in $@
do
	if test "$LAST" = ""
	then
		if test "$d" = "-o"
		then
			LAST="$d"
		else
			if test "$CMD" = ""
			then
				CMD="$d"
			else
				if test "$TARGET" = ""
				then
					DIRS="$DIRS $d"
				else
					ARGS="$ARGS $d"
				fi
			fi
		fi
	else
		case "$LAST" in
		-o )
			TARGET="$d"
			LAST=
			;;
		* )
			echo 1>&2 $ARG0: unknown flag $d
			exit 1
			;;
		esac
	fi
done

if test "$CMD" = ""
then
	echo 1>&2 $ARG0: unknown cmd
	exit 2
fi

if test "$TARGET" = ""
then
	echo 1>&2 $ARG0: unknown target
	exit 3
fi

debugEcho TARGET="$TARGET"

first()
{
	echo $1
}

last()
{
	LAST_N=

	for LAST_M in $@
	do
		LAST_N="$LAST_M"
	done

	echo "$LAST_N"
}

TARGET_DIR=`dirname "$TARGET"`
TARGET_BASE=`basename "$TARGET"`
TARGET_PART=`echo $TARGET_BASE | sed y/./\ /`

debugEcho TARGET_PART=$TARGET_PART
debugEcho TARGET_DIR=$TARGET_DIR

EMITTER=`last $TARGET_PART`
STEM=`first $TARGET_PART`

debugEcho EMITTER=$EMITTER
debugEcho STEM=$STEM
debugEcho DIRS=$DIRS

EMITTEROPT=

if test "$EMITTER" != "idl"
then
	EMITTEROPT="-s$EMITTER"
fi

SRCIDL=

for d in $DIRS
do
	if test -f "$d/$STEM.idl"
	then
		SRCIDL="$d/$STEM.idl"
		break
	fi
done

if test "$SRCIDL" = ""
then
	echo $STEM.idl not found in $DIRS
	exit 1
fi

COMMAND="$CMD $EMITTEROPT -d $TARGET_DIR $ARGS $SRCIDL"

cat <<EOF
$COMMAND
EOF

exec $COMMAND
