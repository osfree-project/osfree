# tools/exehdr/msg/makefile
#
#

PROJ    = exh
TRGT    = $(PROJ).msg
MSGDIR = msg$(SEP)exh$(SEP)

!ifdef TARGETBLD
DEST = os2tk45$(SEP)msg
!else
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)msg
!endif

!include $(%ROOT)tools/mk/appsos2.mk
!include $(MYDIR)..$(SEP)exehdr.mk
!include $(%ROOT)tools/mk/port.mk

TARGETS = $(PATH)$(TRGT)

$(PATH)exh.msg:

$(PATH)exh.pl:

$(PATH)exh.ru:

$(PATH)exh.rsf: .always
 @echo RSF_RECIPE PLATFORM=[$(PLATFORM)] PATH=[$(PATH)] TARGET=[$@]
 @$(MAKE) $(MAKEOPT) srcdir=$(PORT_BASE)$(MSGDIR) PLATFORM=$(PLATFORM) T=$^. rsf
