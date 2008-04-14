#
# Makefile for microFSD's and stage0.
# 07/10/04 valerius
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS      = 1       # Use 32-bit C compiler
DEFINES      = -dNO_DECOMPRESSION # -dSTAGE1_5 -dNO_BLOCK_FILES -dOS2 -d__WATCOM__
ADD_COPT     = -s $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=..$(SEP)include -i=. -i=..
ADD_ASMOPT   = $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=include -i=. -i=..

DIR          = minilibc
MYDIR        = $(ROOT)$(SEP)bootseq$(SEP)preldr$(SEP)$(DIR)$(SEP)
PATH         = $(MYDIR)$(PS)$(MYDIR)$(SEP)minilibc

!include $(%ROOT)/mk/bootseq.mk

MINILIBC_OBJS         = strcmp.$(O) putchar.$(O) memmove.$(O) aton.$(O) memcmp.$(O) strncpy.$(O) &
			convtoascii.$(O) sprintf.$(O) strstr.$(O) safeparsemaxint.$(O) &
			index.$(O) memset.$(O) strcat.$(O) substr.$(O) isspace.$(O) printf.$(O) &
			tolower.$(O) putchar.$(O) strcpy.$(O) toupper.$(O) memcheck.$(O) &
                        putstr.$(O) strlen.$(O)

all: minilibc.lib

minilibc.lib: $(MINILIBC_OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @$(DC) $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(DC) $(TARGETS)
 