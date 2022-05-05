#
# A Makefile for HDPMI16
# (c) osFree project,
# author, date
#
PROJ       = hdpmi16
TRGT       = $(PROJ).exe
DESC       = 286 MS-DOS Extender
srcfiles   = $(p)a20gate$(e) $(p)clients$(e) $(p)except$(e) $(p)hdpmi$(e) $(p)heap$(e) $(p)helpers$(e) $(p)i31deb$(e) $(p)i31dos$(e) $(p)i31fpu$(e) &
$(p)i31int$(e) $(p)i31mem$(e) $(p)i31sel$(e) $(p)i31swt$(e) $(p)init$(e) $(p)int13api$(e) $(p)int21api$(e) $(p)int2fapi$(e) &
$(p)int2xapi$(e) $(p)int31api$(e) $(p)int33api$(e) $(p)int41api$(e) $(p)intxxapi$(e) $(p)movehigh$(e) $(p)pagemgr$(e) &
$(p)putchr$(e) $(p)putchrr$(e) $(p)switch$(e) $(p)vxd$(e)
JWASM      = 1
ADD_ASMOPT = -D?32BIT=0 -c -Cp -Sg 

!include $(%ROOT)/mk/appsdos.mk
