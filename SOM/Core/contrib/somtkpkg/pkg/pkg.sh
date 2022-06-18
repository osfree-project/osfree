#!/bin/sh -ex

. ../version.sh

if test "$VERSION" = ""
then
	VERSION=0.0.0.1
fi

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

../pkg/mkfs.sh -d "$OUTDIR_DIST" -t "$INTDIR" -r "$OUTDIR"

../../toolbox/dir2pkg.sh "$INTDIR" "$INTDIR/somtk.comp" "$OUTDIR_DIST" <<EOF
CATEGORY="utility"
NAME="SOMTK IDL compiler"
PKG="RHBsomtkc"
VERSION="$VERSION"
BASEDIR="/opt/somtk"
EOF

../../toolbox/dir2pkg.sh "$INTDIR" "$INTDIR/somtk.rte" "$OUTDIR_DIST" <<EOF
CATEGORY="utility"
NAME="SOMTK runtime"
PKG="RHBsomtkr"
VERSION="$VERSION"
BASEDIR="/opt/somtk"
EOF

../../toolbox/dir2pkg.sh "$INTDIR" "$INTDIR/somtk.ir" "$OUTDIR_DIST" <<EOF
CATEGORY="utility"
NAME="SOMTK interface repository"
PKG="RHBsomtki"
VERSION="$VERSION"
BASEDIR="/opt/somtk"
EOF

../../toolbox/dir2pkg.sh "$INTDIR" "$INTDIR/somtk.util" "$OUTDIR_DIST" <<EOF
CATEGORY="utility"
NAME="SOMTK utility classes"
PKG="RHBsomtku"
VERSION="$VERSION"
BASEDIR="/opt/somtk"
EOF

../../toolbox/dir2pkg.sh "$INTDIR" "$INTDIR/somtk.dsom" "$OUTDIR_DIST" <<EOF
CATEGORY="utility"
NAME="SOMTK CORBA ORB"
PKG="RHBsomtkd"
VERSION="$VERSION"
BASEDIR="/opt/somtk"
EOF

../../toolbox/dir2pkg.sh "$INTDIR" "$INTDIR/somtk.tests" "$OUTDIR_DIST" <<EOF
CATEGORY="utility"
NAME="SOMTK tests"
PKG="RHBsomtkz"
VERSION="$VERSION"
BASEDIR="/opt/somtk"
EOF

echo done
