content: lib/symbols/memmgr \
         shared/include/os3/memmgr.h

lib/symbols/memmgr:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $(dir $@)/

shared/include/os3/memmgr.h:
	mkdir -p include/os3
	cp $(REP_DIR)/$@ include/os3
