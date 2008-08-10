#
# A Makefile for UNIAPI
# (c) osFree project,
#

PROJ = uni2h
DESC = Unified Interfaces to C/C++ header files converter

32_BITS  = 1
TARGETS = inst

!include $(%ROOT)/mk/all.mk

#fpc386  -o<x>  change the name of the executable produced to <x>
#On GNU/Linux this file is an ELF file without the exe extension.

#$(DIR)$(SEP)$(PROJ).exe: .symbolic
# $(PC) -S2h $(PCOPT) -o$(PROJ).exe $(PROJ)
# $(CP) $(PROJ).exe $(DIR)

inst: $(PROJ).exe # $(PROJ).sym
 $(CP) $< $(FILESDIR)$(SEP)tools
 @cd $(ROOT)$(SEP)UNI
 @$(MAKE) $(MAKEOPT) all
 @cd $(ROOT)$(SEP)tools$(SEP)uniapi
