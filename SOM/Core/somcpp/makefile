# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = somcpp
TRGT = $(PROJ).exe
DESC = SOM Compiler Preprocessor
srcfiles = $(p)main$(e) $(p)directive$(e) $(p)eval$(e) $(p)expand$(e) &
           $(p)support$(e) $(p)system$(e) $(p)mbchar$(e)

!ifdef HOSTBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)bin

SRC = $(MYDIR)..$(SEP)..$(SEP)
!else
DEST = os2tk45$(SEP)som$(SEP)bin

ADD_COPT = -u__LINUX__ -d__OS2__
SRC = $(MYDIR)..$(SEP)
!endif

#Notes:
# awk lex and yacc files not added to repository

ADD_COPT += -i=..$(SEP)include -sg -d2
STACKSIZE = 6553500

!ifdef HOSTBLD
!include $(%ROOT)tools/mk/tools.mk
!else
UNI2H = 1
!include $(%ROOT)tools/mk/appsos2.mk
!endif

.c: $(SRC)somcpp
.h: $(SRC)somcpp
