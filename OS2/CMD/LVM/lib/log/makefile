#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = lvm2mirr
DESC     = LVM2 mirror library
DIR      = $(ROOT)$(SEP)bin$(SEP)os2
srcfiles = $(p)log$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)datastruct -i=$(MYDIR)..$(SEP)misc -i=$(MYDIR)..$(SEP)config -i=$(MYDIR)..$(SEP)commands -i=$(MYDIR)..$(SEP)log -i=$(MYDIR)..$(SEP)metadata -i=$(MYDIR)..$(SEP)device -i=$(MYDIR)..$(SEP)uuid -i=$(MYDIR)..$(SEP)display -i=$(MYDIR)..$(SEP)format_text -i=$(MYDIR)..$(SEP)activate -i=$(MYDIR)..$(SEP)mm -bd
ADD_LINKOPT  = segment   type DATA shared lib memlock.lib
DLL      = 1
OPTIONS  = manyautodata, caseexact
DLLOPT   = initglobal
EXPORTS  = init_segtype
DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
