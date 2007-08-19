#
# A Makefile for pmdrag.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = pmdrag
DESC     = Drag and Drop API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk import
 @%append $^&.lnk   DRGACCESSDRAGINFO                PMCTLS.41,
 @%append $^&.lnk   DRGADDSTRHANDLE                  PMCTLS.42,
 @%append $^&.lnk   DRGALLOCDRAGINFO                 PMCTLS.43,
 @%append $^&.lnk   DRGALLOCDRAGTRANSFER             PMCTLS.44,
 @%append $^&.lnk   DRGDELETEDRAGINFOSTRHANDLES      PMCTLS.45,
 @%append $^&.lnk   DRGDELETESTRHANDLE               PMCTLS.46,
 @%append $^&.lnk   DRGDRAG                          PMCTLS.47,
 @%append $^&.lnk   DRGFREEDRAGINFO                  PMCTLS.48,
 @%append $^&.lnk   DRGFREEDRAGTRANSFER              PMCTLS.49,
 @%append $^&.lnk   DRGGETPS                         PMCTLS.50,
 @%append $^&.lnk   DRGPOSTTRANSFERMSG               PMCTLS.51,
 @%append $^&.lnk   DRGPUSHDRAGINFO                  PMCTLS.52,
 @%append $^&.lnk   DRGQUERYDRAGITEM                 PMCTLS.53,
 @%append $^&.lnk   DRGQUERYDRAGITEMCOUNT            PMCTLS.54,
 @%append $^&.lnk   DRGQUERYDRAGITEMPTR              PMCTLS.55,
 @%append $^&.lnk   DRGQUERYNATIVERMF                PMCTLS.56,
 @%append $^&.lnk   DRGQUERYNATIVERMFLEN             PMCTLS.57,
 @%append $^&.lnk   DRGQUERYSTRNAME                  PMCTLS.58,
 @%append $^&.lnk   DRGQUERYSTRNAMELEN               PMCTLS.59,
 @%append $^&.lnk   DRGQUERYTRUETYPE                 PMCTLS.60,
 @%append $^&.lnk   DRGQUERYTRUETYPELEN              PMCTLS.61,
 @%append $^&.lnk   DRGRELEASEPS                     PMCTLS.62,
 @%append $^&.lnk   DRGSENDTRANSFERMSG               PMCTLS.63,
 @%append $^&.lnk   DRGSETDRAGPOINTER                PMCTLS.64,
 @%append $^&.lnk   DRGSETDRAGIMAGE                  PMCTLS.65,
 @%append $^&.lnk   DRGSETDRAGITEM                   PMCTLS.66,
 @%append $^&.lnk   DRGVERIFYNATIVERMF               PMCTLS.67,
 @%append $^&.lnk   DRGVERIFYRMF                     PMCTLS.68,
 @%append $^&.lnk   DRGVERIFYTRUETYPE                PMCTLS.69,
 @%append $^&.lnk   DRGVERIFYTYPE                    PMCTLS.70,
 @%append $^&.lnk   DRGVERIFYTYPESET                 PMCTLS.71,
 @%append $^&.lnk   DrgAccessDraginfo                PMCTLS.74,
 @%append $^&.lnk   DrgAddStrHandle                  PMCTLS.75,
 @%append $^&.lnk   DrgAllocDraginfo                 PMCTLS.76,
 @%append $^&.lnk   DrgAllocDragtransfer             PMCTLS.77,
 @%append $^&.lnk   DrgDeleteDraginfoStrHandles      PMCTLS.78,
 @%append $^&.lnk   DrgDeleteStrHandle               PMCTLS.79,
 @%append $^&.lnk   DrgDrag                          PMCTLS.80,
 @%append $^&.lnk   DrgFreeDraginfo                  PMCTLS.81,
 @%append $^&.lnk   DrgFreeDragtransfer              PMCTLS.82,
 @%append $^&.lnk   DrgGetPS                         PMCTLS.83,
 @%append $^&.lnk   DrgPostTransferMsg               PMCTLS.84,
 @%append $^&.lnk   DrgPushDraginfo                  PMCTLS.85,
 @%append $^&.lnk   DrgQueryDragitem                 PMCTLS.86,
 @%append $^&.lnk   DrgQueryDragitemCount            PMCTLS.87,
 @%append $^&.lnk   DrgQueryDragitemPtr              PMCTLS.88,
 @%append $^&.lnk   DrgQueryNativeRMF                PMCTLS.89,
 @%append $^&.lnk   DrgQueryNativeRMFLen             PMCTLS.90,
 @%append $^&.lnk   DrgQueryStrName                  PMCTLS.91,
 @%append $^&.lnk   DrgQueryStrNameLen               PMCTLS.92,
 @%append $^&.lnk   DrgQueryTrueType                 PMCTLS.93,
 @%append $^&.lnk   DrgQueryTrueTypeLen              PMCTLS.94,
 @%append $^&.lnk   DrgReleasePS                     PMCTLS.95,
 @%append $^&.lnk   DrgSendTransferMsg               PMCTLS.96,
 @%append $^&.lnk   DrgSetDragPointer                PMCTLS.97,
 @%append $^&.lnk   DrgSetDragImage                  PMCTLS.98,
 @%append $^&.lnk   DrgSetDragitem                   PMCTLS.99,
 @%append $^&.lnk   DrgVerifyNativeRMF              PMCTLS.100,
 @%append $^&.lnk   DrgVerifyRMF                    PMCTLS.101,
 @%append $^&.lnk   DrgVerifyTrueType               PMCTLS.102,
 @%append $^&.lnk   DrgVerifyType                   PMCTLS.103,
 @%append $^&.lnk   DrgVerifyTypeSet                PMCTLS.104,
 @%append $^&.lnk   DRGDRAGFILES                    PMCTLS.72,
 @%append $^&.lnk   DRGACCEPTDROPPEDFILES           PMCTLS.73,
 @%append $^&.lnk   DrgDragFiles                    PMCTLS.105,
 @%append $^&.lnk   DrgAcceptDroppedFiles           PMCTLS.106,
 @%append $^&.lnk   DrgLazyDrag                     PMCTLS.284,
 @%append $^&.lnk   DrgCancelLazyDrag               PMCTLS.285,
 @%append $^&.lnk   DrgLazyDrop                     PMCTLS.286,
 @%append $^&.lnk   DrgQueryDraginfoPtr             PMCTLS.287,
 @%append $^&.lnk   DrgQueryDraginfoPtrFromHwnd     PMCTLS.288,
 @%append $^&.lnk   DrgQueryDraginfoPtrFromDragitem PMCTLS.289,
 @%append $^&.lnk   DrgQueryDragStatus              PMCTLS.290,
 @%append $^&.lnk   DrgReallocDraginfo              PMCTLS.291,
 @%append $^&.lnk   Undocumented1                   PMCTLS.107,
 @%append $^&.lnk   Undocumented2                   PMCTLS.108
 @%append $^&.lnk export
 @%append $^&.lnk   DRGACCESSDRAGINFO.            00001,
 @%append $^&.lnk   DRGADDSTRHANDLE.              00002,
 @%append $^&.lnk   DRGALLOCDRAGINFO.             00003,
 @%append $^&.lnk   DRGALLOCDRAGTRANSFER.         00004,
 @%append $^&.lnk   DRGDELETEDRAGINFOSTRHANDLES.  00005,
 @%append $^&.lnk   DRGDELETESTRHANDLE.           00006,
 @%append $^&.lnk   DRGDRAG.                      00007,
 @%append $^&.lnk   DRGFREEDRAGINFO.              00008,
 @%append $^&.lnk   DRGFREEDRAGTRANSFER.          00009,
 @%append $^&.lnk   DRGGETPS.                     00010,
 @%append $^&.lnk   DRGPOSTTRANSFERMSG.           00011,
 @%append $^&.lnk   DRGPUSHDRAGINFO.              00012,
 @%append $^&.lnk   DRGQUERYDRAGITEM.             00013,
 @%append $^&.lnk   DRGQUERYDRAGITEMCOUNT.        00014,
 @%append $^&.lnk   DRGQUERYDRAGITEMPTR.          00015,
 @%append $^&.lnk   DRGQUERYNATIVERMF.            00016,
 @%append $^&.lnk   DRGQUERYNATIVERMFLEN.         00017,
 @%append $^&.lnk   DRGQUERYSTRNAME.              00018,
 @%append $^&.lnk   DRGQUERYSTRNAMELEN.           00019,
 @%append $^&.lnk   DRGQUERYTRUETYPE.             00020,
 @%append $^&.lnk   DRGQUERYTRUETYPELEN.          00021,
 @%append $^&.lnk   DRGRELEASEPS.                 00022,
 @%append $^&.lnk   DRGSENDTRANSFERMSG.           00023,
 @%append $^&.lnk   DRGSETDRAGPOINTER.            00024,
 @%append $^&.lnk   DRGSETDRAGIMAGE.              00025,
 @%append $^&.lnk   DRGSETDRAGITEM.               00026,
 @%append $^&.lnk   DRGVERIFYNATIVERMF.           00027,
 @%append $^&.lnk   DRGVERIFYRMF.                 00028,
 @%append $^&.lnk   DRGVERIFYTRUETYPE.            00029,
 @%append $^&.lnk   DRGVERIFYTYPE.                00030,
 @%append $^&.lnk   DRGVERIFYTYPESET.             00031,
 @%append $^&.lnk   DrgAccessDraginfo.            00032,
 @%append $^&.lnk   DrgAddStrHandle.              00033,
 @%append $^&.lnk   DrgAllocDraginfo.             00034,
 @%append $^&.lnk   DrgAllocDragtransfer.         00035,
 @%append $^&.lnk   DrgDeleteDraginfoStrHandles.  00036,
 @%append $^&.lnk   DrgDeleteStrHandle.           00037,
 @%append $^&.lnk   DrgDrag.                      00038,
 @%append $^&.lnk   DrgFreeDraginfo.              00039,
 @%append $^&.lnk   DrgFreeDragtransfer.          00040,
 @%append $^&.lnk   DrgGetPS.                     00041,
 @%append $^&.lnk   DrgPostTransferMsg.           00042,
 @%append $^&.lnk   DrgPushDraginfo.              00043,
 @%append $^&.lnk   DrgQueryDragitem.             00044,
 @%append $^&.lnk   DrgQueryDragitemCount.        00045,
 @%append $^&.lnk   DrgQueryDragitemPtr.          00046,
 @%append $^&.lnk   DrgQueryNativeRMF.            00047,
 @%append $^&.lnk   DrgQueryNativeRMFLen.         00048,
 @%append $^&.lnk   DrgQueryStrName.              00049,
 @%append $^&.lnk   DrgQueryStrNameLen.           00050,
 @%append $^&.lnk   DrgQueryTrueType.             00051,
 @%append $^&.lnk   DrgQueryTrueTypeLen.          00052,
 @%append $^&.lnk   DrgReleasePS.                 00053,
 @%append $^&.lnk   DrgSendTransferMsg.           00054,
 @%append $^&.lnk   DrgSetDragPointer.            00055,
 @%append $^&.lnk   DrgSetDragImage.              00056,
 @%append $^&.lnk   DrgSetDragitem.               00057,
 @%append $^&.lnk   DrgVerifyNativeRMF.          00058,
 @%append $^&.lnk   DrgVerifyRMF.                00059,
 @%append $^&.lnk   DrgVerifyTrueType.           00060,
 @%append $^&.lnk   DrgVerifyType.               00061,
 @%append $^&.lnk   DrgVerifyTypeSet.            00062,
 @%append $^&.lnk   DRGDRAGFILES.                 00063,
 @%append $^&.lnk   DRGACCEPTDROPPEDFILES.        00064,
 @%append $^&.lnk   DrgDragFiles.                00065,
 @%append $^&.lnk   DrgAcceptDroppedFiles.       00066,
 @%append $^&.lnk   DrgLazyDrag.                 00067,
 @%append $^&.lnk   DrgCancelLazyDrag.           00068,
 @%append $^&.lnk   DrgLazyDrop.                 00069,
 @%append $^&.lnk   DrgQueryDraginfoPtr.         00070,
 @%append $^&.lnk   DrgQueryDraginfoPtrFromHwnd. 00071,
 @%append $^&.lnk   DrgQueryDraginfoPtrFromDragitem. 00072,
 @%append $^&.lnk   DrgQueryDragStatus.          00073,
 @%append $^&.lnk   DrgReallocDraginfo.          00074,
 @%append $^&.lnk   Undocumented1.               00075,
 @%append $^&.lnk   Undocumented2.               00076
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

install: .SYMBOLIC
 $(CP) $(PROJ).exe ..$(SEP)bin

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
