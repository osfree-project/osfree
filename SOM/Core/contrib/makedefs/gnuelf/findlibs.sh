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

X11CONTENDERS="/usr/X11R6 /opt/X11R6 /usr/X11R7 /opt/X11R7"

. ../../toolbox/findlibs.sh

if test "$SOCKLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS SOCKLIBS"
fi

if test "$UUIDLIBS" != ""
then
	HAVE_LIBUUID=true
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS UUIDLIBS"
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

if test "$DLLIBS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS DLLIBS"
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

if test "$HAVE_LIBEXPAT" = ""
then
	HAVE_LIBEXPAT=false
fi

if test "$HAVE_LIBCURL" = ""
then
	HAVE_LIBCURL=false
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

if test "$HAVE_LIBCRYPTO" = ""
then
	HAVE_LIBCRYPTO=false
fi

if test "$HAVE_LIBSSL" = ""
then
	HAVE_LIBSSL=false
fi

MAKEDEFS_DEFS="$MAKEDEFS_DEFS HAVE_LIBUSB HAVE_LIBPCSC HAVE_LIBEXPAT HAVE_LIBX11 HAVE_LIBUUID HAVE_LIBCURL HAVE_LIBXT HAVE_LIBXM HAVE_LIBCRYPTO HAVE_LIBSSL"
