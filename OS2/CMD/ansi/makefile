#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = ansi
TRGT = $(PROJ).exe
DESC = Control utility for ANSI
#defines object file names in format $(p)objname$(e)
# msg.obj is temporary here. We must use MSGBIND tool here. Normal flow is to use MSGEXTRT, then MKMSGF and later - MSGBIND
srcfiles = $(p)ansi$(e) $(p)msg$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)..$(SEP)include
ADD_ASMOPT = -I=$(PATH)
NODEFAULTLIBS = 1
UNI2H = 1
JWASM = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk

$(PATH)msg.obj: msg.asm $(PATH)msg.inc

$(PATH)msg.inc: $(PATH)$(PROJ).txt
  $(verbose)$(SAY) MKMSGF   $^.
  $(verbose)$(MC) $[@ $(PATH)msg.inc -Q -C -I$(MYDIR)$(PS)$(%OS2TK)$(SEP)h

$(PATH)$(PROJ).txt: $(PATH)$(PROJ).ext
  $(verbose)$(SAY) MSGEXTRT $^.
  $(verbose)$(ME) $< $^@

$(PATH)$(PROJ).ext: .symbolic
 @%create $^@
 @%append $^@ >msg.asm
 @%append $^@ <$(%ROOT)build$(SEP)bin$(SEP)os2$(SEP)system$(SEP)oso001.msg
 @%append $^@ DOS1705
 @%append $^@ DOS1706
 @%append $^@ DOS1001
