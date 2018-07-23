content: src/server/os2exec \
         shared/shared/server/os2exec \
         mk/osfree.mk

src/server/os2exec: shared/shared/server/os2exec
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

shared/shared/server/os2exec:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
