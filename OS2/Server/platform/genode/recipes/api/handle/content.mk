content: shared/include/os3/handlemgr.h \
         lib/mk/handle.mk \
         mk/osfree.mk

shared/include/os3/handlemgr.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

lib/mk/handle.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
