# Makefile for common shared libs

PROJ = libIDL
ADD_COPT = -i=$(PATH)..$(SEP)..$(SEP)include -dLIBIDL_VERSION="0.8.10" -dHAVE_CPP_PIPE_STDIN=1 -dCPP_NOSTDINC="" -dCPP_PROGRAM="notsupported" -dYYTEXT_POINTER=1 -dSTDC_HEADERS=1 -dHAVE_STDDEF_H=1 -dHAVE_WCHAR_H=1 -dYYDEBUG=1 -dIDL_LIBRARY -dG_LOG_DOMAIN="libIDL" $(ADD_COPT)
TARGETS  = $(PATH)$(PROJ).lib
srcfiles =  $(p)parser$(e)        &
	    $(p)lexer$(e)         &
	    $(p)ns$(e)  	  &
	    $(p)util$(e)	  &
	    $(p)ghash$(e)         &
	    $(p)gnode$(e)         &
	    $(p)gtree$(e)         &
	    $(p)gatomic$(e)       &
	    $(p)glist$(e)         &
	    $(p)gslist$(e)        &
	    $(p)gprimes$(e)
#           $(p)gstring$(e)       &
#           $(p)gthread$(e)       &

!include $(%ROOT)/mk/libs.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

$(PATH)parser.c: $(MYDIR)parser.y

$(PATH)lexer.c: $(MYDIR)lexer.l $(PATH)parser.h
 $(DC) $@
 lex -t $[@ | sed -e "s/yy/__IDL_/g" >$@

s: .SYMBOLIC
 $(SAY) $(PATH)
 $(SAY) $(MYDIR)
