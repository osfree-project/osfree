content: lib/mk/handle.mk \
         lib/symbols/handle \
         src/lib/handle \
         mk/osfree.mk

lib/mk/handle.mk:
	mkdir -p lib/mk
	cp $(REP_DIR)/$@ lib/mk

lib/symbols/handle:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ lib/symbols

src/lib/handle: shared/shared/lib/handle
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/lib/handle:
	mkdir -p shared/shared/lib/handle
	cp -r $(REP_DIR)/shared/shared/lib/handle/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk/
