#define INCL_DOSERRORS
#include <os2.h>
#include <bdcalls.h>

APIRET16 APIENTRY16 Nls16QueryBidiAtt ( LONG       Effect,
                                       PRETINFO   RetInfo)
{
  return NO_ERROR;
}

APIRET16 APIENTRY16 Nls16SetBidiAtt ( LONG       Effect,
                                      PSETINFO   SetInfo)
{
  return NO_ERROR;
}

APIRET16 APIENTRY16 Nls16ShapeBidiString ( ULONG      BidiAtts,
                                          ULONG      Reserved,
                                          PCHAR16    Source  ,
                                          PCHAR16    Target  ,
                                          ULONG      Length  ,
                                          ULONG      Increment)
{
  return NO_ERROR;
}

APIRET16 APIENTRY16 Nls16EditShape ( ULONG      BidiAtts,
                                    LONG       Effect,
                                    PCSDRec    Source,
                                    PCSDRec    Target,
                                    PULONG     CSDState,
                                    ULONG      Increment)
{
  return NO_ERROR;
}


APIRET16 APIENTRY16 Nls16InverseString( PCHAR16    Source,
                                       PCHAR16    Target,
                                       ULONG      Length,
                                       ULONG      Increment)
{
  return NO_ERROR;
}


APIRET16 APIENTRY16 Nls16ConvertBidiNumerics ( ULONG    BidiAtts,
                                              LONG     Reserved,
                                              PUCHAR16 Source,
                                              PUCHAR16 Target,
                                              ULONG    Length,
                                              ULONG    Increment)
{
  return NO_ERROR;
}


APIRET16 APIENTRY16 Nls16SetBidiPrint ( ULONG  BidiAtts,
                                       ULONG  Length,
                                       USHORT FileHandle)
{
  return NO_ERROR;
}


APIRET16 APIENTRY16 Nls16ConvertBidiString ( PUCHAR16 Source,
                                            PUCHAR16 Target,
                                            ULONG    Length,
                                            ULONG    Increment,
                                            ULONG    SBidiAtts,
                                            ULONG    TBidiAtts,
                                            ULONG    Reserved)
{
  return NO_ERROR;
}


APIRET16 APIENTRY16 NL16POPUP(void)
{
  return NO_ERROR;
}

APIRET16 APIENTRY16 Nls16IntSetBidiAtt (LONG Scope,
                                           LONG Effect,
                                           PSETINFO setinfo)
{
  return NO_ERROR;
}

APIRET16 APIENTRY16 Nls16IntQueryBidiAtt (LONG Scope,
                                              LONG Effect,
                                              PRETINFO retinfo)
{
  return NO_ERROR;
}

