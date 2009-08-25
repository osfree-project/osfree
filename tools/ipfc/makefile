#
# A Makefile for mapsym
# (c) osFree project,
# prokushev, 2008/02/27
#
#

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = ipfc
DESC = IPF Compiler

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe .SYMBOLIC $(DIR)$(SEP)$(PROJ).sym

#fpc386  -o<x>  change the name of the executable produced to <x>
#On GNU/Linux this file is an ELF file without the exe extension.

$(DIR)$(SEP)$(PROJ).exe: lex.pas yacc.pas ipfc.pas
 $(PC) -Sg2 $(PCOPT) -o$(PROJ).exe $(PROJ)
 $(CP) $(PROJ).exe $(DIR)

lex.pas: ipfc.l
  plex ipfc.l lex.pas

yacc.pas: ipfc.y
  pyacc ipfc.y yacc.pas

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
