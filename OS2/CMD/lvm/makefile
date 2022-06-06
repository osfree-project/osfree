# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = lvm
TRGT = $(PROJ).exe
DESC = Enhanced OW assembler
srcfiles = $(p)Bad_Block_Relocation$(e) &
$(p)BootManager$(e) &
$(p)constant$(e) &
$(p)CRC$(e) &
$(p)disk$(e) &
$(p)DiskIO$(e) &
$(p)Dlist$(e) &
$(p)Drive_Linking$(e) &
$(p)engine$(e) &
$(p)getsectors$(e) &
$(p)Handle_Manager$(e) &
$(p)help$(e) &
$(p)interface$(e) &
$(p)logging$(e) &
$(p)lvm$(e) &
$(p)LVM_Cli$(e) &
$(p)Partition_Manager$(e) &
$(p)Pass_Thru$(e) &
$(p)show$(e) &
$(p)strings$(e) &
$(p)user$(e) &
$(p)volume$(e) &
$(p)menu$(e) &
$(p)Volume_Manager$(e)

ADD_COPT = -ecs -dNDEBUG -i=$(MYDIR)alvm$(SEP)include
ADD_LINKOPT = lib clib3r
STACKSIZE = 0x20000
#UNI2H = 1

!include $(%ROOT)/mk/appsos2_cmd.mk

.c: $(MYDIR)alvm

.h: $(MYDIR)alvm$(SEP)include
