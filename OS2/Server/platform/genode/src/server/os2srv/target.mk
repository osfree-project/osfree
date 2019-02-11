include $(REP_DIR)/mk/osfree.mk

TARGET = os2srv
CC_CXX_WARN_STRICT = 
SRC_CC = main.cc exec.cc
SRC_C  = os2app.c api/api.c api/cfg.c processmgr.c sysinit.c
INC_DIR += $(OS3_DIR)/shared/server/os2srv/api
LIBS   = base libc compat os2srv os2app handle

vpath %.c $(OS3_DIR)/shared/server/os2srv
