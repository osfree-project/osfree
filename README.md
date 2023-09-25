-=== osFree, Open Source OS/2 Clone ===-
osFree(OS/3): The reincarnation of OS/2.

[![Build status](https://github.com/osfree-project/osfree/actions/workflows/build_win.yml/badge.svg)](https://github.com/osfree-project/osfree/actions?query=workflow%3Abuild_win.yml)

[![Build status](https://github.com/osfree-project/osfree/actions/workflows/build_lnx.yml/badge.svg)](https://github.com/osfree-project/osfree/actions?query=workflow%3Abuild_lnx.yml)

Английский:

English:

# Getting the source code

osFree sources consist of the main Git repository and several submodules.
The main git repo is osfree, and submodules are:

FamilyAPI, MacroLib, fat32, jfs, os3 and others.

Hence, you need to clone the main osfree repo first, and then
pull the submodules:

> git clone https://github.com/osfree-project/osfree.git

> git submodule update --init --recursive

> git submodule update --remote --recursive

# osFree build system

First, start %root%\\conf\\scripts\\_setup.{cmd|sh|bat}, to pull the build dependencies.

Start _setup.* for downloading the build tools.
After finishing the script work, you can start build with _wcc.{sh|cmd}.

Note that _wcc.cmd autodetects OS (OS/2, win32 or win64).

Generally, we use .bat scripts specific for windows, .cmd specific to OS/2, and
.sh specific to UNIX/Linux.

---

Русский:

Russian:

# Получение исходников

Исходные коды osFree состоят из главного Git-репозитория и нескольких
субмодулей. Главный репозиторий: osfree и субмодули: 

FamilyAPI, MacroLib, fat32, jfs, os3 и другие.

Поэтому нужно сначала скачать главный репозиторий osfree, и затем подтянуть 
субмодули:

> git clone https://github.com/osfree-project/osfree.git

> git submodule update --init --recursive

> git submodule update --remote --recursive

# Система сборки osFree

Сначала, запустите %root%\\conf\\scripts\\_setup.{cmd|sh|bat}, чтобы скачать необходимые
зависимости для сборки.

Запустите _wcc.{sh|cmd} для
сборки.

Замечание. _wcc.cmd автоопределяет ОС (OS/2, win32 или win64) автоматически.

В общем случае, скрипты с расширением .bat у нас обычно специфичны для windows хоста,
.cmd для OS/2 хоста, и .sh для UNIX/Linux хоста.


(c) osFree project, 2002-2022
http://www.osfree.org/
