# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS=$(IDLH)boa.h &
$(IDLH)cntxt.h &
$(IDLH)impldef.h &
$(IDLH)implrep.h &
$(IDLH)nvlist.h &
$(IDLH)om.h &
$(IDLH)orb.h &
$(IDLH)poa.h &
$(IDLH)principl.h &
$(IDLH)request.h &
$(IDLH)rhborbst.h &
$(IDLH)rhbtypes.h &
$(IDLH)servmgr.h &
$(IDLH)servreq.h &
$(IDLH)somdcprx.h &
$(IDLH)somdfenc.h &
$(IDLH)somdmprx.h &
$(IDLH)somdobj.h &
$(IDLH)somdom.h &
$(IDLH)somdserv.h &
$(IDLH)somdstub.h &
$(IDLH)somdtcdt.h &
$(IDLH)somdtype.h &
$(IDLH)somoa.h &
$(IDLH)stexcep.h &
$(IDLH)unotypes.h


ADD_SCOPT = -D__PRIVATE__ &
            -DEMIT_SOMDTYPES &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somd &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somestrm &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somnmf &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somabs1 &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somu &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somcorba &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somref &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somos &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somcslib &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somir

!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somd
.h: $(IDLH)
