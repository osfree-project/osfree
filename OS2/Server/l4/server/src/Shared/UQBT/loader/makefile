# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

#
ARCH = $(%SERVERENV)

PROJ = uqbt_loader
srcfiles = &
$(p)ArchiveFile$(e) &
$(p)AutoDetectBF$(e) &
$(p)bffDump$(e) &
$(p)BinaryFile$(e) &
$(p)ElfArchiveFile$(e) &
$(p)ElfBinaryFile$(e) &
$(p)ElfDetails$(e) &
$(p)ExeBinaryFile$(e) &
$(p)HpSomBinaryFile$(e) &
$(p)PalmBinaryFile$(e) &
$(p)SymTab$(e) &
$(p)Win32BinaryFile$(e)


ADD_COPT= -d2 -db -xs
ADD_LINKOPT = debug all
DEBUG=all

!include $(%ROOT)/mk/libsos2server.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

