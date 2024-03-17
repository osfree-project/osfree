
DEST = os2tk45$(SEP)som$(SEP)include

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)somcorba.h &
	$(PATH)som.h &
	$(PATH)som.xh &
	$(PATH)somapi.h &
	$(PATH)sombtype.h &
	$(PATH)somcdev.h &
	$(PATH)somltype.h &
	$(PATH)somnames.h &
	$(PATH)somplatf.h &
	$(PATH)somtypes.h

SOURCES = $(PORT_BASE)somkpub$(SEP)include$(SEP)somcorba.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)som.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)som.xh &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)somapi.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)sombtype.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)somcdev.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)somltype.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)somnames.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)somplatf.h &
	$(PORT_BASE)somkpub$(SEP)include$(SEP)somtypes.h

$(TARGETS): $(SOURCES)
 $(verbose)$(CP) $(PORT_BASE)somkpub$(SEP)include$(SEP)$^. $^@ $(BLACKHOLE)
