#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

LANG = russian
LNG = $(MYDIR)..$(SEP)$(LANG).err

!include $(%ROOT)/mk/tools.mk

DEST = none
TARGETS = $(PATH)strings.err

.SUFFIXES: .err

.err: $(PATH)

$(PATH)strings.err: $(MYDIR)..$(SEP)default.err
 @$(CP) $[@ $(PATH) $(BLACKHOLE)
 @critstrs.exe /dir $(PATH) $(LNG) $(BLACKHOLE)
