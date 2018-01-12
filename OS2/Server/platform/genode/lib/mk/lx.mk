include $(REP_DIR)/mk/osfree.mk

SHARED_LIB = yes
SRC_C = debug.c fixup.c load.c loadobj.c lx.c mod.c
SRC_CC = l4alloc_mem.cc
LIBS = libc compat

vpath %.cc $(REP_DIR)/src/lib/ixf/lx
vpath %.c  $(OS3_DIR)/shared/lib/ixf/lx
