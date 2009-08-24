$
$! build procedure for regina on vms
$! - sample environment and invoke mmk/mms
$ SET NOON
$ _V = F$VERIFY(1)
$!
$ IF P1 .EQS. "CLEAN"
$ THEN
$   DELETE/NOLOG *.OBJ;*,*.EXE;*
$   EXIT $STATUS
$ ENDIF
$!
$ DEBUG=0
$ IF P1 .EQS. "DEBUG"
$ THEN
$   DEBUG=1
$   DEBUGGING=1
$ ENDIF
$!
$ MACRO1 = "UNDEF"
$ MACRO2 = "UNDEF"
$ MACRO3 = "UNDEF"
$!
$ IF F$GETSYI("VERSION") .GTS. "V5.2"
$ THEN
$!
$   IF F$EDIT(F$GETSYI("ARCH_NAME"),"UPCASE") .EQS. "ALPHA"
$   THEN
$     MACRO1 = "ALPHA"
$   ELSE
$     MACRO1 = "VAX"
$   ENDIF
$!
$   IF F$GETSYI("VERSION") .LTS. "V6.2"
$   THEN
$     MACRO2 = "DOESNT_HAVE_UNAME"
$   ELSE
$     IF F$GETSYI("VERSION") .LES. "V7.0"
$     THEN
$       MACRO2 = "DOESNT_HAVE_UNAME"
$     ENDIF
$   ENDIF
$!
$ ELSE
$   MACRO1 = "VAX"      ! accounts for early versions of vms
$ ENDIF
$!
$ ! double quote character
$ DQ = """
$ _MAKE="MMK /DESCRIP=DESCRIP.MMS /OUTPUT=''MACRO1'_BUILD.LOG"
$ IF MACRO3 .NES. "UNDEF"
$ THEN
$   _MACRO = DQ + MACRO1 + DQ + "," + DQ + MACRO2 + DQ + "," + DQ + MACRO3 + DQ
$ ELSE
$   IF MACRO2 .NES. "UNDEF"
$   THEN
$     _MACRO = DQ + MACRO1 + DQ + "," + DQ + MACRO2 + DQ
$   ELSE
$     _MACRO = DQ + MACRO1 + DQ
$   ENDIF
$ ENDIF
$
$ IF DEBUG THEN _MACRO = _MACRO + "," + DQ + "DEBUG" + DQ
$
$ '_MAKE' /MACRO=('_MACRO')
$
$! IF F$SEARCH("REXX.EXE") .NES. "" THEN COPY REXX.EXE REXX.EXE_'MACRO1'
$! IF F$SEARCH("REGINA.EXE") .NES. "" THEN COPY REGINA.EXE REGINA.EXE_'MACRO1'
$! IF F$SEARCH("EXERCISER.EXE") .NES. "" THEN COPY EXERCISER.EXE EXERCISER.EXE_'MACRO1'
$!
$ _V = F$VERIFY(_V)     ! reset
$ EXIT $STATUS
