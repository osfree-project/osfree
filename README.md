Русский:

Russian:

# Система сборки osFree

Запустите _setup.bat для скачивания и установки системы сборки.
После окончания работы скрипта можно запустить сборку с помощью _build.cmd

---

Английский:

English:

# osFree build system

For  compilation,  fix  build.conf  to  suit  your  system.  (There are 3
versions  of  build.conf  for  OS/2, Linux and Windows in the root directory.
Also developer-specific config exists with names like build-valerius.conf,
build-prokushev.conf, etc).
OpenWatcom  compiler  is used for development. I tried OpenWatcom versions 1.3
as  well  as  1.5.  Also,  for *.cmd scripts a REXX language interpreter is
needed.      For      Linux      and     Windows     users     Regina     REXX
(http://regina-rexx.sourceforge.net/)  may  be  used.  Before compilation, for
setting  environment  variables,  use  setvars{.cmd  --  for OS/2, .bat -- for
Windows, and .sh -- for Linux}.

Валерий В. Седлецкий, aka valerius,
Valery V. Sedletski, aka valerius,
(c) osFree project,
http://www.osfree.org/

--=={-- osFree, Open Source OS/2 Clone --}==--
osFree(OS/3): The reincarnation of OS/2.

# Many thanks to those projects:

Regina REXX - our Classic REXX replacement http://regina-rexx.sourceforge.net/index.html

THE - our TEDIT replacement http://hessling-editor.sourceforge.net/index.html

And lot of other things from Mark Hessling. His products allowed us not to reinvent the wheel.

OS2Linux project as a base for CPI implementation on top of Fiasco microkernel

FreePM project as base of PM reimplementation

DROPS as base of most Personality Neutral Servers

L4Linux as Linux personality core

osFree/CMD project as base for most of command line tools

FreeLDR project which was evolved in full featured kernel loader

GRUB which was good codebase for FreeLDR project

MTDORB used as SOM CPP base

somFree used as SOM replacement base

GBM used as core of Image Format I/O Procedures

JPGIO used as core of Image Format I/O Procedures

ZLIB used in many parts

LIBPNG used in many parts

LIBJPEG used in many parts

GDLIB used in FreePM

LIBTIFF used in many parts

Fiasco reused as microkernel

PParser from FPK used as UNI2H tool base

TWIN library as win16 base

MINSTALL by KIEWITZ

MMImage Pak as Image File Format I/O Procedures

MMAudio Pak 2 as Audio File Format I/O Procedures

FreeDos parts as DOS subsystem base

Ralf Brown documentation (Interrupt list). Great source of DOS and DOS VM information.


and other projects here. Please add it if we missed one.
