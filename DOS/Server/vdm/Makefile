PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= vdm
MODE		= l4env_base
#DEFAULT_RELOC	= 0x00F00000
SYSTEMS		= x86-l4v2

SRC_C		= emu.c
PRIVATE_INCDIR	= ../../include
CFLAGS		= -fomit-frame-pointer
LIBS		= -lx86emu -lcon -lcontxt -lconstream-server

DEBUG		= 0

include $(L4DIR)/mk/prog.mk
