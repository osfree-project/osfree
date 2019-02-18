PKGDIR		?= ../../..
REP_DIR		?= $(PKGDIR)

include $(REP_DIR)/mk/osfree.mk

TARGET		= libos2compat.a
# MODE		= l4env
# SYSTEMS		= x86-l4v2 # arm-l4v2 amd64-l4v2
SRC_C		= log.c # fileprov.c thread.c dataspace.c rm.c err.c segment.c \
		  cfgparser.c env.c path.c string.c token.c semaphore.c lock.c
LIBS		=
CFLAGS          += -I$(PKGDIR)/include

vpath %.c = $(OS3_DIR)/shared/lib/compat

include $(L4DIR)/mk/lib.mk
