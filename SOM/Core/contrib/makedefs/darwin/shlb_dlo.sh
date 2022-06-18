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


if test "$PLATFORM" = ""
then
	echo PLATFORM not configured
	exit 1
fi

if test "$1" = ""
then
	echo usage: $0 \[-s\] \[-p\] \[-e\] \[dllname ....\]
	exit 1
fi

DLLSUFFIX=.dylib
DLLPREFIX=lib

LASTFLAG=

while test "$1" != ""
do
	case "$1" in
		-p ) 
			if test "$LASTFLAG" = "-s"
			then
				echo "static char sz_DLLPREFIX[]=\"$DLLPREFIX\";"
			else
				echo $DLLPREFIX
			fi
			;;
		-e ) 
			if test "$LASTFLAG" = "-s"
			then
				echo "static char sz_DLLSUFFIX[]=\"$DLLSUFFIX\";"
			else
				echo $DLLSUFFIX
			fi
			;;
		-s ) LASTFLAG=$1
			 DLLPREFIX=
			 DLLSUFFIX=.dll ;;
		-* ) LASTFLAG=$1 ;;
		* ) 
			if test "$LASTFLAG" = "-s"
			then
				echo "static char sz_dllname_$1[]=\"$DLLPREFIX$1$DLLSUFFIX\";"
			else
				echo $DLLPREFIX$1$DLLSUFFIX
			fi
			;;
	esac

	shift
done
