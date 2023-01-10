# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = helpers
TRGT = $(PROJ).lib
DESC = XWPHelpers
srcfiles = $(p)acpih$(e) &
$(p)animate$(e) &
$(p)apmh$(e) &
$(p)apps$(e) &
$(p)cctl_checkcnr$(e) &
$(p)cctl_cnr$(e) &
$(p)cctl_cnr_dtls$(e) &
$(p)cctl_combo$(e) &
$(p)cctl_progbar$(e) &
$(p)cctl_splitwin$(e) &
$(p)cctl_toolbar$(e) &
$(p)cctl_tooltip$(e) &
$(p)cctl_xframe$(e) &
$(p)cnrh$(e) &
$(p)comctl$(e) &
$(p)configsys$(e) &
$(p)datetime$(e) &
$(p)debug$(e) &
$(p)eah$(e) &
$(p)encodings$(e) &
$(p)except$(e) &
$(p)gpih$(e) &
$(p)level$(e) &
$(p)linklist$(e) &
$(p)map_vac$(e) &
$(p)math$(e) &
$(p)memdebug$(e) &
$(p)memdebug_win$(e) &
$(p)mmpmh$(e) &
$(p)nlscache$(e) &
$(p)prfh$(e) &
$(p)prfh2$(e) &
$(p)procstat$(e) &
$(p)regexp$(e) &
$(p)resh$(e) &
$(p)sem$(e) &
$(p)shapewin$(e) &
$(p)stringh$(e) &
$(p)syssound$(e) &
$(p)textview$(e) &
$(p)textv_html$(e) &
$(p)threads$(e) &
$(p)timer$(e) &
$(p)tmsgfile$(e) &
$(p)tree$(e) &
$(p)vcard$(e) &
$(p)wphandle$(e) &
$(p)xmldefs$(e) &
$(p)xmlrole$(e) &
$(p)xprf$(e) &
$(p)xprf2$(e) &
$(p)xsecapi$(e) &
$(p)xstring$(e) &
$(p)cctl_chart$(e) &
$(p)dosh$(e) &
$(p)lan$(e) &
$(p)winh$(e) &
$(p)xml$(e) &
$(p)xmlparse$(e) &
$(p)xmltok$(e) &
$(p)exeh$(e) &
$(p)nls$(e) &
$(p)dosh2$(e)
#$(p)dialog$(e)


ADD_COPT = -ecs -dNDEBUG
ADD_LINKOPT = lib clib3r
IMPORTS=WinGetFrameTreePPSize PMWIN.972, &
       WinGetFrameTreePPs PMWIN.973

#UNI2H = 1

!include $(%ROOT)tools/mk/libsos2.mk

.c: $(MYDIR)contrib$(SEP)src$(SEP)helpers

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
