content: lib/mk/lx.mk \
         src/lib/ixf/lx \
         mk/osfree.mk \
         LICENSE

lib/mk/lx.mk:
	mkdir -p lib/mk
	cp $(REP_DIR)/$@ lib/mk

src/lib/ixf/lx: shared/shared/lib/ixf/lx
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/lib/ixf/lx:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk/

LICENSE:
	touch $@
