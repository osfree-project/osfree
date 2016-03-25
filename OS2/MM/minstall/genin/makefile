#
# A Makefile for MINSTALL
# (c) osFree project,
# author, date
#

PROJ = genin
DESC = Multimedia Installer GENIN/Stub-DLL
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)genin$(e)
DLL     = 1
DLLOPT  = initinstance terminstance op manyautodata
# defines additional options for C compiler
exports = &
          InitGenin, &
          GeninReplacementIdentifier

ADD_LINKOPT = lib clib3r

!include $(%ROOT)/mk/appsos2_cmd.mk
