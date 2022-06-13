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


first()
{
	echo $1
}

echo "-n $2"

if test -f ../../$1/unix/$2.exp
then
	echo --export ../../$1/unix/$2.exp
fi

if test -f ../../$1/win32/$2.def
then
	VERSION=`../../toolbox/win32def.sh VERSION 0.0 <../../$1/win32/$2.def`
	if test "$VERSION" != ""
    then
		echo " -v "
		echo $VERSION
#        first `echo $VERSION | sed y/./\ /`
    fi
fi
