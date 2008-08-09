#
# Makefile for microFSD's and stage0.
# 07/10/04 valerius
#

libsrcs      = $(p)strcmp$(e) $(p)putchar$(e) $(p)memmove$(e) $(p)aton$(e) $(p)memcmp$(e) $(p)strncpy$(e) &
               $(p)convtoascii$(e) $(p)sprintf$(e) $(p)strstr$(e) $(p)safeparsemaxint$(e) &
               $(p)index$(e) $(p)memset$(e) $(p)strcat$(e) $(p)substr$(e) $(p)isspace$(e) $(p)printf$(e) &
               $(p)tolower$(e) $(p)putchar$(e) $(p)strcpy$(e) $(p)toupper$(e) $(p)memcheck$(e) &
               $(p)putstr$(e) $(p)strlen$(e)

DIRS         =
DIR          = minilibc
RELDIR       = bootseq$(SEP)loader$(SEP)$(DIR)$(SEP)
MYDIR        = $(%ROOT)$(SEP)$(RELDIR)
PATH         = $(RELDIR)

!include $(%ROOT)/mk/loader.mk

LIBS         = $(PATH)minilibc.lib
TARGETS      = $(LIBS)

p            = $(PATH)
e            = .$$(O)
OBJS         = $+$(libsrcs)$-

$(LIBS): $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @$(DC) $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)
