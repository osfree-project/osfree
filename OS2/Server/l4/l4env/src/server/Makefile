PKGDIR	?= ..
L4DIR	?= $(PKGDIR)/../../..

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.
SERVERS  = os2srv os2fs os2exec
MISC     = os2app ixf
DIRS     = include idl lib server src
TARGET   = os2fs os2srv os2exec os2app

all clean::
	for j in $(DIRS); do \
	    for i in $(SERVERS); do \
		if [ -d $$i/$$j ]; then \
		    PWD=$(PWD)/$$i/$$j $(MAKE) -C $$i/$$j $@ $(MKFLAGS); \
		fi \
	    done \
	done
	for i in $(MISC); do \
	    PWD=$(PWD)/$$i $(MAKE) -C $$i $@ $(MKFLAGS); \
	done

include $(L4DIR)/mk/subdir.mk
