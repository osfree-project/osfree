#!/bin/sh -ex

for d in $@
do
	if test -f "$d"
	then
		BN=`basename "$d"`
		(
			cd `dirname "$d"`

			case `uname -s` in
			IRIX* )
				rm -rf "$BN".z
				pack -f "$BN"
				;;
			Linux )
				rm -rf "$BN".gz
				gzip -f "$BN"
				;;
			* )
				;;
			esac
		)
	fi
done

