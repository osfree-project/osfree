
DEST = os2tk45$(SEP)som$(SEP)include

!include $(%ROOT)SOM/common2.mk

TARGETS = $(PATH)mlink.idl &
                $(PATH)tpll.idl &
                $(PATH)tpllitr.idl

SOURCES = $(PORT_BASE)somuc$(SEP)mlink.idl &
                $(PORT_BASE)somuc$(SEP)tpll.idl &
                $(PORT_BASE)somuc$(SEP)tpllitr.idl

$(TARGETS): $(SOURCES)
 $(verbose)pdl.exe -o $(PATH) $< #$(BLACKHOLE)
