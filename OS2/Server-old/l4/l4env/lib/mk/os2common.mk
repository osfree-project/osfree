DIR = $(REP_DIR)/src/lib/common

LIBS   += cxx libc
SRC_CC  = io.cc # path.c string.c token.c env.c

INC_DIR += $(DIR)

vpath %    $(DIR)
