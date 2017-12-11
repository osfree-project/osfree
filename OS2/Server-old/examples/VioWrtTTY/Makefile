#
# $Id: Makefile 11 2007-02-26 19:08:41Z l4check $
#

PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

TARGET	= VioWrtTTY_test
DEFAULT_RELOC = 0x00930000
MODE	= sigma0
SRC_C	= VioWrtTTY.c
LIBS	= -los2server
L4LIBS	=  -static -lmain -lnames -ll4util -ll4sys

LOGTAG	= $(addprefix ",$(addsuffix ",$(TARGET)))

CPPFLAGS+= -D__L4__ -DLOG_TAG='$(LOGTAG)'

include $(L4DIR)/mk/prog.mk
