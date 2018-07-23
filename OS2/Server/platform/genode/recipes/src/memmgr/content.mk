content: lib/mk/memmgr.mk \
         src/lib/memmgr \
         mk/osfree.mk \
         LICENSE

lib/mk/memmgr.mk:
	mkdir -p lib/mk
	cp $(REP_DIR)/$@ lib/mk

src/lib/memmgr: shared/shared/lib/memmgr
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/lib/memmgr:
	mkdir -p shared/shared/lib/memmgr
	cp -r $(REP_DIR)/shared/shared/lib/memmgr/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk/

LICENSE:
	touch $@
