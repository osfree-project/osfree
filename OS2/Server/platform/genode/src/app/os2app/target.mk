include $(REP_DIR)/mk/osfree.mk

TARGET = os2app
CC_CXX_WARN_STRICT =
SRC_CC = main.cc kal/thread.cc
SRC_C  = initdone.c api/api.c \
         kal/util.c kal/start.c kal/kal.c kal/dl.c
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
map: $(CURDIR)/app/os2app/os2app bin app
	echo $(CURDIR)
	@(if [ -f $(CURDIR)/os2app ]; then \
	nm $(CURDIR)/os2app | grep 'Kal' | awk '{printf "0x%s %s\n", $$1, $$3}' >$(CURDIR)/os2app.1; \
	else \
	touch $(CURDIR)/os2app; touch $(CURDIR)/os2app.1; \
	fi && wc -l $(CURDIR)/os2app.1 | awk '{print $$1}' >$(CURDIR)/kal.map && \
	cat $(CURDIR)/os2app.1 >>$(CURDIR)/kal.map)
	# @cd ../..
	@cd bin && ln -sf $(CURDIR)/kal.map kal.map

bin:
	@if [ ! -d $@ ]; then \
	    mkdir -p $@; \
	fi

app:
	@if [ ! -d $@ ]; then \
	    mkdir -p $@; \
	fi

$(CURDIR)/app/os2app/os2app:
	@if [ ! -d $(dir $@) ]; then \
	    mkdir -p $(dir $@); \
	fi
