{ .SYM symbol files format support }

Unit SYM;

{$IFDEF FPC}
  {$MODE OBJFPC}
  {$H-} // Don't use huge strings by default. Use pascal strings.
{$ENDIF}

Interface

Type
  TConstant=record
    Value: Word;
    Name: String;
  end;

Type
  TSymbol=record
    Value: Longint;
    Name: String;
  end;

Type
  TSegment=record
    Number: Word;
    Name: String;
    SymbolsCount: Word;
    Symbols: Array of TSymbol;
  end;

Type
  TSYMFile=Class
  private
    AFile: File of Byte;
    bFlags:        Byte;                { symbol types                          }
    bReserved1:    Byte;                { reserved                              }
    pSegEntry:     Word;                { segment entry point value             }
    pConstDef:     Word;                { pointer to constant chain             }
    ppSegDef:      Word;                { paragraph pointer to first segment    }
    cbMaxSym:      Byte;                { maximum symbol-name length            }
  public
    constructor Create(FileName: String);
    ModuleName:    String;              { module name }
    ConstantsCount:Word;                { count of constants in map             }
    SegmentsCount: Word;                { count of segments in map              }
    Constants: Array of TConstant;
    Segments: Array of TSegment;
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
    wSymVal:       Longint;                { symbol address or constant            }
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
  SEG32BitSegment:=(a.bFlags and SEG_FLAGS_32BIT)=SEG_FLAGS_32BIT;
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

Function offset(a: word): word;
begin
  offset:=a * $10;
end;

Constructor TSymFile.Create(FileName: String);
Var
  A: Word;
  AHdr: TMapDef;
  ASeg: TSegDef;
  ASym16: TSymDef16;
  ASym32: TSymDef32;
  I: word;
  J: word;
  TempOffset: Word;
  SegmentOffset: Word;
Begin
  inherited Create;

  Assign(AFile, FileName);
  Reset(AFile);
  BlockRead(AFile, AHdr, SizeOf(AHdr), a);

  bFlags:=AHdr.bFlags;
  bReserved1:=AHdr.bReserved1;
  pSegEntry:=AHdr.pSegEntry;
  ConstantsCount:=AHdr.cConsts;
  pConstDef:=AHdr.pConstDef;
  SegmentsCount:=AHdr.cSegs;
  ppSegDef:=AHdr.ppSegDef;
  cbMaxSym:=AHdr.cbMaxSym;
  ModuleName:=AHdr.strModName;

//  While (offset(AHdr.ppNextMap)<>0) do
//  begin
//    WriteLn(AHdr.strModName);
//    Seek(AFile, offset(AHdr.ppNextMap));
//    BlockRead(AFile, AHdr, SizeOf(AHdr), a);
//  end;

//  Seek(AFile,pConstDef);
  If ConstantsCount>0 then
  begin
    SetLength(Constants, ConstantsCount);
    For I:=0 to ConstantsCount-1 do
    begin
      Seek(AFile,pConstDef+I*2);
      BlockRead(AFile, TempOffset, SizeOf(Word), a);
      Seek(AFile,TempOffset);
      BlockRead(AFile, asym16, SizeOf(TSymDef16), a);
      Constants[I].Name:=asym16.strSymname;
      Constants[I].Value:=asym16.wSymVal;
    end;
  end;

  Seek(AFile,offset(ppSegDef));

  SegmentOffset:=offset(ppSegDef);
  If SegmentsCount>0 then
  begin
    SetLength(Segments, SegmentsCount);
    For I:=0 to SegmentsCount-1 do
    begin
      BlockRead(AFile, ASeg, SizeOf(TSegDef), a);
      Segments[I].Name:=aseg.strSegname;
      Segments[I].SymbolsCount:=aseg.cSymbols;
      Segments[I].Number:=aseg.wSegNum;

      WriteLn(aseg.wReserved2);
      WriteLn(aseg.wReserved3);
      WriteLn(aseg.wReserved4);

      WriteLn(aseg.bReserved1);
      WriteLn(aseg.bReserved2);
      WriteLn(aseg.bReserved3);

      SetLength(Segments[I].Symbols, Segments[I].SymbolsCount);

      For J:=0 to aseg.cSymbols-1 do
      begin
        Seek(AFile,Segmentoffset+aseg.pSymDef+J*2);
        BlockRead(AFile, TempOffset, SizeOf(Word), a);
        Seek(AFile,SegmentOffset+TempOffset);
        If SEG32BitSegment(ASeg) then
        begin
          BlockRead(AFile, asym32, SizeOf(TSymDef32), a);
          Segments[I].Symbols[J].Name:=asym32.strSymname;
          Segments[I].Symbols[J].Value:=asym32.wSymVal;
        end else begin
          BlockRead(AFile, ASym16, SizeOf(TSymDef16), a);
          Segments[I].Symbols[J].Name:=asym16.strSymname;
          Segments[I].Symbols[J].Value:=asym16.wSymVal;
        end;
      end;

      Seek(AFile,offset(aseg.ppNextSeg));
      SegmentOffset:=offset(aseg.ppNextSeg);
    end;
  end;

  Close(AFile);
End;

Begin
  {No initialization code}
End.
