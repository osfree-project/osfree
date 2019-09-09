#####################################################################
#  @(#)makefile32.inc	1.6 
#
#  Copyright (c) 1995-1997, Willows Software Inc.  All rights reserved.
#
#  Sample makefile driver, it is included from the individual makefile.
#  This shows how to build an arbitrary source.  See the samples
#  directory for example uses of this file.
#
#####################################################################

include $(TWINDIR)/Makefile.config

WINRT	= -ltwin32 $(TWIN_LIBS)

all:	$(project)

$(project):   $(components) $(libraries) 
	${CC} ${LDFLAGS} -o $(project) $(components) $(linklibs) $(WINRT)

$(resource).res.c: $(resource).rc $(resource_depend)
	$(RC) $(TWIN_IFLAGS) -DWIN32 -o $@ $(resource).rc

clean: clean-libs
	rm -f *.o $(project) \
	   $(components) \
	   $(resource).res.c \
	   core

