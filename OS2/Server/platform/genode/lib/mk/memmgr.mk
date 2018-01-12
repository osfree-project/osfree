include $(REP_DIR)/mk/osfree.mk

SHARED_LIB = yes
SRC_C  = memmgr.c
LIBS   = libc

vpath %.c  $(OS3_DIR)/shared/lib/memmgr
