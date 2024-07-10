#!/bin/sh
#
#

# canonicalize ROOT
ROOT=`readlink -f $ROOT`/
watcom=/opt/watcom
WATCOM=$watcom
FPCPATH=/usr/lib/fpc/3.2.2
OS=LINUX
ENV=LINUX
HOST=linux
#LOG=yes
SHELL=`which sh`
REXX=`which rexx`
REXX_PATH=`which rexx`
SERVERENV=os2
ARCH=`uname -m`
TMP=/tmp
PATCH=`which patch`
WGET=`which wget`
INTERP=""

if [ "$ARCH" != "x86_64" -a "$ARCH" != "i386" ]; then
    INTERP="$ROOT/`which interp.sh` "
fi

TOOLS=$ROOT/build/bin/host/$HOST/bin
OS2TK=$ROOT/build/bin/host/$HOST/os2tk45
INCLUDE=$watcom/h:$watcom/h/dos
LIB=$watcom/lib386:$watcom/lib386/dos:$watcom/lib286:$ROOT/lib
PATH=$OS2TK/bin:$OS2TK/som/bin:$TOOLS:$watcom/binl:$FPCPATH:$ROOT:$PATH
WD_PATH=$watcom/binl
# LANG=$watcom
WIPFC=$watcom/wipfc

# list of all vars
VARS="OS SHELL REXX REXX_PATH ENV HOST WATCOM \
  ROOT OS2TK TOOLS INCLUDE LIB PATH WD_PATH LANG \
  WIPFC SERVERENV FPC ARCH INTERP _CWD LOG TMP PATCH WGET"

# export all vars
export $VARS
