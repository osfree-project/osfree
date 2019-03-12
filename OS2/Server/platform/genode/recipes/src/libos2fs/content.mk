content: lib/mk/os2fs.mk \
         src/lib/os2fs/fs.cc \
         src/lib/os2fs/target.mk \
         mk/osfree.mk

lib/mk/os2fs.mk:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@

src/lib/os2fs/fs.cc:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

src/lib/os2fs/target.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@
