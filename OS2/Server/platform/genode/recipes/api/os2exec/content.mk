content: shared/include/os3/exec.h \
         lib/symbols/os2exec \
         mk/osfree.mk

lib/symbols/os2exec:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $(dir $@)/

shared/include/os3/exec.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $@

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
