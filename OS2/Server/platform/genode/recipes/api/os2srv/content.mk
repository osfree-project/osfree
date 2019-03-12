content: shared/include/os3/cpi.h \
         lib/symbols/os2srv \
         mk/osfree.mk

lib/symbols/os2srv:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $(dir $@)/

shared/include/os3/cpi.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
