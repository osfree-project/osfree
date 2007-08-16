#
# Makefile for OS/3 loader muFSD's common code.
# 06/09/29 valerius
#

DIR = muFSD

!include ../build.conf
!include ../mk/site.mk

DEFINES    = -dOS2 -d__KERNEL__ -d__NO_LILO__ -dMICROFSD -dMICROFSD_TRACE

!ifeq UNIX FALSE

MYDIR      = $(ROOT)\$(DIR)
ADD_COPT   = $(DEFINES) -i=$(ROOT)\include\muFSD -i=.
ADD_ASMOPT = $(DEFINES)

!else

MYDIR      = $(ROOT)/$(DIR)
ADD_COPT   = $(DEFINES) -i=$(ROOT)/include/muFSD -i=.
ADD_ASMOPT = $(DEFINES)

!endif

DIRS       = lilo ext2 # fat isofs tftp dos3ldr

OBJS       = entry.$(O) start.$(O) diskread.$(O) wrappers.$(O) &
             panic.$(O) video.$(O) vsprintf.$(O) pathutil.$(O) &
             term.$(O)

!include ../mk/bootseq.mk

all: $(OBJS) $(LIBC) .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

$(LIBC): $(LIBDIR)/$(LIBC)
 $(SAY) Making $(LIBC) ... $(LOG)
 $(CP) $< $^@ $(LOG)
 $(LIB) $^@ -*cstart.obj $(LOG)
 $(DC)  cstart.$(O)
 $(DC)  $^&.bak

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
