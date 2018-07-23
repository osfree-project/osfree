content: src/app/os2app \
         mk/osfree.mk

src/app/os2app:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
