# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS=$(IDLH)boa.h &
$(IDLH)cntxt.h &
#$(IDLH)impldef.h &
#$(IDLH)implrep.h &
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
#$(IDLH)somdtype.h &
#$(IDLH)somoa.h &
#$(IDLH)stexcep.h &
$(IDLH)unotypes.h


ADD_SCOPT = -D__PRIVATE__ &
            -DEMIT_SOMDTYPES &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somcdr &
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

IDL =$(MYDIR)..$(SEP)contrib$(SEP)somd

.idl: $(IDL)
.h: $(IDLH)

$(IDLH)boa.h: $(IDL)$(SEP)boa.idl
$(IDLH)cntxt.h: $(IDL)$(SEP)cntxt.idl
$(IDLH)impldef.h: $(IDL)$(SEP)impldef.idl
$(IDLH)implrep.h: $(IDL)$(SEP)implrep.idl
$(IDLH)nvlist.h: $(IDL)$(SEP)nvlist.idl
$(IDLH)om.h: $(IDL)$(SEP)om.idl
$(IDLH)orb.h: $(IDL)$(SEP)orb.idl
$(IDLH)poa.h: $(IDL)$(SEP)poa.idl
$(IDLH)principl.h: $(IDL)$(SEP)principl.idl
$(IDLH)request.h: $(IDL)$(SEP)request.idl
$(IDLH)rhborbst.h: $(IDL)$(SEP)rhborbst.idl
$(IDLH)rhbtypes.h: $(IDL)$(SEP)rhbtypes.idl
$(IDLH)servmgr.h: $(IDL)$(SEP)servmgr.idl
$(IDLH)servreq.h: $(IDL)$(SEP)servreq.idl
$(IDLH)somdcprx.h: $(IDL)$(SEP)somdcprx.idl
$(IDLH)somdfenc.h: $(IDL)$(SEP)somdfenc.idl
$(IDLH)somdmprx.h: $(IDL)$(SEP)somdmprx.idl
$(IDLH)somdobj.h: $(IDL)$(SEP)somdobj.idl
$(IDLH)somdom.h: $(IDL)$(SEP)somdom.idl
$(IDLH)somdserv.h: $(IDL)$(SEP)somdserv.idl
$(IDLH)somdstub.h: $(IDL)$(SEP)somdstub.idl
$(IDLH)somdtcdt.h: $(IDL)$(SEP)somdtcdt.idl
$(IDLH)somdtype.h: $(IDL)$(SEP)somdtype.idl
$(IDLH)somoa.h: $(IDL)$(SEP)somoa.idl
$(IDLH)stexcep.h: $(IDL)$(SEP)stexcep.idl
$(IDLH)unotypes.h: $(IDL)$(SEP)unotypes.idl
