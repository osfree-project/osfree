#
#
#
#

PROJ     = rexxmath
TRGT     = $(PROJ).dll
srcfiles = $(p)rexxmath$(e)

DEST         = os2$(SEP)dll

EXPORTS     = &
              MATHLOADFUNCS.1  = mathloadfuncs, &
              MATHDROPFUNCS.2  = mathdropfuncs, &
              MATHFUNC1.3      = mathfunc1, &
              POW.4            = mathpow, &
              POWER.5          = mathpow, &
              XTOY.6           = mathpow, &
              FACT.7           = mathfact, &
              ATAN.8           = mathatan, &
              ACOS.9           = mathfunc1, &
              ACOSH.10         = mathfunc1, &
              ASIN.11          = mathfunc1, &
              ASINH.12         = mathfunc1, &
              ATANH.13         = mathfunc1, &
              CEIL.14          = mathfunc1, &
              COS.15           = mathfunc1, &
              COSH.16          = mathfunc1, &
              COT.17           = mathfunc1, &
              COTAN.18         = mathfunc1, &
              CSC.19           = mathfunc1, &
              EXP.20           = mathfunc1, &
              FLOOR.21         = mathfunc1, &
              INT.22           = mathfunc1, &
              LN.23            = mathfunc1, &
              LOG.24           = mathfunc1, &
              LOG10.25         = mathfunc1, &
              NINT.26          = mathfunc1, &
              SEC.27           = mathfunc1, &
              SIN.28           = mathfunc1, &
              SINH.29          = mathfunc1, &
              SQRT.30          = mathfunc1, &
              TAN.31           = mathfunc1, &
              TANH.32          = mathfunc1

!include $(%ROOT)/mk/dirs.mk
!include $(MYDIR)..$(SEP)rxmath.mk
