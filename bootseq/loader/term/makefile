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
blackboxes           = $(p)console$(e) $(p)hercules$(e) $(p)serial$(e) $(p)graphics$(e)

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
OBJS                    = $(common_OBJS) $(p)serial$(e) $(p)terminfo$(e) $(p)cmdline$(e) $(p)tparm$(e) $(p)modesw-npt$(e)
!else ifeq basename graphics
t = -g
OBJS                    = $(common_OBJS) $(p)graphics$(e) $(p)vmode$(e) $(p)console$(e) $(p)modesw-npt$(e)
!endif

MOD_BASE             = EXT_BUF_BASE
DEST         = boot$(SEP)loader$(SEP)term

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
rels         = $+$(blackboxes)$-

p            = $(PATH)
e            = .mdl
TARGETS      = $+$(blackboxes)$- $(rels)

files     = $(bbx)

.c: $(MYDIR)..

p =
e =
spec_SRCS = # modesw.asm,-npt terminit.c,-c terminit.c,-h terminit.c,-s  console.asm,-c

$(PATH)modesw-npt.$(O): $(MYDIR)..$(SEP)modesw.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dNO_PROT -dBLACKBOX -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)modesw-npt.$(SO): $(MYDIR)..$(SEP)modesw.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dNO_PROT -dBLACKBOX -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-c.$(O): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=0 -dTERM_CONSOLE $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-c.$(SO): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=$(SHIFT) -dTERM_CONSOLE $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-h.$(O): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=0 -dTERM_HERCULES $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-h.$(SO): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=$(SHIFT) -dTERM_HERCULES $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-s.$(O): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=0 -dTERM_SERIAL $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-s.$(SO): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=$(SHIFT) -dTERM_SERIAL $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-g.$(O): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=0 -dTERM_GRAPHICS $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)terminit-g.$(SO): $(MYDIR)terminit.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=$(SHIFT) -dTERM_GRAPHICS $(COPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)console-c.$(O): $(MYDIR)console.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dSHIFT=0 -dTERM_CONSOLE $(ASMOPT) -fr=$^*.err -fo=$^@ $< $(LOG)

$(PATH)console-c.$(SO): $(MYDIR)console.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dSHIFT=$(SHIFT) -dTERM_CONSOLE $(ASMOPT) -fr=$^*.err -fo=$^@ $< $(LOG)

F =

!ifeq sh
defs="-dterm_$(basename) -dNO_PROT -dBLACKBOX -dSHIFT=0"
!else
defs="-dterm_$(basename) -dNO_PROT -dBLACKBOX -dSHIFT=$$$$(SHIFT)"
!endif
