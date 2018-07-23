content: src/server/os2srv \
         shared/shared/server/os2srv \
         mk/osfree.mk

src/server/os2srv: shared/shared/server/os2srv
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/server/os2srv:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
