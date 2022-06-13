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

MAKEDEFS=../../makedefs/unix/makedefs.mk

include $(MAKEDEFS)

CONFIG_MK=../../config/$(PLATFORM)/$(BUILDTYPE)/config.mk

all: $(PARTS)

clean:
	PARTS_R="" ; for d in $(PARTS); do PARTS_R="$$d $$PARTS_R"; done; \
	PARTS_R="$$PARTS_R"; \
	for d in $$PARTS_R; do \
		if test -f ../../$$d/unix/$$d.mak; then \
			if test -d ../../$$d/$(PLATFORM); then \
				cd ../../$$d/$(PLATFORM); \
				$(MAKE) -f ../unix/$$d.mak PLATFORM=$(PLATFORM) MAKEDEFS=$(MAKEDEFS) INTDIR=$(BUILDTYPE) $@ ; \
				if test "$$?" != "0"; then \
					echo $$d clean failed ; exit "1"; \
				fi; \
				for e in $(BUILDTYPE)/clean.*.mak; do \
					if test -f $$e; then \
						$(MAKE) -f $$e PLATFORM=$(PLATFORM) MAKEDEFS=$(MAKEDEFS) INTDIR=$(BUILDTYPE) $@; \
						rm $$e; \
					fi; \
				done; \
			fi; \
		fi; \
	done; \
	for d in $$PARTS_R; do e=../../$$d/$(PLATFORM); \
		if test -d $$e/$(BUILDTYPE)/ii_files; then \
			rm -r $$e/$(BUILDTYPE)/ii_files; \
		fi; \
		if test -d $$e/$(BUILDTYPE); then \
			rmdir $$e/$(BUILDTYPE); \
			if test "$$?" != "0"; then \
				echo $$d clean failed ; exit "1"; \
			fi; \
		fi; \
	done

$(PARTS):
	if test -f ../../$@/unix/$@.mak; \
	then \
		if test ! -d ../../$@/$(PLATFORM); \
		then mkdir ../../$@/$(PLATFORM) ; \
		fi ; \
		if test ! -d ../../$@/$(PLATFORM)/$(BUILDTYPE); \
		then \
			mkdir ../../$@/$(PLATFORM)/$(BUILDTYPE) ; \
		fi; \
		cd ../../$@/$(PLATFORM); \
		$(MAKE) -f ../unix/$@.mak PLATFORM=$(PLATFORM) INTDIR=$(BUILDTYPE) `if test -f $(CONFIG_MK); then echo MAKEDEFS=$(CONFIG_MK); else echo MAKEDEFS=$(MAKEDEFS); fi` ; \
	fi

install dist:
	for d in $(PARTS); do \
		if test -f ../../$$d/unix/$$d.mak; then \
			if test -d ../../$$d/$(PLATFORM); then \
				cd ../../$$d/$(PLATFORM); \
				$(MAKE) -f ../unix/$$d.mak PLATFORM=$(PLATFORM) MAKEDEFS=$(MAKEDEFS) INTDIR=$(BUILDTYPE) $@ ; \
				if test "$$?" != "0"; then \
					echo $$d $@ failed ; exit "1"; \
				fi; \
			fi; \
		fi; \
	done

# EOF


