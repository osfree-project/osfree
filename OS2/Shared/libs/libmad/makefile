# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = libmad
TRGT = $(PROJ).lib
#DIRS = dll
ADD_COPT = -dHAVE_CONFIG_H -dOPT_OCCURACY -dFPM_INTEL -d_MSC_VER -i=$(MYDIR)

srcfiles = &
 $(p)version$(e) $(p)fixed$(e) $(p)bit$(e) $(p)timer$(e) $(p)stream$(e) &
 $(p)frame$(e) $(p)synth$(e) $(p)decoder$(e) $(p)layer12$(e) $(p)layer3$(e) $(p)huffman$(e) 

!include $(%ROOT)tools/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib # subdirs

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

qwe: .symbolic
 @echo $(ADD_COPT)
