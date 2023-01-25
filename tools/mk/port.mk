#
# This is for this file to be not included twice
#
!ifndef __port_mk__
!define __port_mk__

!include $(%ROOT)/tools/mk/all.mk

prep: .symbolic
!ifeq PORT_TYPE wget
 @if not exist $(CONTRIB)$(PORT_NAME).flg $(MAKE) $(MAKEOPT) prep_wget
!else ifeq PORT_TYPE git
 @if not exist $(CONTRIB)$(PORT_NAME).flg $(MAKE) $(MAKEOPT) prep_git
!else ifeq PORT_TYPE svn
 @if not exist $(CONTRIB)$(PORT_NAME).flg $(MAKE) $(MAKEOPT) prep_svn
!endif

prep_wget: .symbolic
 $(verbose)$(SAY) PREP     $(PORT_NAME) $(LOG)
 $(verbose)wget $(PORT_URL) -O $(%TMP)$(SEP)qw.zip
 $(verbose)unzip -o $(%TMP)$(SEP)qw.zip -d $(PORT_BASE)
 $(verbose)$(DC) $(%TMP)$(SEP)qw.zip
 $(verbose)wtouch $(CONTRIB)$(PORT_NAME).flg

prep_git: .symbolic
 $(verbose)$(SAY) PREP     $(PORT_NAME) $(LOG)
 $(verbose)git clone $(PORT_URL) $(PORT_BASE)
 $(verbose)$(CD) $(PORT_BASE) && git checkout $(PORT_REV)
 $(verbose)wtouch $(CONTRIB)$(PORT_NAME).flg

prep_svn: .symbolic

!endif
