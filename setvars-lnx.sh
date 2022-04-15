#!/bin/sh
#
#

CWD=`pwd`
ROOT=$CWD/
#WATCOM=/mnt/data1/WATCOM
watcom=/opt/watcom
WATCOM=$watcom
FPC=/usr/bin
OS=LINUX
ENV=LINUX
SHELL=`which sh`
REXX=`which rexx`
REXX_PATH=`which rexx`
SERVERENV=os2
SEP=/

TOOLS=$ROOT/bin/tools
INCLUDE=$watcom/h:$watcom/h/os2:$watcom/h/dos:$watcom/h/win
LIB=$watcom/lib386:$watcom/lib386/dos:$watcom/lib286:$ROOT/lib
PATH=$TOOLS:$watcom/binl:$FPC/bin:$ROOT:$PATH
WD_PATH=$watcom/binl
LANG=$watcom
WIPFC=$watcom/wipfc

VARS="OS REXX REXX_PATH WATCOM ROOT TOOLS INCLUDE LIB PATH WD_PATH LANG WIPFC"

export $VARS

for var in $VARS; do
    export $var
done
