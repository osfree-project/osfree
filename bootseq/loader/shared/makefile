#
# Makefile for microFSD's and stage0.
# 07/10/04 valerius
#

srcfiles     = $(p)strcmp$(e) $(p)putchar$(e) $(p)memmove$(e) $(p)aton$(e) $(p)memcmp$(e) $(p)strncpy$(e) &
               $(p)convtoascii$(e) $(p)sprintf$(e) $(p)strstr$(e) $(p)safeparsemaxint$(e) &
               $(p)index$(e) $(p)memset$(e) $(p)strcat$(e) $(p)substr$(e) $(p)isspace$(e) $(p)printf$(e) &
               $(p)tolower$(e) $(p)putchar$(e) $(p)strcpy$(e) $(p)toupper$(e) $(p)memcheck$(e) &
               $(p)putstr$(e) $(p)strlen$(e) $(p)i8ls$(e) $(p)u8m$(e) $(p)istable$(e)

!include $(%ROOT)/mk/libs.mk

library      = $(PATH)ldr_shared.lib
TARGETS      = library $(library)
ADD_COPT     = -s $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include -i=$(MYDIR)..$(SEP)include -i=. -i=..

$(TARGETS): $(OBJS)
