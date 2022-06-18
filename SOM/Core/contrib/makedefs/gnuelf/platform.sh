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


PLATFORM_TMP=$INTDIR/platform-$$.d
CC_FLAG_FPIC=-fPIC
CXX_FLAG_FPIC=-fPIC
SHARED_FPIC="-shared $CC_FLAG_FPIC"

PLATFORM_LIB=../../makedefs/$PLATFORM/$BUILDTYPE/lib

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

mkdir $PLATFORM_TMP

if test "$?" != "0"
then
	echo failed to create $PLATFORM_TMP
	exit 1
fi

if test -f /lib/ld.so.1
then
	if test ! -d $PLATFORM_LIB
	then
		mkdir $PLATFORM_LIB

		if test "$?" != "0"
		then
			echo failed to create $PLATFORM_LIB 1>&2
			exit 1
		fi
	fi

	if test ! -f $PLATFORM_LIB/libld.so
	then
		ln -s /lib/ld.so.1 $PLATFORM_LIB/libld.so

		if test "$?" != "0"
		then
			echo failed to create $PLATFORM_LIB/libld.so 1>&2
			exit 1
		fi
	fi
fi


platform_cleanup()
{
	if test -d $PLATFORM_TMP
	then
#		ls -l $PLATFORM_TMP
		rm -r $PLATFORM_TMP
	fi
}

platform_quiet()
{
	if $@ 1>/dev/null 2>&1
	then
		return 0
	else
		return 1
	fi
}

platform_not()
{
	if $@
	then
		return 1
	else
		return 0
	fi
}

platform_cc()
{
	if test "$MAKE" = ""
	then
		MAKE=make
	fi

	if test "$CC" = ""
	then
		cat <<EOF >$PLATFORM_TMP/Makefile
all:
	echo \$(CC)
EOF
#		cat Makefile 1>&2
		CC=`$MAKE -s -f $PLATFORM_TMP/Makefile`
		if test "$CC" = ""
		then
			CC="cc"
		fi
	fi
	
	echo $CC
}

platform_ldd_test()
{
	if test "$PLATFORM" != "$PLATFORM_HOST"
	then
		case "$PLATFORM" in
		*-*-linux-* )
			return 0;
			;;
		esac

		return 1;
	fi


	LD_LIBRARY_PATH=$PLATFORM_TMP
	export LD_LIBRARY_PATH

	if test "$CC" = ""
	then
		CC=`platform_cc`
	fi

	cat <<EOF >$PLATFORM_TMP/one.c
func_one() { }
EOF

	cat <<EOF >$PLATFORM_TMP/two.c
func_two() { func_one(); }
EOF

	cat <<EOF >$PLATFORM_TMP/three.c
func_one() { func_two(); }
EOF

	cat <<EOF >$PLATFORM_TMP/four.c
main(argc,argv) int argc; char **argv; { if (!argc) func_one(); return 0; }
EOF

#	ls -l

	if platform_not platform_quiet $CC $SHARED_FPIC \
			$PLATFORM_TMP/one.c \
			-o $PLATFORM_TMP/libone.so \
			-L$PLATFORM_TMP  $CC_NO_UNDEFS
	then
		return 1
	fi

	if platform_not platform_quiet objdump -p $PLATFORM_TMP/libone.so
	then
		return 1
	fi

	if platform_not platform_quiet $CC $SHARED_FPIC \
			$PLATFORM_TMP/two.c \
			-o $PLATFORM_TMP/libtwo.so \
			-L$PLATFORM_TMP $CC_NO_UNDEFS -lone
	then
		return 1
	fi

	if platform_not platform_quiet objdump -p $PLATFORM_TMP/libtwo.so
	then
		return 1
	fi

	if platform_not platform_quiet $CC $SHARED_FPIC \
			$PLATFORM_TMP/three.c \
			-o $PLATFORM_TMP/libone.so \
			-L$PLATFORM_TMP $CC_NO_UNDEFS -ltwo
	then
		return 1
	fi

	if platform_not platform_quiet objdump -p $PLATFORM_TMP/libone.so
	then
		return 1
	fi
	
	if platform_not platform_quiet $CC $PLATFORM_TMP/four.c -o $PLATFORM_TMP/four -L$PLATFORM_TMP -lone
	then
		return 1
	fi

	if platform_not platform_quiet ldd $PLATFORM_TMP/four
	then
		if test -f ldd.core
		then
			rm ldd.core
		fi

		return 1
	fi

	if ( cd $PLATFORM_TMP; LD_LIBRARY_PATH=. ./four )
	then
		return 0;
	fi

	return 1
}

platform_dl_test()
{
	LD_LIBRARY_PATH=$PLATFORM_TMP
	export LD_LIBRARY_PATH

	if test "$CC" = ""
	then
		CC=`platform_cc`
	fi

	cat <<EOF >$PLATFORM_TMP/main.c
main() { dlopen(0,0); }
EOF
	
	if platform_not platform_quiet $CC $PLATFORM_TMP/main.c -o $PLATFORM_TMP/main $@
	then
		return 1
	fi

	if test "$PLATFORM_HOST" = "$PLATFORM"
	then
		if platform_not platform_quiet ldd $PLATFORM_TMP/main
		then
			return 1
		fi
	fi

#	ls -l

	return 0
}

platform_compile()
{
	if test "$CC" = ""
	then
		CC=`platform_cc`
	fi

	cat >$PLATFORM_TMP/a.c
#	cat $PLATFORM_TMP/a.c

	echo $CC $@

	$CC $PLATFORM_TMP/a.c $@ -o $PLATFORM_TMP/a.out >/dev/null 2>/dev/null

	platform_rc=$?

#	if test -f libld.so
#	then
#		rm libld.so
#	fi

	if test "$platform_rc" = "0"
	then
		if platform_quiet objdump -p $PLATFORM_TMP/a.out
		then
			exit 0
		fi
	fi

	exit 1
}

if ( platform_compile $SHARED_FPIC <<EOF
func() { }
EOF
)
then
	echo built shared lib
	if ( platform_compile $SHARED_FPIC <<EOF
func() { no_such_func(); }
EOF
	)
	then
		echo allowed library with undefined symbols to be built
		for d in "-Wl,-z,defs" \
				"-Wl,--no-undefined" \
				"-Wl,-z,defs -ldl" \
				"-Wl,--no-undefined -ldl" \
				"-Wl,-z,defs -lc" \
				"-Wl,--no-undefined -lld" \
				"-Wl,--no-undefined -L$PLATFORM_LIB -lld" \
				"-Wl,--no-undefined /lib/ld.so.1"
		do
			if test "$CC_NO_UNDEFS" = ""
			then
				if ( platform_compile $SHARED_FPIC $d <<EOF
func() { no_such_func(); }
EOF
				)
				then
					echo error $d still allowed building with undefined symbol
				else
					echo $d correctly stopped building with undefined symbol
					if ( platform_compile $SHARED_FPIC -L$PLATFORM_TMP $d <<EOF
func() {}
EOF
					)
					then
						echo $d successfully built shared lib
						CC_NO_UNDEFS="$d"
					else
						echo error $d failed to build empty lib
					fi
				fi
			fi
		done
	else
		echo failed to build library with undefined symbol
		exit 1
	fi
else
	echo failed to build shared lib
	exit 1
fi

if test "$CC_NO_UNDEFS" = ""
then
	echo CC_NO_UNDEFS is empty
	CC_NO_UNDEFS="-Wl,-u,stat -Wl,-u,fstat"
fi

if test "$CC_NO_UNDEFS" != ""
then
	MAKEDEFS_DEFS="$MAKEDEFS_DEFS CC_NO_UNDEFS"
fi

if ( platform_not platform_ldd_test  )
then
#	echo ldd does not like recursive libraries 1>&2

	SHLB_REF_STRONG=$OUTDIR/lib
	SHLB_REF_WEAK=$OUTDIR/implib
else
#	echo ldd is happy with recursive libraries
	SHLB_REF_STRONG=$OUTDIR/lib:$OUTDIR/implib
	SHLB_REF_WEAK=
fi

if ( platform_not platform_dl_test >/dev/null 2>/dev/null )
then
	if ( platform_not platform_dl_test "-ldl" >/dev/null 2>/dev/null )
	then
		echo failed to link with dlopen
		exit 1
	fi

#	echo dlopen needs "-ldl"

	CONFIG_LIBS="$CONFIG_LIBS -ldl"
fi

if platform_not_member -D_PLATFORM_UNIX_ $PLATFORM_SCFLAGS
then
	PLATFORM_SCFLAGS="$PLATFORM_SCFLAGS -D_PLATFORM_UNIX_"
fi

SHLB_REF_NAME=$OUTDIR/reflib

STRIP=strip

platform_cleanup

MAKEDEFS_EXPORTS="$MAKEDEFS_EXPORTS SHLB_REF_NAME SHLB_REF_STRONG SHLB_REF_WEAK"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_CFLAGS PLATFORM_CXXFLAGS CONFIG_OPTS"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS CC_FLAG_FPIC CXX_FLAG_FPIC CONFIG_LIBS"
MAKEDEFS_DEFS="$MAKEDEFS_DEFS PLATFORM_SCFLAGS STRIP"
