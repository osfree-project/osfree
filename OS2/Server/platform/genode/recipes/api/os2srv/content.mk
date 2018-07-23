content: include/cpi_session

include/cpi_session:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/
