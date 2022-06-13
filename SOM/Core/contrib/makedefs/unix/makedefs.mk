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

include ../../makedefs/$(PLATFORM)/$(BUILDTYPE)/makedefs.mk
include ../../makedefs/$(PLATFORM_PROTO)/makedefs.mk

SHLB_EXP=../../makedefs/$(PLATFORM_PROTO)/shlb_exp.sh
SHLB_MAP=../../makedefs/$(PLATFORM_PROTO)/shlb_map.sh
SHLB_REF=../../makedefs/$(PLATFORM_PROTO)/shlb_ref.sh
SHLB_ENT=../../makedefs/$(PLATFORM_PROTO)/shlb_ent.sh
SHLB_DLO=../../makedefs/$(PLATFORM_PROTO)/shlb_dlo.sh
ARLB_REF=../../makedefs/$(PLATFORM_PROTO)/arlb_ref.sh

OUTDIR_BIN=$(OUTDIR)/bin
OUTDIR_SBIN=$(OUTDIR)/sbin
OUTDIR_SHLIB=$(OUTDIR)/lib
OUTDIR_LIB=$(OUTDIR)/lib
OUTDIR_IMPLIB=$(OUTDIR)/implib
OUTDIR_REFLIB=$(OUTDIR)/reflib
OUTDIR_TOOLS=$(OUTDIR)/tools
OUTDIR_TESTS=$(OUTDIR)/tests
OUTDIR_INCLUDE=$(OUTDIR)/include
OUTDIR_ETC=$(OUTDIR)/etc
OUTDIR_DIST=$(OUTDIR)/dist
OUTDIR_MAN=$(OUTDIR)/man
OUTDIR_FRAMEWORKS=$(OUTDIR)/frameworks

HOSTDIR_BIN=$(HOSTDIR)/bin
HOSTDIR_LIB=$(HOSTDIR)/lib
HOSTDIR_IMPLIB=$(HOSTDIR)/implib
HOSTDIR_TOOLS=$(HOSTDIR)/tools
HOSTDIR_TESTS=$(HOSTDIR)/tests

CLEAN=rm -f

EXEC_TEST=HOSTDIR_TOOLS=$(HOSTDIR_TOOLS) ../../toolbox/exectest.sh
EXEC_TOOL=HOSTDIR_TOOLS=$(HOSTDIR_TOOLS) ../../toolbox/exectool.sh
EXEC_CONFIG=HOSTDIR_TOOLS=$(HOSTDIR_TOOLS) ../../toolbox/execconf.sh
