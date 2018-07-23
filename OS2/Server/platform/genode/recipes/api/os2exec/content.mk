content: include/exec_session

include/exec_session:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/
