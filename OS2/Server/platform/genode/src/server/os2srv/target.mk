include $(REP_DIR)/mk/osfree.mk

TARGET = os2srv
SRC_CC = main.cc
SRC_C  =
INC_DIR += $(OS3_DIR)/shared/server/os2srv/api
LIBS   = libc compat

vpath %.c $(OS3_DIR)/shared/server/os2srv
