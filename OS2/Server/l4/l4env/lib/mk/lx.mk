SHARED_LIB = yes
DIR = $(REP_DIR)/src/lib/ixf/ne

LIBS  += cxx libc startup
SRC_C = # debug.c fixup.c alocmem.c load.c loadobj.c lx.c mod.c

INC_DIR += $(DIR)

vpath %    $(DIR)
