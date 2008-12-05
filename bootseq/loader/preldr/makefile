#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

#
# $+ switches on and $- switches off
# immediate macros substitution
#

STAGE0_OBJS      = $(p)segord$(e) $(p)init$(e) $(p)..$(SEP)modesw$(e) $(p)asmcode$(e) $(p)apm$(e) $(p)biosdisk$(e) $(p)filesys$(e) $(p)filesys16$(e) &
                   $(p)video$(e) $(p)vbe$(e) $(p)common$(e) $(p)stage0_fs$(e) $(p)func$(e) $(p)..$(SEP)setdev$(e) $(p)..$(SEP)cmdline$(e) &
                   $(p)dskaccess$(e) $(p)part$(e) $(p)..$(SEP)cfgparse$(e) $(p)idt$(e) $(p)bios$(e) $(p)end$(e)

#STAGE0_LT_OBJS   = $(p)segord$(e) $(p)init$(e) $(p)..$(SEP)modesw$(e) $(p)asmcode$(e) $(p)apm$(e) $(p)biosdisk$(e) $(p)filesys$(e) &
#                   $(p)dskaccess$(e) $(p)part$(e) $(p)video$(e) $(p)common$(e) $(p)stage0_fs$(e) $(p)func$(e) $(p)..$(SEP)setdev$(e) &
#                   $(p)bios$(e) $(p)end$(e)

MOD_BASE         = STAGE0_BASE
DEST             = boot$(SEP)loader

#
# filesystems list with path (p) and extension (e) placeholder
#
ldrfiles         = $(p)preldr0$(e)

# filesys without $(p) and $(e):
p                =
e                =
bbx              = $+$(ldrfiles)$-

OBJS             = $(STAGE0_OBJS)

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
rels         = $+$(ldrfiles)$-

p            = $(PATH)
e            = .mdl
TARGETS      = $+$(ldrfiles)$- $(rels)

files     = $(bbx)

p = fsys_
e = .c
spec_SRCS = # $+$(blackboxes)$-


F =

#!ifeq sh
#defs=-d$(basename) -dSHIFT=0
#!else
#defs=-d$(basename) -dSHIFT=$$(SHIFT)
#!endif

$(PATH)..$(SEP)func.$(O): $(MYDIR)..$(SEP)func.c
 $(CC) -dSTAGE0 -dSHIFT=0 $(COPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)func.$(LO): $(MYDIR)..$(SEP)func.c
 $(CC) -dSTAGE0 -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)func.$(SO): $(MYDIR)..$(SEP)func.c
 $(CC) -dSTAGE0 -dSHIFT=$(SHIFT) $(COPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)cfgparse.$(O): $(MYDIR)..$(SEP)cfgparse.c

$(PATH)..$(SEP)cfgparse.$(SO): $(MYDIR)..$(SEP)cfgparse.c
