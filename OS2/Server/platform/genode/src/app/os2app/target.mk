include $(REP_DIR)/mk/osfree.mk

TARGET = os2app
CC_CXX_WARN_STRICT =
SRC_CC = main.cc
SRC_C  = dl.c api/api.c kal/util.c kal/start.c kal/kal.c
LIBS = base libc compat os2srv os2fs os2exec

ifeq ($(filter-out $(SPECS),x86_32),)
	SRC_C += kal/arch/x86_32/tramp.c
endif
ifeq ($(filter-out $(SPECS),x86_64),)
	SRC_C += kal/arch/x86_64/tramp.c
endif
ifeq ($(filter-out $(SPECS),arm),)
	SRC_C += kal/arch/arm/tramp.c
endif

vpath %.c $(OS3_DIR)/shared/app/os2app

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
	@cd bin && ln -sf $(CURDIR)/app/os2app/kal.map kal.map

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
