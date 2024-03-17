
DEST = os2tk45$(SEP)som$(SEP)include

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)sm.h &
	$(PATH)smcidl.h
#	$(PATH)som.xh &
#	$(PATH)somapi.h &
#	$(PATH)sombtype.h &
#	$(PATH)somcdev.h &
#	$(PATH)somltype.h &
#	$(PATH)somnames.h &
#	$(PATH)somplatf.h &
#	$(PATH)somtypes.h

SOURCES = $(PORT_BASE)somc$(SEP)include$(SEP)sm.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)smcidl.h

$(TARGETS): $(SOURCES)
 $(verbose)$(CP) $(PORT_BASE)somc$(SEP)include$(SEP)$^. $^@ $(BLACKHOLE)
