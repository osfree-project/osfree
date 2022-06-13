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

PREAMBLE=preamble-darwin
FRAMEWORK_LIST=AppleTalk PCSC CoreFoundation IOKit
INCL_PLAT=../../include/$(PLATFORM)/$(BUILDTYPE)
LIB_PLAT=$(OUTDIR)/otherlib
FRAMEWORKS_ROOT=$(PLATFORM_ISYSROOT)/System/Library/Frameworks

all: $(INCL_PLAT) $(LIB_PLAT) $(PREAMBLE)

$(INCL_PLAT) $(LIB_PLAT):
	DIRLIST=; \
	DIR=$@; \
	while test ! -d "$$DIR"; do DIRLIST="$$DIR $$DIRLIST"; DIR=`dirname $$DIR`; done; \
	for DIR in $$DIRLIST; do mkdir "$$DIR"; \
	if test "$$?" != "0"; then exit 1; fi; done

$(PREAMBLE):
	for d in $(FRAMEWORK_LIST) ; \
	do \
		if test -e $(FRAMEWORKS_ROOT)/$$d.framework/PrivateHeaders; \
		then \
			if test ! -e $(INCL_PLAT)/$$d; \
			then \
				ln -s $(FRAMEWORKS_ROOT)/$$d.framework/PrivateHeaders $(INCL_PLAT)/$$d; \
				if test "$$?" != "0"; then exit 1; fi; \
			fi; \
		fi; \
		if test -e $(FRAMEWORKS_ROOT)/$$d.framework/$$d; \
		then \
			if test ! -e $(LIB_PLAT)/lib$$d.dylib; \
			then \
				ln -s $(FRAMEWORKS_ROOT)/$$d.framework/$$d $(LIB_PLAT)/lib$$d.dylib; \
				if test "$$?" != "0"; then exit 1; fi; \
			fi; \
		fi; \
	done;

clean:
	for d in $(FRAMEWORK_LIST); \
	do \
		if test -e $(INCL_PLAT)/$$d; \
		then \
			rm $(INCL_PLAT)/$$d; \
		fi; \
		if test -e $(LIB_PLAT)/lib$$d.dylib; \
		then \
			rm $(LIB_PLAT)/lib$$d.dylib; \
		fi; \
	done;
