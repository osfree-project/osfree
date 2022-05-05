#
# A Makefile for KERNEL.EXE
# (c) osFree project,
#



PROJ = kernel
TRGT = $(PROJ).exe
DESC = Windows Real Mode Kernel
srcfiles = $(p)dpmildr$(e) $(p)hmemset$(e) $(p)lstrcat$(e) $(p)lstrcpy$(e) $(p)lstrlen$(e) $(p)profstrg$(e) $(p)kernel16$(e)

ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=0 -D?REAL=1
JWASM = 1


!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR)..


