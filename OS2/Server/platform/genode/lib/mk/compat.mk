include $(REP_DIR)/mk/osfree.mk

SRC_CC = log.cc fileprov.cc err.cc dataspace.cc rm.cc
SRC_C  = cfgparser.c env.c path.c token.c string.c
LIBS = libc stdcxx

vpath %.c  $(OS3_DIR)/src/lib/compat
vpath %.cc $(REP_DIR)/src/lib/compat
