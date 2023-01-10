#
#
#

DIRS    =
PROJ    = oso001
TRGT    = $(PROJ).msg
DEST    = os2$(SEP)system

!include $(%ROOT)tools/mk/appsos2.mk

TARGETS = $(PATH)oso001.msg  $(PATH)oso001.pl  $(PATH)oso001.ru

$(PATH)oso001.msg:

$(PATH)oso001.pl:

$(PATH)oso001.ru:

$(PATH)oso001.rsf: .always
 @$(MAKE) $(MAKEOPT) T=$^. rsf
