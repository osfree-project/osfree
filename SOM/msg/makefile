#
#
#

DIRS    =
PROJ    = somk
TRGT    = $(PROJ).msg
DEST    = os2tk45$(SEP)som$(SEP)common$(SEP)system

!include $(%ROOT)tools/mk/appsos2.mk

TARGETS =  $(PATH)$(PROJ).msg $(PATH)$(PROJ).pl $(PATH)$(PROJ).ru $(FILESDIR)os2$(SEP)dll$(SEP)somk.msg

$(PATH)$(PROJ).msg:

$(PATH)$(PROJ).pl:

$(PATH)$(PROJ).ru:

$(PATH)$(PROJ).rsf: .always
 @$(MAKE) $(MAKEOPT) T=$^. rsf

$(FILESDIR)os2$(SEP)dll$(SEP)somk.msg: $(PATH)somk.msg
 $(verbose)$(SAY) INST     $^. $(LOG)
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)
