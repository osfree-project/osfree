content: shared/include/os3/fs.h \
         lib/symbols/os2fs \
         mk/osfree.mk

lib/symbols/os2fs:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $(dir $@)/

shared/include/os3/fs.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
