#
#
#

PROJ    = exhh
TRGT    = $(PROJ).msg
MSGDIR = msg$(SEP)exhh$(SEP)

!ifdef TARGETBLD
DEST = os2tk45$(SEP)msg
!else
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)msg
!endif

!include $(%ROOT)tools/mk/appsos2.mk
!include $(MYDIR)..$(SEP)exehdr.mk

TARGETS = $(PATH)$(TRGT)

$(PATH)exhh.msg:

$(PATH)exhh.pl:

$(PATH)exhh.ru:

$(PATH)exhh.rsf: .always
 @$(MAKE) $(MAKEOPT) srcdir=$(PORT_BASE)$(MSGDIR) PLATFORM=$(PLATFORM) T=$^. rsf
