#!/bin/sh
#
#

ROOT=/mnt/data1/src/os2/svn/osfree
#WATCOM=/mnt/data1/WATCOM
watcom=/mnt/data1/watcom
WATCOM=$watcom

TOOLS=$ROOT/tools/bin
INCLUDE=$watcom/h:$watcom/h/dos:$watcom/h/os2
LIB=$watcom/lib386:$watcom/lib386/dos:$watcom/lib286:$ROOT/lib
PATH=$watcom/binl:$TOOLS:$ROOT:$PATH
WD_PATH=$watcom/binl
LANG=$watcom

export WATCOM ROOT TOOLS INCLUDE LIB PATH WD_PATH LANG
