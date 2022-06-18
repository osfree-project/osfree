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

# needed for GNU ld 2.11 when using shared libraries and -Wl,-z,defs as it 
# complains about unresolved externals from dependent libraries
# this trawls through the libraries then looks up the NEEDED 
# using objdump -p and recursively resolves complete library paths
# once all libraries have been identified they are represented as 
# link line entries with a name derived from using the actual SONAME
# the same library is not listed twice, libpath entries are pruned

# this is normally passed onto toolbox/asneeded.sh itself which does the reverse 
# and tries to link by successively removing libraries from the link line....!

# the upshot is that a shared library (or program) will be linked with
# the correct minimum set of libraries for the environment

# of course, -Wl,--as-needed does all of this...

# echo $0 $@

if test "$LIBPREFIX" = ""
then
	echo $0: LIBPREFIX not defined 1>&2
	exit 1
fi

if test "$LIBSUFFIX" = ""
then
	echo $0: LIBSUFFIX not defined 1>&2
	exit 1
fi

FILELIST=
DONELIST=
TODOLIST=
OTHERARGS=
LASTPATH=
LIBLIST=
NOTFOUND=
PROGRAM_NAME=`basename $0`

rotate()
{
	shift
	echo $@
}

first()
{
	echo $1
}

get_soname()
{
	soname_arg1=`basename $1`
	case $soname_arg1 in
	lib*.a )
		echo $soname_arg1
		;;
	lib*.dylib )
		otool -D $1 | grep -v ":"
		;;
	* )
		objdump -p $1 | grep SONAME | ( read SONAME soname; echo $soname; )
		;;
	esac;
}

get_needed()
{
	needed_arg1=`basename $1`
	case $needed_arg1 in
	lib*.a )
		needed_arg2=`echo $needed_arg1 | sed s/lib// | sed y/./\ /`
		needed_arg2=`first $needed_arg2`
		needed_arg2=`dirname $1`/$needed_arg2.dep
		if test -f "$needed_arg2"
		then
			cat "$needed_arg2"
		fi
		;;
	lib*.dylib )
		otool -L $1 | grep compatibility | ( while read file flags; do echo $file; done ) 
		;;
	* )	
		objdump -p $1 | grep NEEDED | ( while read NEEDED needed; do echo $needed; done )
		;;
	esac;
}

get_rpath()
{
	needed_arg1=`basename $1`
	case $needed_arg1 in
	lib*.a )
		;;
	* )
		objdump -p $1 | grep RPATH | ( while read RPATH rpath; do echo $rpath; done )
		;;
	esac;
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

add_after()
{
	add_posn=$1
	shift
	add_data=$2
	shift
	while d in $@
	do
		if test "$d" = "$add_posn"
		then
			echo $d $add_data
			add_posn=
			add_data=
		else
			echo $d
		fi
	done
}

get_fullpath()
{
	found=
	one=$1
	if test "$one" != ""
	then
		shift
		for d in $@
		do
			if test "$found" = ""
			then
				try_lib="$d/$one"
#				echo trying $try_lib 1>&2
				if test -f "$try_lib"
				then
					found="$try_lib"
#					echo found $try_lib 1>&2
				fi
			fi
		done

		if test "$found" = ""
		then
			if not_member $one $NOTFOUND
			then
				NOTFOUND="$NOTFOUND $one"
#				echo dependant library \"$one\" not found 1>&2
			fi

			return 1;
		fi
	
		echo $found

		return 0;
	fi

	echo get_fullpath: someone looked for empty string 1>&2

	return 1;
}

get_needed_files()
{
	get_needed $1 | while read f; do get_fullpath $f $RPATH_LINK; done
}

get_filestem()
{
	for d in $@
	do
		echo $d | ( IFS='.' read stem ext ; echo $stem; )
	done
}

get_linkargs()
{
	for one in $@
	do
		case $one in
		-l* )
			name=`echo $one | sed s/-l//`
			if not_member $name $LIBLIST
			then
				LIBLIST="$LIBLIST $name"
				echo $one
			fi
			;;
		* )
			dir=`dirname $one`
			bname=`basename $one`
			case "$bname" in
			${LIBPREFIX}*${LIBSUFFIX}* )
				name=`echo $bname | sed s/${LIBPREFIX}//`
				name=`get_filestem $name`

				fullrefname=`get_fullpath ${LIBPREFIX}$name${LIBSUFFIX} $dir $RPATH_LINK`

				if test -f "$fullrefname"
				then
					echo "$PROGRAM_NAME:" $name - $fullrefname 1>&2
				else
					echo "$PROGRAM_NAME:" failed to find $name 1>&2
					name=""
				fi

				;;
			*.dll )
				name=`get_filestem $bname`
				;;
			* )
				name="$bname"
				name=""
				;;
			esac;

			if test "$name" != ""
			then
				if not_member $name $LIBLIST
				then
					LIBLIST="$LIBLIST $name"

					wouldbe=`get_fullpath $bname $RPATH_LINK`
	
					if test "$one" != "$wouldbe"
					then
						if test "$LASTPATH" != "$dir"
						then
							echo "-L$dir -l$name"
							LASTPATH="$dir"
							RPATH_LINK="$RPATH_LINK $LASTPATH"
						else
							echo "-l$name"
						fi
					else
						echo "-l$name"
					fi
				fi
			fi
		esac
	done
	exit 0
}

# TODOLIST can contains complete paths or -llib refs

for arg in $@
do
	orig=$arg

	case $arg in
	 -l* )
		arg=`echo $arg | sed s/-l//`

		arg=`get_fullpath ${LIBPREFIX}$arg${LIBSUFFIX} $RPATH_LINK`

#		echo fullpath of $orig is $arg

		if test "$arg" = ""
		then
			TODOLIST="$TODOLIST $orig"
			arg=""
			orig=""
		fi

		if test "$arg" != ""
		then
			soname=`get_soname $arg`

#			echo soname of $arg is $soname

			if test "$soname" != ""
			then
				case $soname in
				${LIBPREFIX}*${LIBSUFFIX}* )
					fullpath=`get_fullpath $soname $RPATH_LINK`

#					echo fullpath of $soname is $fullpath
					;;
				* )
					echo dont know how to link nicely to $soname

					fullpath=""
					;;
				esac;

				if test "$fullpath" != ""
				then
					if not_member $fullpath $TODOLIST
					then
						TODOLIST="$TODOLIST $fullpath"
					fi
				else
					OTHERARGS="$OTHERARGS $orig"
				fi
			else
				OTHERARGS="$OTHERARGS $orig"
			fi
		else
			OTHERARGS="$OTHERARGS $orig"
		fi
		;;
	-L* )
		arg=`echo $arg | sed s/-L//`

		if test "$LASTPATH" != "$arg"
		then
			OTHERARGS="$OTHERARGS -L$arg"
			RPATH_LINK="$RPATH_LINK $arg"
			LASTPATH="$arg"
		fi

		;;
	-Wl,-rpath-link,* )
		arg=`echo $arg | sed s/-Wl,-rpath-link,//`
		arg=`echo $arg | ( IFS=":" read n; echo $n )`
		RPATH_LINK="$arg $RPATH_LINK"
		OTHERARGS="$OTHERARGS $orig"
		for d in $arg
		do
			OTHERARGS="$OTHERARGS -L$arg"
		done
		;;
	* )
		OTHERARGS="$OTHERARGS $orig"
		;;
	esac;
		
done

echo TODOLIST=$TODOLIST
#echo RPATH_LINK=$RPATH_LINK
#echo FILELIST=$FILELIST

while test "$TODOLIST" != ""
do
#	echo TODOLIST=$TODOLIST

	dll=`first $TODOLIST`
	TODOLIST=`rotate $TODOLIST`

	if test "$dll" != ""
	then
#		mypath=`get_rpath $dll`
#
#		if test "$mypath" != ""
#		then	
#			echo RPATH `basename $dll` $mypath
#		fi

		if not_member $dll $DONELIST
		then
			DONELIST="$DONELIST $dll"
			FILELIST="$FILELIST $dll"

			case "$dll" in
			-l* )
				;;
			* )
				for needed in `get_needed_files $dll`
				do
					if not_member $needed $DONELIST
					then
						if not_member $needed $TODOLIST
						then
							TODOLIST="$needed $TODOLIST"
						fi
					fi
				done
				;;
			esac
		fi
	fi		
done


#echo FILELIST=$FILELIST
#echo OTHERARGS=$OTHERARGS

CLEANARGS=
DIRLIST=

for d in $OTHERARGS
do
	needed=$d
	case $d in
	-L* )
		arg=`echo $d | sed s/-L//`
		if is_member $arg $DIRLIST
		then
			needed=
		else
			DIRLIST="$DIRLIST $arg"
		fi
		LASTPATH="$arg"
		;;
	-l* )
		arg=`echo $d | sed s/-l//`
		if is_member $arg $LIBLIST
		then
			needed=""
		else
			LIBLIST="$LIBLIST $arg"
		fi
		;;
	* )
		;;
	esac

	CLEANARGS="$CLEANARGS $needed"	
done

#echo LIBLIST=$LIBLIST
#echo CLEANARGS=$CLEANARGS

CMDLINE="$CLEANARGS `LASTPATH=$LASTPATH RPATH_LINK=$RPATH_LINK get_linkargs $FILELIST`"

echo $CMDLINE

exec $CMDLINE

