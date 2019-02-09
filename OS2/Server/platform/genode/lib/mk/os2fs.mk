include $(REP_DIR)/mk/osfree.mk

SRC_CC = fs.cc
LIBS = base libc compat

vpath %.cc $(REP_DIR)/src/lib/os2fs
