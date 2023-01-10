# Makefile for common shared libs

32_BITS  = 1

PROJ = tools_shared
TRGT = $(PROJ).lib
DIRS = z
LIBS = $(%ROOT)build$(SEP)lib$(SEP)z.lib
PLATFORM = host$(SEP)$(%HOST)$(SEP)

!include $(%ROOT)tools/mk/libs.mk

$(PATH)$(PROJ).lib: $(LIBS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
