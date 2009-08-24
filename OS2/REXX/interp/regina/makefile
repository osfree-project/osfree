#
#
#
#


PROJ        = regina
DESC        = Regina REXX interpreter
DEST        = os2
OPTIONS     = caseexact
srcfiles    = $(p)regina$(e)
ADD_COPT    = $(THREADING) $(HAVE_GCI) $(DYN_CFLAGS) #-bd
ADD_LINKOPT = libpath $(PATH)..$(SEP)implib lib $(PROJ).lib

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)regina.obj: $(SRC)regina.c $(HFILES)

qwe: .symbolic
 @echo $(COPT)
