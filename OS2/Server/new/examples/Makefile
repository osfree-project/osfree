PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../..

#DEPENDS_PKGS    = l4con

include $(L4DIR)/mk/Makeconf

TARGET		= parser
MODE            = loader
SYSTEMS		= x86-l4v2 arm-l4v2 amd64-l4v2
LIBS		= -lgeneric_fprov -lparsecmdline
#LIBS		= -ldoscalls -lloader.s -lcontxt -lconstream-server -lrtc -lc_be_time -lrtc -lgeneric_fprov -lparsecmdline

SRC_C		= parser.c

include $(L4DIR)/mk/prog.mk
