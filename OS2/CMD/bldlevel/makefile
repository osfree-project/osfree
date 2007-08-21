# $Id: makefile,v 1.4 2004/08/16 04:51:09 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

ROOT=..$(SEP)..$(SEP)..
DIR = $(ROOT)$(SEP)bin$(SEP)os2
PROJ = bldlevel
DESC = Build level display facility

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe

$(DIR)$(SEP)$(PROJ).exe: $(PROJ).exe
  $(CP) $(PROJ).exe $(DIR)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 cd 0
 $(DC) /n *.*
 cd ..
 rmdir 0

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
