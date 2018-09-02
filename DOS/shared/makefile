# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

PROJ      = dos_shared
TRGT      = $(PROJ).lib
#32_BITS = 0
ADD_COPT  = -i=$(MYDIR)..$(SEP)include
srcfiles  = $(p)os2vdm$(e)            &
            $(p)helpers$(e)           &
            $(p)BadDynLink$(e)        &
# Process
            $(p)dosbeep$(e)           &
            $(p)dosexit$(e)           &
            $(p)dossleep$(e)          &
            $(p)dosexecpgm$(e)        &
# File Manager  
            $(p)dosbufreset$(e)       &
            $(p)doschdir$(e)          &
            $(p)doschgfileptr$(e)     &
            $(p)dosclose$(e)          &
            $(p)dosdelete$(e)         &
            $(p)dosduphandle$(e)      &
            $(p)dosmkdir$(e)          &
            $(p)dosmove$(e)           &
            $(p)dosqcurdir$(e)        &
            $(p)dosqcurdisk$(e)       &
            $(p)dossetfilemode$(e)    &
            $(p)dosopen$(e)           &
            $(p)dosqfileinfo$(e)      &
            $(p)dosread$(e)           &
            $(p)dosqfilemode$(e)      &
            $(p)dosqfsinfo$(e)        &
            $(p)dosqverify$(e)        &
            $(p)dosrmdir$(e)          &
            $(p)dosselectdisk$(e)     &
            $(p)dosfindclose$(e)      &
            $(p)dosfindfirst$(e)      &
            $(p)dosfindnext$(e)       &
            $(p)dossetfileinfo$(e)    &
            $(p)dossetverify$(e)      &
            $(p)doswrite$(e)          &
            $(p)dosfilelocks$(e)      &
            $(p)dossetfhandstate$(e)  &
            $(p)dosnewsize$(e)        &
# Memory Manager
            $(p)dosfreeseg$(e)        &
            $(p)dossuballoc$(e)       &
            $(p)dossubfree$(e)        &
            $(p)dossubset$(e)         &
            $(p)dosallochuge$(e)      &
            $(p)dosallocseg$(e)       &
            $(p)dosreallochuge$(e)    &
            $(p)dosreallocseg$(e)     &
            $(p)dosgethugeshift$(e)   &
            $(p)doscreatecsalias$(e)  &
# NLS
            $(p)doscasemap$(e)        &
            $(p)dosgetctryinfo$(e)    &
            $(p)dosgetdbcsev$(e)      &
            $(p)dossetctrycode$(e)    &
# Date & time
            $(p)dossetdatetime$(e)    &
            $(p)dosgetdatetime$(e)    &
# Device
            $(p)dosdevconfig$(e)      &
            $(p)dosdevioctl$(e)       &
# Signals
            $(p)dosholdsignal$(e)     &
            $(p)dossetsighandler$(e)  &
# Misc
            $(p)dosgetenv$(e)         &
            $(p)dosgetmachinemode$(e) &
            $(p)dosgetmessage$(e)     &
            $(p)dosgetversion$(e)     &
            $(p)dosinsmessage$(e)     &
            $(p)dosputmessage$(e)     &
            $(p)doserror$(e)          &
# Keyboard
            $(p)kbdcharin$(e)         &
            $(p)kbdflushbuffer$(e)    &
            $(p)kbdgetstatus$(e)      &
            $(p)kbdsetstatus$(e)      &
            $(p)kbdstringin$(e)       &
            $(p)kbdpeek$(e)           &
# Video
            $(p)viogetbuf$(e)         &
            $(p)viogetcurpos$(e)      &
            $(p)viogetcurtype$(e)     &
            $(p)viogetphysbuf$(e)     &
            $(p)vioreadcellstr$(e)    &
            $(p)vioreadcharstr$(e)    &
            $(p)vioscrolldn$(e)       &
            $(p)vioscrolllf$(e)       &
            $(p)vioscrollrt$(e)       &
            $(p)vioscrunlock$(e)      &
            $(p)vioscrlock$(e)        &
            $(p)viosetcurpos$(e)      &
            $(p)viosetcurtype$(e)     &
            $(p)viosetmode$(e)        &
            $(p)vioshowbuf$(e)        &
            $(p)viowrtcellstr$(e)     &
            $(p)viowrtcharstr$(e)     &
            $(p)viowrtcharatratt$(e)  &
            $(p)viowrtnattr$(e)       &
            $(p)viowrtncell$(e)       &
            $(p)viowrtnchar$(e)       &
            $(p)viowrttty$(e)

TARGETS  = $(PATH)$(TRGT)

!include $(%ROOT)/mk/libsdos.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
