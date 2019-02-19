include $(REP_DIR)/mk/osfree.mk

CC_CXX_WARN_STRICT =
SHARED_LIB = yes
SRC_C = debug.c fixup.c load.c \
        loadobj.c lx.c mod.c allocmem.c
LIBS = base libc compat

vpath %.cc $(REP_DIR)/src/lib/ixf/lx
vpath %.c  $(OS3_DIR)/shared/lib/ixf/lx
