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

CONFIG_OPTS="$CONFIG_OPTS -I ../../rhbseh/include"
CONFIG_LIBS="\`if test -f $OUTDIR/lib/librhbseh.so; then echo -L$OUTDIR/lib -lrhbseh; fi\` $CONFIG_LIBS"

# let's solve a problem with netbsdelf 1.x

TRYNAME=/tmp/$$.make.seh

cat >$TRYNAME.c <<EOF
int f() { printf(""); }
#ifdef RHBSEH_NETBSDELF1_CRT0
char **environ; char **__progname;
#endif
EOF

CC_TRYNAME="$CC -shared $CC_NO_UNDEFS -lc -o $TRYNAME $TRYNAME.c"

$CC_TRYNAME 2>/dev/null

if test "$?" != "0"
then
	if $CC_TRYNAME -DRHBSEH_NETBSDELF1_CRT0
	then
		RHBSEH_CFLAGS="$RHBSEH_CFLAGS -DRHBSEH_NETBSDELF1_CRT0"
		MAKEDEFS_DEFS="$MAKEDEFS_DEFS RHBSEH_CFLAGS"
	else
		echo problem build strongly linked shared library, see $TRYNAME.c
		exit 1
	fi
fi

for d in $TRYNAME.c $TRYNAME
do
	if test -f $d
	then
		rm $d
	fi
done

