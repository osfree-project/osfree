# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

DESCRIPTION = Enhanced OW assembler
SOURCES = apiemu assemble assume atofloat backptch &
  bin branch cmdline codegen coff condasm     &
  context cpumodel data dbgcv directiv elf    &
  end equate errmsg expans expreval extern     &
  fastpass fixup fpfixup hll input invoke     &
  label linnum listing loop lqueue macro      &
  mangle memalloc msgtext omf omffixup omfint &
  option parser posndir preproc proc queue    &
  reswords safeseh segment simsegm string     &
  symbols tbyte tokenize types main

!ifeq %OS OS2
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
defs = $(%WATCOM)$(SEP)lh
!endif

!include $(MYDIR)jwasm.mk
!include $(%ROOT)tools/mk/build.mk

ADD_COPT = -dNDEBUG -i=$(PORT_BASE)src$(SEP)H -i=$(defs)
ADD_LINKOPT = lib clib3r
STACKSIZE = 0x20000
#UNI2H = 1
C = r

.c: $(PORT_BASE)src

.h: $(PORT_BASE)src$(SEP)H

.$(O): $(PATH)

$(PATH)msgtext.$(O): $(PORT_BASE)src$(SEP)msgtext.c &
                     $(PORT_BASE)src$(SEP)H$(SEP)msgdef.h &
                     $(PORT_BASE)src$(SEP)H$(SEP)globals.h

$(PATH)reswords.$(O): $(PORT_BASE)src$(SEP)reswords.c &
                      $(PORT_BASE)src$(SEP)H$(SEP)instruct.h &
                      $(PORT_BASE)src$(SEP)H$(SEP)special.h  &
                      $(PORT_BASE)src$(SEP)H$(SEP)directve.h
