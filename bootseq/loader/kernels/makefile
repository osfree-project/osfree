#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

DIRS = ufsd mFSD

#
# $+ switches on and $- switches off
# immediate macros substitution
#

#
# filesystems list with path (p) and extension (e) placeholder
#
header           = $(p)ufsd.inc
kernels          = $(p)linux$(e) $(p)chain$(e) $(p)bootos2$(e) # $(p)ufsd$(e)

msw              = $(p)modesw-npc$(e) $(p)modesw-npl$(e)

# filesys without $(p) and $(e):

p                =
e                =
#i                =
bbx              = $+$(kernels)$-

linux_OBJS       = $(p)linux$(e) $(p)linuxc$(e) $(p)modesw-npl$(e) $(p)vsprintf$(e) &
                   $(p)cmdline$(e) $(p)istable$(e) $(p)serial$(e) $(p)wrap$(e)

chain_OBJS       = $(p)chain$(e) $(p)modesw-npc$(e) $(p)biosdisk-c$(e) &
                   $(p)chainc$(e) $(p)cmdline$(e) $(p)..$(SEP)wrap$(e) $(p)setdev$(e) &
                   $(p)bios$(e)

bootos2_OBJS     = $(p)startup$(e) $(p)modesw-npo$(e) $(p)bootos2$(e) $(p)os2inc$(e) $(p)cfgsys$(e) $(p)serial$(e) &
                   $(p)vsprintf$(e) $(p)istable$(e) $(p)edit$(e) $(p)cfgparsr$(e) &
                   $(p)cmdline$(e)

#ufsd_OBJS        = $(p)ufsd$(e) $(p)ufsdc$(e)

common_SRCS      = #..$(SEP)cmdline.c ..$(SEP)wrap.c ..$(SEP)end.asm
common_OBJS      = $(p)..$(SEP)end$(e)

ADD_COPT   = -dOS2 -dMB_KERN #-dMINIFSD
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
!endif

MOD_BASE         = KERN_BASE

DEST             = boot$(SEP)loader

p            = 
e            = .mdl
INSTALL      = $+$(kernels)$-

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
#i            =
rels         = $+$(kernels)$-

p            = $(PATH)
e            = .mdl
#i            = $(PATH)ufsd.inc
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

.c:   $(MYDIR)

.c:   $(MYDIR)..

.c:   $(MYDIR)..$(SEP)preldr

.asm: $(MYDIR)

.asm: $(MYDIR)..

.asm: $(MYDIR)..$(SEP)preldr

.inc: $(PATH)

$(MYDIR)bootos2.asm: .symbolic

$(MYDIR)modesw-npc.asm: .symbolic

$(MYDIR)modesw-npl.asm: .symbolic

$(PATH)modesw-npo.$(O): $(MYDIR)..$(SEP)modesw.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dNO_PROT -dMB_KERN -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)modesw-npc.$(O): $(MYDIR)..$(SEP)modesw.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dNO_PROT -dMB_KERN -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)modesw-npl.$(O): $(MYDIR)..$(SEP)modesw.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dNO_PROT -dMB_KERN -dREAL_BASE=0x8000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)modesw-npc.$(SO): $(MYDIR)..$(SEP)modesw.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dNO_PROT -dMB_KERN -dREAL_BASE=0x90000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)modesw-npl.$(SO): $(MYDIR)..$(SEP)modesw.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dNO_PROT -dMB_KERN -dREAL_BASE=0x8000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)biosdisk-c.$(O): $(MYDIR)..$(SEP)preldr$(SEP)biosdisk.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dMB_KERN -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)biosdisk-c.$(SO): $(MYDIR)..$(SEP)preldr$(SEP)biosdisk.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dMB_KERN -dREAL_BASE=0x90000 -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

.rel: $(PATH)ufsd

$(PATH)bootos2.mdl: $(PATH)ufsd.inc $(PATH)mfsd.inc $(PATH)urel.inc

$(PATH)ufsd.inc: $(PATH)ufsd$(SEP)mbi.mdl
 @$(SAY) INC2H    $^. $(LOG)
 $(verbose)$(REXX) mkheader.cmd $< $^@ $(LOG)

$(PATH)mfsd.inc: $(PATH)mFSD$(SEP)mbi.dll
 @$(SAY) INC2H    $^. $(LOG)
 $(verbose)$(REXX) mkheader.cmd $< $^@ $(LOG)

$(PATH)urel.inc: $(PATH)ufsd$(SEP)mbi.rel
 @$(SAY) INC2H    $^. $(LOG)
 $(verbose)$(REXX) mkheader.cmd $< $^@  $(LOG)

$(TARGETS):
