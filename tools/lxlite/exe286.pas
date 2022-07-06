(****************************************************************************)
(*  Title:       exe286.pas                                                 *)
(*  Description: Data structure definitions for the OS/2 old executable     *)
(*               file format (segmented model)                              *)
(****************************************************************************)
(*                  (C) Copyright IBM Corp 1984-1992                        *)
(*               (C) Copyright Microsoft Corp 1984-1987                     *)
(*            C->Pascal conversion (c) FRIENDS software, 1996               *)
(****************************************************************************)
{&G3+}
{$ifndef fpc}
{$AlignCode-,AlignData-,AlignRec-,Speed-,Frame-,Use32+}
{$else}
{$Align 1}
{$Optimization STACKFRAME}
{$endif}
Unit exe286;

Interface uses MiscUtil;

{ DOS EXE file header structure }
type
 pEXEheader = ^tEXEheader;
 tEXEheader = record
  exeMagic    : Word16;             { Magic number }
  exeCbLP     : Word16;             { Bytes on last page of file }
  exeCp       : Word16;             { Pages in file }
  exeCrlc     : Word16;             { Relocations }
  exeCparhdr  : Word16;             { Size of header in paragraphs }
  exeMinAlloc : Word16;             { Minimum extra paragraphs needed }
  exeMaxAlloc : Word16;             { Maximum extra paragraphs needed }
  exeSS       : Word16;             { Initial (relative) SS value }
  exeSP       : Word16;             { Initial SP value }
  exeCRC      : Word16;             { Checksum }
  exeIP       : Word16;             { Initial IP value }
  exeCS       : Word16;             { Initial (relative) CS value }
  exeLfarlc   : Word16;             { File address of relocation table }
  exeOvNo     : Word16;             { Overlay number }
  exeRes      : array[1..4] of Word16;{ Reserved words }
  exeOEMid    : Word16;             { OEM identifier (for exeOEMinfo) }
  exeOEMinfo  : Word16;             { OEM information; exeOEMid specific }
  exeRes2     : array[1..10] of Word16;{ Reserved words }
  exeLFAnew   : Longint;            { File address of new exe header }
 end;

(*-----------------------------------------------------------------*)
(*  OS/2 & WINDOWS .EXE FILE HEADER DEFINITION - 286 version       *)
(*-----------------------------------------------------------------*)

const
 neMagic          = $454E;          { `New` magic number }
 neDebugMagic     = $424E;          { 'NB', codeview debug-info signature }
 neResBytes       = 8;              { Eight bytes reserved (now) }
 neCRC            = 8;              { Offset into new header of neCRC }

type
 pNEheader = ^tNEheader;
 tNEheader = record                 { `New` .EXE header }
  neMagic       : Word16;           { Magic number neMAGIC }
  neVer         : Byte;             { Version number }
  neRev         : Byte;             { Revision number }
  neEntTab      : Word16;           { Offset of Entry Table }
  neCbEntTab    : Word16;           { Number of bytes in Entry Table }
  neCRC         : Longint;          { Checksum of whole file }
  neFlags       : Word16;           { Flag word }
  neAutoData    : Word16;           { Automatic data segment number }
  neHeap        : Word16;           { Initial heap allocation }
  neStack       : Word16;           { Initial stack allocation }
  neCSIP        : Longint;          { Initial CS:IP setting }
  neSSSP        : Longint;          { Initial SS:SP setting }
  neCSeg        : Word16;           { Count of file segments }
  neCMod        : Word16;           { Entries in Module Reference Table }
  neCbNResTab   : Word16;           { Size of non-resident name table }
  neSegTab      : Word16;           { Offset of Segment Table }
  neRsrcTab     : Word16;           { Offset of Resource Table }
  neResTab      : Word16;           { Offset of resident name table }
  neModTab      : Word16;           { Offset of Module Reference Table }
  neImpTab      : Word16;           { Offset of Imported Names Table }
  neNResTab     : Longint;          { Offset of Non-resident Names Table }
  neCMovEnt     : Word16;           { Count of movable entries }
  neAlign       : Word16;           { Segment alignment shift count }
  neCRes        : Word16;           { Count of resource entries }
  neExeTyp      : Byte;             { Target operating system }
  neFlagsOthers : Byte;             { Other .EXE flags }
  neReserved    : array[1..neResBytes] of byte;{ Pad structure to 64 bytes }
 end;

{* Target operating systems *}
const
 neUnknown = $0;                    { Unknown (any "new-format" OS) }
 neOS2     = $1;                    { OS/2 (default)  }
 neWindows = $2;                    { Windows }
 neDos4    = $3;                    { DOS 4.x }
 neDev386  = $4;                    { Windows 386 }

{*  Format of NE_FLAGS(x):                                                  *}
{*  p                                   Not-a-process                       *}
{*   x                                  Unused                              *}
{*    e                                 Errors in image                     *}
{*     x                                Unused                              *}
{*      b                               Bound Family/API                    *}
{*       ttt                            Application type                    *}
{*          f                           Floating-point instructions         *}
{*           3                          386 instructions                    *}
{*            2                         286 instructions                    *}
{*             0                        8086 instructions                   *}
{*              P                       Protected mode only                 *}
{*               p                      Per-process library initialization  *}
{*                i                     Instance data                       *}
{*                 s                    Solo data                           *}
const
 neNotP         = $8000;            { Not a process }
 neIerr         = $2000;            { Errors in image }
 neBound        = $0800;            { Bound Family/API }
 neAppTyp       = $0700;            { Application type mask }
 neNotWinCompat = $0100;            { Not compatible with P.M. Windowing }
 neWinCompat    = $0200;            { Compatible with P.M. Windowing }
 neWinAPI       = $0300;            { Uses P.M. Windowing API }
 neFltP         = $0080;            { Floating-point instructions }
 neI386         = $0040;            { 386 instructions }
 neI286         = $0020;            { 286 instructions }
 neI086         = $0010;            { 8086 instructions }
 neProt         = $0008;            { Runs in protected mode only }
 nePPLI         = $0004;            { Per-Process Library Initialization }
 neInst         = $0002;            { Instance data }
 neSolo         = $0001;            { Solo data }

{*  Format of NE_FLAGSOTHERS(x):                                            *}
{*                                                                          *}
{*      7 6 5 4 3 2 1 0  - bit no                                           *}
{*      |         | | |                                                     *}
{*      |         | | +---------------- Support for long file names         *}
{*      |         | +------------------ Windows 2.x app runs in prot mode   *}
{*      |         +-------------------- Windows 2.x app gets prop. font     *}
{*      +------------------------------ WLO appl on OS/2 (markwlo.exe)      *}
const
 neLongFileNames = $01;
 neWinIsProt     = $02;
 neWinGetPropFon = $04;
 neWLoAppl       = $80;

type
 pNEseg = ^tNEseg;
 tNEseg = record                    { New .EXE segment table entry }
  Sector   : Word16;                { File sector of start of segment }
  CbSeg    : Word16;                { Number of bytes in file }
  Flags    : Word16;                { Attribute flags }
  MinAlloc : Word16;                { Minimum allocation in bytes }
 end;

{*  Format of NS_FLAGS(x)                                                   *}
{*                                                                          *}
{*  Flag word has the following format:                                     *}
{*                                                                          *}
{*      15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no                     *}
{*          |  |  |  |  | | | | | | | | | | |                               *}
{*          |  |  |  |  | | | | | | | | +-+-+--- Segment type DATA/CODE     *}
{*          |  |  |  |  | | | | | | | +--------- Iterated segment           *}
{*          |  |  |  |  | | | | | | +----------- Movable segment            *}
{*          |  |  |  |  | | | | | +------------- Segment can be shared      *}
{*          |  |  |  |  | | | | +--------------- Preload segment            *}
{*          |  |  |  |  | | | +----------------- Execute/read-only for code/data segment*}
{*          |  |  |  |  | | +------------------- Segment has relocations    *}
{*          |  |  |  |  | +--------------------- Code conforming/Data is expand down*}
{*          |  |  |  +--+----------------------- I/O privilege level        *}
{*          |  |  +----------------------------- Discardable segment        *}
{*          |  +-------------------------------- 32-bit code segment        *}
{*          +----------------------------------- Huge segment/GDT allocation requested *}
const
 nesType     = $0007;               { Segment type mask }

 nesCode     = $0000;               { Code segment }
 nesData     = $0001;               { Data segment }
 nesIter     = $0008;               { Iterated segment flag }
 nesMove     = $0010;               { Movable segment flag }
 nesShared   = $0020;               { Shared segment flag }
 nesPreload  = $0040;               { Preload segment flag }
 nesExRdOnly = $0080;               { Execute-only (code segment), or read-only (data segment) }
 nesReloc    = $0100;               { Segment has relocations }
 nesConform  = $0200;               { Conforming segment }
 nesExpDown  = $0200;               { Data segment is expand down }
 nesDPL      = $0C00;               { I/O privilege level (286 DPL bits) }
 nesDiscard  = $1000;               { Segment is discardable }
 nes32bit    = $2000;               { 32-bit code segment }
 nesHuge     = $4000;               { Huge memory segment, length of   }
                                    { segment and minimum allocation   }
                                    { size are in segment sector units }
 nesGDT      = $8000;               { GDT allocation requested }
 nesPure     = nesShared;           { For compatibility }
 nesAlign    = 9;                   { Segment data aligned on 512 byte boundaries }
 nesLoaded   = $0004;               { nesSector field contains memory addr }

type
 tNEsegData = record
  case boolean of
   TRUE  : (Iter : record
                    nIter  : Word16;      { number of iterations }
                    nBytes : Word16;      { number of bytes }
                    Data   : array[0..0] of Byte;{ iterated data bytes }
                   end);
   FALSE : (Data : array[0..0] of Byte);
  end;

 tNErelocInfo = record              { Relocation info }
  nReloc : Word16;                  { number of relocation items that follow }
 end;

 pNEreloc = ^tNEreloc;
 tNEreloc = record
  sType : Byte;                     { Source type }
  flags : Byte;                     { Flag byte }
  soff  : Word16;                   { Source offset }
  rel   : record case byte of
  { Internal reference }
   0 : (segNo   : Byte;            { Target segment number }
        Resvd   : Byte;            { Reserved }
        Entry   : Word16);         { Target Entry Table offset }
  { Import }
   1 : (modIndx : Word16;          { Index into Module Reference Table }
        Proc    : Word16);         { Procedure ordinal or name offset }
  { Operating system fixup }
   2 : (osType  : Word16;          { OSFIXUP type }
        osRes   : Word16);         { reserved }
  end;
 end;

{*  Format of NR_STYPE(x) and R32_STYPE(x):   *}
{*                                            *}
{*       7 6 5 4 3 2 1 0  - bit no            *}
{*               | | | |                      *}
{*               +-+-+-+--- source type       *}
const
 nerSType  = $0F;                   { Source type mask }
 nerSByte  = $00;                   { lo byte (8-bits)}
 nerSSeg   = $02;                   { 16-bit segment (16-bits) }
 nerSPtr   = $03;                   { 16:16 pointer (32-bits) }
 nerSoff   = $05;                   { 16-bit offset (16-bits) }
 nerPtr48  = $06;                   { 16:32 pointer (48-bits) }
 nerOff32  = $07;                   { 32-bit offset (32-bits) }
 nerSOff32 = $08;                   { 32-bit self-relative offset (32-bits) }

{*  Format of NR_FLAGS(x) and R32_FLAGS(x):   *}
{*                                            *}
{*       7 6 5 4 3 2 1 0  - bit no            *}
{*                 | | |                      *}
{*                 | +-+--- Reference type    *}
{*                 +------- Additive fixup    *}
const
 nerRTyp = $03;                     { Reference type mask }
 nerRInt = $00;                     { Internal reference }
 nerROrd = $01;                     { Import by ordinal }
 nerRNam = $02;                     { Import by name }
 nerROsf = $03;                     { Operating system fixup }
 nerAdd  = $04;                     { Additive fixup }

type
 pNEentryBundle = ^tNEentryBundle;
 tNEentryBundle = record
  Flags : Byte;
  Ref   : record case byte of
   0 : (fixOfs : Word16);
   1 : (movInt3F : Word16; movSegNo : Byte; movOfs : Word16);
  end;
 end;

{Type definition for resource description blocks}
{(REAL ONE, NOT FROM WINDOWS EXECUTABLES!)}
type
 tNEresource = record
  resType : Word16;
  resID   : Word16;
 end;

Implementation

end.
