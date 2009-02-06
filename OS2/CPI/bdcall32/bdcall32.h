/*
 *
 *
 *
 */

#pragma pack(2)

typedef struct _csdrec {
    ULONG   RecLength;
    PCHAR   Buffer;
    ULONG   BufferLength;
    ULONG   BufferIndex;
} CSDRec;

#pragma pack()

typedef CSDRec *PCSDRec;


APIRET APIENTRY NlsConvertBidiNumerics (ULONG    BidiAtts,
                                        LONG     Reserved,
                                        PUCHAR   Source,
                                        PUCHAR   Target,
                                        ULONG    Length,
                                        ULONG    Increment);

APIRET APIENTRY NlsConvertBidiString (PUCHAR   Source,
                                      PUCHAR   Target,
                                      ULONG    Length,
                                      ULONG    Increment,
                                      ULONG    SBidiAtts,
                                      ULONG    TBidiAtts,
                                      ULONG    Reserved);

APIRET APIENTRY NlsInverseString (PCHAR      Source,
                                  PCHAR      Target,
                                  ULONG      Length,
                                  ULONG      Increment);

APIRET APIENTRY NlsEditShape (ULONG      BidiAtts,
                              LONG       Effect,
                              PCSDRec    Source,
                              PCSDRec    Target,
                              PULONG     CSDState,
                              ULONG      Increment);

APIRET APIENTRY NlsShapeBidiString (ULONG      BidiAtts,
                                    ULONG      Reserved,
                                    PCHAR      Source  ,
                                    PCHAR      Target  ,
                                    ULONG      Length  ,
                                    ULONG      Increment);
