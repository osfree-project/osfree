#
# Makefile for tftp microFSD. 06/09/29 valerius
#

!include ../../../build.conf
!include ../../../mk/site.mk

FS           = tftp
ADD_ASMOPT   =
DEFINES      = -dMICROFSD_TEST -dOS2 -d__KERNEL__ -d__WATCOM__

!ifeq UNIX FALSE

DIR          = bootseq\uFSD\$(FS)
MYDIR        = $(ROOT)\$(DIR)
ADD_COPT     = $(DEFINES) -i=$(ROOT)\include\muFSD -i=. -i=..
PATH         = $(MYDIR);$(MYDIR)\..;$(MYDIR)\..\lilo
LIBPATH      = $(MYDIR)\..
LIBC_MY      = ..\$(LIBC)

!else

DIR          = bootseq/uFSD/$(FS)
MYDIR        = $(ROOT)/$(DIR)
ADD_COPT     = $(DEFINES) -i=$(ROOT)/include/muFSD -i=. -i=..
PATH         = $(MYDIR):$(MYDIR)/..:$(MYDIR)/../lilo
LIBPATH      = $(MYDIR)/..
LIBC_MY      = ../$(LIBC)

!endif

#
# tftp specific objects
#
SPEC_OBJS    = tftp.$(O)
#
# general objects
#
GEN_OBJS     = entry.$(O) start.$(O) diskread.$(O) wrappers.$(O) &
               panic.$(O) video.$(O) vsprintf.$(O) pathutil.$(O) &
               term.$(O)

OBJS         = $(GEN_OBJS) $(SPEC_OBJS)

!include ../../../mk/bootseq.mk

all: bb_$(FS) .SYMBOLIC

bb_$(FS): bb_$(FS).exe
 $(DC) $^@
 $(MKBIN) $< $^@
 $(DC) $<

bb_$(FS).exe: $(SPEC_OBJS) $(LIBC_MY)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk OPTION NODEFAULTLIBS
 @%append $^&.lnk ALIAS mu_Start=_mu_Start
 @%append $^&.lnk ALIAS mu_Terminate=_mu_Terminate
 @%append $^&.lnk ALIAS mu_Open=_mu_Open
 @%append $^&.lnk ALIAS mu_Close=_mu_Close
 @%append $^&.lnk ALIAS mu_Read=_mu_Read
 @%append $^&.lnk ALIAS _stub_mu_Open=stub_mu_Open
 @%append $^&.lnk ALIAS _stub_mu_Read=stub_mu_Read
 @%append $^&.lnk ALIAS _stub_mu_Close=stub_mu_Close
 @%append $^&.lnk ALIAS _stub_mu_Terminate=stub_mu_Terminate
 @%append $^&.lnk ALIAS parm_cs=_parm_cs
 @%append $^&.lnk ALIAS parm_ds=_parm_ds
 @%append $^&.lnk ALIAS parm_es=_parm_es
 @%append $^&.lnk ALIAS parm_ss=_parm_ss
 @%append $^&.lnk ALIAS output_com=output_com_
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk PATH $(PATH)
 @%append $^&.lnk LIBPATH $(LIBPATH)
 @%append $^&.lnk LIBRARY $(LIBC)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
