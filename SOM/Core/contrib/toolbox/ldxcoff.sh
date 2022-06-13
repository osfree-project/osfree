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
LDXCOFF_SH="$0"
NEW_CMD=
LAST_FLAG=
ENTRY_SYMBOL=
EXPORT_FILE=
EXPORT_FILE_ARG=
OUTPUT_FILE=
HAS_NOENTRY=
LAUNDRY_LIST=

first()
{
	echo $1
}

if test "$AR" = ""
then
	AR=ar
fi

if test "$ARFLAGS" = ""
then
	ARFLAGS="-rv"
fi

EXEC_AR="$AR $ARFLAGS $PLATFORM_ARFLAGS"

PRIVATE_EXPORTS="SOMInitModule DllGetClassObject DllMain DllRegisterServer DllUnregisterServer DllCanUnloadNow"

cleanup()
{
	for d in $LAUNDRY_LIST
	do
		if test -f $d
		then
			echo cleanup $d
			rm $d
		fi
	done
}

trap cleanup 0

# echo 1>&2 This is for AIX
# echo $ORIGINAL_CMD

is_member()
{
    is_member_1=$1
    shift
    for is_member_i in $@
    do
        if test "$is_member_i" = "$is_member_1"
        then
            return 0
        fi
    done
    return 1
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

get_colon_arg()
{
	echo $@ | sed y/:/\ / | ( read A B; echo $B )
}

get_filestem()
{
	basename $@ | sed y/./\ / | ( read A B; echo $A )
}

get_fileext()
{
	basename $@ | sed y/./\ / | ( read A B; echo $B )
}

for d in $ORIGINAL_CMD
do
	if test "$LAST_FLAG" = ""
	then
		case "$d" in
		-bE:* | -bexport:* )
#			echo found export flag $d

			EXPORT_FILE=`get_colon_arg $d`
			;;
		-e | -o )
			LAST_FLAG=$d
			d=
			;;
		-bnoentry )
			HAS_NOENTRY=$d
			;;
		-e* )
#			echo found entry included flag
			ENTRY_SYMBOL=`echo $d | sed "s/-e//"`
			;;
		* ) 
			;;
		esac

		if test "$d" != ""
		then
			if test "$NEW_CMD" = ""
			then
				NEW_CMD="$d"
			else
				NEW_CMD="$NEW_CMD $d"
			fi
		fi
	else
		case "$LAST_FLAG" in
		-e )
			ENTRY_SYMBOL=$d
			;;
		-o )
			OUTPUT_FILE=$d
			LAST_FLAG=
			d=
			;;
		* )
			echo $LDXCOFF_SH: 1>&2 LAST_FLAG $LAST_FLAG not handled
			exit 1
			;;
		esac

		NEW_CMD="$NEW_CMD $LAST_FLAG $d"

		LAST_FLAG=
	fi
done

# echo NEW_CMD=$NEW_CMD

if test "$OUTPUT_FILE" = ""
then
	echo $LDXCOFF_SH: 1>&2 no output file recognized
	exit 1
fi

if test -f "$OUTPUT_FILE" 
then
	echo $LDXCOFF_SH: 1>&2 output file already exists "$OUTPUT_FILE"
	rm "$OUTPUT_FILE"

	if test "$?" != "0"
	then
		exit 1
	fi
fi

if test "$INTDIR" = ""
then
	INTDIR=`dirname "$OUTPUT_FILE"`
	if test "$INTDIR" = ""
	then
		INTDIR=.
	fi
fi

LDXCOFF_C=`basename $LDXCOFF_SH | sed y/./\ /`
LDXCOFF_C=`dirname $LDXCOFF_SH`/src/`first $LDXCOFF_C`.c

if test ! -f $LDXCOFF_C
then
	echo $LDXCOFF_SH: missing $LDXCOFF_C 1>&2
	exit 1
fi

LDXCOFF_O=`dirname $OUTPUT_FILE`/a.out

LAUNDRY_LIST="$LAUNDRY_LIST $LDXCOFF_O"

if $CC $CFLAGS $PLATFORM_CFLAGS -c $LDXCOFF_C -o $LDXCOFF_O -DTEST_32BIT >/dev/null 2>/dev/null
then
	SHARED_OBJ=shr.o
	SHARED_EXP=shr.exp
	SHARED_BITS=32
else
	if $CC $CFLAGS $PLATFORM_CFLAGS -c $LDXCOFF_C -o $LDXCOFF_O -DTEST_64BIT >/dev/null 2>/dev/null
	then
		SHARED_OBJ=shr_64.o
		SHARED_EXP=shr_64.exp
		SHARED_BITS=64
	else
		echo $LDXCOFF_SH: unknown machine size 1>&2
		exit 1
	fi
fi

TMPDIR=$INTDIR/ldxcoff-$$.d

case `basename $OUTPUT_FILE` in
lib*.a )
	BINARY_FILE=$TMPDIR/$SHARED_OBJ
	;;
* )
	BINARY_FILE=$OUTPUT_FILE
	;;
esac

NEW_CMD="$NEW_CMD -o $BINARY_FILE"

EXPORT_FILE_ARG=$EXPORT_FILE

if test "$ENTRY_SYMBOL" = ""
then
#	echo has no entry symbol
	if test "$HAS_NOENTRY" = ""
	then
		NEW_CMD="$NEW_CMD -bnoentry"
	fi
else
#	echo ENTRY_SYMBOL=$ENTRY_SYMBOL

	if test "$EXPORT_FILE" = ""
	then
		EXPORT_FILE=$INTDIR/`basename $OUTPUT_FILE`.exp
		NEW_CMD="$NEW_CMD -bE:$EXPORT_FILE"
		echo "#!"`basename $OUTPUT_FILE` >$EXPORT_FILE

		LAUNDRY_LIST="$LAUNDRY_LIST $EXPORT_FILE"
	fi

	if test -f "$EXPORT_FILE"
	then
		if not_member $ENTRY_SYMBOL `cat $EXPORT_FILE`
		then
#			echo $ENTRY_SYMBOL appended to $EXPORT_FILE
			echo $ENTRY_SYMBOL >>$EXPORT_FILE
		fi
	else
		echo $LDXCOFF_SH: 1>&2 $EXPORT_FILE does not exist
		exit 1
	fi
fi

if test "`dirname $BINARY_FILE`" = "$TMPDIR"
then
	if test -f "$TMPDIR"
	then
		ls -ld $TMPDIR
	else
		mkdir $TMPDIR
	fi
fi

echo $NEW_CMD
$NEW_CMD

RC=$?

if test -d "$TMPDIR"
then
	if test "$RC" = "0"
	then
		case "$BINARY_FILE" in
		$TMPDIR/$SHARED_OBJ )
			echo "# autoload" >$TMPDIR/$SHARED_EXP
			OUTPUT_BASE=`basename $OUTPUT_FILE`

			echo "#!"$OUTPUT_BASE\($SHARED_OBJ\) >$TMPDIR/$SHARED_EXP
			echo "# $SHARED_BITS" >>$TMPDIR/$SHARED_EXP

			if test -f $EXPORT_FILE_ARG
			then
				grep -v "#" $EXPORT_FILE_ARG | while read N
				do
					if test "$N" != ""
					then
						if not_member $N $PRIVATE_EXPORTS
						then
							echo $N >>$TMPDIR/$SHARED_EXP
						fi
					fi
				done		
			fi

			( 
				cd $TMPDIR
				$EXEC_AR `basename $OUTPUT_FILE` $SHARED_EXP $SHARED_OBJ
				RC=$?

				exit $RC
			)

			RC=$?

			if test "$RC" = "0"
			then
				cp $TMPDIR/`basename $OUTPUT_FILE` $OUTPUT_FILE
				RC=$?
			fi

			;;
		* )
			;;
		esac
	fi
#	echo removing $TMPDIR
#	ls -ld $TMPDIR/*
	rm -r $TMPDIR
fi

if test "$RC" != "0"
then
	exit $RC
fi

if test "$EXPORT_FILE_ARG"
then
	if test -f "$EXPORT_FILE_ARG"
	then
		case `basename $OUTPUT_FILE` in
		*.dll )
			OUTPUT_DIR=`dirname $OUTPUT_FILE`
			OUTPUT_STEM=`get_filestem $OUTPUT_FILE`
			EXPORT_FILE=$OUTPUT_DIR/$OUTPUT_STEM.exp

			echo "#!"`basename $OUTPUT_FILE` >$EXPORT_FILE
			echo "# $SHARED_BITS" >>$EXPORT_FILE

			grep -v "#" $EXPORT_FILE_ARG | while read N
			do
				if test "$N" != ""
				then
					if not_member $N $PRIVATE_EXPORTS
					then
						echo $N >>$EXPORT_FILE
					fi
				fi
			done

			LINKTOOL_MAK=$INTDIR/clean.$OUTPUT_STEM.exp.mak
			CLEAN_FILES=$EXPORT_FILE

			if $EXEC_AR $OUTPUT_DIR/lib$OUTPUT_STEM.a $EXPORT_FILE
			then
				rm $EXPORT_FILE

				LINKTOOL_MAK=$INTDIR/clean.lib$OUTPUT_STEM.a.mak
				CLEAN_FILES=$OUTPUT_DIR/lib$OUTPUT_STEM.a
			fi

			echo "clean:" >$LINKTOOL_MAK
			for d in $CLEAN_FILES
			do
				echo "	if test -f \"$d\"; then rm \"$d\"; fi" >>$LINKTOOL_MAK
			done

			;;
		* )
			;;
		esac
	fi
fi

exit 0

