#
#
#

DIRS    =
PROJ    = lvm
TRGT    = $(PROJ).msg
DEST    = os2$(SEP)system

!include $(%ROOT)/mk/appsos2.mk

TARGETS =  $(PATH)lvm.msg $(PATH)lvm.pl $(PATH)lvm.ru

$(PATH)lvm.msg:

$(PATH)lvm.pl:

$(PATH)lvm.ru:

$(PATH)lvm.rsf: .always
 @$(MAKE) $(MAKEOPT) T=$^. rsf
