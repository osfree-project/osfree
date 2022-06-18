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

LIBNAME="$1"
EXPNAME="$2"

if test "$LIBNAME" = ""
then
	echo 1>&2 usage: $0 libname.so.1 name.exp
	exit 1
fi

if test "$EXPNAME" = ""
then
	echo 1>&2 usage: $0 libname.so.1 name.exp
	exit 1
fi

if test ! -f "$LIBNAME"
then
	echo 1>&2 $0: $LIBNAME does not exist
	exit 1
fi

if test ! -f "$EXPNAME"
then
	echo 1>&2 $0: $EXPNAME does not exist
	exit 1
fi

LIBNAME2=`basename $LIBNAME`

strlen()
{
	NAME="$1"
	echo -n "$NAME" | wc -c
}

STRLEN=`strlen "$LIBNAME2"`
STRLEN=`echo $STRLEN`

cat <<EOF
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define errstr(p,l)   write(2,(p),(l))

static void fatal(const char *str,int len) 
{
	errstr("fatal: called stub function ",28);
	errstr(str,len);
	errstr(" in ",4);
	errstr("$LIBNAME2",$STRLEN);
	errstr("\n",1);
	exit(1);
}
EOF

is_export()
{
	N="$1"

	while read M
	do
		if test "$M" = "$N" 
		then
			return 0
		fi
	done <"$EXPNAME"

	return 1
}

objdump -T "$LIBNAME" | while read ADDR FLAGS1 FLAGS2 SEG SIZE SCOPE NAME
do
	if test "$NAME" = ""
	then
		case "$SIZE" in
		0* )
			NAME="$SCOPE"
			;;
		* )
			;;
		esac
	fi

	if test "$NAME" != ""
	then
		if is_export "$NAME"
		then
			case "$SEG" in
			".data" | ".sdata" )
				echo "char $NAME[0x$SIZE]={1};"
				;;
			".bss" | ".sbss" )
				echo "char $NAME[0x$SIZE];"
				;;
			".text" )
				STRLEN=`strlen "$NAME"`
				STRLEN=`echo $STRLEN`
				echo "void $NAME(void) { fatal(\"$NAME\",$STRLEN); }"
				;;
			esac
		fi
	fi
done

cat <<EOF
#ifdef __cplusplus
}
#endif
EOF
