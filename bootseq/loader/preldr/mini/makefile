#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

#
# $+ switches on and $- switches off
# the immediate macros substitution
#

DEST             =  boot$(SEP)loader

!include $(%ROOT)/mk/dirs.mk

STAGE0_OBJS   =    $(p)segord$(e) $(p)init$(e) $(p)modesw$(e) $(p)asmcode$(e) $(p)biosdisk$(e) $(p)filesys$(e) &
                   $(p)common$(e) $(p)stage0_fs$(e) $(p)func$(e) $(p)setdev$(e) $(p)dskaccess$(e) $(p)part$(e) &
                   $(p)bios$(e) $(p)end$(e)

ADD_COPT         = -s -j -dSTAGE1_5 -dNO_BLOCK_FILES -i=$(MYDIR)..$(SEP)..$(SEP)include
ADD_ASMOPT       = -dSTAGE1_5 -dNO_BLOCK_FILES -i=$(MYDIR)..$(SEP)..$(SEP)include
!define ALIASES  init_=_init
MOD_BASE         = STAGE0_BASE

#
# filesystems list with path (p) and extension (e) placeholder
#
ldrfiles         = $(p)preldr_mini$(e)

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

.c:   $(MYDIR)..

.c:   $(MYDIR)..$(SEP)..

.asm: $(MYDIR)..

.asm: $(MYDIR)..$(SEP)..


$(PATH)func.$(O): $(MYDIR)..$(SEP)func.c
 $(CC) -dSTAGE0 -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)func.$(SO): $(MYDIR)..$(SEP)func.c
 $(CC) -dSTAGE0 -dSHIFT=$(SHIFT) $(COPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)cfgparse.$(O): $(MYDIR)..$(SEP)..$(SEP)cfgparse.c

$(PATH)cfgparse.$(SO): $(MYDIR)..$(SEP)..$(SEP)cfgparse.c

