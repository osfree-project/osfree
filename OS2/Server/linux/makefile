# Makefile for common shared libs
#

ARCH = $(%SERVERENV)
PROJ = os2server_$(ARCH)
srcfiles =  $(p)io$(e)
            #$(p)mmap$(e)
            # $(p)native_dynlink$(e)

TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libsos2server.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
