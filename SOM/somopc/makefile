
PROJ = somopc
TRGT = $(PROJ).exe
srcfiles =  $(p)oidll$(e) $(p)oidl$(e)

DEST = os2tk45$(SEP)som$(SEP)bin

!ifdef HOSTBLD
OS2TK = $(%OS2TK)$(SEP)
!else
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)
!endif


!include $(%ROOT)SOM/common.mk

ADD_COPT = $(ADD_COPT) -i=$(PORT_BASE)somipc$(SEP)include -i=$(PORT_BASE)somirfmt$(SEP)include
ADD_COPT = $(ADD_COPT) -i=$(OS2TK)$(SEP)som$(SEP)include 

ADD_LINKOPT = lib $(OS2TK)som$(SEP)lib$(SEP)somtk

$(PATH)oidll.obj: .symbolic
 @$(SAY) YACC     $^. $(LOG)
!ifeq UNIX TRUE
 $(verbose)$(DC) $^*.h $(BLACKHOLE)
 $(verbose)$(DC) $^*.c $(BLACKHOLE)
!else
 $(verbose)if exist $^*.h $(verbose)$(DC) $^*.h $(BLACKHOLE)
 $(verbose)if exist $^*.c $(verbose)$(DC) $^*.c $(BLACKHOLE)
!endif
 $(verbose)yacc.exe -y -d -o $(PATH)oidl.c $(PORT_BASE)somopc$(SEP)src$(SEP)oidl.y  $(LOG2)
 @$(SAY) LEX      $^. $(LOG)
!ifeq UNIX TRUE
 $(verbose)$(DC) $^@ $(BLACKHOLE)
!else
 $(verbose)if exist $^@ $(verbose)$(DC) $^@ $(BLACKHOLE)
!endif
 $(verbose)lex.exe -t $(PORT_BASE)somopc$(SEP)src$(SEP)oidl.l >$(PATH)oidll.c $(LOG2)
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC)  $(COPT)   -fr=$^*.err -fo=$(PATH)oidll.obj $(PATH)oidll.c $(LOG2)


$(PATH)oidl.obj: .symbolic
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC)  $(COPT)   -fr=$^*.err -fo=$(PATH)oidl.obj $(PATH)oidl.c $(LOG2)


