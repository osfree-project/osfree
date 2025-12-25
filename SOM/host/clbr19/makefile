
PROJ = clbs19
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)common$(SEP)dll

!include $(%ROOT)tools/mk/all.mk

TARGETS = $(PATH)$(PROJ).dll

$(PATH)$(PROJ).lnk: .symbolic

$(PATH)$(PROJ).dll: $(%WATCOM)$(SEP)binnt$(SEP)$(PROJ).dll
 @$(CP) $< $^@ $(BLACKHOLE)
