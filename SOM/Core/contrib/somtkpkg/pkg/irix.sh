#!/bin/sh -ex
#
#  Copyright 2012, Roger Brown
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

. ../version.sh

if test "$VERSION" = ""
then
	VERSION=0.0.0.1
fi

READLINK=/usr/lib/desktop/readlink

FLAG=

INTDIR=.

for d in $@
do
	if test "$FLAG" = ""
	then
		FLAG="$d"
	else
		case "$FLAG" in
		-d )
			OUTDIR_DIST="$d"
			;;
		-r )
			OUTDIR="$d"
			;;
		-t )
			INTDIR="$d"
			;;
		* )
			unknown option $d
			exit 1
		esac
		FLAG=
	fi
done

case "$BUILDTYPE" in
*64 )
	LIBDIRNAME=lib64
	;;
* )
	LIBDIRNAME=lib32
	;;
esac

../pkg/mkfs.sh -d "$OUTDIR_DIST" -t "$INTDIR" -r "$OUTDIR" -l "$LIBDIRNAME"

rm -rf "$INTDIR/somtk.man"
mkdir -p "$INTDIR/somtk.man/usr/relnotes/somtk"
cp ../../somtkman/$PLATFORM/$BUILDTYPE/relnotes/* "$INTDIR/somtk.man/usr/relnotes/somtk"

find "$INTDIR" -print | xargs ls -ld

isManFile()
{
	RETVAL=1

	if test -d "$1"
	then
		case "$1" in
		*/man )
			RETVAL=0
			;;
		* )
			;;
		esac
	fi

	if test "$RETVAL" == 1
	then
		DN=`dirname "$1"`
		if test "$DN" != .
		then
			isManFile "$DN"
			RETVAL=$?
		fi
	fi

	return $RETVAL
}

doSpec()
{
	cd "$INTDIR/$1"
	for d in *
	do
		if test -d "$d"
		then
			find $d -type d | while read N
			do
				if test "$N" != "$d"
				then
					SUBPKG="$2"

					if isManFile "$N"
					then
						SUBPKG=somtk.man.manpages
					fi

					echo d 0555 root sys "$N" "$1/$N" "$SUBPKG"
				fi
			done
		fi
		if test -d "$d"
		then
			find $d | while read N
			do
				if test ! -d "$N"
				then
					SUBPKG="$2"

					if isManFile "$N"
					then
						SUBPKG=somtk.man.manpages
					fi

					if test -h "$N"
					then
						SYMVAL=`$READLINK "$N"`
						echo l 0755 root sys "$N" "$1/$N" "$SUBPKG" symval\($SYMVAL\)
					else
						FLAGS=0444
						if test -x "$N" 
						then
							FLAGS=0555
						fi
						echo f $FLAGS root sys "$N" "$1/$N" "$SUBPKG"
					fi
				fi
			done
		fi
	done
}

(
	(
		doSpec somtk.man somtk.man.relnotes
	)

	for d in rte dsom comp util ir tests
	do
		(
			doSpec somtk.$d somtk.sw.$d
		)
	done
) | sort -k5 >"$INTDIR/somtk.idb"

cat "$INTDIR/somtk.idb"

cat >"$INTDIR/somtk.spec" <<EOF
product somtk
    id "SOM Toolkit"
    image sw
        id "Software"
        version 1
        order 9999
        subsys rte default
            id "Run Time Environment"
            replaces self
            exp somtk.sw.rte
        endsubsys
        subsys comp default
            id "Compiler Software"
            replaces self
            exp somtk.sw.comp
        endsubsys
        subsys ir default
            id "Interface Repository"
            replaces self
            exp somtk.sw.ir
        endsubsys
        subsys util default
            id "Utilities"
            replaces self
            exp somtk.sw.util
        endsubsys
        subsys dsom default
            id "Distributed SOM"
            replaces self
            exp somtk.sw.dsom
        endsubsys
    endimage
    image man
        id "Man Pages"
        version 1
        order 9999
        subsys manpages default
            id "Man Pages"
            replaces self
            exp somtk.man.manpages
        endsubsys
        subsys relnotes default
            id "Release Notes"
            replaces self
            exp somtk.man.relnotes
        endsubsys
    endimage
endproduct
EOF

rm -rf "$INTDIR/dist"

mkdir "$INTDIR/dist"

case "$INTDIR" in
/* )
	HERE="$INTDIR"
	;;
* )
	HERE=`pwd`/$INTDIR
	;;
esac

gendist -rbase "$HERE"				\
		-sbase "$HERE" 				\
		-dist "$HERE/dist"			\
		-spec "$HERE/somtk.spec" 	\
		-idb "$HERE/somtk.idb"		\
		-verbose					\
		"somtk*"

find "$INTDIR/dist" | xargs ls -ld

(
	cd "$INTDIR/dist"
	tar cf - *
) > $INTDIR/somtk.tardist

mv "$INTDIR/somtk.tardist" "$OUTDIR_DIST/somtk-$VERSION.tardist"
					
rm -rf "$INTDIR/dist" "$INTDIR/somtk.idb" "$INTDIR/somtk.spec"
