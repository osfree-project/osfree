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

CONFIG_C=$INTDIR/platform.c
CONFIG_OBJ=$INTDIR/platform.o
CONFIG_CPP=$INTDIR/platform.cpp
CONFIG_EXE=$INTDIR/platform.exe

echo INTDIR=$INTDIR
echo PWD=`pwd`

SHLB_REF_STRONG=$OUTDIR/lib:$OUTDIR/implib

#LINKAPP_MAGIC="-Wl,-blibpath:/usr/lib/threads:/usr/lib:/lib -Wl,-brtl"
#LINKAPP_MAGIC="-Wl,-blibpath:/usr/lib:/lib -Wl,-brtl"
LIBPATH_IGNORE="/usr/lib /lib"

platform_not_member()
{
    not_member_d=$1
	if test "$2"
	then
	    shift
    	for not_member_i in $@
    	do
        	if test "$not_member_i" = "$not_member_d"
        	then
            	return 1
	        fi
    	done
	fi
    return 0;
}

platform_has_member()
{
    not_member_d=$1
	if test "$2"
	then
	    shift
    	for not_member_i in $@
    	do
        	if test "$not_member_i" = "$not_member_d"
        	then
            	return 0
	        fi
    	done
	fi
    return 1;
}

platform_first()
{
	echo "$1"
}

platform_find_lib()
{
	lib=$1

	if test -f "$lib"
	then
		nm $AIX_X_CODE -e "$lib" | grep T >/dev/null
		if test "$?" = "0"
		then
			echo "$lib"
			return 0;
		fi
	fi

	shift

	for d in $@
	do
		if test -d "$d"
		then
			if test -f "$d/$lib"
			then
				nm $AIX_X_CODE -e "$d/$lib" | grep T >/dev/null
				if test "$?" = "0"
				then
					echo "$d/$lib"
					return 0;
				fi
			fi
		fi
	done

	return 1;
}

platform_libs()
{
	LIBS_SOFAR=
	DIRS_SOFAR=

	for d in `echo $LIBPATH | sed y/:/\ /`
	do
		if test "$d" != ""
		then
			if platform_not_member "$d" $DIRS_SOFAR
			then
				echo "-L$d"
				DIRS_SOFAR="$DIRS_SOFAR $d"
			fi
		fi	
	done

	$@ | (
		FLAG=0
		while read INDEX DLLPATH BASE MEMBER
		do
			if test "$INDEX $DLLPATH" = "INDEX PATH"
			then
				FLAG=1
			else
				case "$FLAG" in
				0 )
					;;
				1 | 2)
					FLAG=2
					case "$DLLPATH" in
					lib*.a | */lib*.a )
						BASE="$DLLPATH"
						;;
					* )
						for DIR in `echo $DLLPATH | sed y/:/\ /`
						do
							if platform_not_member "$DIR" $DIRS_SOFAR
							then
								echo "-L$DIR"
								DIRS_SOFAR="$DIRS_SOFAR $DIR"
							fi
						done
						;;
					esac

					if test "$BASE" != ""
					then
						DLLPATH=`dirname $BASE`

						case "$DLLPATH" in
						. )
							;;
						* )
							if test "$DLLPATH" != ""
							then
								if platform_not_member "$DLLPATH" $DIRS_SOFAR
								then
									echo "-L$DLLPATH"
									DIRS_SOFAR="$DIRS_SOFAR $DLLPATH"
								fi
							fi
							;;
						esac
	
						BASE=`platform_find_lib $BASE $DIRS_SOFAR $LIBPATH_IGNORE`

						if test "$BASE" != ""
						then
							case `basename $BASE` in
							lib*.a )
								if platform_not_member $BASE $LIBS_SOFAR $LIBPATH_IGNORE
								then
									LIBS_SOFAR="$LIBS_SOFAR $BASE"
								fi	
								;;
							* )
								;;
							esac
						fi
					fi
					;;
				* )
					;;
				esac
			fi
		done

		for d in $LIBS_SOFAR
		do
			echo $d

#			if test libstdc++.a = `basename $d`
#			then
#				echo `dirname $d`/libsupc++.a
#			fi
		done
	)
}

if platform_not_member -D_REENTRANT $CFLAGS $PLATFORM_CFLAGS
then
	PLATFORM_CFLAGS="$PLATFORM_CFLAGS -D_REENTRANT"
fi

if platform_not_member -D_REENTRANT $CXXFLAGS $PLATFORM_CXXFLAGS
then
	PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -D_REENTRANT"
fi

if platform_not_member -D_PLATFORM_UNIX_ $PLATFORM_SCFLAGS
then
	PLATFORM_SCFLAGS="$PLATFORM_SCFLAGS -D_PLATFORM_UNIX_"
fi

cat >$CONFIG_C <<EOF
#include <stdio.h>
#ifdef TEST_64
int my_x;
#	ifdef _LP64
#	else
#		error 32 bit
#	endif
#elif TEST_GCC
#	ifdef __GNUC__
#	else
#		error
#	endif
#else
#ifdef __cplusplus
class f { int x; };
#endif
int main(int argc,char **argv) 
{ 
#ifdef __cplusplus
	f *g=new f;
	if (!g) return 1;
#endif

	return 0; 
}
#endif
EOF

cp $CONFIG_C $CONFIG_CPP

$CC $CFLAGS -c $CONFIG_C -DTEST_GCC -o $CONFIG_OBJ 1>/dev/null 2>&1
CC_RES=$?

if test "$CC_RES" = "0"
then
	CC_FLAG_FPIC=""
	CXX_FLAG_FPIC=""
	COMMON_OPTS="-Wall -Werror"
	PLATFORM_CFLAGS="$PLATFORM_CFLAGS $COMMON_OPTS"
	PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS $COMMON_OPTS"
	PLATFORM_LIBS="-lpthreads"
	CONFIG_CFLAGS=""
	CONFIG_CXXFLAGS=""
else
	CC_FLAG_FPIC="-qpic=small"
	CXX_FLAG_FPIC="-qpic=small"
	COMMON_OPTS="-qhalt=i -qinfo=ret:pro -qsuppress=1501-220"
	PLATFORM_CFLAGS="$PLATFORM_CFLAGS -qproto $COMMON_OPTS"
	PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS $COMMON_OPTS"
	CONFIG_CFLAGS=""
	CONFIG_CXXFLAGS=""
fi

$CC $CFLAGS -c $CONFIG_C -DTEST_64 -o $CONFIG_OBJ 1>/dev/null 2>&1
CC_RES=$?

if test "$CC_RES" = "0"
then
	AIX_X_CODE="-X 64"
	AIX_B_CODE="-b64"
else
	AIX_X_CODE="-X 32"
	AIX_B_CODE="-b32"
fi

echo AIX_X_CODE=$AIX_X_CODE

$CC $CFLAGS $PLATFORM_CFLAGS -c $CONFIG_C -o $CONFIG_OBJ

CC_RES=$?

if test "$CC_RES" != "0"
then
	exit "$CC_RES"
fi

$CC $CFLAGS $PLATFORM_CFLAGS $LINKAPP_MAGIC $CONFIG_OBJ $PLATFORM_LIBS -o $CONFIG_EXE

CC_RES=$?

if test "$CC_RES" != "0"
then
	exit "$CC_RES"
fi

STDLIB=`platform_libs dump -H $AIX_X_CODE $CONFIG_EXE`

$CXX $CXXFLAGS $PLATFORM_CXXFLAGS -c $CONFIG_CPP -o $CONFIG_OBJ

CC_RES=$?

if test "$CC_RES" != "0"
then
	exit "$CC_RES"
fi

$CXX $CXXFLAGS $PLATFORM_CXXFLAGS $LINKAPP_MAGIC $CONFIG_OBJ $PLATFORM_LIBS -o $CONFIG_EXE

CC_RES=$?

if test "$CC_RES" != "0"
then
	exit "$CC_RES"
fi

STDLIBXX=`platform_libs dump -H $AIX_X_CODE $CONFIG_EXE`

for platform_d in $CONFIG_C $CONFIG_OBJ $CONFIG_CPP $CONFIG_EXE
do
	if test -f $platform_d
	then
		rm $platform_d
	fi
done

LD_SHARED="$LD_SHARED $LDFLAGS"
LDXX_SHARED="$LDXX_SHARED $LDFLAGS"
PLATFORM_ARFLAGS="$PLATFORM_ARFLAGS $AIX_X_CODE"
PLATFORM_LDFLAGS="$PLATFORM_LDFLAGS $AIX_B_CODE"

#  -bnoquiet
#LINKDLL_TAIL=`echo $LINKDLL_TAIL`

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS SHLB_REF_STRONG SHLB_REF_WEAK"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS PLATFORM_CFLAGS PLATFORM_CXXFLAGS"
MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS PLATFORM_ARFLAGS PLATFORM_LDFLAGS"

MAKEDEFS_DEFS="$MAKEDEFS_DEFS CONFIG_CFLAGS CONFIG_CXXFLAGS"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS CC_FLAG_FPIC CXX_FLAG_FPIC PLATFORM_SCFLAGS"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS STDLIB STDLIBXX LINKDLL_TAIL"
