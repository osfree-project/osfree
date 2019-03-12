content: lib/symbols/handle \
         shared/include/os3/handlemgr.h \
         mk/osfree.mk

lib/symbols/handle:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $(dir $@)/

shared/include/os3/handlemgr.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
