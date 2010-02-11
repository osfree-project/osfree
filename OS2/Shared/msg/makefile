#
#
#

DIRS    =
DEST    = os2$(SEP)system

!include $(%ROOT)/mk/appsos2.mk

TARGETS = $(PATH)OSO001.msg  $(PATH)oso001.pl  $(PATH)OSO001.ru &
          $(PATH)OSO001h.msg $(PATH)oso001h.pl $(PATH)OSO001h.ru

$(PATH)oso001.rsf: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) T=$^. rsf

$(PATH)oso001h.rsf: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) T=$^. rsf
