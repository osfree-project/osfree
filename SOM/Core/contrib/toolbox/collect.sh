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

echo $0 $@

EXPANDED_CMD=
RPATH_INIT="/usr/lib /lib"
RPATH_LINK="$RPATH_INIT"

case "$ARFLAGS" in
-* )
	ARHYPHEN="-"
	;;
* )
	ARHYPHEN=
	;;
esac

PREV=
for d in $@
do
	if test "$PREV" = "-o"
	then
		COLLECT_D=`dirname $d`/collect.$$
		break
	fi
	PREV=$d
done

if test "$COLLECT_D" = ""
then
	echo COLLECT_D not defined
	exit 1
	COLLECT_D=collect.d
fi

if test -d $COLLECT_D
then
	rm -r $COLLECT_D
fi

mkdir $COLLECT_D

if test "$?" != "0"
then
	exit 1
fi

first()
{
    echo $1
}

rotate()
{
	if test "$1" != ""
	then
	    shift
		echo $@
	fi
}

not_member()
{
    not_member_1=$1

	if test "$not_member_1" != ""
	then
		shift
		for not_member_i in $@
		do
			if test "$not_member_1" = "$not_member_i"
			then
				return 1
			fi
		done
	fi

    return 0
}

remove()
{
	removal=$1

	if test "$removal" != ""
	then
		shift
		for remove_it in $@
		do
			if test "$removal" != "$remove_it"
			then
				echo $remove_it
			fi
		done
	fi
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

get_depends()
{
    depends_arg1=`basename $1`
    case $depends_arg1 in
    lib*.a )
		depends_arg2=`echo $depends_arg1 | sed s/lib// | sed y/./\ /`
		echo `dirname $1`/`first $depends_arg2`.dep
        ;;
    * )
        echo $1.dep
        ;;
    esac;
}

get_depbase()
{
	basename `get_depends $1`
}

get_needed()
{
    needed_arg1=`basename $1`
    case $needed_arg1 in
    lib*.a )
		needed_arg2=`get_depends $1`
		if test -f "$needed_arg2"
		then
			cat "$needed_arg2"
		else
			case $needed_arg1 in
			libXext.a )
				echo libX11.a
				;;
			libXt.a )
				echo libX11.a
				;;
			libX11.a )
				echo libc.a
				;;
			* )
				;;
			esac
		fi
        ;;
    * )
        objdump -p $1 | grep NEEDED | ( while read NEEDED needed; do echo $needed; done )
        ;;
    esac;
}

get_fullpath()
{
    foundpath=
    baseoffile=$1

	if test "." != `dirname $baseoffile`
	then
#		echo $baseoffile already includes path 1>&2
		exit 1
	fi

    if test "$baseoffile" != ""
    then
		if test -f $COLLECT_D/$baseoffile.p
		then
			cat $COLLECT_D/$baseoffile.p
			return 0;
		fi

        shift
        for possdir in $@
        do
            if test "$foundpath" = ""
            then
                posspath="$possdir/$baseoffile"
                if test -f "$posspath"
                then
                    foundpath="$posspath"
                fi
            fi
        done

        if test "$foundpath" = ""
        then
            if not_member $baseoffile $NOTFOUND
            then
                NOTFOUND="$NOTFOUND $baseoffile"
#                echo library \"$baseoffile\" not found 1>&2
            fi

            return 1;
        fi

		echo $foundpath >$COLLECT_D/$baseoffile.p

        echo $foundpath

        return 0;
    fi

    echo get_fullpath: someone looked for empty string 1>&2

    return 1;
}

get_needed_files()
{
    get_needed $1 | while read f
	do 
		get_fullpath $f $RPATH_LINK
	done
}

get_all_needed_files()
{
	rootfile=$1
	baseroot=`get_depends $rootfile`
	baseroot=`basename $baseroot`

	if test -f $COLLECT_D/$baseroot
	then
		cat $COLLECT_D/$baseroot
		return 0;
	fi

#	echo Generating $COLLECT_D/$baseroot 1>&2

	TODOLIST=$rootfile
	DONELIST=

	while test "$TODOLIST" != ""
	do
		MORETODO=

		for a in $TODOLIST
		do
			if test "$a" != ""
			then
				if not_member $a $DONELIST
				then
					DONELIST="$DONELIST $a"
					for b in `get_needed_files $a`
					do
						if test "$b" != ""
						then
							if not_member $b $TODOLIST
							then
								if not_member $b $DONELIST
								then
									if not_member $b $MORETODO
									then
										MORETODO="$MORETODO $b"
									fi
								fi
							fi
						fi
					done
				fi
			fi
		done

		TODOLIST="$MORETODO"
	done

	DONELIST=`remove $rootfile $DONELIST`

#	for a in $DONELIST
#	do
#		
#		if test "$a" != "$rootfile"
#		then
#			echo "$a"
#		fi
#	done

	echo $DONELIST	

	echo $DONELIST >$COLLECT_D/$baseroot
}

get_filestem()
{
    for filestem in $@
    do
        echo $filestem | ( IFS='.' read stem ext ; echo $stem; )
    done
}

select_libs()
{
	for d in $@
	do
		case $d in
		*/lib*.a )
			echo $d
			;;
		* )
			;;
		esac
	done
}

add_to_collection()
{
	tgtlib=$1

	TARGET_NEEDED=`get_depends $tgtlib`
	
	shift

	NEEDED_FILES=

	for addlib in $@
	do
#		echo adding $addlib to $tgtlib

		NEEDED=`get_needed $tgtlib`

#		echo current NEEDED is $NEEDED

		addbase=`basename $addlib`
		adddep=`get_depends $addlib`
		adddep=`basename $adddep`

		NEEDED_FILES="$NEEDED_FILES $COLLECT_D/$adddep"

		NEEDED=`remove $addbase $NEEDED`

#		echo new NEEDED is $NEEDED

		ADDNEEDS=`get_needed $addlib`

#		echo ADDNEEDS is $ADDNEEDS

		for ndy in $ADDNEEDS
		do
			if not_member $ndy $NEEDED
			then
				if test "$tgtlib" != "`cat $COLLECT_D/$ndy.p`"
				then
					NEEDED="$NEEDED $ndy"
				fi
			fi
		done

		filestoadd=

		for ndy in `$AR ${ARHYPHEN}t $addlib`
		do
			case $ndy in
			*.o )
				filestoadd="$filestoadd $ndy"
				;;
			* )
				;;
			esac
		done

		if test "$filestoadd" != ""
		then
			for ndy in $filestoadd
			do
				$AR ${ARHYPHEN}p $addlib $ndy >$COLLECT_D/$ndy
			done
		fi

		filestoadd="$filestoadd"

		if ( cd $COLLECT_D
			if $AR ${ARHYPHEN}r `basename $tgtlib` $filestoadd
			then
				rm $filestoadd
			else
				exit 1
			fi
		)
		then
			true
		else
			echo error $AR ${ARHYPHEN}r `basename $tgtlib` $filestoadd
			exit 1
		fi
	done

#	echo TARGET_NEEDED=$TARGET_NEEDED 1>&2
#	echo NEEDED_FILES=$NEEDED_FILES 1>&2

	if test -f $TARGET_NEEDED
	then
		NEEDED=`cat $TARGET_NEEDED`
		for ndy in $NEEDED_FILES
		do
			if test -f $ndy
			then
				ADDNEEDS=`cat $ndy`
				for filestoadd in $ADDNEEDS
				do
					if not_member $filestoadd $NEEDED
					then
						if test "$tgtlib" != "$filestoadd"
						then
							NEEDED="$NEEDED $filestoadd"
						fi
					fi
				done

				rm $ndy
			else
				echo file $ndy does not exist in add_to_collection $tgtlib 1>&2
				exit 1
			fi
		done

		echo $NEEDED >$TARGET_NEEDED
	fi
}

change_needed()
{
	change_to=$1
	change_from=$2

#	echo changing references from $change_from to $change_to

	for d in $COLLECT_D/*.dep
	do
		if test -f $d
		then
			NEEDED=`cat $d`
			if is_member $change_from $NEEDED
			then
#				echo modifying file $d
				NEEDED=`remove $change_from $NEEDED`
				echo $NEEDED $change_to >$d
			fi
		fi
	done
}

for d in $@
do

	case $d in
	-L* )
		EXPANDED_CMD="$EXPANDED_CMD $d"
		d=`echo $d | sed s/-L//`
		RPATH_LINK="$RPATH_LINK $d"
		;;
	-l* )
		d=`echo $d | sed s/-l//`
		p=`get_fullpath lib$d.a $RPATH_LINK`
		if test "$p" = ""
		then
			echo lib$d.a not found in $RPATH_LINK
			EXPANDED_CMD="$EXPANDED_CMD -l$d"
		else
#			echo found $p
			EXPANDED_CMD="$EXPANDED_CMD $p"
		fi
		;;
	* )
		EXPANDED_CMD="$EXPANDED_CMD $d"
		;;
	esac
done

# echo EXPANDED_CMD=$EXPANDED_CMD

#for d in $EXPANDED_CMD
#do
#	case $d in
#	*/lib*.a )
#		echo looking at $d
#		deps=`RPATH_LINK="$RPATH_LINK" get_all_needed_files $d`
#		for e in $deps
#		do
#			echo ... $e
#		done
#		;;
#
#	* )
#		;;
#	esac
#
#done

# echo RPATH_LINK=$RPATH_LINK

LOOPFOUND="1"

while test "$LOOPFOUND" != ""
do
	LOOPFOUND=
	LHS=
	RHS=$EXPANDED_CMD

#	echo Loop started....
#	echo RHS=$RHS

	while test "$RHS" != ""
	do
		arg=`first $RHS`

		RHS=`rotate $RHS`

		if test "$arg" != ""
		then
			case $arg in
			*/lib*.a )
				SWAPPED=1

				while test "$SWAPPED" = "1"
				do
					SWAPPED=

					RHSLIBS=`select_libs $RHS`

					if is_member $arg $RHSLIBS
					then
						echo $arg is still in RHSLIBS 1>&2
						exit 1					
					else
						for lib in $RHSLIBS
						do
							if test "$LOOPFOUND" = ""
							then
								if is_member $arg `get_all_needed_files $lib`
								then
#									echo `basename $lib` depends on `basename $arg`
									if is_member $lib `get_all_needed_files $arg`
									then
#										echo `basename $arg` and `basename $lib` are mutually dependent
# echo $arg `dirname $arg`
	
	if test `dirname $arg` != "$COLLECT_D"
	then
		bn=`basename $arg`

		arg2="$COLLECT_D/`get_filestem $bn`Z.a"

		if test -f $COLLECT_D/`get_depbase $bn`
		then
			mv $COLLECT_D/`get_depbase $bn` `get_depends $arg2`
		fi

#		echo creating $arg2 from $arg
		change_needed $arg2 $arg
		cp $arg $arg2
		echo $arg2 >$COLLECT_D/`basename $arg`.p
		echo $arg2 >$arg2.p
		arg=$arg2
	fi
	
	change_needed $arg $lib

	add_to_collection $arg $lib

	if test "" != "$RANLIB"
	then
		echo using $RANLIB $arg
		(
			cd `dirname $arg`
			pwd
			$RANLIB `basename $arg`
		)
	fi

#	$AR ${ARHYPHEN}s $arg

	RHS=`remove $lib $RHS`

	echo $arg >$COLLECT_D/`basename $lib`.p

#	for needed in $COLLECT_D/lib*.a.n
#	do
#		if test -f $needed
#		then
#		echo removing file $needed
#			rm $needed
#		fi
#	done

	LOOPFOUND=1
	SWAPPED=

									else
#										echo `basename $arg` and `basename $lib` are in the wrong order
										RHS=`remove $lib $RHS`
										RHS="$arg $RHS"
										arg=$lib
										SWAPPED=
										LOOPFOUND=1
									fi
								fi
							fi
						done
					fi
				done
				if test "$arg" != ""
				then
					LHS="$LHS $arg"
				fi
				;;
			* )
				LHS="$LHS $arg"
				;;
			esac
		fi
	done	

	EXPANDED_CMD="$LHS"
done

# echo EXPANDED_CMD=$EXPANDED_CMD

NEW_CMD=""
RPATH_LINK="$RPATH_INIT"
LASTPATH=`first $RPATH_LINK`

check_path()
{
	f=$1
	shift
	for p in $@
	do
		trylib=$p/$f
		if test -f $trylib
		then
			echo $trylib
			return 0;
		fi
	done

	return 1;
}

DIRLIST="$RPATH_INIT"

for d in $EXPANDED_CMD
do
	case $d in
	*/lib*.a )
		bn=`basename $d`
		sn=`echo $bn | sed s/lib// | sed y/./\ /`
		sn=`first $sn`
		found=`check_path $bn $DIRLIST`
		if test "$found" != "$d"
		then
			LASTPATH=`dirname $d`
			NEW_CMD="$NEW_CMD -L$LASTPATH"
			DIRLIST="$DIRLIST $LASTPATH"
		fi
		NEW_CMD="$NEW_CMD -l$sn"
		;;
	-L* )
		bn=`echo $d | sed s/-L//`
		if not_member $bn $DIRLIST
		then
			LASTPATH="$bn"
			NEW_CMD="$NEW_CMD -L$LASTPATH"
			DIRLIST="$DIRLIST $LASTPATH"
		fi
		;;
	* )
		NEW_CMD="$NEW_CMD $d"
		;;
	esac
done

for d in $COLLECT_D/lib*.a.p $COLLECT_D/*.dep
do
	if test -f "$d"
	then
		rm $d
	fi
done

echo $NEW_CMD

# ls -l $COLLECT_D

$NEW_CMD

res="$?"

echo result was $res

if test -d $COLLECT_D
then
#	ls -l $COLLECT_D
	rm -r $COLLECT_D
else
	echo $COLLECT_D disappeared
fi

exit $res
