#
# A Makefile for MINSTALL
# (c) osFree project,
# author, date
#

PROJ = genin
TRGT = $(PROJ).dll
DESC = Multimedia Installer GENIN/Stub-DLL
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)genin$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)include
# ADD_LINKOPT = lib clib3r
UNI2H   = 1
DLL     = 1
DLLOPT  = initinstance terminstance op manyautodata
# defines additional options for C compiler
exports = &
          InitGenin, &
          GeninReplacementIdentifier

DEST        = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
