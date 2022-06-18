#!/bin/sh -ex

if test "$GTAR" = ""
then
	if ../../toolbox/pkgtool.sh gtar
	then
		GTAR=`../../toolbox/pkgtool.sh gtar`
	fi

	if test "$GTAR" = ""
	then
		exit 0
	fi
fi

getField()
{
	while read N M
	do
		if test "$N" = "$2"
		then
			echo "$M"
			break
		fi	
	done <"$1"
}

INTDIR=$1
BASEDIR=$2
DEBDIR=$3

test -d "$INTDIR"
test -d "$BASEDIR"
test -d "$DEBDIR"

case "$BASEDIR" in
/* )
	;;
* )
	BASEDIR=`pwd`/"$BASEDIR"
	;;
esac

CONTROLFILE="$INTDIR/control"

cat >"$CONTROLFILE"

APPNAME=`getField "$CONTROLFILE" Package:`
VERSION=`getField "$CONTROLFILE" Version:`
ARCH=`getField "$CONTROLFILE" Architecture:`
PACKAGE_NAME="$APPNAME"_"$VERSION"_"$ARCH".deb

SCRIPTLIST="./preinst ./postinst ./postrm ./prerm"

for d in $SCRIPTLIST
do
	if test ! -f "$INTDIR/$d"
	then
		cat >"$INTDIR/$d" <<EOF
#!/bin/sh
EOF
		chmod +x "$INTDIR/$d"
	fi
done

(
	cd "$BASEDIR"

	$GTAR --owner=0 --group=0 --create --file - ./*	
) >"$INTDIR/data.tar"

(
	cd "$INTDIR"
	rm -rf "$PACKAGE_NAME"
	$GTAR --owner=0 --group=0 --create --file control.tar ./control $SCRIPTLIST	
	echo "2.0" >debian-binary
	gzip data.tar
	gzip control.tar
	ar r "$PACKAGE_NAME" debian-binary control.tar.gz data.tar.gz
	rm -rf data.tar.gz control.tar.gz debian-binary
)

rm -rf "$CONTROLFILE"

for d in $SCRIPTLIST
do
	rm "$INTDIR/$d"
done

mv "$INTDIR/$PACKAGE_NAME" "$DEBDIR"
