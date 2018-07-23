content: src/server/os2fs \
         shared/shared/server/os2fs \
         mk/osfree.mk

src/server/os2fs: shared/shared/server/os2fs
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/server/os2fs:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
