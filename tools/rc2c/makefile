DESCRIPTION = RC2C
SOURCES = rc2c rc2c_emit rc2c_parse rc2c_lex
ADD_COPT = -i=$(MYDIR)..$(SEP)include
LIBS = tools_shared

!include $(%ROOT)tools/mk/build.mk

$(PATH)rc2c_parse.c: $(MYDIR)rc2c_parse.y

$(PATH)rc2c_lex.c: $(MYDIR)rc2c_lex.l

.c: $(PATH)

#.h: $(PATH)
