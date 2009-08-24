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
kernels          = $(p)mbi$(e)

# filesys without $(p) and $(e):

p                =
e                =
i                =
bbx              = $+$(kernels)$-

ufsd_OBJS        = $(p)ufsd$(e) $(p)drv$(e) $(p)ufsdc$(e) $(p)filesys$(e) $(p)modesw$(e) $(p)idt$(e) &
                   $(p)vsprintf$(e) $(p)serial$(e) $(p)istable$(e) $(p)biosdisk$(e) $(p)rawread$(e) &
                   $(p)cmdline$(e) $(p)bios$(e)
common_OBJS      = $(p)end$(e)

ADD_COPT   = -dOS2 -dMB_KERN -dMINIFSD -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include
ADD_ASMOPT = -dOS2 -dMB_KERN -dMINIFSD -i=$(PATH) -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include

OBJS             = $(ufsd_OBJS) $(common_OBJS)

MOD_BASE         = REL1_BASE

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
TARGETS      = subdirs $+$(kernels)$- $(rels)

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

.asm:   $(MYDIR)..$(SEP)..$(SEP)preldr
.c:     $(MYDIR)..$(SEP)..$(SEP)preldr
.asm:   $(MYDIR)..$(SEP)..
.c:     $(MYDIR)..$(SEP)..
.c:     $(MYDIR)..

