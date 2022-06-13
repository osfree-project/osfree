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

for d in `echo $SHLB_REF_STRONG | sed y/:/\ /`
do
#	echo looking for strong lib $d $2 1>&2

	if test -f $d/$SHLIBPREFIX$2$SHLIBSUFFIX
	then
		echo -L$d -l$2
		exit 0
	fi
done

for d in `echo $SHLB_REF_WEAK | sed y/:/\ /`
do
#	echo looking for weak lib $d $2 1>&2

	if test -f $d/$SHLIBPREFIX$2$SHLIBSUFFIX
	then
		if test -f $INTDIR/shlb_imp
		then
			echo -L$d -l$2
		else
			echo -impdir $d -implib $2
#			echo $2 >>$INTDIR/_weak.ref
		fi
		exit 0
	fi
done

echo failed to find \"$SHLIBPREFIX$2$SHLIBSUFFIX\" in \"$SHLB_REF_STRONG\" or \"$SHLB_REF_WEAK\" 1>&2
