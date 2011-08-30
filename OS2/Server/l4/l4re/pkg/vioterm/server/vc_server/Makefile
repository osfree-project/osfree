# directories we need to know
PKGDIR        ?= ../..
L4DIR         ?= $(PKGDIR)/../../../..

SYSTEMS        = x86-l4v2 arm-l4v2 amd64-l4v2

PRIVATE_INCDIR = $(PKGDIR)/server/common    \
                 $(PKGDIR)/server/vc_server

vpath %          $(PKGDIR)/server/common

DEPENDS_PKGS   = dope

SRC_C          = main.c basic_io.c clients.c term.c init.c rw.c

TARGET         = vc_server
MODE           = l4env_base
DEFAULT_RELOC  = 0x002400000
LIBS           = -ll4vfs_term_server_main-server -ll4vfs_term_server_main   \
                 -ll4vfs_name_server-server -ldope_term -lvt100 -ldope_term \
                 -ll4dope -lvscreen -ll4vfs_object_server-server            \
                 -ll4vfs_common_io_notify -ll4vfs_common_io_notify-server   \
                 -ll4vfs_term_server_internal                               \
                 -ll4vfs_term_server_internal-server                        \
                 -ll4vfs_common_io_notify-server                            \
                 -lparsecmdline -ll4vfs_select -ll4vfs_select_notify-server

include $(L4DIR)/mk/prog.mk
