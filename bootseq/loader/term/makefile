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
# blackboxes list with path (p) and extension (e) placeholder
#
blackboxes           = $(p)console$(e) $(p)hercules$(e) $(p)serial$(e)

# filesys without $(p) and $(e):
p   =
e   =
bbx                  = $+$(blackboxes)$-

common_SRCS          = termstrt.asm terminit.c
common_OBJS          = $(p)termstrt$(e)  $(p)terminit$(t)$(e)


!     ifeq basename console
t = -c
OBJS                    = $(common_OBJS) $(p)console-c$(e) $(p)consolec$(e) $(p)modesw-npt$(e)
!else ifeq basename hercules
t = -h
OBJS                    = $(common_OBJS) $(p)hercules$(e) $(p)console$(e) $(p)modesw-npt$(e)
!else ifeq basename serial
t = -s
OBJS                    = $(common_OBJS) $(p)serial$(e) $(p)terminfo$(e) $(p)tparm$(e) $(p)modesw-npt$(e)
!endif

DIRS         =
DIR          = term
RELDIR       = bootseq$(SEP)loader$(SEP)$(DIR)$(SEP)
MYDIR        = $(%ROOT)$(SEP)$(RELDIR)
PATH         = $(RELDIR)

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
TARGETS      = $+$(blackboxes)$-

p            = $(PATH)
e            = .bbx
PROJ         = $+$(blackboxes)$- $(TARGETS)

files     = $(bbx)

p =
e =
spec_SRCS = # modesw.asm,-npt terminit.c,-c terminit.c,-h terminit.c,-s  console.asm,-c

$(PATH)modesw-npt.$(O): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dBLACKBOX -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $<

$(PATH)modesw-npt.$(SO): $(MYDIR)..$(SEP)modesw.asm
 $(ASM) -dNO_PROT -dBLACKBOX -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $<

$(PATH)terminit-c.$(O): $(MYDIR)terminit.c
 $(CC) -dSHIFT=0 -dTERM_CONSOLE $(COPT) -fr=$^*.err -fo=$^@ $<

$(PATH)terminit-c.$(SO): $(MYDIR)terminit.c
 $(CC) -dSHIFT=$(SHIFT) -dTERM_CONSOLE $(COPT) -fr=$^*.err -fo=$^@ $<

$(PATH)terminit-h.$(O): $(MYDIR)terminit.c
 $(CC) -dSHIFT=0 -dTERM_HERCULES $(COPT) -fr=$^*.err -fo=$^@ $<

$(PATH)terminit-h.$(SO): $(MYDIR)terminit.c
 $(CC) -dSHIFT=$(SHIFT) -dTERM_HERCULES $(COPT) -fr=$^*.err -fo=$^@ $<

$(PATH)terminit-s.$(O): $(MYDIR)terminit.c
 $(CC) -dSHIFT=0 -dTERM_SERIAL $(COPT) -fr=$^*.err -fo=$^@ $<

$(PATH)terminit-s.$(SO): $(MYDIR)terminit.c
 $(CC) -dSHIFT=$(SHIFT) -dTERM_SERIAL $(COPT) -fr=$^*.err -fo=$^@ $<

$(PATH)console-c.$(O): $(MYDIR)console.asm
 $(ASM) -dSHIFT=0 -dTERM_CONSOLE $(ASMOPT) -fr=$^*.err -fo=$^@ $<

$(PATH)console-c.$(SO): $(MYDIR)console.asm
 $(ASM) -dSHIFT=$(SHIFT) -dTERM_CONSOLE $(ASMOPT) -fr=$^*.err -fo=$^@ $<

F =

!ifeq sh
defs="-dterm_$(basename) -dNO_PROT -dBLACKBOX -dSHIFT=0"
!else
defs="-dterm_$(basename) -dNO_PROT -dBLACKBOX -dSHIFT=$$$$(SHIFT)"
!endif
