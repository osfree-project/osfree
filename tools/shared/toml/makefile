TARGET_CLASS = LIBRARY
LIBS = ccl

!ifndef TARGETBLD
SRCDIR     = $(MYDIR)
!else
SRCDIR = $(%ROOT)tools$(SEP)shared$(SEP)toml$(SEP)
TARGET_API      = OS2
TARGET_SUBCLASS = STATIC
!endif

ADD_COPT  = -i=$(SRCDIR)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.c: $(SRCDIR)
