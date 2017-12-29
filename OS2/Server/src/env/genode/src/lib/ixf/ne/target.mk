TARGET = ne-dummy
LIBS = ne

all: ne.ixf

ne.ixf:
	@cd bin && ln -sf ne.lib.so ne.ixf
