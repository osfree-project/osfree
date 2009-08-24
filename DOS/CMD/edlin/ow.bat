@echo off
REM Tested on Win XP Home SP2 w/ OpenWatcom 1.5
REM builds FreeDOS's edlin 2.10c (by Gregory Pietsch:  gpietsch@comcast.net)
REM
REM rugxulo@bellsouth.net (this .BAT file's author)
REM Saturday, December 30, 2006   11:57pm
REM
REM
REM USAGE:
REM
REM = default is to build 16-bit DOS .EXE (slower, but 8086s: use this!)
REM = "/32" = DOS/4GW .EXE (uses virtual mem, runs much faster on Win XP)
REM   = see below about changing/stubbing the DOS extender w/ WDOS/X
REM
REM NOTES:
REM
REM = If environment space is low, "command /e:2048 /c edlin-ow /32"
REM = building the 16-bit .EXE works on pure DOS (thankfully): 20s on P166
REM   = so I'm guessing 32-bit .EXE will build too!  :-)
REM = I have kept the 16-bit and 32-bit flags settings separate in case
REM     they get changed/expanded later.
REM = I've not (yet) found a good way to set/use an environ. variable with
REM     the program version number.
REM
REM TODO: (hopefully within the next six months!)
REM
REM = message file for a certain popular conlang (hint: see my e-mail addy)
REM   = N.B. I'm no great scholar, but I can read it fairly well.  :-)
REM

if not exist edlin-2.10c\nul if exist edlin-2.10c.zip unzip edlin-2.10c.zip
if not exist edlin-2.10c\nul goto end
cd edlin-2.10c

REM new FreeDOS i18n support (replaces old msgs-en.h, etc. files)
copy /b msgscats.h msgs.h

REM make CONFIG.H
if not exist config.h copy /b config-h.ow config.h
if not exist config.h copy /b config-h.bc config.h
if not exist config.h goto end

REM 16-bit DOS:
REM
REM -zq = quiet, -ox = max. optimize, -bt = target OS, -mh = model huge
set FLAGS1=-i=%WATCOM%\h -fi=nl_types.h -dUSE_CATGETS -zq -ox -bt=dos -mh
set MYCC=wcc

REM 32-bit DOS:  (N.B. Use WCC386.EXE instead of WCC.EXE)
REM
REM -mf = model flat
if not "%1"=="/32" goto compile
set FLAGS1=-i%WATCOM%\h -fi=nl_types.h -dUSE_CATGETS -zq -ox -bt=dos -mf
set MYCC=wcc386

:compile
for %%f in (catgets defines dynstr edlib edlin) do %MYCC% %%f.c %FLAGS1%

REM EDLIN32 uses DOS/4GW by default:
REM   http://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/devel/c/
REM        openwatcom/1.5/ext_dos4gw.zip (210k)
REM
REM else do "stubit edlin32.exe" w/ WDOS/X 0.97 for stand-alone .EXE:
REM   http://tippach.business.t-online.de/wdosx/wdosx097.zip (690k)
REM
REM Other DOS extenders will work too but haven't been tested by me (yet).

set W1=dos4g name edlin32
if not "%1"=="/32" set W1=dos name edlin16

wlink system %W1% file catgets,defines,dynstr,edlib,edlin

:end
set FLAGS1=
set MYCC=
set W1=

REM (EOF) 