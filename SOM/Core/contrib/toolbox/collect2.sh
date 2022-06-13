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

ORIGINAL_CMD=$@

echo ORIGINAL_CMD=$ORIGINAL_CMD

TARGET=
PATH_LIST=
LIB_LIST=
LLIB_LIST=
OBJ_LIST=
OPT_LIST=
LAST_FLAG=
HAVE_LIBS=false

find_lib()
{
	for d in $@
	do
		case "$d" in 
		-lm | -lc )
			echo $d
			;;
		-l* )
			g="$d"
			d=`echo $d | sed s/-l//`
			for e in $PATH_LIST 
			do
				f="$e/lib$d.a"
				if test -f "$f"
				then
					g="$f"
					break
				fi
			done
			echo "$g"
			;;
		* )
			echo $d
			;;
		esac
	done
}

for d in $ORIGINAL_CMD
do
	if test "$LAST_FLAG" = ""
	then
		case "$d" in
		-o )
			LAST_FLAG="$d"
			;;
		*.o )
			OBJ_LIST="$OBJ_LIST $d"
			;;
		-l* )
			e=`find_lib $d`
			case "$e" in
			-l* )
				LLIB_LIST="$LLIB_LIST $e"
				;;
			* )
				LIB_LIST="$LIB_LIST $e"
				HAVE_LIBS=true
				;;
			esac
			;;
		-L* )
			d=`echo $d | sed s/-L//`
			PATH_LIST="$PATH_LIST $d"
			;;
		* )
			OPT_LIST="$OPT_LIST $d"
			;;
		esac
	else
		case "$LAST_FLAG" in
		-o )
			TARGET=$d
			;;
		* )
			echo unknown option $d 1>&2

			exit 1
			;;
		esac

		LAST_FLAG=
	fi
done

echo OPT_LIST=$OPT_LIST
echo OBJ_LIST=$OBJ_LIST
echo TARGET=$TARGET
echo PATH_LIST=$PATH_LIST
echo LIB_LIST=$LIB_LIST
echo LLIB_LIST=$LLIB_LIST

if test "$TARGET" = ""
then
	echo unknown target 1>&2 
	exit 1
fi

TARGET_DIR=`dirname $TARGET`
TARGET_TMP="$TARGET_DIR/$$"

RC=1

if $HAVE_LIBS
then
	mkdir $TARGET_TMP

	if test "$?" != "0"
	then
		exit $RC
	fi

	for d in $LIB_LIST
	do
		ar -t $d | while read e
		do
			case "$e" in 
			*.o )
				CONTENDER="$TARGET_TMP/$e"

				if test -f "$CONTENDER"
				then
					NUM=1

					while true
					do
						CONTENDER="$TARGET_TMP/$NUM-$e"
						if test -f "$CONTENDER"
						then
							NUM=`echo $NUM+1 | bc`
						else
							break
						fi
					done
				fi
				;;
			* )
				;;
			esac

			ar -p $d $e >$CONTENDER
		done
	done

	(
		case `uname -s` in
		A/UX )
			XARGS_N="-n10"
			;;
		* )
			XARGS_N="-n 10"
			;;
		esac

		cd $TARGET_TMP
		find . -name \*.o -print | xargs $XARGS_N ar -r libbogus.a
		RC="$?"
	
		if test -n "$RANLIB"
		then
			$RANLIB libbogus.a
			RC=$?
		fi

		exit $RC			
	)

	RC=$?
fi

NEW_COMMAND="$OPT_LIST $OBJ_LIST -o $TARGET "

if test -f "$TARGET_TMP/libbogus.a"
then
	NEW_COMMAND="$NEW_COMMAND -L$TARGET_TMP -lbogus "
fi

for d in $PATH_LIST
do
	NEW_COMMAND="$NEW_COMMAND -L$d "
done

for d in "$LLIB_LIST"
do
	NEW_COMMAND="$NEW_COMMAND $d "
done

echo NEW_COMMAND="$NEW_COMMAND"

$NEW_COMMAND

RC=$?

if $HAVE_LIBS
then
	if test -d "$TARGET_TMP"
	then
		rm -r "$TARGET_TMP"
	fi
fi

exit $RC
