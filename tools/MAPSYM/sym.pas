{ .SYM symbol files format suppoer }

Unit SYM;

{$IFDEF FPC}
  {$MODE OBJFPC}
  {$H-} // Don't use huge strings by default. Use pascal strings.
{$ENDIF}

Interface

Type
  TSYMFile=Class
  private
    AFile: File of Byte;
    bFlags:        Byte;                { symbol types                          }
    bReserved1:    Byte;                { reserved                              }
    pSegEntry:     Word;                { segment entry point value             }
    cConsts:       Word;                { count of constants in map             }
    pConstDef:     Word;                { pointer to constant chain             }
    cSegs:         Word;                { count of segments in map              }
    ppSegDef:      Word;                { paragraph pointer to first segment    }
    cbMaxSym:      Byte;                { maximum symbol-name length            }
    strModName:    String;              { module name }
  public
    constructor Create(FileName: String);
  end;

Implementation

{ Pointer means offset from beginning of file or beginning of struct }

Type
  TMAPDEF=record
    ppNextMap:     Word;                { paragraph pointer to next map         }
    bFlags:        Byte;                { symbol types                          }
    bReserved1:    Byte;                { reserved                              }
    pSegEntry:     Word;                { segment entry point value             }
    cConsts:       Word;                { count of constants in map             }
    pConstDef:     Word;                { pointer to constant chain             }
    cSegs:         Word;                { count of segments in map              }
    ppSegDef:      Word;                { paragraph pointer to first segment    }
    cbMaxSym:      Byte;                { maximum symbol-name length            }
    strModName:    String;              { module name }
  end;

Type
  TLAST_MAPDEF=record
    ppNextMap:   Word;               { always zero                           }
    release:     Byte;               { release number (minor version number) }
    version:     Byte;               { major version number                  }
  end;

Type
  TSEGDEF=record
    ppNextSeg:    Word;                { paragraph pointer to next segment     }
    cSymbols:     Word;                { count of symbols in list              }
    pSymDef:      Word;                { offset of symbol chain                }
    wSegNum:      Word;                { segment number (1 based)              }
    wReserved2:   Word;                { reserved                              }
    wReserved3:   Word;                { reserved                              }
    wReserved4:   Word;                { reserved                              }
    bFlags:       Byte;                { symbol types                          }
    bReserved1:   Byte;                { reserved                              }
    ppLineDef:    Word;                { offset of line number record          }
    bReserved2:   Byte;                { reserved                              }
    bReserved3:   Byte;                { reserved (0xff)                       }
    strSegName:   String;              { segment name                          }
  end;

Type
  TSYMDEF16=record
    wSymVal:       Word;                { symbol address or constant            }
    strSymName:    String;              { symbol name }
  end;

Type
  TSYMDEF32=record
    wSymVal:      Word;                { symbol address or constant            }
    strSymName:    String;              { symbol name }
  end;

Type
  TLINEDEF=record
    ppNextLine:    Word;                { ptr to next linedef (0 if last)       }
    wReserved1:    Word;                { reserved                              }
    pLines:        Word;                { pointer to line numbers               }
    cLines:        Word;                { reserved                              }
    strFileName:   String;              { filename }
  end;

Type
  TLINEINF=record
    wCodeOffset:  Word;  { executable offset                     }
    dwFileOffset: Word;  { source offset                         }
  end;


Const
  SEG_FLAGS_32BIT = $01;      { 32bit segment is set. 16-bit segment if clear }

Function SEG32BitSegment(A: TSegDef): Boolean;
Begin
  SEG32BitSegment:=(a.bFlags and $01)=SEG_FLAGS_32BIT;
End;

{$IFDEF 0}
#define SEGDEFPTR(pvSym, MapDef)   \
            ((PSEGDEF)((char*)(pvSym) + ( (unsigned long)((MAPDEF)(MapDef)).ppSegDef * 16) ) )
#define NEXTSEGDEFPTR(pvSym, SegDef) \
            ((PSEGDEF)( ((SEGDEF)(SegDef)).ppNextSeg != 0 ? \
                        (char*)(pvSym) + ( (unsigned long)((SEGDEF)(SegDef)).ppNextSeg * 16) \
                        : NULL \
                       ) \
             )

#define SEGDEFOFFSET(MapDef)     ((unsigned long)MapDef.ppSegDef*16)
#define NEXTSEGDEFOFFSET(SegDef)  ((unsigned long)SegDef.ppNextSeg*16)


#define ASYMPTROFFSET(SegDefOffset,Segdef) (SegDefOffset+SegDef.pSymDef)
#define SYMDEFOFFSET(SegDefOffset,SegDef,n) (ASYMPTROFFSET(SegDefOffset,SegDef)+(n)*(sizeof(unsigned short int)))

#define SYMDEFPTR32(pvSym, pSegDef, iSym) \
            ((PSYMDEF32) \
            ((char*)(pSegDef) + \
             *(unsigned short int *)((char*)(pSegDef) + ((PSEGDEF)(pSegDef))->pSymDef + (((int)(iSym)) * sizeof(unsigned short int))) \
            ))


#define ACONSTPTROFFSET(MapDef) (MapDef.ppConstDef)
#define CONSTDEFOFFSET(MapDef,n) ((MapDef.ppConstDef)+(n)*(sizeof(unsigned short int)))

#define LINEDEFOFFSET(SegDef) (SegDef.ppLineDef*16))
#define NEXTLINEDEFOFFSET(LineDef) (LineDef.ppNextLine*16)
#define LINESOFFSET(LinedefOffset,LineDef) ((LinedefOffset)+LineDef.pLines)

#define NEXTMAPDEFPTR(pvSym, MapDef) \
            ( (PMAPDEF)( ((MAPDEF)(MapDef)).ppNextMap != 0 ? \
                         (char*)pvSym +  ( (unsigned long)((MAPDEF)(MapDef)).ppNextMap * 16) \
                         : NULL \
                        ) \
             )

{$ENDIF}

Constructor TSymFile.Create(FileName: String);
Var
  A: Word;
  AHdr: TMapDef;
Begin
  inherited Create;

  Assign(AFile, FileName);
  Reset(AFile);
  BlockRead(AFile, AHdr, SizeOf(AHdr), a);
  WriteLn(aHdr.strmodname);
  Close(AFile);
End;

Begin
  {No initialization code}
End.
