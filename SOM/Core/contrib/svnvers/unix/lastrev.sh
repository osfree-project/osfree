#!/bin/sh
#
#  Copyright 2010, Roger Brown
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
#  $Id$

if test "$SVN" = ""
then
	for d in /usr/local/bin/svn /usr/bin/svn /usr/nekoware/bin/svn
	do
		if test -x "$d"
		then
			SVN="$d"
		fi
	done
fi

if test "$SVN" = ""
then
	SVN=svn
fi

LASTREV=1

while read X
do
	"$SVN" info "$X" | grep "Last Changed Rev:"
done | (
	while read A B C D
	do
		if test "$D" -gt "$LASTREV"
		then
			LASTREV="$D"
		fi
	done
	echo $LASTREV
)
