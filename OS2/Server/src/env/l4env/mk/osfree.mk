include $(REP_DIR)/build.cfg

CFLAGS += -I$(OS3_DIR)/include/os2
CFLAGS += -I$(OS3_DIR)/include

.DEFAULT_GOAL := all

#default:
#	chroot $(LENNY) @$(MAKE) $(MAKEFLAGS) all

#subdirs = $(filter-out ., `cd $(REP_DIR)/src; find . -type d | tr '\n' ' '`)

#$(subdirs):
#	if [ -d $@ ]; then \
#	    $(MAKE) -C $@ O=$(BLD_DIR) all; \
#	fi

# newer qemu
#run: $(ROOT)/cd/boot/grub/menu.lst $(OS3_DIR)/src/env/l4env/os2 \
#	$(BLD_DIR)/bin/$(arch)/$(l4api)/os2 \
#	$(REP_DIR)/tftpboot
#	qemu-system-i386 \
#		-no-kvm \
#		-m $(MEM) \
#		-boot n \
#		-netdev user,bootfile=$(GRUB),tftp=$(REP_DIR),id=net0 \
#		-device ne2k_pci,netdev=net0 \
#		-L $(REP_DIR)/tools/bootrom \
#		-display vnc=$(DISPLAY) \
#		-serial stdio \
#	| tee $(REP_DIR)/qemu.log

# old qemu from Debian 5 Lenny
run: symlinks
	qemu-system-i386 \
		-no-kqemu \
		-m $(MEM) \
		-boot n \
		-tftp $(REP_DIR) \
		-bootp $(GRUB) \
		-net nic,model=ne2k_pci,vlan=0 \
		-net user,vlan=0 \
		-L $(REP_DIR)/tools/bootrom \
		-vnc $(DISPLAY) \
		-serial stdio \
	| tee $(REP_DIR)/qemu.log

symlinks: $(ROOT)/cd/boot/grub/menu.lst \
	  $(OS3_DIR)/src/env/l4env/os2 \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/os2 \
	  $(REP_DIR)/tftpboot \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco_symbols \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco_lines \
	  $(BLD_DIR)/bin/$(arch)/$(l4api)/l4con

$(ROOT)/cd/boot/grub/menu.lst: $(OS3_DIR)/src/env/l4env/tools/menu.lst
	@if [ ! -d $(dir $@) ]; then \
	    @mkdir -p $(dir $@) \
	fi
	@ln -s $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/os2: $(OS3_DIR)/src/env/l4env/os2
	@ln -s $< $@

$(OS3_DIR)/src/env/l4env/os2: $(OS3_DIR)/filesys/os2
	@ln -s $< $@

$(REP_DIR)/tftpboot: $(ROOT)
	@ln -s $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco: $(FIASCO_BLD_DIR)/main
	@ln -s $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco_symbols: $(FIASCO_BLD_DIR)/Symbols
	@ln -s $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/fiasco_lines: $(FIASCO_BLD_DIR)/Lines
	@ln -s $< $@

$(BLD_DIR)/bin/$(arch)/$(l4api)/l4con: $(BLD_DIR)/bin/$(arch)/$(l4api)/con
	@ln -s $< $@
