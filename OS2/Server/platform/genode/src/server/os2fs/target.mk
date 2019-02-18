include $(REP_DIR)/mk/osfree.mk

TARGET = os2fs
CC_CXX_WARN_STRICT = 
SRC_CC = main.cc
SRC_C  = utility.c globals.c mountreg.c initdone.c \
         api/api.c api/misc.c
INC_DIR += $(OS3_DIR)/shared/server/os2fs/api
LIBS   = base libc compat os2srv

vpath %.c $(OS3_DIR)/shared/server/os2fs
