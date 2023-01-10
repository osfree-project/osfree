#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ  = mmiogbm
TRGT  = $(PROJ).dll
PROJ0 = mmiogbm
DESC  = GBM IOProc generator
#defines object file names in format objname.$(O)
srcfiles = $(p)mmioproc$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(%ROOT)include$(SEP)os3$(SEP)gbm -i=$(%WATCOM)$(SEP)h$(SEP)os2
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
ADD_LINKOPT  = lib mmpm2.lib, gbm.lib, libtiff.lib, libjpeg.lib, libpng.lib, libz.lib, ojpeg.lib &
               segment type DATA shared
UNI2H    = 1
DLL = 1
EXPORTS = &
  BMPproc.1, &
  VGAproc.2, &
  BGAproc.3, &
  RLEproc.4, &
  DIBproc.5, &
  RL4proc.6, &
  RL8proc.7, &
  GIFproc.8, &
  PCXproc.9, &
  PCCproc.10, &
  TIFproc.11, &
  TIFFproc.12, &
  TGAproc.13, &
  VSTproc.14, &
  AFIproc.15, &
  IFFproc.16, &
  LBMproc.17, &
  VIDproc.18, &
  PBMproc.19, &
  PGMproc.20, &
  PPMproc.21, &
  PNMproc.22, &
  KPSproc.23, &
  IAXproc.24, &
  XBMproc.25, &
  XPMproc.26, &
  SPRproc.27, &
  SPRITEproc.28, &
  PSEproc.29, &
  PSEGproc.30, &
  PSEG38PPproc.31, &
  PSEG3820proc.32, &
  IMGproc.33, &
  XIMGproc.34, &
  CVPproc.35, &
  PNGproc.36, &
  JPGproc.37, &
  JPEGproc.38, &
  JPEproc.39, &
  JP2proc.40, &
  J2Cproc.41, &
  J2Kproc.42, &
  JPCproc.43, &
  JPTproc.44

DEST = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
