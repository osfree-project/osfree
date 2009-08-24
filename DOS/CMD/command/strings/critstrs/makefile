#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

LANG = russian
LNG = $(MYDIR)..$(SEP)$(LANG).err
DEST = none

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)strings.err

.SUFFIXES: .err

.err: $(PATH)

$(PATH)strings.err: $(MYDIR)..$(SEP)default.err
 $(CP) $[@ $(PATH)
 critstrs.exe /dir $(PATH) $(LNG)
