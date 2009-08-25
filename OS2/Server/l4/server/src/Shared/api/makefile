#
#
#
#


PROJ     = apistub
DESC     = Kernel Abstraction Layer API
srcfiles = $(p)apistub$(e)
ADD_COPT =  -d2 -db -i=$(MYDIR).. -i=$(MYDIR)
ADD_LINKOPT  = debug all op nod lib clib3r.lib segment   type DATA shared
ARCH = $(%SERVERENV)

TARGETS  = $(PATH)$(PROJ).lib

!include $(%ROOT)/mk/libsos2server.mk

$(TARGETS): $(OBJS)
 echo $(%INCLUDE)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library

