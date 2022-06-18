#!/bin/sh -ex

INTDIR="$1"

for d in somtk.ir somtk.rte somtk.dsom somtk.comp somtk.util somtk.tests somtk.man root
do
	if test -d "$INTDIR/$d"
	then
		find "$INTDIR/$d" -type d -print | while read N
		do
			chmod +w "$N"
		done

		find "$INTDIR/$d" -type f -print | while read N
		do
			chmod +w "$N"
		done

		rm -rf "$INTDIR/$d"
	fi
done
