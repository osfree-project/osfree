CC_OPT += -D__genode__

ifeq ($(filter-out $(SPECS),x86_64),)
	CC_OPT += -D__64bit__
endif

OS3_DIR := $(REP_DIR)/shared
CC_OPT += -I$(OS3_DIR)/include/os2
CC_OPT += -I$(OS3_DIR)/include
