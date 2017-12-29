TARGET = lx-dummy
LIBS = lx

all: lx.ixf

lx.ixf:
	@cd bin && ln -sf lx.lib.so lx.ixf
