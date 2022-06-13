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

TRY_CPP=$INTDIR/try.cpp
TRY_EXE=$INTDIR/try.exe

SHLB_REF_STRONG=$OUTDIR/lib:$OUTDIR/implib
SHLB_REF_WEAK=
SHLB_REF_NAME=$OUTDIR/reflib

platform_first()
{
	echo $1
}

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

# want to use pthreads

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

# these are flags for Digitals C Compiler
# enumcalc

if platform_not_member -check $CFLAGS $PLATFORM_CFLAGS
then
    PLATFORM_CFLAGS="$PLATFORM_CFLAGS -std1 -check -msg_error cvtdiftypes,ptrmismatch,warnimplfunc,warnimplfunc1,implicitfunc,embedcomment,nestedcomment,opencomment -msg_disable ignorecallval,nestincl,unusedtop,unnecincl,unusedincl,strctpadding"
fi

cat >$TRY_CPP <<EOF
#include <pthread.h>
static void cleanup(void *pv) {}
class my_obj
{
public: int x; my_obj() { x=0; } ~my_obj() {}
void test(void) { throw x; }
};
void test(void)
{
my_obj obj; obj.test();
}
int main(int argc,char **argv)
{
	pthread_cleanup_push(cleanup,argv);
	test();
	pthread_cleanup_pop(1);

	return 0;
}
EOF

$CXX $CXXFLAGS $PLATFORM_CXXFLAGS $TRY_CPP -o $TRY_EXE -lpthread

if test "$?" != "0"
then
	rm $TRY_CPP
	exit 1
fi

for d in `odump -D $TRY_EXE | grep RPATH`
do
	case "$d" in
	*lib* )
		if test -d $d
		then
			CXX_RUNTIME="$CXX_RUNTIME -L$d"
		fi
		;;
	* )
		;;
	esac
done

for d in `odump -D $TRY_EXE | grep NEEDED`
do
	d=`basename $d`
	case "$d" in
	lib*.so* )
		d=`echo $d | sed s/lib// | sed y/./\ /`
		d=`platform_first $d`
		CXX_RUNTIME="$CXX_RUNTIME -l$d"
		;;
	* )
		;;
	esac
done

rm $TRY_EXE $TRY_CPP

. ../../toolbox/findlibs.sh

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS PLATFORM_CFLAGS PLATFORM_CXXFLAGS"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS SHLB_REF_STRONG SHLB_REF_WEAK SHLB_REF_NAME"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS CXX_RUNTIME"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_SCFLAGS"

if test "$X11LIBS" != ""
then
	HAVE_LIBX11=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11LIBS"
fi

if test "$XTLIBS" != ""
then
	HAVE_LIBXT=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS XTLIBS"
fi

if test "$XMLIBS" != ""
then
	HAVE_LIBXM=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS XMLIBS"
fi

if test "$USBLIBS" != ""
then
	HAVE_LIBUSB=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS USBLIBS"
fi

if test "$CRYPTOLIBS" != ""
then
	HAVE_LIBCRYPTO=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS CRYPTOLIBS"
fi

if test "$SSLLIBS" != ""
then
	HAVE_LIBSSL=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS SSLLIBS"
fi

if test "$PCSCLIBS" != ""
then
	HAVE_LIBPCSC=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS PCSCLIBS"
fi

if test "$UUIDLIBS" != ""
then
	HAVE_LIBUUID=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS UUIDLIBS"
fi

if test "$CURLLIBS" != ""
then
	HAVE_LIBCURL=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS CURLLIBS"
fi

if test "$EXPATLIBS" != ""
then
	HAVE_LIBEXPAT=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS EXPATLIBS"
fi

if test "$HAVE_LIBUSB" = ""
then
	HAVE_LIBUSB=false
fi

if test "$HAVE_LIBPCSC" = ""
then
	HAVE_LIBPCSC=false
fi

if test "$HAVE_LIBUUID" = ""
then
	HAVE_LIBUUID=false
fi

if test "$HAVE_LIBX11" = ""
then
	HAVE_LIBX11=true
fi

if test "$HAVE_LIBEXPAT" = ""
then
	HAVE_LIBEXPAT=false
fi

if test "$HAVE_LIBCURL" = ""
then
	HAVE_LIBCURL=false
fi

MAKEDEFS_DEFS="$MAKEDEFS_DEFS HAVE_LIBUSB HAVE_LIBPCSC HAVE_LIBUUID HAVE_LIBX11 HAVE_LIBEXPAT HAVE_LIBCURL HAVE_LIBSSL HAVE_LIBCRYPTO HAVE_LIBXT HAVE_LIBXM"
