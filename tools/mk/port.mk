#
# This is for this file to be not included twice
#
!ifndef __port_mk__
!define __port_mk__

!include $(%ROOT)/tools/mk/all.mk

prep: .symbolic
!ifeq PORT_TYPE wget
 $(verbose)if not exist $(PORT_FLAG) @$(MAKE) $(MAKEOPT) prep_wget
!else ifeq PORT_TYPE git
 $(verbose)if not exist $(PORT_FLAG) @$(MAKE) $(MAKEOPT) prep_git
!else ifeq PORT_TYPE svn
 $(verbose)if not exist $(PORT_FLAG) @$(MAKE) $(MAKEOPT) prep_svn
!endif

patch: .symbolic
 $(verbose)$(SAY) PATCH    $(PORT_NAME) $(LOG)
!ifeq %HOST linux
 $(verbose)if [ -f $(MYDIR)patches$(SEP)fixcase.cmd ]; then &
    $(CD) $(PORT_BASE) && $(REXX) $(MYDIR)patches$(SEP)fixcase.cmd $(PORT_BASE) $(BLACKHOLE); fi
!else
 $(verbose)if exist $(MYDIR)patches$(SEP)fixcase.cmd &
  @$(CD) $(PORT_BASE) && $(REXX) $(MYDIR)patches$(SEP)fixcase.cmd $(PORT_BASE) $(BLACKHOLE)
!endif
!ifeq %HOST win32
 $(verbose)$(MDHIER) $(PATH)
 $(verbose)for %i in ($(PORT_PATCHES)) do $(verbose)unix2dos -q -f -n $(MYDIR)patches$(SEP)%i $(PATH)%i
 $(verbose)for %i in ($(PORT_PATCHES)) do $(verbose)$(CD) $(PORT_BASE) && $(verbose)$(%PATCH) -s -p1 <$(PATH)%i
!else
 for %i in ($(PORT_PATCHES)) do $(CD) $(PORT_BASE) && $(%PATCH) -s -p1 <$(MYDIR)patches$(SEP)%i
!endif

prep_wget: .symbolic
 $(verbose)$(SAY) PREP     $(PORT_NAME) $(LOG)
 $(verbose)$(%WGET) -q --no-check-certificate $(PORT_URL) -O $(%TMP)$(SEP)qw.zip
 $(verbose)$(MDHIER) $(PORT_BASE)
 $(verbose)unzip -q -o $(%TMP)$(SEP)qw.zip -d $(PORT_BASE)
 $(verbose)$(DC) $(%TMP)$(SEP)qw.zip
 $(verbose)if exist $(MYDIR)patches $(verbose)$(MAKE) $(MAKEOPT) patch
 $(verbose)$(%INTERP)wtouch $(PORT_FLAG)

prep_git: .symbolic
 $(verbose)$(SAY) PREP     $(PORT_NAME) $(LOG)
 $(verbose)git clone -qq $(PORT_URL) $(PORT_BASE) --recursive
 $(verbose)$(CD) $(PORT_BASE) && git checkout -qq $(PORT_REV)
 $(verbose)if exist $(MYDIR)patches $(verbose)$(MAKE) $(MAKEOPT) patch
 $(verbose)$(%INTERP)wtouch $(PORT_FLAG)

prep_svn: .symbolic
 $(verbose)$(SAY) PREP     $(PORT_NAME) $(LOG)
!ifeq %HOST win32
 $(verbose)$(MDHIER) $(PORT_BASE)
 $(verbose)git svn clone -qq $(PORT_URL)$(PORT_REV) $(PORT_BASE)
!else
 $(verbose)svn --non-interactive --trust-server-cert co $(PORT_URL)$(PORT_REV) $(PORT_BASE)
!endif
 $(verbose)if exist $(MYDIR)patches $(verbose)$(MAKE) $(MAKEOPT) patch
 $(verbose)$(%INTERP)wtouch $(PORT_FLAG)

!endif
