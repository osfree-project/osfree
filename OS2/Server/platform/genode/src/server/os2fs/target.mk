include $(REP_DIR)/mk/osfree.mk

TARGET = os2fs
SRC_CC = main.cc
SRC_C  = utility.c globals.c mountreg.c \
         api/api.c api/misc.c
INC_DIR += $(OS3_DIR)/shared/server/os2fs/api
LIBS   = libc compat

vpath %.c $(OS3_DIR)/shared/server/os2fs
