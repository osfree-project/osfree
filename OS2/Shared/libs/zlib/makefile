# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include ../../../build.conf
!include ../../../mk/site.mk

ROOT=..\..\..

32_BITS  = 1

ADD_COPT = -i=..\include

!include ../../../mk/all.mk



all: $(ROOT)\lib\zlib.lib

OBJS = adler32.obj compress.obj &
            deflate.obj infblock.obj &
            infcodes.obj inffast.obj &
            inflate.obj inftrees.obj &
            infutil.obj trees.obj &
            uncompr.obj zutil.obj


$(ROOT)\lib\zlib.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @wlib $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
