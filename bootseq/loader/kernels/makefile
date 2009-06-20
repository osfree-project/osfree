#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

DIRS = ufsd

#
# $+ switches on and $- switches off
# immediate macros substitution
#

#
# filesystems list with path (p) and extension (e) placeholder
#
header           = $(p)ufsd.inc
kernels          = $(p)linux$(e) $(p)chain$(e) $(p)ufsd$(e) $(i) $(p)bootos2$(e)

msw              = $(p)modesw-npc$(e) $(p)modesw-npl$(e)

# filesys without $(p) and $(e):

p                =
e                =
i                =
bbx              = $+$(kernels)$-

linux_OBJS    = $(p)linux$(e) $(p)linuxc$(e) $(p)modesw-npl$(e) $(p)vsprintf$(e) $(p)cmdline$(e) $(p)wrap$(e)

chain_OBJS    = $(p)chain$(e) $(p)modesw-npc$(e) $(p)biosdisk-c$(e) &
                $(p)chainc$(e) $(p)..$(SEP)cmdline$(e) $(p)..$(SEP)wrap$(e) $(p)..$(SEP)setdev$(e) &
                $(p)..$(SEP)preldr$(SEP)bios$(e)

bootos2_OBJS      = $(p)startup$(e) $(p)bootos2$(e)

ufsd_OBJS     = $(p)ufsd$(e) $(p)ufsdc$(e)

common_SRCS      = #..$(SEP)cmdline.c ..$(SEP)wrap.c ..$(SEP)end.asm
common_OBJS      = $(p)..$(SEP)end$(e)

ADD_COPT   = -dOS2
ADD_ASMOPT = -i=$(PATH)
CLEAN_ADD  = ufsd.inc mfsd.inc urel.inc

!     ifeq basename linux
#t = -c
OBJS                    = $(linux_OBJS) $(common_OBJS)
!else ifeq basename chain
#t = -h
OBJS                    = $(chain_OBJS) $(common_OBJS)
!else ifeq basename bootos2
#t = -o
OBJS                    = $(bootos2_OBJS) $(common_OBJS)
!else ifeq basename ufsd
OBJS                    = $(ufsd_OBJS)
!endif

MOD_BASE         = KERN_BASE

DEST             = boot$(SEP)loader

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

!     ifeq basename linux
base = 0x8000
!else ifeq basename chain
base = 0x90000
!else ifeq basename startup
base = 0x90000
!endif

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

$(PATH)modesw-npc.$(O): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)modesw-npl.$(O): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x8000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)modesw-npo.$(O): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)modesw-npc.$(SO): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)modesw-npl.$(SO): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x8000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)modesw-npo.$(SO): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)biosdisk-c.$(O): $(MYDIR)..$(SEP)preldr$(SEP)biosdisk.asm
 $(ASM) -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)biosdisk-c.$(SO): $(MYDIR)..$(SEP)preldr$(SEP)biosdisk.asm
 $(ASM) -dREAL_BASE=0x90000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)bootos2.mdl: $(PATH)ufsd.inc $(PATH)mfsd.inc $(PATH)urel.inc

$(PATH)ufsd.inc: $(PATH)ufsd$(SEP)ufsd.mdl
 $(REXX) mkheader.cmd $< $^@

$(PATH)mfsd.inc: $(MYDIR)os2boot
 $(REXX) mkheader.cmd $< $^@

.rel: $(PATH)ufsd

$(PATH)urel.inc: $(PATH)ufsd$(SEP)ufsd.rel
 $(REXX) mkheader.cmd $< $^@
