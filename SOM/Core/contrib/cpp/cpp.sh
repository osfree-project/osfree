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

FILELIST=
SWITCHLIST=
LASTFLAG=

cleanup()
{
	if test -f "$MYSTDIN"
	then
		rm "$MYSTDIN"
	fi
}

for ARG in $@
do
	if test "$LASTFLAG" = ""
	then
		case "$ARG" in 
		-I | -D )
			LASTFLAG="$ARG"
			;;
		-I* | -D* )
			SWITCHLIST="$SWITCHLIST $ARG"
			;;
		-* )
			;;
		* )
			FILELIST="$FILELIST $ARG"
			;;
		esac
	else
		case "$LASTFLAG" in
		-I | -D )
			SWITCHLIST="$SWITCHLIST $LASTFLAG$ARG"
			;;
		* )
			;;
		esac 
		LASTFLAG=
	fi
done

if test "$CC" = ""
then
	CC=cc
fi

if test "$CC_CPP_OPTS" = ""
then
	case `uname -s` in
	IRIX | IRIX64 )
		CC_CPP_OPTS="-E -ansi"
		;;
	* )
		CC_CPP_OPTS="-E"
		;;
	esac		
fi

if test "$CPP_STDIN" = ""
then
	case `uname -s` in
	AIX )
		CPP_STDIN="/usr/lib/cpp"
		;;
	OSF1 )
		CPP_STDIN="cpp -p -std"
		;;
	* )
		CPP_STDIN="$CC $CC_CPP_OPTS"
		SWITCHLIST="$SWITCHLIST -"
		;;
	esac
fi

umask 077

if test "$TMP" = ""
then
	MYSTDIN="/tmp/cpp.$$.c"
else
	MYSTDIN="$TMP/cpp.$$.c"
fi

if test -f "$MYSTDIN"
then
	echo "$MYSTDIN" already exists 1>&2
	exit 1
fi

trap cleanup 0

# echo $CPP_STDIN $SWITCHLIST 1>&2

if test "$FILELIST" = ""
then
	$CPP_STDIN $SWITCHLIST

	RC=$?
else
    for ARG in $FILELIST
	do
		echo "#include" \"$ARG\" >>"$MYSTDIN"
	done

	$CPP_STDIN $SWITCHLIST <"$MYSTDIN"

	RC=$?
fi

cleanup

exit $RC
