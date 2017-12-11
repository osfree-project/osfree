include $(REP_DIR)/mk/osfree.mk

SRC_C  = memmgr.c
LIBS   = libc

vpath %.c  $(OS3_DIR)/src/lib/memmgr
