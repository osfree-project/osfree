#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

#
# $+ switches on and $- switches off
# immediate macros substitution
#

#
# filesystems list with path (p) and extension (e) placeholder
#
kernels          = $(p)ufsd$(e)

# filesys without $(p) and $(e):

p                = 
e                =
i                =
bbx              = $+$(kernels)$-

ufsd_OBJS     = $(p)ufsd$(e) $(p)ufsdc$(e)
common_OBJS      = $(p)..$(SEP)..$(SEP)end$(e)

ADD_COPT   = -dOS2 -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include
ADD_ASMOPT = -i=$(PATH) -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include

OBJS             = $(ufsd_OBJS) $(common_OBJS)

MOD_BASE         = REL_BASE

DEST             = none

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
i            =
rels         = $+$(kernels)$-

p            = $(PATH)
e            = .mdl
i            = $(PATH)ufsd.inc
TARGETS      = subdirs $+$(kernels)$- # $(rels)

files     = $(bbx)

p =
e = .asm
spec_SRCS = $+$(kernels) $(msw)$- startup.asm

F =

base = 0x90000

!ifeq sh sh_
defs=-dNO_PROT -dREAL_BASE=$(base) -dSHIFT=$$(SHIFT)
!else
defs=-dNO_PROT -dREAL_BASE=$(base) -dSHIFT=0
!endif

.$(O):  $(PATH).
.$(SO): $(PATH).
.$(O):  $(PATH)..
.$(SO): $(PATH)..
.$(O):  $(PATH)..$(SEP)preldr
.$(SO): $(PATH)..$(SEP)preldr
.inc:   $(PATH)
