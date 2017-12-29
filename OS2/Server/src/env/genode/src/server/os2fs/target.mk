include $(REP_DIR)/mk/osfree.mk

TARGET = os2fs
SRC_CC = main.cc # api/rpc.cc
SRC_C  = utility.c globals.c mountreg.c \
         api/api.c misc.c fprov.c
INC_DIR += $(OS3_DIR)/src/server/os2fs/api
LIBS   = libc compat

vpath %.c $(OS3_DIR)/src/server/os2fs
