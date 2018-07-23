content: lib/mk/compat.mk \
         src/lib/compat \
         mk/osfree.mk \
         LICENSE

lib/mk/compat.mk:
	mkdir -p lib/mk
	cp $(REP_DIR)/$@ lib/mk

src/lib/compat: shared/shared/lib/compat
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/lib/compat:
	mkdir -p shared/shared/lib/compat
	cp -r $(REP_DIR)/shared/shared/lib/compat/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk/

LICENSE:
	touch $@
