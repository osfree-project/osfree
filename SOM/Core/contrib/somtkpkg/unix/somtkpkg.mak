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
#  $Id$

include $(MAKEDEFS)

PART=somtkpkg
PART_MAK=../$(PLATFORM_PROTO)/$(PART).mak

all:
	if test -f "$(PART_MAK)"; then \
		$(MAKE) \
			-f "$(PART_MAK)" \
			MAKEDEFS="$(MAKEDEFS)" \
			PLATFORM="$(PLATFORM)" \
			BUILDTYPE="$(BUILDTYPE)" \
			PLATFORM_HOST="$(PLATFORM_HOST)" \
			BUILDTYPE_HOST="$(BUILDTYPE_HOST)" \
			PLATFORM_PROTO="$(PLATFORM_PROTO)" \
			$@; \
		if test "$$?" != "0"; then exit 1; fi \
	fi
	if test "$(SOMTK_IDL)" = ""; \
	then \
		echo "SOMTK_IDL=../../somidl" >>"$(MAKEDEFS)"; \
	fi
	if test "$(SOMTKLIBS)" = ""; \
	then \
		echo SOMTKLIBS=`$(SHLB_REF) somem somem` `$(SHLB_REF) somu somu` `$(SHLB_REF) som som` >>"$(MAKEDEFS)"; \
	fi

clean:
	if test -f "$(PART_MAK)"; then \
		$(MAKE) \
			-f "$(PART_MAK)" \
			MAKEDEFS="$(MAKEDEFS)" \
			PLATFORM="$(PLATFORM)" \
			BUILDTYPE="$(BUILDTYPE)" \
			PLATFORM_HOST="$(PLATFORM_HOST)" \
			BUILDTYPE_HOST="$(BUILDTYPE_HOST)" \
			PLATFORM_PROTO="$(PLATFORM_PROTO)" \
			$@; \
		if test "$$?" != "0"; then exit 1; fi \
	fi
	MAKE="$(MAKE)" ../pkg/clean.sh "$(INTDIR)"

dist:
	if test -x ../pkg/$(PLATFORM_PROTO).sh; then \
		MAKE="$(MAKE)" ../pkg/$(PLATFORM_PROTO).sh -r "$(OUTDIR)" -d "$(OUTDIR_DIST)" -t "$(INTDIR)"; \
	fi

install:

