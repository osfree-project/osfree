content: shared/include/os3/cpi.h \
         lib/mk/os2srv.mk \
         mk/osfree.mk

shared/include/os3/cpi.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

lib/mk/os2srv.mk:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
