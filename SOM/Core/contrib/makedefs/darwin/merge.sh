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


PLATFORM=i386-apple-darwin
PLATFORM_HOST=`develop/config/unix/config.guess`

for group in products 
do
	for subdir in bin lib tools tests
	do
		FAT=develop/$group/fat-apple-darwin

		for file in develop/$group/$PLATFORM_HOST/$subdir/*
		do
			b=`basename $file`
		
			if test -f develop/$group/$PLATFORM/$subdir/$b
			then
				if test -L develop/$group/$PLATFORM/$subdir/$b
				then
					echo $b is a link
				else
					mkdir -p $FAT/$subdir

					lipo develop/$group/$PLATFORM/$subdir/$b \
						develop/$group/$PLATFORM_HOST/$subdir/$b \
						-create -output $FAT/$subdir/$b
				fi
			fi
		done
	done
done
