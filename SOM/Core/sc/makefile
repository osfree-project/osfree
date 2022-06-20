# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = sc
TRGT = $(PROJ).exe
DESC = SOM Compiler
srcfiles = $(p)sc$(e)
DEST = os2tk45$(SEP)som$(SEP)bin

!ifeq %OS OS2
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS WIN32
OPTS = -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_HPP -dHAVE_CONFIG_H
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS WIN64
OPTS = -bm -br -dWIN32_LEAN_AND_MEAN -d_WIN32 -dHAVE_CONFIG_HPP -dHAVE_CONFIG_H
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS DOS
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)dos
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS LINUX
OPTS = -d_PLATFORM_UNIX_ -dHAVE_CONFIG_HPP -dHAVE_CONFIG_H
defs = $(%WATCOM)$(SEP)lh
ADD_LINKOPT = lib clib3r
!endif

ADD_COPT = -dNDEBUG -i=$(defs) -i=$(MYDIR)..$(SEP)contrib$(SEP)include -bc $(OPTS)
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!include $(%ROOT)/mk/tools.mk

.c: $(MYDIR)..$(SEP)contrib$(SEP)sc$(SEP)src

.h: $(MYDIR)..$(SEP)contrib$(SEP)include

#.$(O): $(PATH)

