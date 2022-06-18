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

ORIGINAL_CMD="$@"
PROGRAM_NAME=`basename $0`
TOOLBOX=`dirname $0`

# echo ASNEEDED.SH $ORIGINAL_CMD

first()
{
	echo $1
}

remove()
{
	removal=$1

	shift
	for remove_it in $@
	do
		if test "$removal" != "$remove_it"
		then
			echo $remove_it
		fi
	done
}

not_member()
{
	not_member_1=$1
	shift
	for not_member_i in $@
	do
		if test "$not_member_1" = "$not_member_i"
		then
			return 1
		fi
	done
	return 0
}

ASNEEDED_C=`basename $0 | sed y/./\ /`
ASNEEDED_C=`first $ASNEEDED_C`
ASNEEDED_C=`dirname $0`/src/$ASNEEDED_C.c

if test ! -f $ASNEEDED_C
then
	echo $0: $ASNEEDED_C not found 1>&2
	exit 1
fi

asneeded()
{
	ASNEEDED_RC=0
	LAST_ARG=
	OUTPUT_FILE=
	IS_SHARED=
	SEARCH_PATHS=
	MORE_ARGS=

	for my_d in $@
	do
		if test "$LAST_ARG" = ""
		then
			case $my_d in
			-L* )
				SEARCH_PATH=`echo $my_d | sed s/-L/\ /`
				SEARCH_PATHS="$SEARCH_PATHS	$SEARCH_PATH"
				;;
			-o )
				LAST_ARG=$my_d
				;;
			-shared )
				IS_SHARED=$my_d
				;;
			* )
				;;
			esac
		else
			case $LAST_ARG in
			-o )
				OUTPUT_FILE=$my_d
				;;
			* )
				;;
			esac
			LAST_ARG=
		fi
	done

	if test "$OUTPUT_FILE" = ""
	then
		echo cannot determine output file from $@
		return 1
	fi

	if test "$IS_SHARED" = ""
	then
		if test "$SHLB_REF_WEAK" != ""
		then
			LDD_PATHS=

			for dd in $SEARCH_PATHS
			do
				LDD_PATHS="$LDD_PATHS -L$dd"
			done

			$TOOLBOX/ldd.sh "$OUTPUT_FILE" $LDD_PATHS -L$PLATFORM_ROOT/usr/lib -L$PLATFORM_ROOT/lib

			ASNEEDED_RC=$?

			if test "$ASNEEDED_RC" != 0
			then
				rm -rf "$OUTPUT_FILE"
			fi
		fi
	else
		for my_d in $SEARCH_PATHS
		do
			MORE_ARGS="$MORE_ARGS -L$my_d -Wl,-rpath,$my_d"
		done

		$CC $CFLAGS -c $ASNEEDED_C -o $INTDIR/asneeded.o 

		IS_SHARED=$?

		if test "$IS_SHARED" = "0"
		then
			$CC $CFLAGS $INTDIR/asneeded.o $OUTPUT_FILE $MORE_ARGS -o $INTDIR/a.out

			IS_SHARED=$?
		fi

		if test -f $INTDIR/asneeded.o
		then
			rm $INTDIR/asneeded.o
		fi

		if test -f $INTDIR/a.out
		then
			rm $INTDIR/a.out
		fi

		ASNEEDED_RC=$IS_SHARED
	fi

	return $ASNEEDED_RC
}

# see if original will build

$ORIGINAL_CMD

if test "$?" != "0"
then
	exit 1
fi

asneeded $ORIGINAL_CMD >/dev/null

ASNEEDED_RC=$?

if test "$ASNEEDED_RC" != "0"
then
	exit $ASNEEDED_RC
fi

DROPPED=bogus

while test "$DROPPED" != ""
do
	LIBLIST=
	DROPPED=

	for d in $ORIGINAL_CMD
	do
		case $d in
		-l* )
			LIBLIST="$LIBLIST $d"
			;;
		* )
			;;
		esac
	done	

#	echo LIBLIST=$LIBLIST

	for d in $LIBLIST
	do
		if not_member $d $REQUIRED_LIBS
		then
#			echo trying without $d

			NEW_CMD=`remove $d $ORIGINAL_CMD`

			if $NEW_CMD >/dev/null 2>&1
			then
				if asneeded $NEW_CMD >/dev/null 2>/dev/null
				then
					echo "$PROGRAM_NAME:" dropping $d
					ORIGINAL_CMD="$NEW_CMD"
					DROPPED=$d
				fi
			fi
		fi
	done

done

exec $ORIGINAL_CMD

