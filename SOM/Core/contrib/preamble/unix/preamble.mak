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

PART=preamble
PART_MAK=../$(PLATFORM_PROTO)/$(PART).mak

DIRS=	$(OUTDIR) \
		$(OUTDIR_MAN) \
		$(OUTDIR_DIST) \
		$(OUTDIR_TOOLS) \
		$(OUTDIR_TESTS) \
		$(OUTDIR_BIN) \
		$(OUTDIR_SBIN) \
		$(OUTDIR_SHLIB) \
		$(OUTDIR_IMPLIB) \
		$(OUTDIR_REFLIB) \
		$(OUTDIR_ETC) \
		$(OUTDIR)/otherlib \
		$(OUTDIR)/include	\
		$(OUTDIR_FRAMEWORKS)

all:
	for DIR in $(DIRS); \
	do DIRLIST=; \
		while test ! -d "$$DIR"; do DIRLIST="$$DIR $$DIRLIST"; DIR=`dirname $$DIR`; done; \
		for DIR in $$DIRLIST; \
		do echo mkdir "$$DIR"; \
			mkdir "$$DIR"; \
			if test "$$?" != "0"; then exit 1; fi; \
		done; \
	done
	if test -f "$(PART_MAK)"; then \
		$(MAKE) \
			-f "$(PART_MAK)" \
			MAKEDEFS="$(MAKEDEFS)" \
			PLATFORM="$(PLATFORM)" \
			BUILDTYPE="$(BUILDTYPE)" \
			PLATFORM_PROTO="$(PLATFORM_PROTO)" \
			$@; \
		if test "$$?" != "0"; then exit 1; fi \
	fi

clean:
	if test -f "$(PART_MAK)"; then \
		$(MAKE) \
			-f "$(PART_MAK)" \
			MAKEDEFS="$(MAKEDEFS)" \
			PLATFORM="$(PLATFORM)" \
			BUILDTYPE="$(BUILDTYPE)" \
			PLATFORM_PROTO="$(PLATFORM_PROTO)" \
			$@; \
		if test "$$?" != "0"; then exit 1; fi \
	fi
	DIRS_R=; for d in $(DIRS); do DIRS_R="$$DIRS_R $d"; done; \
	for d in $$DIRS_R; do if test -d $$d; then rmdir $$d; \
	if test "$$?" != "0"; then exit 1; fi; fi; done

dist install:

