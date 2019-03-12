content: src/lib/os2app/app.cc \
         src/lib/os2app/target.mk \
         lib/mk/os2app.mk \
         mk/osfree.mk

lib/mk/os2app.mk:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@

src/lib/os2app/app.cc:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

src/lib/os2app/target.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@
