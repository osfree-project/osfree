# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = tedit
DESC = Tiny Editor
OBJ1 = $(p)box$(e) $(p)colour$(e) $(p)comm1$(e) $(p)comm2$(e) $(p)comm3$(e) $(p)comm4$(e) $(p)comm5$(e) &
       $(p)commset1$(e) $(p)commset2$(e) $(p)commsos$(e) $(p)cursor$(e) $(p)default$(e) &
       $(p)edit$(e) $(p)error$(e) $(p)execute$(e) $(p)linked$(e) $(p)column$(e) $(p)mouse$(e) $(p)memory$(e) &
       $(p)nonansi$(e) $(p)prefix$(e) $(p)parser$(e) $(p)reserved$(e) $(p)scroll$(e) $(p)show$(e) $(p)sort$(e) &
       $(p)target$(e) $(p)the$(e) $(p)util$(e) $(p)print$(e) $(p)regex$(e) $(p)alloca$(e)
OBJ2 = $(p)commutil$(e) #$(p)trace$(e)
OBJ3 = $(p)getch$(e)
OBJ4 = $(p)query$(e)
OBJ5 = $(p)thematch$(e)
OBJ6 = $(p)directry$(e) $(p)file$(e)
OBJ7 = $(p)rexx$(e)
OBJ8 = $(p)os2eas$(e) $(p)mygetopt$(e)
OBJX = $(XTRAOBJ)
srcfiles = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJX)

ADD_COPT = -i=$(MYDIR)..$(SEP)include -dOS2 -dUSE_OS2REXX # -dSTDC_HEADERS -dPROTO -dOS2 -zz -oneatx -ei -zq -mf
ADD_LINKOPT = LIB rexx,pdcurses,os2386,clib3r
STACK=65536
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
INSTALL_ADD = 1
IMPORTS = KbdGetStatus                EMXWRAP.210, &
          KbdSetStatus                EMXWRAP.211, &
          MouGetNumQueEl              EMXWRAP.313, &
          KbdPeek                     EMXWRAP.222, &
          MouReadEventQue             EMXWRAP.320, &
          KbdCharIn                   EMXWRAP.204, &
          MouFlushQue                 EMXWRAP.307, &
          KbdFlushBuffer              EMXWRAP.213, &
          MouOpen                     EMXWRAP.317, &
          MouDrawPtr                  EMXWRAP.326, &
          MouClose                    EMXWRAP.309, &
          MouSetEventMask             EMXWRAP.316

!include $(%ROOT)/mk/appsos2_cmd.mk

install_add: .SYMBOLIC
 $(CP) tedit.hlp $(DEST)
