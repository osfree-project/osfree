#!/bin/sh

ARCHS=
FILE="$1"

for d in `lipo -info "$FILE"`
do
	case "$d" in
	*: )
		ARCHS=
		;;
	* )
		ARCHS="$ARCHS $d"
		;;
	esac
done

echo $ARCHS
