include $(REP_DIR)/mk/osfree.mk

TARGET = os2app
SRC_CC = main.cc
LIBS = base

all: map

.PHONY: map
map: app/os2app bin app app/os2app
	@(cd app/os2app && \
	if [ -f os2app ]; then \
	nm os2app | grep 'Kal' | awk '{printf "0x%s %s\n", $$1, $$3}' >os2app.1; \
	else touch os2app; touch os2app.1; fi && \
	wc -l os2app.1 | awk '{print $$1}' >kal.map && \
	cat os2app.1 >>kal.map)
	@cd ../..
	@cd bin && ln -sf ../app/os2app/kal.map kal.map

bin:
	@if [ ! -d $@ ]; then \
	    mkdir -p $@; \
	fi

app:
	@if [ ! -d $@ ]; then \
	    mkdir -p $@; \
	fi

app/os2app:
	@if [ ! -d $@ ]; then \
	    mkdir -p $@; \
	fi
