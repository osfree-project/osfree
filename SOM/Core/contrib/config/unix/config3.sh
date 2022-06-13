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

if test -f "$CONFIG_H"
then 
	HAVE_CONFIG_H="-DHAVE_CONFIG_H"
fi

if test -f "$CONFIG_HPP"
then 
	HAVE_CONFIG_HPP="-DHAVE_CONFIG_HPP"
else
	HAVE_CONFIG_HPP="$HAVE_CONFIG_H"
fi

echo "MAKEDEFS_CFLAGS=$@ $HAVE_CONFIG_H"
echo "MAKEDEFS_CXXFLAGS=$@ $HAVE_CONFIG_HPP"
echo "MAKEDEFS_ORIG=$MAKEDEFS"
echo "include $MAKEDEFS"
