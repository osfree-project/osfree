
PROJ = winrc
TRGT = $(PROJ).exe
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
# $(p)mac$(e)
$(p)mdc$(e)

ADD_COPT = -dNOAPIPROTO -u_MAX_DRIVE -i=..$(SEP)include -i=. -i=$(%ROOT)DOS$(SEP)WIN16$(SEP)include

!include $(%ROOT)/mk/tools.mk

#TARGETS = $(PATH)$(PROJ).exe

$(PATH)rclex.obj: $(PATH)rclex.c

$(PATH)rcyacc.obj: $(PATH)rcyacc.c

$(PATH)ifyacc.obj: $(PATH)ifyacc.flg

.c: $(PATH)

.h: $(PATH)

$(PATH)ifyacc.flg: $(PATH)ifyacc.h $(PATH)ifyacc.c
  @sed.exe -e "s/yy/zz/g" -e "s/YY/ZZ/g" $(PATH)ifyacc.c >$(PATH)rc.z.tab.c
  @sed.exe -e "s/yy/zz/g" -e "s/YY/ZZ/g" $(PATH)ifyacc.h >$(PATH)rc.z.tab.h
  @$(DC) $(PATH)ifyacc.c $(BLACKHOLE)
  @$(DC) $(PATH)ifyacc.h $(BLACKHOLE)
  @$(RN) $(PATH)rc.z.tab.c $(PATH)ifyacc.c $(BLACKHOLE)
  @$(RN) $(PATH)rc.z.tab.h $(PATH)ifyacc.h $(BLACKHOLE)
  @wtouch $(PATH)ifyacc.flg

