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

PATH_LIST=
LIB_LIST=

for d in $@
do
	case "$d" in
	-L* )
		THIS_PATH=`echo $d | sed s/-L//`
		PATH_LIST="$PATH_LIST $THIS_PATH"
		;;
	-* )
		echo 1>&2 unknown option "$d"
		exit 1
		;;
	* )
		if test -f "$d"
		then
			TODO="$TODO $d"
		else
			echo 1>&2 "$d"
			exit 1
		fi
		;;
	esac
done

last()
{
	LAST=
	for LASTN in $@
	do
		LAST=$LASTN
	done
	echo $LAST
}

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

needed()
{
	objdump -p "$1" | while read N M
	do
		case "$N" in
		NEEDED )
			echo $M
			;;
		* )
		esac
	done
}

find_lib()
{
	FLRC=0

	for fl in $@
	do
		MISSING=true

		if test -f "$fl"
		then
			echo "$fl"
			MISSING=false
		else
			for fd in $PATH_LIST
			do
				if test -f "$fd/$fl"
				then
					MISSING=false
					echo "$fd/$fl"
					break
				fi		
			done
		fi
		if $MISSING
		then
			echo library missing: $fl 1>&2
			FLRC=1
		fi
	done

	return $FLRC
}

RC=0

while test "$TODO" != ""
do
	CUR_LIST="$TODO"
	LIB_LIST="$LIB_LIST $CUR_LIST"
	TODO=

	for d in $CUR_LIST
	do
		for NL in `needed $d`
		do
			if find_lib $NL >/dev/null
			then
				for e in `find_lib $NL`
				do
					if not_member $e $TODO $LIB_LIST
					then
						TODO="$TODO $e"
					fi
				done
			else
				RC=1
			fi
		done
	done
done

if test "$RC" != 0
then
	exit $RC
fi

SYM_LIST=

symlist()
{
	for X in $@
	do
		objdump -T $X | while read N
		do
			LAST=
			PRINT=false

			for M in $N
			do
				case "$M" in
				.text | .data | .bss | .rodata | .sbss )
					PRINT=true
					;;
				* )
					;;
				esac
				LAST=$M
			done

			if $PRINT
			then
				echo $LAST
			fi			
		done
	done
}

symlist2()
{
	THAT=
	symlist $@ | sort | while read THIS
	do
		if test "$THAT" != "$THIS"
		then
			echo "$THIS"
		fi
		THAT="$THIS"
	done
}

undeflist()
{
	for X in $@
	do
		objdump -T $X | while read N
		do
			NOT_WEAK=true
			LAST=
			PRINT=false

			for M in $N
			do
				case "$M" in
				"w" )
					NOT_WEAK=false
					;;
				"*UND*" )
					PRINT=$NOT_WEAK
					;;
				* )
					;;
				esac
				LAST=$M
			done

			if $PRINT
			then
				case "$LAST" in
				__* )
					;;
				errno | h_errno | _rtld_global_ro )
					;;
				* )
					echo $LAST
					;;
				esac
			fi			
		done
	done
}

undeflist2()
{
	THAT=
	undeflist $@ | sort | while read THIS
	do
		if test "$THAT" != "$THIS"
		then
			echo "$THIS"
		fi
		THAT="$THIS"
	done
}

# echo $LIB_LIST

for d in $LIB_LIST
do
	echo $d
done

SYM_FILE=/tmp/ldd.$$.sym
UNDEF_FILE=/tmp/ldd.$$.undef

cleanup()
{
	rm -rf $SYM_FILE
	rm -rf $UNDEF_FILE
}

trap cleanup 0

symlist2 $LIB_LIST >$SYM_FILE
undeflist2 $LIB_LIST >$UNDEF_FILE

match_list()
{
	RC=0

	while read O
	do
		case "$O" in
		\>* )
			RC=1
			echo 1>&2 $O
			;;
		* )
			;;
		esac
	done

	exit $RC
}

# undeflist2 $LIB_LIST | match_list

RC=0

diff $SYM_FILE $UNDEF_FILE | match_list

RC=$?

exit $RC
