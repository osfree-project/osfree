#!/bin/sh
#
#

ROOT=/media/osfree/
#WATCOM=/mnt/data1/WATCOM
watcom=/media/watcom
WATCOM=$watcom
OS=LINUX
ENV=LINUX
SHELL=Bourne
REXX=/usr/local/bin/rexx

TOOLS=$ROOT/bin/tools
INCLUDE=$watcom/h:$watcom/h/dos:$watcom/h/os2
LIB=$watcom/lib386:$watcom/lib386/dos:$watcom/lib286:$ROOT/lib
PATH=$watcom/binl:$TOOLS:$ROOT:$PATH
WD_PATH=$watcom/binl
LANG=$watcom

export OS REXX WATCOM ROOT TOOLS INCLUDE LIB PATH WD_PATH LANG
