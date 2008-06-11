# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include -dENABLE_IJG -dENABLE_PNG -dENABLE_TIF


!include $(%ROOT)/mk/all.mk



all: $(ROOT)$(SEP)lib$(SEP)gbm.lib .SYMBOLIC

OBJS = gbm.$(O) gbmpbm.$(O)  gbmpgm.$(O) gbmppm.$(O) gbmpnm.$(O)  &
          gbmbmp.$(O) gbmtga.$(O)  gbmkps.$(O) gbmiax.$(O) gbmpcx.$(O)  &
          gbmtif.$(O) gbmlbm.$(O)  gbmvid.$(O) gbmgif.$(O) gbmxbm.$(O)  &
          gbmspr.$(O) gbmpsg.$(O)  gbmgem.$(O) gbmcvp.$(O) gbmjpg.$(O)  &
          gbmpng.$(O) gbmxpm.$(O)  gbmxpmcn.$(O) gbmhelp.$(O) gbmmap.$(O)

$(ROOT)$(SEP)lib$(SEP)gbm.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @wlib $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
