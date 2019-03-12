content: lib/mk/os2exec.mk \
         src/lib/os2exec/exec.cc \
         src/lib/os2exec/target.mk \
         mk/osfree.mk

lib/mk/os2exec.mk:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@

src/lib/os2exec/exec.cc:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@

src/lib/os2exec/target.mk:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@
