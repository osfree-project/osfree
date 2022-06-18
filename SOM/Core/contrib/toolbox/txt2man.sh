#!/bin/sh

# might be best to use "awf" for IRIX

if test "$NROFF" = ""
then
	NROFF=nroff
fi

case `uname -s` in
IRIX* )
	$NROFF -man | {
		while read N
		do
			if test "$N" != ""
			then
				echo "$N"
				break
			fi
		done

		cat
	}
	;;
* )
	cat
	;;
esac
