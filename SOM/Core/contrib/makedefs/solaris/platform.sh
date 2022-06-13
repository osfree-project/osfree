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

CONFIG_SRC=$INTDIR/platform.c
CONFIG_OBJ=$INTDIR/platform.o

platform_not_member()
{
    platform_not_member_1=$1
    shift
    for platform_not_member_i in $@
    do
        if test "$platform_not_member_1" = "$platform_not_member_i"
        then
            return 1
        fi
    done
    return 0
}

mywhich()
{
	WHICH="$1"
	if test ! -x "$WHICH"
	then
		for WHICH_PATH in `echo $PATH /usr/ccs/bin | sed y/\:/\ /`
		do
			if test -x "$WHICH_PATH/$WHICH"
			then
				WHICH="$WHICH_PATH/$WHICH"
				break
			fi
		done
	fi

	if test -x "$WHICH"
	then
		echo "$WHICH"
	fi
}

cat >$CONFIG_SRC <<EOF
#include <stdio.h>
int x;
#ifdef TEST_64
#	ifdef _LP64
#	else
#		error
#	endif
#elif TEST_GCC
#	ifdef __GNUC__
#	else
#		error
#	endif
#else
#	error unknown test
#endif
EOF

if test "$DLLPATHENV" = ""
then
	$CC $CFLAGS -c $CONFIG_SRC -DTEST_64 -o $CONFIG_OBJ 1>/dev/null 2>&1
	CC_RES=$?

	if test "$CC_RES" = "0"
	then
		echo 64-bit build
		DLLPATHENV=LD_LIBRARY_PATH_64
	else
		echo 32-bit build
		DLLPATHENV=LD_LIBRARY_PATH
	fi
fi

$CC $CFLAGS -c $CONFIG_SRC -DTEST_GCC -o $CONFIG_OBJ 1>/dev/null 2>&1
CC_RES=$?

#  -Wl,-u,__eprintf

if test "$CC_RES" = "0"
then
#	echo compiler is GCC
	CC_SHARED="$CC -shared -Wl,-z,ignore -Wl,-z,defs -Wl,-B,symbolic"
	CXX_SHARED="$CXX -shared -Wl,-z,ignore -Wl,-z,defs -Wl,-B,symbolic -lstdc++"
	CC_FLAG_FPIC=
	CXX_FLAG_FPIC=
	PLATFORM_CFLAGS="-fPIC -Wall -Werror"
	PLATFORM_CXXFLAGS="-fPIC -Wall -Werror"
	LINKAPP_HEAD="-Wl,-z,ignore"
	LINKDLL_TAIL="-lgcc -lc"
	CC_LINK_EXPORT="-Wl,-M,"
	CC_LINK_SONAME="-Wl,-h,"
	CC_LINK_ENTRY="-Wl,-e,"
else
#	echo not GCC
	CC_SHARED="$CC -G -Wl,-zignore -Wl,-zdefs -Wl,-Bsymbolic"
	CXX_SHARED="$CXX -G -Wl,-zignore -Wl,-zdefs -Wl,-Bsymbolic"
	case "$PLATFORM" in
	sparc* )
		if test "$DLLPATHENV" = "LD_LIBRARY_PATH_64"
		then
			PLATFORM_CFLAGS="-xcode=pic32 -errwarn"
			PLATFORM_CXXFLAGS="-xcode=pic32"
			CC_FLAG_FPIC=
			CXX_FLAG_FPIC=
		else
			PLATFORM_CFLAGS="-xcode=pic32 -errwarn"
			PLATFORM_CXXFLAGS="-xcode=pic32"
			CC_FLAG_FPIC=
			CXX_FLAG_FPIC=
		fi
		;;
	* )
		PLATFORM_CFLAGS="-Kpic -errwarn"
		PLATFORM_CXXFLAGS="-Kpic"
		CC_FLAG_FPIC=
		CXX_FLAG_FPIC=
		;;
	esac
	LINKAPP_HEAD="-Wl,-zignore"
	LINKDLL_TAIL="-lCstd -lCrun -lc"
	CC_LINK_EXPORT="-Wl,-M"
	CC_LINK_SONAME="-Wl,-h"
	CC_LINK_ENTRY="-Wl,-e"
fi

if platform_not_member -D_REENTRANT $CFLAGS $PLATFORM_CFLAGS
then
    PLATFORM_CFLAGS="$PLATFORM_CFLAGS -D_REENTRANT"
fi

if platform_not_member -D_REENTRANT $CXXFLAGS $PLATFORM_CXXFLAGS
then
    PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -D_REENTRANT"
fi

if platform_not_member -D_PLATFORM_UNIX_ $PLATFORM_SCFLAGS
then
	PLATFORM_SCFLAGS="$PLATFORM_SCFLAGS -D_PLATFORM_UNIX_"
fi

for platform_d in $CONFIG_SRC $CONFIG_OBJ
do
	if test -f $platform_d
	then
		rm $platform_d
	fi
done

SHLB_REF_STRONG=$OUTDIR/lib:$OUTDIR/implib
SHLB_REF_WEAK=
SHLB_REF_NAME=$OUTDIR/reflib

if test "$AR" = ""
then
	AR=ar
fi

for d in `mywhich $AR`
do
	if test -x "$d"
	then
		AR="$d"
	fi
done

if test "$STRIP" = ""
then
	STRIP=strip
fi

for d in `mywhich $STRIP`
do
	if test -x "$d"
	then
		STRIP="$d"
	fi
done

#echo CFLAGS=$CFLAGS
#echo CC_SHARED=$CC_SHARED
#echo CC_FLAG_FPIC=$CC_FLAG_FPIC

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS PLATFORM_CFLAGS PLATFORM_CXXFLAGS DLLPATHENV"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS SHLB_REF_STRONG SHLB_REF_WEAK SHLB_REF_NAME"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS CC_LINK_EXPORT CC_LINK_SONAME CC_LINK_ENTRY"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS CC_FLAG_FPIC CXX_FLAG_FPIC LINKAPP_HEAD LINKDLL_TAIL AR"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS CXX_SHARED CC_SHARED PLATFORM_SCFLAGS STRIP"
