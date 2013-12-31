#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = more
DESC = Show stdin to stdout. One screen at one time.
#defines object file names in format objname.$(O)
srcfiles = $(p)more$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..\include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
IMPORTS     = KbdCharIn                   EMXWRAP.204, &
              KbdGetStatus                EMXWRAP.210, &
              KbdSetStatus                EMXWRAP.211, &
              KbdFlushBuffer              EMXWRAP.213
UNI2H = 1

!include $(%ROOT)/mk/appsos2_cmd.mk
