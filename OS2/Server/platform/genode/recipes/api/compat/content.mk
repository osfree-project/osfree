content: lib/symbols/compat \
         shared/include/os3 \
         shared/include/os3/io.h \
         shared/include/os3/err.h \
         shared/include/os3/dataspace.h \
         shared/include/os3/rm.h \
         shared/include/os3/cfgparser.h \
         shared/include/os3/path.h \
         shared/include/os3/token.h

lib/symbols/compat:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/$@ $(dir $@)/

shared/include/os3:
	mkdir -p include/os3

shared/include/os3/io.h:
	cp $(REP_DIR)/$@ include/os3;

shared/include/os3/err.h:
	cp $(REP_DIR)/$@ include/os3;

shared/include/os3/dataspace.h:
	cp $(REP_DIR)/$@ include/os3;

shared/include/os3/rm.h:
	cp $(REP_DIR)/$@ include/os3;

shared/include/os3/cfgparser.h:
	cp $(REP_DIR)/$@ include/os3;

shared/include/os3/path.h:
	cp $(REP_DIR)/$@ include/os3;

shared/include/os3/token.h:
	cp $(REP_DIR)/$@ include/os3;
