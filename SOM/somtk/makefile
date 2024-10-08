
DEST = os2tk45$(SEP)som$(SEP)include

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)emtypes.h &
	$(PATH)emtypes.xh &
	$(PATH)eventmsk.h &
	$(PATH)eventmsk.xh &
	$(PATH)somd.h &
	$(PATH)somd.xh &
	$(PATH)somderr.h &
	$(PATH)somdext.h &
	$(PATH)somir.h &
	$(PATH)somir.xh &
	$(PATH)soms.h &
	$(PATH)soms.xh &
	$(PATH)somtc.h &
	$(PATH)somtc.xh &
	$(PATH)somtcnst.h &
	$(PATH)somtcnst.xh &
	$(PATH)somthrd.h

SOURCES = $(PORT_BASE)somtk$(SEP)include$(SEP)emtypes.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)emtypes.xh &
	$(PORT_BASE)somtk$(SEP)include$(SEP)eventmsk.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)eventmsk.xh &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somd.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somd.xh &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somderr.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somdext.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somir.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somir.xh &
	$(PORT_BASE)somtk$(SEP)include$(SEP)soms.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)soms.xh &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somtc.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somtc.xh &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somtcnst.h &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somtcnst.xh &
	$(PORT_BASE)somtk$(SEP)include$(SEP)somthrd.h

$(TARGETS): $(SOURCES)
 $(verbose)$(CP) $(PORT_BASE)somtk$(SEP)include$(SEP)$^. $^@ $(BLACKHOLE)
