!
! regina on OpenVMS
! MMS/MMK-makefile
!
!.IGNORE         ! ignore errors, continue processing, like "make -k"
!
.FIRST
        @ write sys$output f$fao("!/==!AS !%D==", -
                "Processing DESCRIP.MMS (Regina) begins at", 0)
.LAST
        @ write sys$output f$fao("!/==!AS !%D==", -
                "Processing DESCRIP.MMS (Regina) concludes at", 0)
!
.IFDEF DEBUGGING
CC=CC/DECC/DEBUG
CFLAGS=/NOOPTIMIZE/STANDARD=RELAXED_ANSI89/WARNINGS=DISABLE=(IMPLICITFUNC,cvtdiftypes) -
        /INCLUDE_DIRECTORY=[]/NAMES=SHORTENED/DEFINE=(VMS,HAVE_DIV,HAVE_ARPA_INET_H)/NOLIST/OBJECT=$(MMS$TARGET_NAME).OBJ
LINK=LINK/DEBUG
.ELSE
CC=CC/DECC/NODEBUG
CFLAGS=/OPTIMIZE/STANDARD=RELAXED_ANSI89/WARNINGS=DISABLE=(IMPLICITFUNC) -
        /INCLUDE_DIRECTORY=[]/NAMES=SHORTENED/DEFINE=(VMS,HAVE_DIV,HAVE_ARPA_INET_H)/OBJECT=$(MMS$TARGET_NAME).OBJ
LINK=LINK/NODEBUG
.ENDIF
!
! platform dependant, ...
!
.IFDEF ALPHA
.ENDIF
!
.IFDEF VAX
.ENDIF
LINKFLAGS=/MAP
!
!OBJ1=builtin.obj,cmath.obj,cmsfuncs.obj,convert.obj,
OBJ1=builtin.obj,client.obj,cmath.obj,cmsfuncs.obj,convert.obj,
OBJ2=dbgfuncs.obj,debug.obj,doscmd.obj,envir.obj,error.obj,expr.obj,
!OBJ3=extlib.obj,files.obj,funcs.obj,
OBJ3=files.obj,funcs.obj,
OBJ4=mt_notmt.obj,rexxbif.obj,instore.obj,extstack.obj,os2funcs.obj,
OBJ5=interp.obj,interprt.obj,lexsrc.obj,library.obj,macros.obj,memory.obj,
OBJ6=misc.obj,options.obj,parsing.obj,rexxext.obj,rexxsaa.obj,shell.obj,
!OBJ6=misc.obj,options.obj,parsing.obj,rexxext.obj,shell.obj,
OBJ7=signals.obj,stack.obj,strings.obj,strmath.obj,tracing.obj,unxfuncs.obj,
.IFDEF DOESNT_HAVE_UNAME
OBJ8=uname.obj,
.ELSE
OBJ8=
.ENDIF
OBJ9=vmsfuncs.obj,vmscmd.obj,variable.obj,wrappers.obj,yaccsrc.obj,alloca.obj,arxfuncs.obj

LIB=LIBRARY
LIBFLAGS=/CREATE regina.olb
!
all : rexx.exe, regina.exe, execiser.exe
!
rexx.exe :      rexx.obj, -
        $(OBJ1)$(OBJ2)$(OBJ3)$(OBJ4)$(OBJ5)$(OBJ6)$(OBJ7)$(OBJ8)$(OBJ9)
        @ write sys$output "Linking $(MMS$TARGET) "
        $(LINK) $(LINKFLAGS) $(MMS$SOURCE_LIST)
        @ write sys$output "Done (linking)."
!
regina.exe :    regina.obj,regina.olb
        @ write sys$output "Linking $(MMS$TARGET) "
        $(LINK) $(LINKFLAGS) regina.obj,regina.olb/LIBRARY
        @ write sys$output "Done (linking)."
!
execiser.exe :  execiser.obj,regina.olb
        @ write sys$output ""
        @ write sys$output "Linking $(MMS$TARGET) "
        $(LINK) $(LINKFLAGS) execiser.obj,regina.olb/LIBRARY
        @ write sys$output "Done (linking)."
!
regina.olb :    drexx.obj,rexxsaa.obj,client.obj -
        $(OBJ1)$(OBJ2)$(OBJ3)$(OBJ4)$(OBJ5)$(OBJ6)$(OBJ7)$(OBJ8)$(OBJ9)
        @ write sys$output "Createing library $(MMS$TARGET) "
        $(LIB) $(LIBFLAGS) $(MMS$SOURCE_LIST)
        @ write sys$output "Done (library)."
!
clean : rexx.exe, regina.exe
        @ delete/nolog  rexx.exe;*,regina.exe;*
        @ delete/nolog *.obj;*
        @ write sys$output "Done (cleaning)."
!
alloca.obj : alloca.c
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
builtin.obj :   builtin.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
client.obj :    client.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
cmath.obj :     cmath.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
cmsfuncs.obj :  cmsfuncs.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
convert.obj :   convert.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
dbgfuncs.obj :  dbgfuncs.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
debug.obj :     debug.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
doscmd.obj :    doscmd.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
envir.obj :     envir.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
error.obj :     error.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
execiser.obj :  execiser.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
expr.obj :      expr.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
extlib.obj :    extlib.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
extstack.obj :    extstack.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
files.obj :     files.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
funcs.obj :     funcs.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
instore.obj :    instore.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
interp.obj :    interp.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
interprt.obj :  interprt.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
lexsrc.obj :    lexsrc.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
library.obj :   library.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
macros.obj :    macros.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
memory.obj :    memory.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
misc.obj :      misc.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
mt_notmt.obj :  mt_notmt.c, rexx.h mt.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
options.obj :   options.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
os2funcs.obj :   os2funcs.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
parsing.obj :   parsing.c, rexx.h, strings.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
r2perl.obj :    r2perl.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
regina.obj :    regina.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
rexx.obj :      rexx.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
drexx.obj :     rexx.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) /DEFINE=RXLIB $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
rexxbif.obj :   rexxbif.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
rexxext.obj :   rexxext.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
rexxsaa.obj :   rexxsaa.c, configur.h, defs.h, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
shell.obj :     shell.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
signals.obj :   signals.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
stack.obj :     stack.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
strings.obj :   strings.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
strmath.obj :   strmath.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
tracing.obj :   tracing.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
unxfuncs.obj :  unxfuncs.c, utsname.h, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
variable.obj :  variable.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
vmscmd.obj :    vmscmd.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
vmsfuncs.obj :  vmsfuncs.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
wrappers.obj :  wrappers.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
yaccsrc.obj :   yaccsrc.c, rexx.h
        @ write sys$output ""
        @ write sys$output "Compiling $(MMS$SOURCE) "
        $(CC) $(CFLAGS) $(MMS$SOURCE)
        @ write sys$output "Done (compiling)."
! -- end of descrip.mms
