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
#  $Id$

include $(MAKEDEFS)

TOOLS=	$(SHLB_EXP)				\
		$(SHLB_MAP)				\
		$(SHLB_REF)				\
		$(SHLB_ENT)				\
		$(SHLB_DLO)				\
		$(ARLB_REF)				\
		../../makedefs/$(PLATFORM_PROTO)/linktool.sh	\
		../collect.sh	\
		../makeliba.sh	\
		../asneeded.sh	\
		../ccirix.sh	\
		../ldirix.sh	\
		../ldhppa.sh	\
		../ldxcoff.sh	\
		../linktool.sh	\
		../adlib.sh		\
		../trycc.sh		\
		../trycxx.sh	\
		../win32def.sh	\
		../findlibs.sh	\
		../repath.sh	\
		../depends.sh	\
		../exectool.sh	\
		../exectest.sh	\
		../execconf.sh	\
		../stderr.sh

all:
	if test "$(SHLIBPREFIX)" = ""; then exit 1; fi
	if test "$(SHLIBSUFFIX)" = ""; then exit 1; fi
	if test "$(DLLSUFFIX)" = ""; then exit 1; fi
	for d in $(TOOLS); \
	do if test -f $$d; \
		then if test ! -x $$d; \
			then chmod u+x $$d; \
			fi; \
		fi; \
	done

clean:


dist install:

