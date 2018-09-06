REM REX35EMX.BAT -- .BAT to build DOS/EMX w/o Make (short cmdlines ftw!)
REM Tested with EMX 0.9d, GCC 2.8.1, Regina 3.5 (on Vista Home Premium 32-bit SP2)

if exist rexx.exe goto end

set OPT=-w -O -DNDEBUG -funsigned-char -Wno-char-subscripts
set SRC=../

REM ============================================================
REM Add these to top of configur.h before beginning!

if exist myemx.h goto config

echo #define DOS >myemx.h
echo #define EMX >>myemx.h
echo #define NO_EXTERNAL_QUEUES >>myemx.h
echo #define REGINA_VERSION_DATE "31 Dec 2009" >>myemx.h
echo #define REGINA_VERSION_MAJOR "3" >>myemx.h
echo #define REGINA_VERSION_MINOR "5" >>myemx.h
echo #define REGINA_VERSION_SUPP "" >>myemx.h

:config
copy %SRC%configur.h configur.h~
:: sed -e "1s/.*/#include \"myemx.h\"\n&/" configur.h~ >configur.h
copy /b myemx.h + configur.h~ configur.h >NUL
if not exist configur.h goto end
REM ============================================================

:begin
gcc %OPT% -c %SRC%funcs.c
gcc %OPT% -c %SRC%builtin.c
gcc %OPT% -c %SRC%error.c
gcc %OPT% -c %SRC%variable.c
gcc %OPT% -c %SRC%interprt.c
gcc %OPT% -c %SRC%debug.c
gcc %OPT% -c %SRC%dbgfuncs.c
gcc %OPT% -c %SRC%memory.c
gcc %OPT% -c %SRC%parsing.c
gcc %OPT% -c %SRC%files.c
gcc %OPT% -c %SRC%misc.c
gcc %OPT% -c %SRC%unxfuncs.c
gcc %OPT% -c %SRC%arxfuncs.c
gcc %OPT% -c %SRC%cmsfuncs.c
gcc %OPT% -c %SRC%os2funcs.c
gcc %OPT% -c %SRC%shell.c
gcc %OPT% -c %SRC%rexxext.c
gcc %OPT% -c %SRC%stack.c
gcc %OPT% -c %SRC%tracing.c
gcc %OPT% -c %SRC%interp.c
gcc %OPT% -c %SRC%cmath.c
gcc %OPT% -c %SRC%convert.c
gcc %OPT% -c %SRC%strings.c
gcc %OPT% -c %SRC%library.c
gcc %OPT% -c %SRC%strmath.c
gcc %OPT% -c %SRC%signals.c
gcc %OPT% -c %SRC%macros.c
gcc %OPT% -c %SRC%envir.c
gcc %OPT% -c %SRC%expr.c
gcc %OPT% -c %SRC%mt_notmt.c
gcc %OPT% -c %SRC%instore.c
gcc %OPT% -c -DYYMAXDEPTH=10000 %SRC%yaccsrc.c
gcc %OPT% -c %SRC%lexsrc.c
gcc %OPT% -c %SRC%options.c
gcc %OPT% -c %SRC%os_other.c
gcc %OPT% -c %SRC%wrappers.c
gcc %OPT% -c %SRC%rexx.c
gcc %OPT% -c %SRC%nosaa.c
REM gcc -o rexx funcs.o builtin.o error.o variable.o interprt.o debug.o \
REM dbgfuncs.o memory.o parsing.o files.o misc.o unxfuncs.o arxfuncs.o \
REM cmsfuncs.o os2funcs.o shell.o rexxext.o stack.o tracing.o interp.o \
REM cmath.o convert.o strings.o library.o strmath.o signals.o macros.o \
REM envir.o expr.o mt_notmt.o instore.o yaccsrc.o lexsrc.o options.o \
REM os_other.o wrappers.o rexx.o nosaa.o
gcc %OPT% -o rexx *.o
emxbind rexx -acm
emxbind -s rexx.exe

gcc %OPT% -c %SRC%execiser.c
gcc %OPT% -c %SRC%client.c
gcc %OPT% -c -DRXLIB %SRC%rexx.c
gcc %OPT% -c %SRC%rexxsaa.c
REM ar cr regina.a funcs.o builtin.o error.o variable.o interprt.o debug.o \
REM dbgfuncs.o memory.o parsing.o files.o misc.o unxfuncs.o arxfuncs.o \
REM cmsfuncs.o os2funcs.o shell.o rexxext.o stack.o tracing.o interp.o \
REM cmath.o convert.o strings.o library.o strmath.o signals.o macros.o \
REM envir.o expr.o mt_notmt.o instore.o yaccsrc.o lexsrc.o options.o \
REM os_other.o wrappers.o client.o drexx.o rexxsaa.o
ar rvs regina.a *.o

gcc %OPT% -o execiser execiser.o regina.a
emxbind execiser -acm

gcc %OPT% -c %SRC%msgcmp.c
gcc %OPT% -o msgcmp msgcmp.o
emxbind msgcmp -acm
msgcmp %SRC%en.mts %SRC%pt.mts %SRC%no.mts %SRC%de.mts %SRC%es.mts %SRC%pl.mts %SRC%tr.mts %SRC%sv.mts

:end
set OPT=

