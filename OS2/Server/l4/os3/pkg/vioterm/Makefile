# directories we need to know
PKGDIR  = .
L4DIR  ?= $(PKGDIR)/../../../..

TARGET  = server lib include examples idl

# include subdir role
include $(L4DIR)/mk/subdir.mk

src: lib include
lib: include idl

examples: lib include
