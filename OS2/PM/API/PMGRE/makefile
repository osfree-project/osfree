#
# A Makefile for pmdrag.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)
PROJ     = pmgre
DESC     = gre
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initinstance terminstance
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import
 @%append $^&.lnk   INNERGREENTRY      PMMERGE.2035,
 @%append $^&.lnk   GREINITIALIZE      PMMERGE.2036,
 @%append $^&.lnk   GREENTRY2          PMMERGE.2037,
 @%append $^&.lnk   GREENTRY3          PMMERGE.2038,
 @%append $^&.lnk   GREENTRY4          PMMERGE.2039,
 @%append $^&.lnk   GREENTRY5          PMMERGE.2040,
 @%append $^&.lnk   GREENTRY6          PMMERGE.2041,
 @%append $^&.lnk   GREENTRY7          PMMERGE.2042,
 @%append $^&.lnk   GREENTRY8          PMMERGE.2043,
 @%append $^&.lnk   GREENTRY9          PMMERGE.2044,
 @%append $^&.lnk   GREENTRY10         PMMERGE.2045,
 @%append $^&.lnk   GETDRIVERINFO      PMMERGE.2046,
 @%append $^&.lnk   SETDRIVERINFO      PMMERGE.2047,
 @%append $^&.lnk   POSTDEVICEMODES    PMMERGE.2048,
 @%append $^&.lnk   BITMAPLIMITWARNING PMMERGE.2049,
 @%append $^&.lnk   GETDRIVERINFO32    PMMERGE.5046,
 @%append $^&.lnk   SETDRIVERINFO32    PMMERGE.5098,
 @%append $^&.lnk   POSTDEVICEMODES32  PMMERGE.5073,
 @%append $^&.lnk   POSTESCAPE32       PMMERGE.5460,
 @%append $^&.lnk   PMWINGREENTRY2     PMMERGE.2050,
 @%append $^&.lnk   PMWINGREENTRY3     PMMERGE.2051,
 @%append $^&.lnk   PMWINGREENTRY4     PMMERGE.2052,
 @%append $^&.lnk   PMWINGREENTRY5     PMMERGE.2053,
 @%append $^&.lnk   PMWINGREENTRY6     PMMERGE.2054,
 @%append $^&.lnk   PMWINGREENTRY7     PMMERGE.2055,
 @%append $^&.lnk   PMWINGREENTRY8     PMMERGE.2056,
 @%append $^&.lnk   PMWINGREENTRY9     PMMERGE.2057,
 @%append $^&.lnk   PMWINGREENTRY10    PMMERGE.2058,
 @%append $^&.lnk   GRE32ENTRY2        PMMERGE.5049,
 @%append $^&.lnk   GRE32ENTRY3        PMMERGE.5050,
 @%append $^&.lnk   GRE32ENTRY4        PMMERGE.5051,
 @%append $^&.lnk   GRE32ENTRY5        PMMERGE.5052,
 @%append $^&.lnk   GRE32ENTRY6        PMMERGE.5053,
 @%append $^&.lnk   GRE32ENTRY7        PMMERGE.5054,
 @%append $^&.lnk   GRE32ENTRY8        PMMERGE.5055,
 @%append $^&.lnk   GRE32ENTRY9        PMMERGE.5056,
 @%append $^&.lnk   GRE32ENTRY10       PMMERGE.5048,
 @%append $^&.lnk   INNERGRE32ENTRY2   PMMERGE.5060,
 @%append $^&.lnk   INNERGRE32ENTRY3   PMMERGE.5061,
 @%append $^&.lnk   INNERGRE32ENTRY4   PMMERGE.5062,
 @%append $^&.lnk   INNERGRE32ENTRY5   PMMERGE.5063,
 @%append $^&.lnk   INNERGRE32ENTRY6   PMMERGE.5064,
 @%append $^&.lnk   INNERGRE32ENTRY7   PMMERGE.5065,
 @%append $^&.lnk   INNERGRE32ENTRY8   PMMERGE.5066,
 @%append $^&.lnk   INNERGRE32ENTRY9   PMMERGE.5067,
 @%append $^&.lnk   INNERGRE32ENTRY10  PMMERGE.5059,
 @%append $^&.lnk   SSALLOCSEG         PMMERGE.2059,
 @%append $^&.lnk   SSALLOCHUGE        PMMERGE.2060,
 @%append $^&.lnk   SSFREESEG          PMMERGE.2061,
 @%append $^&.lnk   SSFLUSHSEGS        PMMERGE.2062,
 @%append $^&.lnk   SSALLOCMEM         PMMERGE.5104,
 @%append $^&.lnk   SSFREEMEM          PMMERGE.5105,
 @%append $^&.lnk   ENABLE_CALL_16     PMMERGE.4010,
 @%append $^&.lnk   DEVMODE_CALL_16    PMMERGE.4000,
 @%append $^&.lnk   DELETEBITMAP16CALL PMMERGE.4021,
 @%append $^&.lnk   PFN4R2             PMMERGE.4015,
 @%append $^&.lnk   PFN5R2             PMMERGE.4016,
 @%append $^&.lnk   PFN6R2             PMMERGE.4017,
 @%append $^&.lnk   PFN7R2             PMMERGE.4018,
 @%append $^&.lnk   PFN8R2             PMMERGE.4019,
 @%append $^&.lnk   PFN9R2             PMMERGE.4020,
 @%append $^&.lnk   PFN10R2            PMMERGE.4014,
 @%append $^&.lnk   DEVNAME_CALL_16    PMMERGE.4001,
 @%append $^&.lnk   DISPATCH16GATE3    PMMERGE.4003,
 @%append $^&.lnk   DISPATCH16GATE4    PMMERGE.4004,
 @%append $^&.lnk   DISPATCH16GATE5    PMMERGE.4005,
 @%append $^&.lnk   DISPATCH16GATE6    PMMERGE.4006,
 @%append $^&.lnk   DISPATCH16GATE7    PMMERGE.4007,
 @%append $^&.lnk   DISPATCH16GATE8    PMMERGE.4008,
 @%append $^&.lnk   DISPATCH16GATE9    PMMERGE.4009,
 @%append $^&.lnk   DISPATCH16GATE10   PMMERGE.4002,
 @%append $^&.lnk   RAMSEMREQUEST32    PMMERGE.5097,
 @%append $^&.lnk   RAMSEMCLEAR32      PMMERGE.5096,
 @%append $^&.lnk   RAMSEMREQUEST16    PMMERGE.2063,
 @%append $^&.lnk   RAMSEMCLEAR16      PMMERGE.2064,
 @%append $^&.lnk   GREMEMPTRS         PMMERGE.6200,
 @%append $^&.lnk   GRENLS             PMMERGE.6100,
 @%append $^&.lnk   SETDEVICESURFACE   PMMERGE.5108
 @%append $^&.lnk export
 @%append $^&.lnk   INNERGREENTRY      .00003,
 @%append $^&.lnk   GREINITIALIZE      .00004,
 @%append $^&.lnk   GREENTRY2          .00012,
 @%append $^&.lnk   GREENTRY3          .00013,
 @%append $^&.lnk   GREENTRY4          .00014,
 @%append $^&.lnk   GREENTRY5          .00015,
 @%append $^&.lnk   GREENTRY6          .00016,
 @%append $^&.lnk   GREENTRY7          .00017,
 @%append $^&.lnk   GREENTRY8          .00018,
 @%append $^&.lnk   GREENTRY9          .00019,
 @%append $^&.lnk   GREENTRY10         .00020,
 @%append $^&.lnk   GETDRIVERINFO      .00030,
 @%append $^&.lnk   SETDRIVERINFO      .00031,
 @%append $^&.lnk   POSTDEVICEMODES    .00032,
 @%append $^&.lnk   BITMAPLIMITWARNING .00034,
 @%append $^&.lnk   GETDRIVERINFO32    .00037,
 @%append $^&.lnk   SETDRIVERINFO32    .00038,
 @%append $^&.lnk   POSTDEVICEMODES32  .00039,
 @%append $^&.lnk   POSTESCAPE32       .00041,
 @%append $^&.lnk   PMWINGREENTRY2     .00042,
 @%append $^&.lnk   PMWINGREENTRY3     .00043,
 @%append $^&.lnk   PMWINGREENTRY4     .00044,
 @%append $^&.lnk   PMWINGREENTRY5     .00045,
 @%append $^&.lnk   PMWINGREENTRY6     .00046,
 @%append $^&.lnk   PMWINGREENTRY7     .00047,
 @%append $^&.lnk   PMWINGREENTRY8     .00048,
 @%append $^&.lnk   PMWINGREENTRY9     .00049,
 @%append $^&.lnk   PMWINGREENTRY10    .00050,
 @%append $^&.lnk   GRE32ENTRY2        .00062,
 @%append $^&.lnk   GRE32ENTRY3        .00063,
 @%append $^&.lnk   GRE32ENTRY4        .00064,
 @%append $^&.lnk   GRE32ENTRY5        .00065,
 @%append $^&.lnk   GRE32ENTRY6        .00066,
 @%append $^&.lnk   GRE32ENTRY7        .00067,
 @%append $^&.lnk   GRE32ENTRY8        .00068,
 @%append $^&.lnk   GRE32ENTRY9        .00069,
 @%append $^&.lnk   GRE32ENTRY10       .00070,
 @%append $^&.lnk   INNERGRE32ENTRY2   .00072,
 @%append $^&.lnk   INNERGRE32ENTRY3   .00073,
 @%append $^&.lnk   INNERGRE32ENTRY4   .00074,
 @%append $^&.lnk   INNERGRE32ENTRY5   .00075,
 @%append $^&.lnk   INNERGRE32ENTRY6   .00076,
 @%append $^&.lnk   INNERGRE32ENTRY7   .00077,
 @%append $^&.lnk   INNERGRE32ENTRY8   .00078,
 @%append $^&.lnk   INNERGRE32ENTRY9   .00079,
 @%append $^&.lnk   INNERGRE32ENTRY10  .00080,
 @%append $^&.lnk   SSALLOCSEG         .00100,
 @%append $^&.lnk   SSALLOCHUGE        .00101,
 @%append $^&.lnk   SSFREESEG          .00102,
 @%append $^&.lnk   SSFLUSHSEGS        .00103,
 @%append $^&.lnk   SSALLOCMEM         .00106,
 @%append $^&.lnk   SSFREEMEM          .00107,
 @%append $^&.lnk   ENABLE_CALL_16     .00200,
 @%append $^&.lnk   DEVMODE_CALL_16    .00201,
 @%append $^&.lnk   DELETEBITMAP16CALL .00202,
 @%append $^&.lnk   PFN4R2             .00203,
 @%append $^&.lnk   PFN5R2             .00204,
 @%append $^&.lnk   PFN6R2             .00205,
 @%append $^&.lnk   PFN7R2             .00206,
 @%append $^&.lnk   PFN8R2             .00207,
 @%append $^&.lnk   PFN9R2             .00208,
 @%append $^&.lnk   PFN10R2            .00209,
 @%append $^&.lnk   DEVNAME_CALL_16    .00210,
 @%append $^&.lnk   DISPATCH16GATE3    .00213,
 @%append $^&.lnk   DISPATCH16GATE4    .00214,
 @%append $^&.lnk   DISPATCH16GATE5    .00215,
 @%append $^&.lnk   DISPATCH16GATE6    .00216,
 @%append $^&.lnk   DISPATCH16GATE7    .00217,
 @%append $^&.lnk   DISPATCH16GATE8    .00218,
 @%append $^&.lnk   DISPATCH16GATE9    .00219,
 @%append $^&.lnk   DISPATCH16GATE10   .00220,
 @%append $^&.lnk   RAMSEMREQUEST32    .00400,
 @%append $^&.lnk   RAMSEMCLEAR32      .00401,
 @%append $^&.lnk   RAMSEMREQUEST16    .00402,
 @%append $^&.lnk   RAMSEMCLEAR16      .00403,
 @%append $^&.lnk   GREMEMPTRS         .00500,
 @%append $^&.lnk   GRENLS             .00501,
 @%append $^&.lnk   SETDEVICESURFACE   .00510
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

install: .SYMBOLIC
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
