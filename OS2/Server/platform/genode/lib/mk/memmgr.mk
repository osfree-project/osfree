include $(REP_DIR)/mk/osfree.mk

SRC_C  = memmgr.c
LIBS   = libc

vpath %.c  $(OS3_DIR)/shared/lib/memmgr

SHARED_LIB = yes
