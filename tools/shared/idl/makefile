# Makefile for common shared libs

PROJ = IDL
ADD_COPT = -i=$(MYPATH)..$(SEP)..$(SEP)include$(SEP)libIDL -dLIBIDL_VERSION="0.8.10" -dCPP_NOSTDINC="" -dCPP_PROGRAM="somcpp" -dYYTEXT_POINTER=1 -dSTDC_HEADERS=1 -dHAVE_STDDEF_H=1 -dHAVE_WCHAR_H=1 -dYYDEBUG=1 -dIDL_LIBRARY -dG_LOG_DOMAIN="libIDL" $(ADD_COPT) -sg -dTRUE=1 -dFALSE=0 -fi="rename.h" -dHAVE_CPP_PIPE_STDIN -d__IDL_lex=__IDLlex -dG_OS_WIN32 -dYYDEBUG=1
TARGETS  = $(PATH)$(PROJ).lib
srcfiles =  $(p)parser$(e)        &
            $(p)lexer$(e)         &
            $(p)ns$(e)            &
            $(p)util$(e)          &
            $(p)gtree$(e)         &
            $(p)ghash$(e)         &
            $(p)gutils$(e)        &
            $(p)gstrfuncs$(e)

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
