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
bbx              = $+$(kernels)$-

mbi_OBJS         = $(p)ufsd$(e) $(p)drv$(e) $(p)ufsdc$(e) $(p)mbifunc$(e) $(p)filesys$(e) $(p)modesw$(e) $(p)idt$(e) &
                   $(p)vsprintf$(e) $(p)serial$(e) $(p)istable$(e) $(p)biosdisk$(e) $(p)rawread$(e) $(p)cmdline$(e) $(p)term$(e) &
                   $(p)console$(e) $(p)consolec$(e) $(p)bios$(e) $(p)pxe$(e) $(p)asmcode$(e) $(p)undi$(e)
common_OBJS      = $(p)end$(e)

ADD_COPT   = -dOS2 -dMICROFSD -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include -i=$(MYDIR)netboot
ADD_ASMOPT = -dOS2 -dMICROFSD -dTERM_CONSOLE -i=$(PATH) -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include

#OBJS             = $(ufsd_OBJS) $(common_OBJS)

!     ifeq basename mbi
OBJS                    = $(mbi_OBJS) $(common_OBJS)
!else ifeq basename zip
OBJS                    = $(zip_OBJS) $(common_OBJS)
!endif

MOD_BASE         = REL1_BASE

DEST             = none

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
rels         = $+$(kernels)$-

p            = $(PATH)
e            = .mdl
TARGETS      = subdirs $+$(kernels)$- $(rels)

files     = $(bbx)

F =

base = 0x90000

!ifeq sh sh_
defs=-dNO_PROT -dREAL_BASE=$(base) -dSHIFT=$$(SHIFT)
!else
defs=-dNO_PROT -dREAL_BASE=$(base) -dSHIFT=0
!endif

.asm:   $(MYDIR)..$(SEP)..$(SEP)term
.c:     $(MYDIR)..$(SEP)..$(SEP)term
.asm:   $(MYDIR)..$(SEP)..$(SEP)preldr
.c:     $(MYDIR)..$(SEP)..$(SEP)preldr
.asm:   $(MYDIR)..$(SEP)..
.c:     $(MYDIR)..$(SEP)..
.c:     $(MYDIR)..
.c:     $(MYDIR)netboot
.asm:   $(MYDIR)netboot

$(PATH)asmcode.$(O): $(MYDIR)..$(SEP)..$(SEP)preldr$(SEP)asmcode.asm
 $(ASM) -dSTAGE1_5 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)asmcode.$(SO): $(MYDIR)..$(SEP)..$(SEP)preldr$(SEP)asmcode.asm
 $(ASM) -dSTAGE1_5 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@
