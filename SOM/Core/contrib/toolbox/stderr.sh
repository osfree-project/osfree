#!/bin/sh
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

LASTFLAG=
OUTPUT=
TMPDIR=/tmp

for ARG in $@
do
	if test "$LASTFLAG" = ""
	then
		case "$ARG" in
		-o )
			LASTFLAG="$ARG"
			;;
		* )
			;;
		esac
	else
		case "$LASTFLAG" in
		-o )
			OUTPUT="$ARG"
			;;
		* )
			;;
		esac

		LASTFLAG=
	fi
done

if test "$OUTPUT" != ""
then
	TMPDIR=`dirname $OUTPUT`
fi

STDERR=$TMPDIR/stderr.$$

$@ 2>$STDERR

RC=$?

if test "$RC" = "0"
then
	( RC=0; while read N; do RC=1; done; exit $RC ) <$STDERR

	RC=$?
fi

cat $STDERR >&2

rm $STDERR

exit $RC
