content: shared/include/os3/app.h \
         lib/mk/os2app.mk \
         mk/osfree.mk

shared/include/os3/app.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

lib/mk/os2app.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
