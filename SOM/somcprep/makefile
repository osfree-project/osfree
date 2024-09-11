
DEST = os2tk45$(SEP)som$(SEP)include

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)sm.h &
	$(PATH)sm.xh &
	$(PATH)smcidl.h &
	$(PATH)smcidl.xh &
	$(PATH)smstd.h &
	$(PATH)smlib.h &
	$(PATH)somc_msg.h &
	$(PATH)sctypes.h &
	$(PATH)symtab.h &
	$(PATH)optlink.h &
	$(PATH)emitlib.h

SOURCES = $(PORT_BASE)somc$(SEP)include$(SEP)sm.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)sm.xh &
	$(PORT_BASE)somc$(SEP)include$(SEP)smcidl.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)smcidl.xh &
	$(PORT_BASE)somc$(SEP)include$(SEP)smstd.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)smlib.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)somc_msg.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)sctypes.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)symtab.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)optlink.h &
	$(PORT_BASE)somc$(SEP)include$(SEP)emitlib.h

$(TARGETS): $(SOURCES)
 $(verbose)$(CP) $(PORT_BASE)somc$(SEP)include$(SEP)$^. $^@ $(BLACKHOLE)
