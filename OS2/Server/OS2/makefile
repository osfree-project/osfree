# Makefile for common shared libs
#

!ifndef ARCH
ARCH=$(%ENV)
!else
ARCH=$(ARCH)
!endif

PROJ = os2server_$(ARCH)
srcfiles =  $(p)io$(e)
            #$(p)mmap$(e)
            # $(p)native_dynlink$(e)

ADD_COPT = -i=..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libsos2server.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
