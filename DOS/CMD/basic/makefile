#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ = basic
TRGT = $(PROJ).exe
DESC = BASIC and BASICA programming language
#defines object file names in format objname.$(O)
srcfiles = &
           $(p)bwb_cmd$(e) &
           $(p)bwb_exp$(e) &
           $(p)bwb_int$(e) &
           $(p)bwb_str$(e) &
           $(p)bwbasic$(e) &
           $(p)bwx_tty$(e) &
           $(p)bwb_cnd$(e) &
           $(p)bwb_fnc$(e) &
           $(p)bwb_prn$(e) &
           $(p)bwb_tbl$(e) &
           $(p)bwd_cmd$(e) &
           $(p)bwb_dio$(e) &
           $(p)bwb_inp$(e) &
           $(p)bwb_stc$(e) &
           $(p)bwb_var$(e) &
           $(p)bwd_fun$(e)

# defines additional options for C compiler
ADD_COPT = -ml -zc -ox -DHAVE_MSDOS=1 -D__WATCOM__ #-bc -ox -q -wx -za 

# additions to install target
INSTALL_ADD = 1

!include $(%ROOT)/mk/appsdos.mk

install_add: $(DEST)$(SEP)$(PROJ).com &
             $(DEST)$(SEP)$(PROJ)a.com &
             $(DEST)$(SEP)gw$(PROJ).exe .SYMBOLIC

$(PATH)$(PROJ1).com:

$(DEST)$(SEP)$(PROJ).com: $(PATH)$(PROJ).exe
 @$(SAY) INST     $^. $(LOG)
 @$(CP) $< $^@ $(BLACKHOLE)

$(DEST)$(SEP)$(PROJ)a.com: $(PATH)$(PROJ).exe
 @$(SAY) INST     $^. $(LOG)
 @$(CP) $< $^@ $(BLACKHOLE)

$(DEST)$(SEP)gw$(PROJ).exe: $(PATH)$(PROJ).exe
 @$(SAY) INST     $^. $(LOG)
 @$(CP) $< $^@ $(BLACKHOLE)
