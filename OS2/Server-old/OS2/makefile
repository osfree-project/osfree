# Makefile for common shared libs
#

ARCH = $(%SERVERENV)
PROJ = os2server_$(ARCH)
TRGT     = $(PROJ).lib
srcfiles =  $(p)io$(e)
            #$(p)mmap$(e)
            # $(p)native_dynlink$(e)
UNI2H   = 1

#TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libsos2server.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
