#!/bin/sh
#
#

_CWD=`pwd`
ROOT=$_CWD/
watcom=/opt/watcom
WATCOM=$watcom
FPC=/usr/bin
OS=LINUX
ENV=LINUX
SHELL=`which sh`
REXX=`which rexx`
REXX_PATH=`which rexx`
SERVERENV=os2
ARCH=`dpkg --print-architecture`

TOOLS=$ROOT/bin/tools
INCLUDE=$watcom/h:$watcom/h/os2:$watcom/h/dos:$watcom/h/win
LIB=$watcom/lib386:$watcom/lib386/dos:$watcom/lib286:$ROOT/lib
PATH=$TOOLS:$watcom/binl:$FPC/bin:$ROOT:$PATH
WD_PATH=$watcom/binl
LANG=$watcom
WIPFC=$watcom/wipfc

VARS="OS SHELL REXX REXX_PATH ENV WATCOM ROOT TOOLS INCLUDE LIB PATH WD_PATH LANG WIPFC SERVERENV FPC ARCH _CWD"

export VARS OS SHELL REXX REXX_PATH ENV WATCOM ROOT TOOLS INCLUDE LIB PATH WD_PATH LANG WIPFC SERVERENV FPC ARCH _CWD
