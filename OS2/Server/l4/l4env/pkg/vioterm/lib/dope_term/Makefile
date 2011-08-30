PKGDIR   ?= ../..
L4DIR    ?= $(PKGDIR)/../../../..

SYSTEMS   = x86-l4v2 arm-l4v2 amd64-l4v2

DEPENDS_PKGS = dope

TARGET    = libdope_term.a
MODE      = l4env_minimal
CLIENTIDL = 

SRC_C     = scrollbar.c openclose.c draw.c callback.c event.c \
			cursor.c

include $(L4DIR)/mk/lib.mk
