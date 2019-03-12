content: src/lib/os2srv/cpi.cc \
         lib/mk/os2srv.mk \
         src/lib/os2srv/target.mk \
         mk/osfree.mk

src/lib/os2srv/cpi.cc:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@

lib/mk/os2srv.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

src/lib/os2srv/target.mk:
	mkdir -p $(dir $@)
	cp -r $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
