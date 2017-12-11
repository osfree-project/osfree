PKGDIR   ?= ../..
L4DIR    ?= $(PKGDIR)/../../../..

SYSTEMS   = x86-l4v2 amd64-l4v2 arm-l4v2

TARGET    = libcon_term.a
MODE      = l4env_minimal
CLIENTIDL = l4con.idl
IDL_PKGDIR = $(OBJ_BASE)/pkg/l4con

SRC_C     = open.c close.c evh.c helper.c color.c cursor.c

WARNINGS += $(call checkcc, -Wno-pointer-sign, )

include $(L4DIR)/mk/lib.mk
