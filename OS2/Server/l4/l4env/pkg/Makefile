PKGDIR	?= ..
L4DIR	?= $(PKGDIR)/../..

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.
SUBDIRS = os2srv os2fs execsrv libs ixf os2app

all::
	for i in $(SUBDIRS); do \
	    PWD=$(PWD)/$$i $(MAKE) -C $$i $@ $(MKFLAGS); \
	done

include $(L4DIR)/mk/subdir.mk
