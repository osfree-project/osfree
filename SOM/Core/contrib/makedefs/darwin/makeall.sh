#!/bin/sh -ex
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

archlist()
{
	ARCH_LIST=

	for d in `lipo -info "$1"`
	do
		case "$d" in
		*: )
			ARCH_LIST=
			;;
		* )
			ARCH_LIST="$ARCH_LIST $d"
			;;
		esac
	done
	echo $ARCH_LIST
}

for d in /Developer/SDKs /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
do
	if test -d "$d"
	then
		echo SDK dir $d

		for e in $d/*
		do
			if test -d "$e"
			then
				OS_VER=
				case `basename "$e"` in
				MacOSX10.3.9.sdk )
					OS_VER=7
					MACOSX_DEPLOYMENT_TARGET=10.3
					;;
				MacOSX10.4u.sdk )
					OS_VER=8
					MACOSX_DEPLOYMENT_TARGET=10.4
					;;
				MacOSX10.5.sdk )
					OS_VER=9
					MACOSX_DEPLOYMENT_TARGET=10.5
					;;
				MacOSX10.6.sdk )
					OS_VER=10
					MACOSX_DEPLOYMENT_TARGET=10.6
					;;
				MacOSX10.7.sdk )
					OS_VER=11
					MACOSX_DEPLOYMENT_TARGET=10.7
					;;
				MacOSX10.8.sdk )
					OS_VER=12
					MACOSX_DEPLOYMENT_TARGET=10.8
					;;
				esac

				if test "$OS_VER" != ""
				then
					if test -f "$e/usr/lib/libSystem.dylib"
					then
						for f in `archlist "$e/usr/lib/libSystem.dylib"`
						do
							PLATFORM="all-apple-darwin$OS_VER"
							BUILDTYPE="$f"
							echo "$PLATFORM/$BUILDTYPE" $e

							ARCH="-arch $f -isysroot $e"

							mkdir -p "makedefs/$PLATFORM/$BUILDTYPE"

							TESTAPP=makedefs/$PLATFORM/$BUILDTYPE/tmp-$$

							echo "int main(int a,char **v) { return 0; }" >"$TESTAPP.c"

							export MACOSX_DEPLOYMENT_TARGET

							if cc $ARCH "$TESTAPP.c" -o "$TESTAPP.out"
							then
								rm "$TESTAPP.out"
								rm "$TESTAPP.c"
							else
								OS_VER=
								rm "$TESTAPP.c"
								rmdir "makedefs/$PLATFORM/$BUILDTYPE"
							fi

							if test "$OS_VER" != ""
							then
								JAVA_INCLUDE="-I/$e/System/Library/Frameworks/JavaVM.framework/Headers" \
								JAVA_BIN="/usr/bin" \
								CFLAGS="$ARCH -Wno-deprecated-declarations -DDLOPEN_NO_WARN" \
								CXXFLAGS="$ARCH" \
								MACOSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET \
								PLATFORM="$PLATFORM" \
								BUILDTYPE="$BUILDTYPE" \
								BUILDTYPE_HOST="default" \
								make $@ 
							fi
						done
					fi
				fi
			fi
		done
	fi
done
