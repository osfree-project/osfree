#
#
#
#

PROJ     = rxmath
srcfiles = $(p)rxmath$(e)

DEST         = os2$(SEP)dll

EXPORTS     = &
              MATHLOADFUNCS.1     = mathloadfuncs, &
              MATHDROPFUNCS.2     = mathdropfuncs, &
              RXCALCCOSH.3        = mathfunc1, &
              RXCALCEXP.4         = mathfunc1, &
              RXCALCLOG.5         = mathfunc1, &
              RXCALCLOG10.6       = mathfunc1, &
              RXCALCSINH.7        = mathfunc1, &
              RXCALCSQRT.8        = mathfunc1, &
              RXCALCTANH.9        = mathfunc1, &
              RXCALCCOS.10        = trigfunc1, &
              RXCALCCOTAN.11      = trigfunc1, &
              RXCALCSIN.12        = trigfunc1, &
              RXCALCTAN.13        = trigfunc1, &
              RXCALCACOS.14       = trigfunc2, &
              RXCALCASIN.15       = trigfunc2, &
              RXCALCATAN.16       = trigfunc2, &
              RXCALCPOW.17        = mathpow, &
              RXCALCPI.18         = mathpi

!include $(%ROOT)/mk/dirs.mk
!include $(MYDIR)..$(SEP)rxmath.mk
