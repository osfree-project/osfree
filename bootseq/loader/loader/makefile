#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

#
# $+ switches on and $- switches off
# immediate macros substitution
#

LDR_OBJS         = $(p)ldrstart$(e) $(p)loader$(e) $(p)..$(SEP)wrap$(e) $(p)commands$(e) $(p)edit$(e) &
                   $(p)boot$(e) $(p)..$(SEP)cmdline$(e) $(p)..$(SEP)cfgparse-l$(e) # $(p)varsubst$(e)

MOD_BASE         = LDR_BASE
DEST             = boot$(SEP)loader
ADD_COPT         = -s -j
#
# filesystems list with path (p) and extension (e) placeholder
#
ldrfiles         = $(p)freeldr$(e)

# filesys without $(p) and $(e):
p                =
e                =
bbx              = $+$(ldrfiles)$-

OBJS             = $(LDR_OBJS)

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
rels         = $+$(ldrfiles)$-

p            = $(PATH)
e            = .mdl
TARGETS      = $+$(ldrfiles)$- # $(rels)

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

.c:   $(MYDIR)

.c:   $(MYDIR)..

$(PATH)..$(SEP)cfgparse-l.$(O): $(MYDIR)..$(SEP)cfgparse.c
 $(CC) -dLOADER $(COPT) -fr=$^*.err -fo=$^@ $[@

$(PATH)..$(SEP)cfgparse-l.$(SO): $(MYDIR)..$(SEP)cfgparse.c
 $(CC) -dLOADER $(COPT) -fr=$^*.err -fo=$^@ $[@

qwe: .symbolic
 $(SAY) $(COPT)
