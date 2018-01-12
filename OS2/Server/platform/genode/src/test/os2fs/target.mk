include $(REP_DIR)/mk/osfree.mk

TARGET = test-os2fs
SRC_C = main.c
SRC_CC = ../shared/skel.cc
LIBS = libc compat os2fs

vpath %.c $(OS3_DIR)/src/test/os2fs
