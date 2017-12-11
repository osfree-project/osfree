DIR = $(REP_DIR)/src/lib/compat

LIBS   += cxx libc
SRC_CC  = log.cc

INC_DIR += $(DIR)

vpath %    $(DIR)
