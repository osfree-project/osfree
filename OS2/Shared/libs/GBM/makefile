# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..\include -dENABLE_IJG -dENABLE_PNG -dENABLE_TIF


!include $(%ROOT)/mk/all.mk



all: $(ROOT)\lib\gbm.lib .SYMBOLIC

OBJS = gbm.obj gbmpbm.obj  gbmpgm.obj gbmppm.obj gbmpnm.obj  &
          gbmbmp.obj gbmtga.obj  gbmkps.obj gbmiax.obj gbmpcx.obj  &
          gbmtif.obj gbmlbm.obj  gbmvid.obj gbmgif.obj gbmxbm.obj  &
          gbmspr.obj gbmpsg.obj  gbmgem.obj gbmcvp.obj gbmjpg.obj  &
          gbmpng.obj gbmxpm.obj  gbmxpmcn.obj gbmhelp.obj gbmmap.obj

$(ROOT)\lib\gbm.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @wlib $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
