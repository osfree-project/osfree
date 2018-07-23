content: lib/mk/ne.mk \
         src/lib/ixf/ne \
         mk/osfree.mk \
         LICENSE

lib/mk/ne.mk:
	mkdir -p lib/mk
	cp $(REP_DIR)/$@ lib/mk

src/lib/ixf/ne: shared/shared/lib/ixf/ne
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/lib/ixf/ne:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk/

LICENSE:
	touch $@
