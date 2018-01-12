include $(REP_DIR)/mk/osfree.mk

TARGET = test-os2fs
SRC_CC = main.cc
LIBS = libc compat os2fs
