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

SHLB_REF_STRONG=$OUTDIR/lib
SHLB_REF_WEAK=$OUTDIR/implib

if test "$LD_LIBRARY_PATH" = ""
then
	LD_LIBRARY_PATH=$SHLB_REF_STRONG
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SHLB_REF_STRONG
fi

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS LD_LIBRARY_PATH SHLB_REF_STRONG SHLB_REF_WEAK"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_CFLAGS PLATFORM_CXXFLAGS CONFIG_OPTS"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS CC_FLAG_FPIC CXX_FLAG_FPIC CONFIG_LIBS"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_SCFLAGS"

X11CONTENDERS=/usr/X11R6

. ../../toolbox/findlibs.sh

if test "$SOCKLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS SOCKLIBS"
fi

if test "$THREADLIBS" != ""
then
	CONFIG_LIBS="$THREADLIBS $CONFIG_LIBS"
fi

if test "$X11INCL" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11INCL"
fi

if test "$X11LIBPATH" != ""
then
	X11APP=`arglist -Wl,-rpath, $X11LIBPATH`
	X11LIBPATH=`arglist -L $X11LIBPATH`

	MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11APP"

	if test "$X11LIBS" != ""
	then
		X11LIBS="$X11LIBPATH $X11LIBS"
	fi
fi

if test "$X11LIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS X11LIBS"
fi

if test "$USBLIBS" != ""
then
	HAVE_LIBUSB=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS USBLIBS"
fi

if test "$PCSCLIBS" != ""
then
	HAVE_LIBPCSC=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS PCSCLIBS"
fi

if test "$HAVE_LIBUSB" = ""
then
	HAVE_LIBUSB=false
fi

if test "$HAVE_LIBPCSC" = ""
then
	HAVE_LIBPCSC=false
fi

MAKEDEFS_DEFS="$MAKEDEFS_DEFS HAVE_LIBUSB HAVE_LIBPCSC"
