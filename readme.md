![osFree Logo © Alex Kramer](http://osfree.org/doku/lib/tpl/adoradark-osf/images/osfree.png)
osFree Logo © Alex Kramer

# osFree, Open Source OS/2 Clone

osFree(OS/3): The reincarnation of OS/2.

## Русский/Russian: Система сборки osFree.

Поправьте  build.conf  -- рядом лежат версии build.conf для OS/2, Linux и
Windows.  Для  работы  скрипта tools\mkbin.cmd нужен интерпретатор языка REXX.
Для   Linux  и  Windows  это  может  быть 
[Regina  REXX](http://regina-rexx.sourceforge.net/)  --  самый  популярный
интерпретатор.  И  самое главное,  нужен компилятор OpenWatcom! ;) Автор пробовал 
как версию 1.3, так и более  новую  версию 1.5. Для установки переменных окружения 
перед компиляцией используйте скрипт setvars.cmd -- для OS/2, .bat -- для Windows, 
и .sh -- для Linux.

## English/Английский: osFree build system

For  compilation,  fix  build.conf  to  suit  your  system.  (There are 3
versions  of  build.conf  for  OS/2, Linux and Windows in the root directory.
Also developer-specific config exists with names like build-valerius.conf,
build-prokushev.conf, etc).
OpenWatcom  compiler  is used for development. I tried OpenWatcom versions 1.3
as  well  as  1.5.  Also,  for *.cmd scripts a REXX language interpreter is
needed. For Linux and Windows users [Regina  REXX](http://regina-rexx.sourceforge.net/)
may  be  used.  Before compilation, for setting  environment  variables,  use  
setvars.cmd  --  for OS/2, .bat -- for Windows, and .sh -- for Linux.

Валерий В. Седлецкий / Valery V. Sedletski, aka [valerius](https://github.com/valerius2k)
© [osFree project](http://osfree.org)

