#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = lvm
DESC = Logical volume management tool
#defines object file names in format objname.$(O)
srcfiles = $(p)lvm$(e) $(p)dumpconfig$(e) $(p)formats$(e) $(p)lvchange$(e) $(p)lvconvert$(e) &
           $(p)lvcreate$(e) $(p)lvdisplay$(e) $(p)lvextend$(e)  &
           $(p)lvmchange$(e) $(p)lvmdiskscan$(e) $(p)lvreduce$(e) $(p)lvremove$(e) &
           $(p)lvrename$(e) $(p)lvresize$(e) $(p)lvscan$(e) $(p)pvchange$(e) $(p)pvck$(e) $(p)pvcreate$(e) &
           $(p)pvdisplay$(e) $(p)pvmove$(e) $(p)pvremove$(e) $(p)pvresize$(e) $(p)pvscan$(e) $(p)reporter$(e) &
           $(p)segtypes$(e) $(p)toollib$(e) $(p)vgcfgbackup$(e) $(p)vgcfgrestore$(e) $(p)vgchange$(e) &
           $(p)vgck$(e) $(p)vgconvert$(e) $(p)vgcreate$(e) $(p)vgdisplay$(e) $(p)vgexport$(e) $(p)vgextend$(e) &
           $(p)vgimport$(e) $(p)vgmerge$(e) $(p)vgmknodes$(e) $(p)vgreduce$(e) $(p)vgremove$(e) $(p)vgrename$(e) &
           $(p)vgscan$(e) $(p)vgsplit$(e) # $(p)lvmcmdline$(e) # $(p)polldaemon$(e)  

# $(p)lvm-static$(e) $(p)lvm2cmd-static$(e) $(p)lvm2cmd$(e) # $(p)lvmcmdlib$(e)

ADD_COPT = -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
           -i=$(MYDIR) -i=$(MYDIR)..$(SEP)lib &
           -i=$(MYDIR)..$(SEP)lib$(SEP)datastruct -i=$(MYDIR)..$(SEP)lib$(SEP)misc -i=$(MYDIR)..$(SEP)lib$(SEP)config -i=$(MYDIR)..$(SEP)lib$(SEP)commands &
           -i=$(MYDIR)..$(SEP)lib$(SEP)log -i=$(MYDIR)..$(SEP)lib$(SEP)metadata -i=$(MYDIR)..$(SEP)lib$(SEP)device -i=$(MYDIR)..$(SEP)lib$(SEP)uuid &
           -i=$(MYDIR)..$(SEP)lib$(SEP)display -i=$(MYDIR)..$(SEP)lib$(SEP)format_text -i=$(MYDIR)..$(SEP)lib$(SEP)activate -i=$(MYDIR)..$(SEP)lib$(SEP)mm &
           -i=$(MYDIR)..$(SEP)lib$(SEP)locking -i=$(MYDIR)..$(SEP)lib$(SEP)cache -i=$(MYDIR)..$(SEP)lib$(SEP)label -i=$(MYDIR)..$(SEP)lib$(SEP)filters &
           -i=$(MYDIR)..$(SEP)lib$(SEP)activate -i=$(MYDIR)..$(SEP)daemons$(SEP)clvmd -i=$(MYDIR)..$(SEP)lib$(SEP)report -bd
ADD_LINKOPT = lib $(PATH)..$(SEP)lib$(SEP)lvm.lib, $(PATH)..$(SEP)devmapper$(SEP)lib$(SEP)devmapper.lib
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk
