content: shared/include \
         include/platform \
         include/genode_env.h \
         mk/osfree.mk

shared/include:
	mkdir -p include/os3
	cp -r $(REP_DIR)/$@/os3/* include/os3/
	cp -r $(REP_DIR)/$@/os2/* include/

include/platform:
	mkdir -p include/platform
	cp -r $(REP_DIR)/$@/* include/platform

include/genode_env.h:
	cp $(REP_DIR)/$@ include

mk/osfree.mk:
	mkdir mk
	cp $(REP_DIR)/$@ mk
