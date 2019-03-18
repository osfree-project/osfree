content: src/server/tar_rom

src/server/tar_rom:
	mkdir -p $@
	cp -r $(GENODE_DIR)/repos/os/$@/* $@/
