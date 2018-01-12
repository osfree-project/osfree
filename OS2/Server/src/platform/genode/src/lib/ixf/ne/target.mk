TARGET = ne-dummy
LIBS = ne

all: ne.ixf

ne.ixf: bin
	cd bin && ln -sf $(CURDIR)/bin/ne.lib.so ne.ixf

bin:
	@if [ ! -d $@ ]; then \
	    mkdir -p $@; \
	fi
