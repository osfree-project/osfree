#!/bin/sh
#
#

ROOT=/mnt/os2f/src/osfree/
#WATCOM=/mnt/data1/WATCOM
watcom=/opt/watcom
WATCOM=$watcom
FPC=/opt/fpc
OS=LINUX
ENV=LINUX
SHELL=Bourne
REXX=/usr/bin/rexx

TOOLS=$ROOT/bin/tools
INCLUDE=$watcom/h:$watcom/h/os2:$watcom/h/dos:$watcom/h/win
LIB=$watcom/lib386:$watcom/lib386/dos:$watcom/lib286:$ROOT/lib
PATH=$TOOLS:$watcom/binl:$FPC/bin:$ROOT:$PATH
WD_PATH=$watcom/binl
LANG=$watcom
WIPFC=$watcom/wipfc

export OS REXX WATCOM ROOT TOOLS INCLUDE LIB PATH WD_PATH LANG WIPFC
