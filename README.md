Русский:

Russian:

# Система сборки osFree

Сначала, запустите %root%\conf\scripts\_setup.{cmd|sh|bat}, чтобы скачать необходимые
зависимости для сборки. Но прежде всего, нужно установить wget (для случая Windows), если
он у вас еще не установлен. Для сборки нужен только OpenWatcom, FreePascal, а также Regina
REXX (но, в случае OS/2 хоста, вы можете использовать уже установленную инсталляцию REXX).

Запустите _setup для скачивания и установки системы сборки.
После окончания работы скрипта можно запустить сборку с помощью _wcc.{cmd|sh}.

Замечание. _wcc.cmd автоопределяет ОС (OS/2, win32 или win64).

В общем случае, скрипты с расширением .bat у нас обычно специфичны для windows хоста,
.cmd для OS/2 хоста, и .sh для UNIX/Linux хоста.

---

Английский:

English:

# osFree build system

First, start %root%\conf\scripts\_setup.{cmd|sh|bat}, to pull the build dependencies.
But prior to that, you need to download wget binary (in case of windows), if it
is not installed. For building these sources, you only need to install OpenWatcom,
FreePascal and Regina REXX (but, in case of OS/2 host, you can use the default REXX
interpreter, which is already in the system).

Then you need to edit %root%\conf\scripts\setvars.{sh|cmd|bat} accordingly, to set tools
paths. After which, you can start build by launching _wcc.{sh|cmd}.

Note that _wcc.cmd autodetects OS (OS/2, win32 or win64).

Generally, we use .bat scripts to be specific for windows, .cmd specific to OS/2, and
.sh specific to UNIX/Linux.

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
