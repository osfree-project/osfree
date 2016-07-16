SHARED_LIB = yes
DIR = $(REP_DIR)/src/lib/ixf/ne

LIBS  += cxx libc startup
SRC_C = ne.c

INC_DIR += $(DIR)

vpath %    $(DIR)
