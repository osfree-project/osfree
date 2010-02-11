# Makefile for common shared libs

32_BITS  = 1

PROJ = tools_shared
DIRS = z #idl
LIBS = $(%ROOT)build$(SEP)lib$(SEP)z.lib $(%ROOT)build$(SEP)lib$(SEP)idl.lib

TARGETS  = subdirs $(PATH)$(PROJ).lib
TARGET   = all

!include $(%ROOT)/mk/libs.mk

$(PATH)$(PROJ).lib: $(LIBS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
