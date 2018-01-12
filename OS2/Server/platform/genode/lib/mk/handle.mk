include $(REP_DIR)/mk/osfree.mk

SRC_C  = handlemgr.c
LIBS   = libc

vpath %.c  $(OS3_DIR)/shared/lib/handle
