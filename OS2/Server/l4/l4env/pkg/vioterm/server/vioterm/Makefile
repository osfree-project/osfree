# directories we need to know
PKGDIR        ?= ../..
L4DIR         ?= $(PKGDIR)/../../../..

SYSTEMS        = x86-l4v2 arm-l4v2 amd64-l4v2

PRIVATE_INCDIR = $(PKGDIR)/server/common

vpath %          $(PKGDIR)/server/common

SRC_C          = init.c term.c clients.c main.c basic_io.c rw.c

TARGET         = vioterm
MODE           = l4env_minimal
#DEFAULT_RELOC  = 0x01400000
DEFAULT_RELOC  = 0x01450000
LIBS           = -u putchar -u puts -u printf                                \
                 -lcon -lvt100 -lcon_term -lconstream-server                 \
                 -ll4vfs_object_server-server -ll4vfs_name_server            \
                 -ll4vfs_term_server_main-server -ll4vfs_name_server-server  \
                 -ll4vfs_name_space_provider -ll4vfs_extendable              \
                 -ll4vfs_basic_io -ll4vfs_basic_name_server                  \
                 -ll4vfs_common_io -ll4vfs_common_io_notify                  \
                 -ll4vfs_term_server_internal                                \
                 -ll4vfs_term_server_main                                    \
                 -ll4vfs_term_server_internal-server                         \
                 -ll4vfs_common_io_notify-server                             \
                 -lparsecmdline -ll4vfs_select -ll4vfs_select_notify-server

include $(L4DIR)/mk/prog.mk
