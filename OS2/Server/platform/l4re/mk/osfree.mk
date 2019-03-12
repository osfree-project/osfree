include $(REP_DIR)/build.cfg

cdefs = -D__l4re__

cdefs += -I$(REP_DIR)/include
cdefs += -I$(OS3_DIR)/include/os2
cdefs += -I$(OS3_DIR)/include

CFLAGS += $(cdefs)
O=$(BLD_DIR)

.DEFAULT_GOAL := all

run: symlinks
	$(MAKE) -C $(L4DIR) O=$(BLD_DIR) \
		MODULES_LIST=$(BLD_DIR)/conf/modules.list \
		QEMU_OPTIONS="-m $(MEM) -display vnc=$(DISPLAY) \
		$(GDB) $(ACCEL) -serial stdio" qemu

symlinks: $(OS3_DIR)/platform/l4re/os2 \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/os2 \
	  $(BLD_DIR)/conf                      \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/l4con

$(OS3_DIR)/platform/l4re/os2: $(OS3_DIR)/filesys/os2
	@ln -sf $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/os2: $(OS3_DIR)/platform/l4re/os2
	@ln -sf $< $@

$(BLD_DIR)/conf: $(OS3_DIR)/platform/l4re/tools
	@ln -sf $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco: $(FIASCO_BLD_DIR)/fiasco
	@ln -sf $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/l4con: $(BLD_DIR)/bin/$(arch)/$(l4api)/con
	@ln -sf $< $@
