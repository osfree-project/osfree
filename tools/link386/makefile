DESCRIPTION = LINK386 linker
SOURCES = deff386 link386 mesg386 xlate386 demangle fuzzy hash parse cmdline cmdscan context error file memory pathconv orlentry orlhash orlflhnd elfentr elfflhn elfload elflwlv coffimpl cofflwlv coffflhn coffload coffentr omfentr omfdrctv omfmunge omfload omfflhn dlltool

!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)bin
SRC  = $(MYDIR)
!else
TARGET_API = OS2
ADD_COPT = -bt=os2
DEST = os2tk45$(SEP)bin
SRC  = $(MYDIR)..$(SEP)..$(SEP)link386$(SEP)
!endif


!include $(%ROOT)tools/mk/build.mk

