
PROJ = winrc
DESC = Win16 .rc to .c compiler
srcfiles = $(p)ifyacc$(e) &
$(p)rcyacc$(e) &
$(p)rclex$(e) &
$(p)define$(e) &
$(p)resources$(e) &
$(p)util$(e) &
$(p)rc$(e) &
$(p)printres$(e) &
$(p)cmdline$(e) &
$(p)buffer$(e) &
$(p)cleanup$(e) &
$(p)mdc$(e)

!include $(%ROOT)/mk/tools.mk

ADD_COPT = -i=..$(SEP)include
TARGETS = $(PATH)$(PROJ).exe

$(PATH)rclex.obj: $(PATH)rclex.c

$(PATH)rcyacc.obj: $(PATH)rcyacc.c

$(p)ifyacc.obj: $(p)ifyacc.c preproc

preproc: .symbolic
  sed -e "s/yy/zz/g" -e "s/YY/ZZ/g" $(PATH)ifyacc.c >$(p)rc.z.tab.c
  sed -e "s/yy/zz/g" -e "s/YY/ZZ/g" $(PATH)ifyacc.h >$(p)rc.z.tab.h
  del $(p)ifyacc.c
  del $(p)ifyacc.h
   move $(p)rc.z.tab.c $(p)ifyacc.c
   move $(p)rc.z.tab.h $(p)ifyacc.h

