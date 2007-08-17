# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include ../../../build.conf
!include ../../../mk/site.mk

ROOT=..\..\..

32_BITS  = 1

ADD_COPT = -i=..\include

!include ../../../mk/all.mk



all: $(ROOT)\lib\lpng.lib

OBJS =pngwtran.$(O) &
      pngwutil.$(O) &
      png.$(O) &
      pngerror.$(O) &
      pnggccrd.$(O) &
      pngget.$(O) &
      pngmem.$(O) &
      pngread.$(O) &
      pngrio.$(O) &
      pngrtran.$(O) &
      pngrutil.$(O) &
      pngset.$(O) &
      pngtrans.$(O) &
      pngvcrd.$(O) &
      pngwio.$(O) &
      pngwrite.$(O)


$(ROOT)\lib\lpng.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @wlib $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
