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


case "$1" in
vote )
	case "$2" in
		*-apple-darwin*  ) 
			echo 100
			;;
		*-darwin*  ) 
			echo 75
			;;
		* )
			exit 1
			;;
	esac
	;;
options )
	echo platform
	;;
name )
	echo darwin
	;;
* )
	exit 1
	;;
esac
