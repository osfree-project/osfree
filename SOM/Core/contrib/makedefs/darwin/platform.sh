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

if test "$OUTDIR" = ""
then
	echo 1>&2 OUTDIR not set
	exit 1
fi

SHLB_REF_STRONG=$OUTDIR/lib:$OUTDIR/implib
SHLB_REF_WEAK=

if test "$RANLIB" = ""
then
	RANLIB=ranlib
fi

platform_getcflag()
{
	GCF_LAST=

	for gcf in $CFLAGS $PLATFORM_CFLAGS
	do
		if test "$GCF_LAST" = ""
		then
			case "$gcf" in
			-* )
					GCF_LAST="$gcf"
					;;
			* )
					;;
			esac
		else
			if test "$GCF_LAST" = "$1"
			then
				echo $gcf
			fi
			GCF_LAST=
		fi
	done 
}

PLATFORM_ISYSROOT=`platform_getcflag -isysroot`

echo PLATFORM_ISYSROOT=$PLATFORM_ISYSROOT 1>&2

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

if test "$MACOSX_DEPLOYMENT_TARGET" = ""
then
	echo MACOSX_DEPLOYMENT_TARGET not defined...

	case `uname -r` in
	6.* )
		MACOSX_DEPLOYMENT_TARGET=10.2
		;;
	7.* )
		MACOSX_DEPLOYMENT_TARGET=10.3
		;;
	8.* )
		MACOSX_DEPLOYMENT_TARGET=10.4
		;;
	9.* )
		MACOSX_DEPLOYMENT_TARGET=10.5
		;;
	10.* )
		MACOSX_DEPLOYMENT_TARGET=10.6
		;;
	11.* )
		MACOSX_DEPLOYMENT_TARGET=10.7
		;;
	* )
		exit 1
		;;
	esac	
fi

for d in AppleTalk IOKit PCSC CoreFoundation
do
	for e in $PLATFORM_ISYSROOT/System/Library/Frameworks/$d.framework/$d
	do
		echo $e...
		if test -f "$e"
		then
			(
				mkdir -p "$OUTDIR/otherlib"
				cd "$OUTDIR/otherlib"
				if test ! -f lib$d.dylib
				then
					ln -s "$e" lib$d.dylib
				fi
			)
		fi
	done	
done

find "$OUTDIR/otherlib" | xargs ls -ld

echo MACOSX_DEPLOYMENT_TARGET is $MACOSX_DEPLOYMENT_TARGET
export MACOSX_DEPLOYMENT_TARGET

if platform_not_member MACOSX_DEPLOYMENT_TARGET $MAKEDEFS_EXPORTS
then
	MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS MACOSX_DEPLOYMENT_TARGET"
fi

if platform_not_member -D_REENTRANT $CFLAGS $PLATFORM_CFLAGS
then
    PLATFORM_CFLAGS="$PLATFORM_CFLAGS -D_REENTRANT"
fi

if platform_not_member -D_REENTRANT $CXXFLAGS $PLATFORM_CXXFLAGS
then
    PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -D_REENTRANT"
fi

if platform_not_member -D_PLATFORM_DARWIN_ $CFLAGS $PLATFORM_CFLAGS
then
    PLATFORM_CFLAGS="$PLATFORM_CFLAGS -D_PLATFORM_DARWIN_"
fi

if platform_not_member -D_PLATFORM_DARWIN_ $CXXFLAGS $PLATFORM_CXXFLAGS
then
    PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -D_PLATFORM_DARWIN_"
fi

if platform_not_member -D_PLATFORM_UNIX_ $PLATFORM_SCFLAGS
then
	PLATFORM_SCFLAGS="$PLATFORM_SCFLAGS -D_PLATFORM_UNIX_"
fi

if platform_not_member RANLIB $MAKEDEFS_EXPORTS
then
	MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS RANLIB"
fi

LIBPREFIX=lib
LIBSUFFIX=.dylib

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS LIBPREFIX LIBSUFFIX SHLB_REF_STRONG SHLB_REF_WEAK"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS PLATFORM_CFLAGS PLATFORM_CXXFLAGS"

X11CONTENDERS="/usr/X11R6 /usr/OpenMotif"

. ../../toolbox/findlibs.sh

if test "$X11LIBPATH" != ""
then
        X11LIBPATH=`arglist -L $X11LIBPATH`

		MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11LIBPATH"

        if test "$X11LIBS" != ""
        then
                X11LIBS="$X11LIBPATH $X11LIBS"
        fi
fi

if test "$THREADLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS $THREADLIBS"
fi

if test "$X11INCL" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11INCL"
fi

if test "$X11LIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11LIBS"
fi

if test "$XTLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS XTLIBS"
	HAVE_LIBXT=true
fi

if test "$XMLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS XMLIBS"
	HAVE_LIBXM=true
fi

MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_SCFLAGS"

if test "$CFLIBS" = ""
then
	if test -f "$OUTDIR/otherlib/libCoreFoundation.dylib"
	then
		CFLIBS="-L$OUTDIR/otherlib -lCoreFoundation"
	fi
fi

if test "$ATLIBS" = ""
then
	if test -f "$OUTDIR/otherlib/libAppleTalk.dylib"
	then
		ATLIBS="-L$OUTDIR/otherlib -lAppleTalk"
	fi
fi

if test "$PCSCLIBS" != ""
then
	HAVE_LIBPCSC=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS PCSCLIBS"
fi

if test "$X11LIBS" != ""
then
	HAVE_LIBX11=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11LIBS"
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

if test "$UUIDLIBS" != ""
then
	HAVE_LIBUUID=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS UUIDLIBS"
fi

if test "$DLLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS DLLIBS"
fi

if test "$EXPATLIBS" != ""
then
	HAVE_LIBEXPAT=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS EXPATLIBS"
fi

if test "$CURLLIBS" != ""
then
	HAVE_LIBCURL=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS CURLLIBS"
fi

if test "$CFLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS CFLIBS"
fi

if test "$ATLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS ATLIBS"
fi

if test "$PLATFORM_ISYSROOT" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_ISYSROOT"
fi

if test "$HAVE_LIBUSB" = ""
then
	HAVE_LIBUSB=false
fi

if test "$HAVE_LIBPCSC" = ""
then
	HAVE_LIBPCSC=false
fi

if test "$HAVE_LIBX11" = ""
then
	HAVE_LIBX11=false
fi

if test "$HAVE_LIBXT" = ""
then
	HAVE_LIBXT=false
fi

if test "$HAVE_LIBXM" = ""
then
	HAVE_LIBXM=false
fi

if test "$HAVE_LIBUUID" = ""
then
	HAVE_LIBUUID=false
fi

if test "$HAVE_LIBEXPAT" = ""
then
	HAVE_LIBEXPAT=false
fi

if test "$HAVE_LIBCURL" = ""
then
	HAVE_LIBCURL=false
fi

if test "$HAVE_LIBCRYPTO" = ""
then
	HAVE_LIBCRYPTO=false
fi

if test "$HAVE_LIBSSL" = ""
then
	HAVE_LIBSSL=false
fi

MAKEDEFS_DEFS="$MAKEDEFS_DEFS HAVE_LIBUSB HAVE_LIBPCSC HAVE_LIBUUID HAVE_LIBX11 HAVE_LIBXT HAVE_LIBXM HAVE_LIBEXPAT HAVE_LIBCURL HAVE_LIBCRYPTO HAVE_LIBSSL"
