#!/bin/sh
#
#  Copyright 2010, Roger Brown
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

ARGS=$@

do_gtar()
{
	GTAR=
	for d in tar gtar /usr/pkg/bin/gtar
	do
		TMPFILE=/tmp/$$.pkgtool.gtar
		touch $TMPFILE

		RC=1

		if $d --create --file - $TMPFILE >/dev/null 2>/dev/null
		then
			if $d --owner=0 --group=0 --create --file - $TMPFILE >/dev/null 2>/dev/null
			then
				RC=0
			fi
		fi
		
		rm $TMPFILE

		if test "$RC" = 0
		then
			GTAR="$d"
			break
		fi		
	done

	if test "$GTAR" = ""
	then
		exit 1
	fi

	echo "$GTAR"
}

last()
{
	LAST=
	for d in $@
	do
		LAST="$d"
	done
	echo $LAST
}

gettoken()
{
	LAST=
	TOKEN=$1
	shift
	while test "$1" != ""
	do
		if test "$LAST" = "$TOKEN"
		then
			echo "$1"
			break
		fi

		LAST="$1"
		shift
	done	
}

do_arch()
{
	RC=1
	FILE="$1"

	for d in objdump elfdump /usr/ccs/bin/elfdump
	do
		if $d -p "$FILE" 1>/dev/null 2>/dev/null
		then
			case `basename $d` in
			objdump )
				FORMAT=`objdump -p "$FILE" | grep "file format"`
				if test "$FORMAT" != ""
				then
					last $FORMAT
					RC=0
				fi
				;;
			elfdump )
				VAL=`$d -e "$FILE" | grep ei_class:`
				VAL=`echo $VAL`
				EICLASS=`gettoken ei_class: $VAL`
				VAL=`echo $VAL`
				VAL=`$d -e "$FILE" | grep e_machine:`
				EMACHINE=`gettoken e_machine: $VAL`

				FORMAT="$EICLASS:$EMACHINE"

				echo $FORMAT
				RC=0
				;;
			esac
		fi

		if test "$RC" = 0
		then
			break
		fi
	done

	return $RC
}

do_dpkgarch()
{
	ARCH="$1"
	case "$ARCH" in
	elf64-x86-64 )
    	ARCH=amd64
    	;;
	elf32-littlearm )
	    ARCH=arm
	    ;;
	elf32-* )
    	ARCH=`echo $ARCH | sed s/elf32-//`
    	;;
	elf64-* )
    	ARCH=`echo $ARCH | sed s/elf64-//`
    	;;
	* )
	    ;;
	esac
	echo "$ARCH"
}

case "$1" in
gtar )
	do_gtar
	;;
arch )
	if test ! -f "$2"
	then 
		echo "$0": "$2" not found 1>&2
		exit 1
	fi
	do_arch "$2"
	;;
dpkg-arch )
	if test "$PLATFORM_HOST" = "$PLATFORM"
	then
		if test "$PLATFORM" != ""
		then
			if dpkg --print-architecture >/dev/null 2>/dev/null
			then
				dpkg --print-architecture
				exit 0
			fi
		fi
	fi
	if test ! -f "$2"
	then 
		echo "$0": "$2" not found 1>&2
		exit 1
	fi
	ARCH=`do_arch "$2"`
	if test "$ARCH" != ""
	then
		do_dpkgarch $ARCH
	fi
	;;	
* )
	echo 1>&2 "$0" unknown task "$1"
	exit 1
	;;
esac

