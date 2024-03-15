
DEST = os2tk45$(SEP)som$(SEP)include

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)somobj.idl &
		$(PATH)somcls.idl &
		$(PATH)somcm.idl &
		$(PATH)attribdf.idl &
		$(PATH)constdef.idl &
		$(PATH)containd.idl &
		$(PATH)containr.idl &
		$(PATH)excptdef.idl &
		$(PATH)intfacdf.idl &
		$(PATH)moduledf.idl &
		$(PATH)operatdf.idl &
		$(PATH)paramdef.idl &
		$(PATH)repostry.idl &
		$(PATH)typedef.idl &
		$(PATH)boa.idl &
		$(PATH)cntxt.idl &
		$(PATH)impldef.idl &
		$(PATH)implrep.idl &
		$(PATH)nvlist.idl &
		$(PATH)om.idl &
		$(PATH)orb.idl &
		$(PATH)poa.idl &
		$(PATH)principl.idl &
		$(PATH)request.idl &
		$(PATH)rhborbst.idl &
		$(PATH)rhbtypes.idl &
		$(PATH)servmgr.idl &
		$(PATH)servreq.idl &
		$(PATH)somdcprx.idl &
		$(PATH)somdfenc.idl &
		$(PATH)somdmprx.idl &
		$(PATH)somdobj.idl &
		$(PATH)somdom.idl &
		$(PATH)somdserv.idl &
		$(PATH)somdstub.idl &
		$(PATH)somdtcdt.idl &
		$(PATH)somdtype.idl &
		$(PATH)somoa.idl &
		$(PATH)stexcep.idl &
		$(PATH)unotypes.idl &
		$(PATH)clientev.idl &
		$(PATH)eman.idl &
		$(PATH)emregdat.idl &
		$(PATH)event.idl &
		$(PATH)sinkev.idl &
		$(PATH)timerev.idl &
		$(PATH)workprev.idl

SOURCES = $(PORT_BASE)somkpub$(SEP)som$(SEP)somobj.idl &
		$(PORT_BASE)somkpub$(SEP)som$(SEP)somcls.idl &
		$(PORT_BASE)somkpub$(SEP)som$(SEP)somcm.idl &
		$(PORT_BASE)somir$(SEP)attribdf.idl &
		$(PORT_BASE)somir$(SEP)constdef.idl &
		$(PORT_BASE)somir$(SEP)containd.idl &
		$(PORT_BASE)somir$(SEP)containr.idl &
		$(PORT_BASE)somir$(SEP)excptdef.idl &
		$(PORT_BASE)somir$(SEP)intfacdf.idl &
		$(PORT_BASE)somir$(SEP)moduledf.idl &
		$(PORT_BASE)somir$(SEP)operatdf.idl &
		$(PORT_BASE)somir$(SEP)paramdef.idl &
		$(PORT_BASE)somir$(SEP)repostry.idl &
		$(PORT_BASE)somir$(SEP)typedef.idl &
		$(PORT_BASE)somd$(SEP)boa.idl &
		$(PORT_BASE)somd$(SEP)cntxt.idl &
		$(PORT_BASE)somd$(SEP)impldef.idl &
		$(PORT_BASE)somd$(SEP)implrep.idl &
		$(PORT_BASE)somd$(SEP)nvlist.idl &
		$(PORT_BASE)somd$(SEP)om.idl &
		$(PORT_BASE)somd$(SEP)orb.idl &
		$(PORT_BASE)somd$(SEP)poa.idl &
		$(PORT_BASE)somd$(SEP)principl.idl &
		$(PORT_BASE)somd$(SEP)request.idl &
		$(PORT_BASE)somd$(SEP)rhborbst.idl &
		$(PORT_BASE)somd$(SEP)rhbtypes.idl &
		$(PORT_BASE)somd$(SEP)servmgr.idl &
		$(PORT_BASE)somd$(SEP)servreq.idl &
		$(PORT_BASE)somd$(SEP)somdcprx.idl &
		$(PORT_BASE)somd$(SEP)somdfenc.idl &
		$(PORT_BASE)somd$(SEP)somdmprx.idl &
		$(PORT_BASE)somd$(SEP)somdobj.idl &
		$(PORT_BASE)somd$(SEP)somdom.idl &
		$(PORT_BASE)somd$(SEP)somdserv.idl &
		$(PORT_BASE)somd$(SEP)somdstub.idl &
		$(PORT_BASE)somd$(SEP)somdtcdt.idl &
		$(PORT_BASE)somd$(SEP)somdtype.idl &
		$(PORT_BASE)somd$(SEP)somoa.idl &
		$(PORT_BASE)somd$(SEP)stexcep.idl &
		$(PORT_BASE)somd$(SEP)unotypes.idl &
		$(PORT_BASE)somem$(SEP)clientev.idl &
		$(PORT_BASE)somem$(SEP)eman.idl &
		$(PORT_BASE)somem$(SEP)emregdat.idl &
		$(PORT_BASE)somem$(SEP)event.idl &
		$(PORT_BASE)somem$(SEP)sinkev.idl &
		$(PORT_BASE)somem$(SEP)timerev.idl &
		$(PORT_BASE)somem$(SEP)workprev.idl

$(TARGETS): $(SOURCES)
 $(verbose)pdl -o $(PATH) $< $(BLACKHOLE)
