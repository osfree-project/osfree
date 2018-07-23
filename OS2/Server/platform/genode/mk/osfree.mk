CC_OPT += -D__genode__

OS3_DIR := $(REP_DIR)/shared
# INC_DIR += $(OS3_DIR)/include/os2
# INC_DIR += $(OS3_DIR)/include
CC_OPT += -I$(OS3_DIR)/include/os2 -I$(OS3_DIR)/include