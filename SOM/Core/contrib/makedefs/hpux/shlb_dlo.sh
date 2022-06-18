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

DLLPREFIX=$SHLIBPREFIX
DLLSUFFIX=$SHLIBSUFFIX

LASTFLAG=

get_soname()
{
	e=$1

	for d in `echo $SHLB_REF_STRONG $SHLB_REF_WEAK $SHLB_REF_NAME | sed s/:/\ /`
	do
		if test -f $d/$e		
		then
			LOOK4_INTERNAL=
			LOOK4_NAME=

			for f in `chatr $d/$e`
			do
				if test "$LOOK4_INTERNAL" = "internal"
				then
					if test "$LOOK4_NAME" = "name:"
					then
						for g in $f $d/$f
						do
							if test -f $g
							then
								echo $f
								return 0
							fi
						done
					fi
				fi

				LOOK4_INTERNAL="$LOOK4_NAME"
				LOOK4_NAME="$f"
			done


			for f in `elfdump -L $d/$e | grep Soname`
			do
				for g in $f $d/$f
				do
					if test -f $g
					then
						echo $f
						return 0
					fi
				done
			done
		fi
	done

	return 1
}

while test "$1" != ""
do
	case "$1" in
		-p ) 
			if test "$LASTFLAG" = "-s"
			then
				echo "static char sz_DLLPREFIX[]=\"$SHLIBPREFIX\";"
			else
				echo $SHLIBPREFIX
			fi
			;;
		-e ) 
			if test "$LASTFLAG" = "-s"
			then
				echo "static char sz_DLLSUFFIX[]=\"$SHLIBSUFFIX\";"
			else
				echo $SHLIBSUFFIX
			fi
			;;
		-* ) LASTFLAG=$1 ;;
		* ) 
			SONAME=`get_soname $1.dll`
			if test "$SONAME" = ""
			then
				SONAME=`get_soname $SHLIBPREFIX$1$SHLIBSUFFIX`
			fi
			if test "$SONAME" = ""
			then
				SONAME=$SHLIBPREFIX$1$SHLIBSUFFIX
			fi
			if test "$LASTFLAG" = "-s"
			then
				echo "static char sz_dllname_$1[]=\"$SONAME\";"
			else
				echo $SONAME
			fi
			;;
	esac

	shift
done
