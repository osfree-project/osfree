include $(REP_DIR)/mk/osfree.mk

TARGET = os2exec
SRC_CC = main.cc ixfutil.cc
SRC_C  = ixfmgr.c modmgr.c api/api.c
INC_DIR += $(OS3_DIR)/src/server/os2exec/api
LIBS   = libc compat

vpath %.c $(OS3_DIR)/src/server/os2exec
