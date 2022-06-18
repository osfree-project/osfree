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

if test "$INTDIR" = ""
then
	INTDIR=.
fi

CONFIG_TMP=$INTDIR/config.tmp.$$
CONFIG_SRC=$CONFIG_TMP/try.c
CONFIG_OBJ=$CONFIG_TMP/try.o
CONFIG_EXE=$CONFIG_TMP/try$EXESUFFIX
CONFIG_NULL=/dev/null

finally()
{
	if test -d $CONFIG_TMP
	then
		rm -r $CONFIG_TMP
	fi
}

# trap finally EXIT

trap finally 0

if test ! -d $CONFIG_TMP
then
	mkdir $CONFIG_TMP
	if test "$?" != "0"
	then
		echo failed to create $CONFIG_TMP
		exit 1
	fi
fi

try_compile()
{
	$CC $CFLAGS -c $CONFIG_SRC -o $CONFIG_OBJ $@ 1>>$CONFIG_NULL 2>>$CONFIG_NULL
	CC_RC=$?

	if test "$CC_RC" = "0"
	then
		$CC $CFLAGS $CONFIG_OBJ -o $CONFIG_EXE $LIB $@ 1>>$CONFIG_NULL 2>>$CONFIG_NULL
		CC_RC=$?
	fi

	return $CC_RC
}

# check for endianess of system

cat >$CONFIG_SRC <<EOF
#include <rhbendia.h>
#ifdef _PLATFORM_BIG_ENDIAN_
int main() { return 0; }
#else
#	error this is not big endian
#endif
EOF
try_compile 

if test "$?" = "0"
then
	echo "must be big!!!" 1>&2
	PLATFORM_ENDIAN=_PLATFORM_BIG_ENDIAN_
else
	cat >$CONFIG_SRC <<EOF
#include <rhbendia.h>
#ifdef _PLATFORM_LITTLE_ENDIAN_
int main() { return 0; }
#else
#	error this is not little endian
#endif
EOF
	try_compile

	if test "$?" = "0"
	then
		echo "must be small!!!" 1>&2
		PLATFORM_ENDIAN=_PLATFORM_LITTLE_ENDIAN_
	fi	 
fi

if test "$PLATFORM_ENDIAN" = ""
then
	exit 1
fi

echo $PLATFORM_ENDIAN

exit 0

