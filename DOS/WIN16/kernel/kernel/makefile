#
# A Makefile for KERNEL.EXE
# (c) osFree project,
#



PROJ = krnl286
TRGT = $(PROJ).exe
DESC = Windows/286 Kernel
srcfiles = $(p)dpmildr$(e) $(p)hmemset$(e) $(p)lstrcat$(e) $(p)lstrcpy$(e) $(p)lstrlen$(e) $(p)profstrg$(e) $(p)kernel16$(e)

DIRS = startup

# defines additional options for C compiler
ADD_COPT = -zW -sg -zw -bw -bg -d3 -db -hw
ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=0
JWASM = 1


!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR)..


