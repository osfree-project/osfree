#
#
#

DIRS    =
DEST    = os2$(SEP)system

!include $(%ROOT)/mk/appsos2.mk

TARGETS = $(PATH)oso001.msg  $(PATH)oso001.pl  $(PATH)oso001.ru &
          $(PATH)oso001h.msg $(PATH)oso001h.pl $(PATH)oso001h.ru

$(PATH)oso001.rsf: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) T=$^. rsf

$(PATH)oso001h.rsf: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) T=$^. rsf
