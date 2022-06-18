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

if test -z "$AR"
then
	AR=ar
fi

if test -z "$ARFLAGS"
then
	ARFLAGS=rv
fi

first()
{
	echo "$1"
}

CLEANUP_LIST=""
OLIST=""
LASTFLAG=""
AR_TGT=""
MODNAME=""
NEEDED=""
DEPENDS=""

for d in $@
do
	if test -n "$LASTFLAG"
	then
		case "$LASTFLAG" in
			-o ) AR_TGT="$d" ;;
			-soname ) MODNAME="$d" ;;
		esac
		LASTFLAG=""
	else
		case "$d" in
			-l* )
				 ADLIB=`echo $d | sed s/-l//`
#				 echo ADLIB is $ADLIB
				 NEEDED="$NEEDED lib$ADLIB.a"
				 ;;
			-L* ) if test "$d" = "-L"
				  then
					LASTFLAG=$d
				  fi
				 ;;
			-o | -soname ) LASTFLAG=$d ;;
			*.o ) OLIST="$OLIST $d" ;;
			* ) echo $0: error at argument $d
				exit 1
				;;		
		esac
	fi
done

# echo $OLIST

if test -z $AR_TGT
then
	echo $0: no -o clause
	exit 1
fi

INTDIR=`dirname $AR_TGT`

if test -z $INTDIR
then
	INTDIR=.
fi

if test -f $AR_TGT
then
	echo removing $AR_TGT
	rm $AR_TGT
fi

DEPENDS=`basename $AR_TGT`

case "$DEPENDS" in
lib*.a )
	DEPENDS=`echo $DEPENDS | sed s/lib// | sed y/./\ /`
	DEPENDS=`first $DEPENDS`
	;;
* )
	;;
esac

DEPENDS="$INTDIR/$DEPENDS.dep"

if test -z "$MODNAME"
then
	MODNAME=`basename $AR_TGT`
fi

# when adding files, prepend module name to each object file in archive

AROLIST=""

for d in $OLIST
do
	if test -f $d
	then
#		ARLNAME="$MODNAME-$d"
#		ln -s $d $ARLNAME
#		AROLIST="$AROLIST $ARLNAME"
		AROLIST="$AROLIST $d"
	fi
done

if test "$NEEDED" != ""
then
	if test -f "$DEPENDS"
	then
		rm "$DEPENDS"
	fi

	for d in $NEEDED
	do
		echo $d >>$DEPENDS
	done

	CLEANUP_LIST="$CLEANUP_LIST"
fi

if test -f RPATH
then
	AROLIST="RPATH $AROLIST"
fi

# echo $AR ${ARHYPHEN}r $AR_TGT $AROLIST

$AR $ARFLAGS $AR_TGT $AROLIST

AR_EXIT=$?

for d in $CLEANUP_LIST
do
	if test -f $d
	then
		rm $d
	fi
done

#if test "$AROLIST" != ""
#then
#	rm $AROLIST
#fi

if test "$AR_EXIT" != "0"
then
	exit 1
fi

if test "$RANLIB" != ""
then
	AR_TGT_DIR=`dirname $AR_TGT`
	AR_TGT_BASE=`basename $AR_TGT`
	echo AR_TGT_DIR=$AR_TGT_DIR AR_TGT_BASE=$AR_TGT_BASE
	cd $AR_TGT_DIR
	RC=$?
	if test "$RC" != "0"
	then
		exit 1
	fi

	$RANLIB $AR_TGT_BASE
fi

