# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

DESCRIPTION = Parser generator
SOURCES  = LR0 allocate closure conflicts &
           derives files getargs gram lalr lex main &
           nullable output print reader reduce symtab &
           warshall getopt getopt1
INSTALL_ADD = 1
ADD_COPT = -i=..$(SEP)include -dHAVE_CONFIG_H
STACKSIZE = 65536

TARGET_API = HOST
TARGET_BITS = 32

!include $(%ROOT)tools/mk/build.mk

#TARGETS = inst

install_add: $(MYDIR)bison.simple $(MYDIR)bison.hairy .symbolic # $(PATH)$(TRGT)
 @$(CP) $(MYDIR)bison.simple $(DEST) $(BLACKHOLE)
 @$(CP) $(MYDIR)bison.hairy $(DEST) $(BLACKHOLE)
 @$(CP) $(PATH)$(TRGT) $(DEST) $(BLACKHOLE)


aaa: .symbolic
 @echo _gen_std: $(_gen_std)
 @echo _std_ext: $(_std_ext)
 @echo TARGET_EXT: $(TARGET_EXT)
 @echo TARGETS: $(TARGETS)
