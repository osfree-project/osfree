# $Id: Makefile,v 1.2 2001/09/09 00:42:33 bird Exp $

#
# Helper makefile for trace stuff.
#
#




#
# Suffixes.
#
.SUFFIXES: .rpn .tdf .tsf .tff


#
# Inference rules.
#
.rpn.tdf:
    DTRACE BUILDTDF $< $@

.tsf.tff:
    if exist $(@B).tdf TRCUST $< $(@B).tdf
    if not exist $(@B).tdf TRCUST $<


#
# dummy all rule.
#
all:


#
# clean rule.
#
clean:
    del *.tdf
    del *.tff

