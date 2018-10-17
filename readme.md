Русский:

Russian:

# Система сборки osFree

Сделайте копию build.conf -- рядом лежат версии build.conf для OS/2, Linux и
Windows -- и настройте параметры.  Для  работы  скрипта tools\mkbin.cmd нужен
интерпретатор языка REXX. Для   Linux  и  Windows  это  может  быть  Regina 
REXX  --  самый  популярный интерпретатор.   Берется на http://regina-rexx.sourceforge.net/.
И  самое главное,  нужен компилятор OpenWatcom! ;) Скачать можно на http://openwatcom.org.
Для установки переменных окружения перед компиляцией
используйте скрипт setenv{.cmd -- для OS/2, .bat -- для Windows, и .sh -- для
Linux}. Также для сборки потребуется Free Pascal (http://www.freepascal.org) для
ряда утилит. OS2 Toolkit 4.5 также необходим для сборки ряда кода.

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

