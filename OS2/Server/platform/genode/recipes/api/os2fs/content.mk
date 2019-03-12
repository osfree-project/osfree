content: shared/include/os3/fs.h \
         lib/mk/os2fs.mk \
         mk/osfree.mk

shared/include/os3/fs.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

lib/mk/os2fs.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
