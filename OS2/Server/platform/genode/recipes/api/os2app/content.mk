content: shared/include/os3/app.h \
         lib/symbols/os2app \
         mk/osfree.mk

lib/symbols/os2app:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $(dir $@)/

shared/include/os3/app.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
