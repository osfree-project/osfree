#! /bin/sh
#
#	@(#)makever.sh	1.4 1/27/96 16:14:51 /users/sccs/src/win/s.makever.sh
#
#	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
# This file is part of Willows Twin.
# 
# This software is distributed under the terms of the Willows Twin
# Public License as published by Willows Software, Inc.  This software
# may not be redistributed without the express written permission of
# Willows Software, Inc.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# Willows Twin Public License for more details.
# 
# You should have received a copy of the Willows Twin Public License
# along with this software; if not, contact Willows Software, Inc.,
# 10295 Saratoga Ave, Saratoga, CA 95070 USA.
#
# http://www.willows.com
# (408) 777-1820 -- VOICE
# (408) 777-1825 -- FAX
#

if [ ! -f .version ]
then
	echo 0 > .version
fi
cycle=`cat .version`
cycle=`expr $cycle + 1`
if [ $cycle -gt 99 ]
then
	cycle=0
fi
echo $cycle > .version
