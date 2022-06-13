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


SHLB_REF_STRONG=$OUTDIR/lib:$OUTDIR/implib
SHLB_REF_WEAK=
SHLB_REF_NAME=$OUTDIR/reflib

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS SHLB_REF_STRONG SHLB_REF_WEAK SHLB_REF_NAME"

AOUT_BINARY=$INTDIR/a.out

platform_not_member()
{
    platform_not_member_1=$1
    shift
    for platform_not_member_i in $@
    do
        if test "$platform_not_member_1" = "$platform_not_member_i"
        then
            return 1
        fi
    done
    return 0
}

first()
{
	echo $1
	return 0
}

getlibname()
{
	first `basename $1 | sed s/lib// | sed y/./\ /`
	return 0;
}

echo "int main(int argc,char **argv) { return 0; }" >$INTDIR/try.cpp

if test "$?" != "0"
then
	echo failed to create $INTDIR/try.cpp
	exit 1
fi

$CXX $CXXFLAGS -c $INTDIR/try.cpp -o $INTDIR/try.o 

RC=$?

rm $INTDIR/try.cpp

if test "$RC" != "0"
then
	echo failed to compile with $CXX $CXXFLAGS
	exit 1
fi

$CXX $CXXFLAGS $INTDIR/try.o -o $AOUT_BINARY 

RC=$?

rm $INTDIR/try.o

if test "$RC" != "0"
then
	echo failed to link with $CXX $CXXFLAGS
	exit 1
fi

MYRPATH=
MYLIBS=

for d in `elfdump -L $AOUT_BINARY | grep RPATH`
do
	for e in `echo $d | sed s/:/\ /`
	do
		if test -d $e
		then
			MYRPATH="$MYRPATH $e"
		fi
	done
done

# echo MYRPATH=$MYRPATH

for d in `elfdump -L $AOUT_BINARY | grep NEEDED`
do
	case $d in
	lib*.so* )
		for e in $MYRPATH
		do
			if test "$d" != ""
			then
				if test -f $e/$d
				then
					d=`getlibname $d`
					if platform_not_member $d $MYLIBS
					then
						MYLIBS="$MYLIBS $d"
					fi
				fi
			fi
		done
		;;
	* )
	esac	
done

# echo MYLIBS=$MYLIBS

CXX_RUNTIME=

for d in $MYRPATH
do
	CXX_RUNTIME="$CXX_RUNTIME -L$d"
done

for d in $MYLIBS
do
	CXX_RUNTIME="$CXX_RUNTIME -l$d"
done

MAKEDEFS_DEFS="$MAKEDEFS_DEFS CXX_RUNTIME"

rm $AOUT_BINARY

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

# echo trying to run INTDIR="$INTDIR" CC="$CC" ../../toolbox/trycc.sh

INTDIR="$INTDIR" CC="$CC" ../../toolbox/trycc.sh $CFLAGS $PLATFORM_CFLAGS <<EOF
#ifdef _LP64
int main(int argc,char **argv) { return (argc && argv) ? 0 : 1; }
#else
#error not 64 bit
#endif
EOF
if test "$?" = "0"
then
	DLLPATHENV=LD_LIBRARY64_PATH
else
	DLLPATHENV=LD_LIBRARYN32_PATH
fi

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS DLLPATHENV PLATFORM_CFLAGS PLATFORM_CXXFLAGS"

# echo trying to run INTDIR="$INTDIR" CC="$CC" ../../toolbox/trycc.sh

INTDIR="$INTDIR" CC="$CC" ../../toolbox/trycc.sh $CFLAGS <<EOF
#ifdef __GNUC__
int main(int argc,char **argv) { return (argc && argv) ? 0 : 1; }
#else
#error not GCC bit
#endif
EOF
if test "$?" = "0"
then
	if platform_not_member -Wall $CFLAGS $PLATFORM_CFLAGS
	then
		PLATFORM_CFLAGS="$PLATFORM_CFLAGS -Wall"
	fi
	if platform_not_member -Werror $CFLAGS $PLATFORM_CFLAGS
	then
		PLATFORM_CFLAGS="$PLATFORM_CFLAGS -Werror"
	fi
else
	if platform_not_member -fullwarn $CFLAGS $PLATFORM_CFLAGS
	then
		PLATFORM_CFLAGS="$PLATFORM_CFLAGS -fullwarn -diag_error 3322,3604,1515,1196,1164,1498,1035,3584"
	fi
fi

# echo trying to run INTDIR="$INTDIR" CXX="$CXX" ../../toolbox/trycxx.sh

INTDIR="$INTDIR" CXX="$CXX" ../../toolbox/trycxx.sh $CXXFLAGS $PLATFORM_CXXFLAGS <<EOF
#ifdef __GNUC__
int main(int argc,char **argv) { return (argc && argv) ? 0 : 1; }
#else
#error not GCC bit
#endif
EOF
if test "$?" = "0"
then
	if platform_not_member -Wall $CXXFLAGS $PLATFORM_CXXFLAGS
	then
		PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -Wall"
	fi
	if platform_not_member -Werror $CXXFLAGS $PLATFORM_CXXFLAGS
	then
		PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -Werror"
	fi
else
	if platform_not_member -fullwarn $CXXFLAGS $PLATFORM_CXXFLAGS
	then
		PLATFORM_CXXFLAGS="$PLATFORM_CXXFLAGS -fullwarn -diag_error 3322,3604,1515,1196,1164,1498,1035,3584"
	fi
fi

# echo reached the end of makedefs/irix/platform.sh

MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_SCFLAGS"
