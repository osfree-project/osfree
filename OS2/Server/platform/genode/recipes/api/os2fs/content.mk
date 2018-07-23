content: include/fs_session

include/fs_session:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/
