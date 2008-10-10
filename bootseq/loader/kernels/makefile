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
kernels          = $(p)linux$(e) $(p)chain$(e) $(p)os2$(e) $(p)ufsd$(e)

msw              = $(p)..$(SEP)modesw-npc$(e) $(p)..$(SEP)modesw-npl$(e)

# filesys without $(p) and $(e):
p                = bt_
e                =
bbx              = $+$(kernels)$-

linux_OBJS    = $(p)linux$(e) $(p)linuxc$(e) $(p)..$(SEP)modesw-npl$(e)

chain_OBJS    = $(p)chain$(e) $(p)chainc$(e) $(p)..$(SEP)setdev$(e) $(p)..$(SEP)modesw-npc$(e)

os2_OBJS      = $(p)os2$(e) $(p)os2c$(e) $(p)..$(SEP)modesw-npo$(e)

ufsd_OBJS     = $(p)ufsd$(e) $(p)ufsdc$(e)

common_SRCS      = #..$(SEP)cmdline.c ..$(SEP)wrap.c ..$(SEP)end.asm
common_OBJS      = $(p)..$(SEP)cmdline$(e) $(p)..$(SEP)wrap$(e) $(p)..$(SEP)end$(e)

!     ifeq basename bt_linux
#t = -c
OBJS                    = $(linux_OBJS) $(common_OBJS)
!else ifeq basename bt_chain
#t = -h
OBJS                    = $(chain_OBJS) $(common_OBJS)
!else ifeq basename bt_os2
#t = -o
OBJS                    = $(os2_OBJS) $(common_OBJS)
!else ifeq basename bt_ufsd
OBJS                    = $(ufsd_OBJS)
!endif

MOD_BASE         = KERN_BASE
DEST             = boot$(SEP)loader

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)bt_
e            = .rel
rels         = $+$(kernels)$-

p            = $(PATH)bt_
e            = .mdl
TARGETS      = $+$(kernels)$- $(rels)

files     = $(bbx)

p =
e = .asm
spec_SRCS = $+$(kernels) $(msw)$-

F =

!     ifeq basename linux
base = 0x8000
!else ifeq basename chain
base = 0x90000
!else ifeq basename os2
base = 0x50000
!else ifeq basename ufsd
base = 0x50000
!endif

!ifeq sh sh_
defs=-dNO_PROT -dREAL_BASE=$(base) -dSHIFT=$$(SHIFT)
!else
defs=-dNO_PROT -dREAL_BASE=$(base) -dSHIFT=0
!endif

$(PATH)..$(SEP)modesw-npc.$(O): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)modesw-npl.$(O): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x8000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)modesw-npo.$(O): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)modesw-npc.$(SO): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)modesw-npl.$(SO): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x8000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)modesw-npo.$(SO): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)ufsd: $(PATH)bt_ufsd.mdl
 $(CUT) 0 0x200 $[@ >$^@
 