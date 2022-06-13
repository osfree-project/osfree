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

OUTDIR_DSOM=$(OUTDIR_ETC)/dsom
SOMENV=$(OUTDIR_BIN)/somenv
SOMENV_INI=$(OUTDIR_ETC)/somenv.ini

DIRS=	$(OUTDIR_DSOM) \
		$(OUTDIR_DSOM)/somlo \
		$(OUTDIR_DSOM)/somnm \
		$(OUTDIR_DSOM)/somsec \
		$(OUTDIR_DSOM)/somos


all:  $(SOMENV)


clean:
	rm -rf "$(OUTDIR_DSOM)" "$(SOMENV)" "$(SOMENV_INI)"

$(DIRS):
	mkdir $@

$(SOMENV): $(DIRS) ../unix/somenv.sh
	chmod +x ../unix/somenv.sh
	DLLPATHENV="$(DLLPATHENV)" SOMBASE="$(OUTDIR)" ../unix/somenv.sh "$@" "$(SOMENV_INI)"


dist install:

