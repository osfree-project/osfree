#!/bin/sh -ex

rootName()
{
	Y=$1

	while true
	do
		X=`dirname $Y`
		if test "$X" = .
		then
			break
		fi
		Y="$X"
	done
	echo "$Y"
}

find . -name "lib*.a" -type f | grep powerpc-ibm-aix | grep "/default/lib" | sort | while read N
do
	echo "$N"

	NAME=`basename "$N"`

	find . -name "$NAME" -type f | while read M
	do
		case "$M" in
		*/implib/$NAME )
			;;
		*/reflib/$NAME )
			;;
		* )
			if test "$N" != "$M"
			then
				P=`rootName "$M"`
				Q=`rootName "$N"`
				if test "$P" = "$Q"
				then
					echo add "$M"

					TGTDIR=`dirname "$N"`
					TMPDIR=aixmerge.$$
					
					mkdir "$TGTDIR/$TMPDIR"
					cp "$M" "$TGTDIR/$TMPDIR/$NAME"

					(
						cd "$TGTDIR/$TMPDIR"
						ar -X 64 xv $NAME
						rm $NAME
						cp ../$NAME .
						for ext in exp o
						do
							find . -name "*.$ext" | xargs ar -X 64 rv $NAME
						done
						mv $NAME ../$NAME
					)

					rm -rf "$TGTDIR/$TMPDIR"
				fi
			fi
			;;
		esac
	done
done
