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

TARGET=$(OUTDIR_BIN)/sc 
ETC_SOMIR=$(OUTDIR_ETC)/som.ir

all: $(TARGET) $(OUTDIR_MAN)/sc.1 config

clean:
	$(CLEAN) $(TARGET) $(OUTDIR_MAN)/sc.1 $(OUTDIR_MAN)/sc.1.* $(ETC_SOMIR)

$(TARGET): ../sc.sh
	cp ../sc.sh $@
	chmod +x $@

dist install:

$(OUTDIR_MAN)/sc.1: ../unix/manpage.txt
	../../toolbox/txt2man.sh <../unix/manpage.txt >$@
	../../toolbox/manpack.sh $@

config:
	if test "$(SC)" = ""; \
	then \
		echo "SC=$(HOSTDIR_BIN)/sc" >>"$(MAKEDEFS)"; \
	fi
	if test "$(SOMIR)" = ""; \
	then \
		echo "SOMIR=$(ETC_SOMIR)" >>"$(MAKEDEFS)"; \
	fi
