#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ = basic
DESC = BASIC and BASICA programming language
#defines object file names in format objname.$(O)
srcfiles = $(p)bwbasic$(e) &
           $(p)bwb_cmd$(e) &
           $(p)bwb_cnd$(e) &
           $(p)bwb_dio$(e) &
           $(p)bwb_elx$(e) &
           $(p)bwb_exp$(e) &
           $(p)bwb_fnc$(e) &
           $(p)bwb_inp$(e) &
           $(p)bwb_int$(e) &
           $(p)bwb_mth$(e) &
           $(p)bwb_ops$(e) &
           $(p)bwb_par$(e) &
           $(p)bwb_prn$(e) &
           $(p)bwb_stc$(e) &
           $(p)bwb_str$(e) &
           $(p)bwb_tbl$(e) &
           $(p)bwb_tcc$(e) &
           $(p)bwb_var$(e) &
           $(p)bwx_tty$(e)

           #$(p)bwx_ncu$(e) # link this for ncurses use

           #$(p)renum$(e)
           #$(p)bwx_iqc$(e) &

# defines additional options for C compiler
ADD_COPT = -ml
#STACKSIZE = 65535000

# additions to install target
INSTALL_ADD = 1

!include $(%ROOT)/mk/appsdos.mk

install_add: .SYMBOLIC
 $(CP) $(PATH)$(PROJ).exe $(DEST)$(SEP)$(PROJ).com
 $(CP) $(PATH)$(PROJ).exe $(DEST)$(SEP)$(PROJ)a.com
 $(CP) $(PATH)$(PROJ).exe $(DEST)$(SEP)gw$(PROJ).exe
 $(CP) $(PATH)$(PROJ).sym $(DEST)$(SEP)$(PROJ).sym
 $(CP) $(PATH)$(PROJ).sym $(DEST)$(SEP)$(PROJ)a.sym
 $(CP) $(PATH)$(PROJ).sym $(DEST)$(SEP)gw$(PROJ).sym
