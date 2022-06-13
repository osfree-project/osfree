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

last()
{
    LAST_P=
    for LAST_V in $@
    do
        LAST_P="$LAST_V"
    done
    echo "$LAST_P"
}

first()
{
	echo "$1"
}

second()
{
	echo "$2"
}

mkdir -p "$INTDIR"

ROOTDIR="$INTDIR/root"

rm -rf "$ROOTDIR"

mkdir "$ROOTDIR"

mkdir -p "$ROOTDIR/Library/Frameworks/SOMTK.framework" "$ROOTDIR/usr/bin"

(
	cd "$OUTDIR/frameworks/SOMTK.framework"
	tar cf - .
) | (
	cd "$ROOTDIR/Library/Frameworks/SOMTK.framework"
	tar xf -
	find . -type l | xargs rm
	find . -type d -name Versions | while read N
	do
		(
				cd "$N"
 
 				LAST_D=`echo * | sort`
 				LAST_D=`last $LAST_D`

 				ln -s "$LAST_D" Current
 		)
 	done
	mkdir Versions/Current/Headers
	mkdir Versions/Current/Resources
	ln -s Versions/Current/Headers .
	ln -s Versions/Current/Resources .
	ln -s Versions/Current/SOMTK .
	ln -s Versions/Current/bin .

	(
		cd Versions/Current/Frameworks

		find . -type f | while read N
		do
			M=`basename "$N"`
			if test -f "$M.framework/Versions/Current/$M"
			then
				(
					cd "$M.framework"
					ln -s "Versions/Current/$M" .
				)
			fi
		done
	)

	for d in `echo Versions/Current/Frameworks/*.framework/*`
	do
		BN=`basename $d`
		DN=`dirname $d`
		case "$BN" in
		SOM* )
			mkdir -p "$DN/Versions/Current/Headers"
			(
				cd "$DN"
				ln -s "Versions/Current/Headers"
			)
			;;
		* )
			;;
		esac
	done

)

BUILDLIST=`echo *`
BUILDLIST_COUNT=`echo $BUILDLIST | wc -w`

if test "$BUILDLIST_COUNT" -gt 1
then
	echo BUILDLIST=$BUILDLIST

	(
		cd "$ROOTDIR/Library/Frameworks"
		find . -type f | while read N
		do
			if otool -L "$N" >/dev/null
			then
				echo "$N"
			fi
		done
	) | while read N
	do
		BUILDLIST_FILE=
		BUILDLIST_ORIG="$ROOTDIR/Library/Frameworks/$N"

		for a in $BUILDLIST
		do
			BUILDLIST_ARCH="$a/frameworks/$N"
			if test -f "$BUILDLIST_ARCH"
			then
				BUILDLIST_FILE="$BUILDLIST_FILE $BUILDLIST_ARCH"
			fi
		done	

		if test "$BUILDLIST_FILE" != ""
		then
			lipo $BUILDLIST_FILE -create -output "$BUILDLIST_ORIG.fat"
			mv "$BUILDLIST_ORIG.fat" "$BUILDLIST_ORIG"
			lipo -info "$BUILDLIST_ORIG"
		fi	
	done
fi

for d in sc
do
	cp "$OUTDIR/bin/$d" "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/bin"
done

(
	cd "$ROOTDIR/usr/bin"

	for d in ../../Library/Frameworks/SOMTK.framework/Versions/Current/bin/*
	do
		if test -x "$d"
		then
			if test -f "$d"
			then
				ln -s "$d" .
			fi
		fi
	done
)

while read P F
do
	for d in $F
	do
		if test -f ../../somidl/$d.idl
		then
			cp ../../somidl/$d.idl "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/$P.framework/Headers"
		fi

		for e in api h xh
		do
			if test -f ../../somidl/$PLATFORM/$d.$e
			then
				cp ../../somidl/$PLATFORM/$d.$e "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/$P.framework/Headers"
			fi
		done
	done
done <<EOF
SOM somcls somobj somcm
SOMD somdobj somdcprx somdom orb somdom request cntxt nvlist somdtype somdserv stexcep unotypes somoa principl om impldef implrep boa servmgr somdmprx
SOMIR repostry intfacdf containd containr operatdf moduledf attribdf typedef paramdef excptdef constdef 
SOMNMF naming biter xnaming xnamingf lname lnamec
SOMESTRM omgestio somestio
SOMREF somref
SOMU snglicls sombacls somida somproxy somsid
SOMU2 somtdm
SOMABS1 omgidobj
SOMOS somos somap
SOMCSLIB xmscssae
EOF

for d in h xh
do
	cp ../../somkpub/include/*.$d "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOM.framework/Headers"
	cp ../../somtk/include/somd*.$d "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOMD.framework/Headers"
	cp ../../somtk/include/somtc*.$d "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOMTC.framework/Headers"
	cp ../../somtk/include/somir*.$d "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOMIR.framework/Headers"
	cp ../../somtk/include/somos*.$d "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOMOS.framework/Headers"
	cp ../../somtk/include/somnm*.$d "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOMNMF.framework/Headers"
done

cp ../../somtk/include/somthrd.h "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOMU.framework/Headers"
cp ../../somtk/include/somuutil.h "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Frameworks/SOMU.framework/Headers"

for d in h xh idl api
do
	(
		cd "$ROOTDIR/Library/Frameworks/SOMTK.framework/Versions/Current/Headers"

		pwd
		ls -l ../../..
	
#		find ../../../Versions/Current/Frameworks/SOM*.framework/Versions/Current/Headers -type f -name "*.$d" | while read N
		find ../Frameworks/SOM*.framework/Versions/Current/Headers -type f -name "*.$d" | while read N
		do
			ln -s "$N"
		done
	)
done

cp "$OUTDIR"/etc/somnew.ir "$ROOTDIR/Library/Frameworks/SOMTK.framework/Resources/"
cp "$OUTDIR"/etc/somenv.ini "$ROOTDIR/Library/Frameworks/SOMTK.framework/Resources/"

VERSION_PARTS=`echo $VERSION | sed y/./\ /`
VERS_1ST=`first $VERSION_PARTS`
VERS_2ND=`second $VERSION_PARTS`
SHORTVERSION="$VERS_1ST.$VERS_2ND"

cat >"$ROOTDIR/Library/Frameworks/SOMTK.framework/Resources/version.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>BuildVersion</key>
	<string>$VERS</string>
	<key>CFBundleShortVersionString</key>
	<string>$SHORTVERSION</string>
	<key>CFBundleVersion</key>
	<string>$VERS</string>
	<key>ProjectName</key>
	<string>SOMTK</string>
	<key>SourceVersion</key>
	<string>$VERS</string>
</dict>
</plist>
EOF

cat >"$ROOTDIR/Library/Frameworks/SOMTK.framework/Resources/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>English</string>
	<key>CFBundleExecutable</key>
	<string>SOMTK</string>
	<key>CFBundleIdentifier</key>
	<string>localhost.somtk</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>SOMTK</string>
	<key>CFBundlePackageType</key>
	<string>FMWK</string>
	<key>CFBundleShortVersionString</key>
	<string>$SHORTVERSION</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleVersion</key>
	<string>$VERS</string>
</dict>
</plist>
EOF

(
	cd "$ROOTDIR"
	find . | xargs ls -ld

)

case "$MACOSX_DEPLOYMENT_TARGET" in
10.2 )
		(
			cd "$ROOTDIR"
			tar cf - *
		) | (
			gzip >"$OUTDIR_DIST/somtk.tar.gz"
		)
	;;
10.3 | 10.4 | 10.5 )
	if ../../toolbox/pkgtool.sh gtar
	then
		GTAR=`../../toolbox/pkgtool.sh gtar`

		if test "$GTAR" != ""
		then
			(
				cd "$ROOTDIR"
				$GTAR --owner=0 --group=0 --mode=-w --create --file - *
			) | (
				gzip >"$OUTDIR_DIST/somtk.tar.gz"
			)
		fi
	fi
	;;
* )
	productbuild --component "$ROOTDIR/Library/Frameworks/SOMTK.framework" /Library/Frameworks "$OUTDIR_DIST/SOMTK.pkg" 
	mv "$OUTDIR_DIST/SOMTK.pkg" "$OUTDIR_DIST/SOMTK-$MACOSX_DEPLOYMENT_TARGET-$VERSION.pkg"
	;;
esac

echo done
