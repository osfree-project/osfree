#!/bin/sh
#
#

ROOT=/mnt/data1/src/os2/boot
WATCOM=/mnt/data1/WATCOM
watcom=/mnt/data1/watcom

TOOLS=$ROOT/tools/bin
INCLUDE=$WATCOM/h:$WATCOM/h/dos:$WATCOM/h/os2
LIB=$WATCOM/lib386:$WATCOM/lib386/dos:$WATCOM/lib286:$ROOT/lib
PATH=$WATCOM/binl:$watcom/binl:$TOOLS:$ROOT:$PATH
WD_PATH=$WATCOM/binl
LANG=$WATCOM

export WATCOM ROOT TOOLS INCLUDE LIB PATH WD_PATH LANG
