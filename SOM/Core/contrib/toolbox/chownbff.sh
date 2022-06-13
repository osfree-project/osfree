#!/bin/sh -e
#
#  Copyright 2011, Roger Brown
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

WHOAMI=`id -u`:`id -g`

stripdot()
{
        while read N
        do
                case "$N" in
                . | .. )
                        ;;
                * )
                        echo "$N"
                        ;;
                esac
        done
}

for d in $@
do
	TMPDIR=/tmp/chownbff-$$

	mkdir "$TMPDIR"

	(
		cd "$TMPDIR"
		restore -x -f -
	) <"$d"

	chown -R "$WHOAMI" "$TMPDIR"

	find "$TMPDIR" | while read N
	do
		touch "$N"
	done
	
	(
		cd "$TMPDIR"
		if test -f "./lpp_name"; then echo "./lpp_name"; fi
		find . | stripdot | grep -v "./lpp_name"
	) | (
		cd "$TMPDIR"
		backup -iqf -
	) >"$d"

	restore -T -vf - <"$d"

	rm -r "$TMPDIR"
done
