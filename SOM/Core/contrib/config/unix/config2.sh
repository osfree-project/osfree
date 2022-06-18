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

# gcc -E -P

CONFIG_OPTS="$@"
if test "$CC" = ""
then
	CC=cc
fi

# CC_EP="$CC -E -P"
# MIPSpro prefers -EP, but am already filtering for "#*"

CC_EP="$CC -E"

CC_OPTIONS=

if test "$CONFIG2_C" = ""
then
	echo CONFIG2_C not defined in $0
	exit 1
fi

if test "$CONFIG_LOG" = ""
then
	echo CONFIG_LOG not defined in $0
	exit 1
fi

# CONFIG_LOG=$INTDIR/config2.log

CONFIG_INC=$INTDIR/config2.inc
CONFIG_LST=$INTDIR/config2.lst
CONFIG_EXE=$INTDIR/config2$EXESUFFIX
CONFIG_OBJ=$INTDIR/config2.o

if test "$CONFIG_H" = ""
then
	echo CONFIG_H not defined in $0
	exit 1
fi

ECHO_N=`echo -n one; echo two`
ECHO_C=`echo "one\c"; echo two`

LIST_OPTIONS=
LIST_DEFINES=

echo_n()
{
	if test "$ECHO_N" = "onetwo"
	then
		echo -n $@
	else
		echo $@"\c"
	fi
}

PLATFORM_CONFIG_SH=../$PLATFORM_PROTO/config.sh

pwd 1>&2

if test -f $PLATFORM_CONFIG_SH
then
	. $PLATFORM_CONFIG_SH
fi

for cc_only_f in $CONFIG_OPTS
do
	case "$cc_only_f" in
	-L* | -l* )
		;;
	* )
		if test -f "$cc_only_f"
		then
			case `basename $cc_only_f` in
			lib*.a )
				;;
			* )
				CC_OPTIONS="$CC_OPTIONS $cc_only_f"
				;;
			esac
		else
			CC_OPTIONS="$CC_OPTIONS $cc_only_f"
		fi
		;;
	esac
done

echo CC_OPTIONS=$CC_OPTIONS

for d in $CONFIG_LOG $CONFIG_INC $CONFIG_LST $CONFIG_H
do
	if test -f $d
	then
		rm $d
	fi
done

if test "$PLATFORM" != ""
then
	echo "#define" CPU_VENDOR_OS \"$PLATFORM\" >$CONFIG_H
else
	touch $CONFIG_H
fi

$CC_EP $CONFIG2_C -DLIST_OPTIONS | while read N M
do
	if test "$N" != ""
	then
		case $N in
		\#* )
			;;
		* )
			echo $N=$M >>$CONFIG_INC
			echo $N>>$CONFIG_LST
			;;
		esac
	fi
done

if test -f $CONFIG_INC
then
	. $CONFIG_INC

	rm $CONFIG_INC
fi

for d in `cat $CONFIG_LST`
do
	eval N=\$$d
	echo_n $N "..." 

	echo $N >>$CONFIG_LOG

	if $CC \
		-o $CONFIG_OBJ \
		-DTRY_$d \
		`for e in $LIST_DEFINES; do echo -D$e; done` \
		$CC_OPTIONS  \
		-c $CONFIG2_C \
		>>$CONFIG_LOG 2>&1
	then \
		echo "Link -" $CC $CONFIG_OBJ -o $CONFIG_EXE $CONFIG_OPTS >>$CONFIG_LOG
		if $CC \
			$CONFIG_OBJ \
			-o $CONFIG_EXE \
			$CONFIG_OPTS \
			>>$CONFIG_LOG 2>&1
		then
			LIST_DEFINES="$LIST_DEFINES $d"
			echo " $d"
			rm $CONFIG_EXE
		else
			echo " nearly"
		fi

		rm $CONFIG_OBJ
	else
		echo " no"
	fi		
done

if grep INVALID_OPTION $CONFIG_LOG >/dev/null
then
	rm $CONFIG_H
	tail $CONFIG_LOG
	exit 1
fi

# echo $LIST_DEFINES

for d in $LIST_DEFINES
do
	echo "#ifndef $d" >>$CONFIG_H
	echo "#   define $d 1" >>$CONFIG_H
	echo "#endif" >>$CONFIG_H
done

exit 0
