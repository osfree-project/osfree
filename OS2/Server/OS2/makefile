# Makefile for common shared libs

PROJ = os2server_arch
srcfiles = $(p)io$(e) #$(p)mmap$(e)
#           $(p)native_dynlink$(e)

ADD_COPT = -i=..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libs.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
