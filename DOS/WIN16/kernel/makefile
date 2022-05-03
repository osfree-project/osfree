#
# A Makefile for KERNEL.EXE
# (c) osFree project,
#

PROJ = kernel
TRGT = $(PROJ).exe
DESC = Windows Kernel
srcfiles = $(p)version$(e) $(p)hmemset$(e) $(p)lstrcat$(e) $(p)lstrcpy$(e) $(p)lstrlen$(e) $(p)profstrg$(e)

# defines additional options for C compiler
ADD_COPT = -zW -sg -zw -bw -bg -d3 -db -hw

!include $(%ROOT)/mk/appsw16.mk
