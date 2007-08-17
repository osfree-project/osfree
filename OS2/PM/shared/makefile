# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

ROOT=../../..

!include ../../../build.conf
!include ../../../mk/site.mk


32_BITS  = 1

ADD_COPT = -bm -od -mf -sg -d2 -mf

!include ../../../mk/all.mk

all: .SYMBOLIC
  $(MAKE) $(MAKEOPT) -f makeclient clean
  $(MAKE) $(MAKEOPT) -f makeclient all
  $(MAKE) $(MAKEOPT) -f makeserver clean
  $(MAKE) $(MAKEOPT) -f makeserver all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
