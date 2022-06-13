#!/bin/sh -ex

getField()
{
	(
		IFS_ORIG="$IFS"
		IFS="="
		while read N M
		do
			(
				IFS="$IFS_ORIG"

				if test "$N" = "$2"
				then
					sh -c "echo $M"
					break
				fi	
			)
		done <"$1"
	)
}

INTDIR=$1
SRCDIR=$2
PKGDIR=$3

ARCH_P=`uname -p`
ARCH_M=`uname -m`

test -d "$INTDIR"
test -d "$SRCDIR"
test -d "$PKGDIR"

PKGINFO="$INTDIR/pkginfo"

cat >"$PKGINFO"

PKG=`getField "$PKGINFO" PKG`
BASEDIR=`getField "$PKGINFO" BASEDIR`
VERSION=`getField "$PKGINFO" VERSION`
ARCH=`getField "$PKGINFO" ARCH`

test "$PKG" != ""
test "$BASEDIR" != ""
test "$VERSION" != ""

if test "$ARCH" = ""
then
	ARCH=$ARCH_P.$ARCH_M
	echo "ARCH=\"$ARCH\"" >>"$PKGINFO"
fi

PKGFILE="$PKGDIR/$PKG-$VERSION-$ARCH_P.pkg"

PKGPROTO="$INTDIR/prototype.$PKG"

mapDirs()
{
	(
		cd "$SRCDIR/$BASEDIR"
		ls
	) | while read N
	do
		if test -d "$SRCDIR/$BASEDIR/$N"
		then
			echo "$SRCDIR/$BASEDIR/$N=$N"
		else
			if test -H "$SRCDIR/$BASEDIR/$N"
			then
				echo "$SRCDIR/$BASEDIR/$N"
			else
				echo "$SRCDIR/$BASEDIR/$N=$N"
			fi
		fi
	done
}

setOwner()
{
	while read A B C D E F
	do
		case "$A" in
		f )
			case "$D" in
			*5* )
				echo "$A" "$B" "$C" "$D" root bin
				;;
			* )
				echo "$A" "$B" "$C" "$D" root root
				;;
			esac
			;;
		s )
			echo "$A" "$B" "$C"
			;;
		d )
			echo "$A" "$B" "$C" "$D"  root root
			;;
		* )
			;;
		esac
	done
}

addInfo()
{
	echo "i" "pkginfo"
	cat
}

pkgproto `mapDirs` | setOwner | addInfo > "$PKGPROTO"

PKGTMP="$INTDIR/image.$PKG"

rm -rf "$PKGTMP"

mkdir "$PKGTMP"

cat "$PKGPROTO"

pkgmk -o -r . -d "$PKGTMP" -f "$PKGPROTO" "$PKG"

cat </dev/null >"$PKGFILE"

pkgtrans -s "$PKGTMP" "$PKGFILE" "$PKG"

rm -rf "$PKGTMP" "$PKGPROTO" "$PKGINFO"
