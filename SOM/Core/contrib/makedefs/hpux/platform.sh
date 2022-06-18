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

# gcc cannot use -Werror due to pthread initializations
# gcc also does not define -D_BIG_ENDIAN by default
# cc/aCC use "+DD64"
# gcc/c++ on ia64 use "-mlp64"

CONFIG_SRC=$INTDIR/platform.c
CONFIG_OBJ=$INTDIR/platform.o
CONFIG_AOUT=$INTDIR/a.out
CONFIG_LINKOPTS=
CONFIG_ELF=false
CC_FLAG_FPIC=
CXX_FLAG_FPIC=

cat >$CONFIG_SRC <<EOF
int main(int argc,char **argv)
{
	return 0;
}
EOF

$CC $CFLAGS $PLATFORM_CFLAGS -c $CONFIG_SRC -o $CONFIG_OBJ

CC_RES=$?

rm $CONFIG_SRC

if test "$CC_RES" != "0"
then
	exit $CC_RES
fi

$CC $CFLAGS $PLATFORM_CFLAGS $CONFIG_OBJ -o $CONFIG_AOUT

CC_RES=$?

rm $CONFIG_OBJ

if test "$CC_RES" != "0"
then
	exit $CC_RES
fi

platform_not_member()
{
	not_member_d=$1
	shift
	for not_member_i in $@
	do
		if test "$not_member_i" = "$not_member_d"
		then
			return 1
		fi
	done
	return 0;
}

DEFAULT_PATH_LIST=
OS_LINK_PATHS=
LDD_FOUND_SO=
LDD_FOUND_SL=

for default_lib in `ldd $CONFIG_AOUT`
do
	if test -f "$default_lib"
	then
		default_dir=`dirname $default_lib`
		case "$default_lib" in
		$default_dir/lib*.* )
			if platform_not_member $default_dir $DEFAULT_PATH_LIST
			then
				DEFAULT_PATH_LIST="$DEFAULT_PATH_LIST $default_dir"
				if test "$OS_LINK_PATHS" = ""
				then
					OS_LINK_PATHS="$default_dir"
				else
					OS_LINK_PATHS="$OS_LINK_PATHS:$default_dir"
				fi
				CONFIG_LINKOPTS="$CONFIG_LINKOPTS -L$default_dir"
			fi
			;;
		* )
			echo ignoring $default_lib
			;;
		esac
		case `basename $default_lib` in
		lib*.sl )
				LDD_FOUND_SL=true
				;;
		lib*.so | lib*.so.* )
				LDD_FOUND_SO=true
				;;
		* )
				;;
		esac
	fi		
done

SHLIBPREFIX=lib

if test "$LDD_FOUND_SO" = "true"
then
	SHLIBSUFFIX=.so
	CONFIG_ELF=true
else
	SHLIBSUFFIX=.sl
fi

rm $CONFIG_AOUT

echo OS_LINK_PATHS=$OS_LINK_PATHS

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
	DLLPATHENV="SHLIB_PATH"

	$CC $CFLAGS -c $CONFIG_SRC -DTEST_64 -o $CONFIG_OBJ 1>/dev/null 2>&1
	CC_RES=$?

	if test "$CC_RES" = "0"
	then
		echo 64-bit build

		CONFIG_ELF=true
	else
		echo 32-bit build
	fi

	export DLLPATHENV
fi

if test "$CONFIG_ELF" = "true"
then
	SHLB_REF_STRONG=$OUTDIR/lib
	SHLB_REF_WEAK=$OUTDIR/implib
else
	SHLB_REF_STRONG=$OUTDIR/lib:$OUTDIR/implib
fi

$CC $CFLAGS -c $CONFIG_SRC -DTEST_GCC -o $CONFIG_OBJ 1>/dev/null 2>&1
CC_RES=$?

# -Bhidden enables declspec(dllexport/dllimport)

if test "$CC_RES" = "0"
then
	CC_FLAG_FPIC="-fPIC"
	CXX_FLAG_FPIC="-fPIC"
	PLATFORM_CFLAGS="-Wall"
	PLATFORM_CXXFLAGS="-Wall"
	CONFIG_CFLAGS="-Werror"
	CONFIG_CXXFLAGS="-Werror"
else
	CC_FLAG_FPIC="+z"
	CXX_FLAG_FPIC="+z"
	PLATFORM_CFLAGS=""
	PLATFORM_CXXFLAGS="-Aa -D_HPUX_SOURCE +e"
	CONFIG_CFLAGS="+We"
	CONFIG_CXXFLAGS="+We"
fi

CONFIG_LINKOPTS_SHARED="+s +b $OS_LINK_PATHS -B symbolic $CONFIG_LINKOPTS"
CONFIG_LINKOPTS="-Wl,+s -Wl,+b,$OS_LINK_PATHS $CONFIG_LINKOPTS"

LD_SHARED="ld -b $CONFIG_LINKOPTS_SHARED"
LDXX_SHARED="ld -b $CONFIG_LINKOPTS_SHARED"

LINKAPP_HEAD="\`echo >\$(INTDIR)/shlb_imp\` $CONFIG_LINKOPTS"
LINKAPP_TAIL="\`if test -f \$(INTDIR)/shlb_imp; then rm \$(INTDIR)/shlb_imp; fi\`"

CC_LINK_SONAME="-Wl,+h"
CC_LINK_ENTRY="-Wl,-e"

if test "$SHLIBSUFFIX" = ".sl"
then
	LD_SHARED="../../toolbox/ldhppa.sh $LD_SHARED"
	LDXX_SHARED="../../toolbox/ldhppa.sh $LDXX_SHARED"
fi

for platform_d in $CONFIG_SRC $CONFIG_OBJ
do
	if test -f $platform_d
	then
		rm $platform_d
	fi
done

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

LD_SHARED="$LD_SHARED $LDFLAGS"
LDXX_SHARED="$LDXX_SHARED $LDFLAGS"

# this gets picked up by toolbox/asneeded.sh
SOCKLIBS="$SOCKLIBS -lxnet"
REQUIRED_LIBS="-lxnet -lpthread"

SHLB_REF_NAME=$OUTDIR/reflib

export LDXX_SHARED
export LD_SHARED
export CFLAGS
export CXXFLAGS
export LINKAPP_HEAD
export LINKAPP_TAIL
export CC_LINK_EXPORT
export CC_LINK_SONAME
export CC_LINK_ENTRY
export CC_FLAG_FPIC
export CXX_FLAG_FPIC
export SHLB_REF_STRONG
export SHLB_REF_WEAK
export SHLB_REF_NAME
export CONFIG_CFLAGS
export REQUIRED_LIBS

echo CFLAGS=$CFLAGS
echo LD_SHARED=$LD_SHARED
echo LDXX_SHARED=$LDXX_SHARED

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS DLLPATHENV REQUIRED_LIBS"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS SHLIBSUFFIX SHLIBPREFIX"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS PLATFORM_CFLAGS PLATFORM_CXXFLAGS"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS SHLB_REF_STRONG SHLB_REF_WEAK SHLB_REF_NAME"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS LD_SHARED LDXX_SHARED"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS CONFIG_CFLAGS CXX_FLAG_FPIC CC_FLAG_FPIC"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS LINKAPP_HEAD LINKAPP_TAIL"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_SCFLAGS SOCKLIBS"
