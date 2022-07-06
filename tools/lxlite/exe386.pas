(****************************************************************************)
(*  Title:       exe386.pas                                                 *)
(*  Description: Data structure definitions for the OS/2                    *)
(*               executable file format (flat model)                        *)
(****************************************************************************)
(*               Copyright (c) IBM Corporation 1987, 1992                   *)
(*                Copyright (c) Microsoft Corp 1988, 1991                   *)
(*            C->Pascal conversion (c) FRIENDS software, 1996               *)
(****************************************************************************)
{&G3+}
{$ifndef fpc}
{&AlignCode-,AlignData-,AlignRec-,Speed-,Frame-,Use32+}
{$else}
{$Align 1}
{$Optimization STACKFRAME}
{$endif}
Unit exe386;

Interface uses miscUtil;

const
 lxMagic         = $584C;             { New magic number  "LX" }
 exeMagic1       = $5A4D;             { EXE file magic number "MZ" }
 exeMagic2       = $4D5A;             { EXE file magic number "ZM" }
 lxResBytes      = 20;                { bytes reserved }
 lxLEBO          = $00;               { Little Endian Byte Order }
 lxBEBO          = $01;               { Big Endian Byte Order }
 lxLEWO          = $00;               { Little Endian Word Order }
 lxBEWO          = $01;               { Big Endian Word Order }
 lxLevel         = 0;                 { 32-bit EXE format level }
 lxCPU286        = $01;               { Intel 80286 or upwardly compatibile }
 lxCPU386        = $02;               { Intel 80386 or upwardly compatibile }
 lxCPU486        = $03;               { Intel 80486 or upwardly compatibile }
 lxCPUP5         = $04;               { Intel P5 or upwardly compatibile }
 lxCPUP6         = $05;               { Intel P6 or upwardly compatibile }
 lxCPUP7         = $06;               { Intel P7 or upwardly compatibile }
 lx386PageSize   = 4096;              { Size of one page in LX file }

type
 pLXheader = ^tLXheader;
 tLXheader = record                   { New 32-bit .EXE header }
  lxMagicID    : Word16;              { magic number LXmagic }
  lxBOrder     : Byte;                { The byte ordering for the .EXE }
  lxWOrder     : Byte;                { The word ordering for the .EXE }
  lxLevel      : Longint;             { The EXE format level for now = 0 }
  lxCpu        : Word16;              { The CPU type }
  lxOS         : Word16;              { The OS type }
  lxVer        : Longint;             { Module version }
  lxMflags     : Longint;             { Module flags }
  lxMpages     : Longint;             { Module # pages }
  lxStartObj   : Longint;             { Object # for instruction pointer }
  lxEIP        : Longint;             { Extended instruction pointer }
  lxStackObj   : Longint;             { Object # for stack pointer }
  lxESP        : Longint;             { Extended stack pointer }
  lxPageSize   : Longint;             { .EXE page size }
  lxPageShift  : Longint;             { Page alignment shift in .EXE }
  lxFixupSize  : Longint;             { Fixup section size }
  lxFixupSum   : Longint;             { Fixup section checksum }
  lxLdrSize    : Longint;             { Loader section size }
  lxLdrSum     : Longint;             { Loader section checksum }
  lxObjTabOfs  : Longint;             { Object table offset }
  lxObjCnt     : Longint;             { Number of objects in module }
  lxObjMapOfs  : Longint;             { Object page map offset }
  lxIterMapOfs : Longint;             { Object iterated data map offset }
  lxRsrcTabOfs : Longint;             { Offset of Resource Table }
  lxRsrcCnt    : Longint;             { Number of resource entries }
  lxResTabOfs  : Longint;             { Offset of resident name table }
  lxEntTabOfs  : Longint;             { Offset of Entry Table }
  lxDirTabOfs  : Longint;             { Offset of Module Directive Table }
  lxDirCnt     : Longint;             { Number of module directives }
  lxFPageTabOfs: Longint;             { Offset of Fixup Page Table }
  lxFRecTabOfs : Longint;             { Offset of Fixup Record Table }
  lxImpModOfs  : Longint;             { Offset of Import Module Name Table }
  lxImpModCnt  : Longint;             { Number of entries in Import Module Name Table }
  lxImpProcOfs : Longint;             { Offset of Import Procedure Name Table }
  lxPageSumOfs : Longint;             { Offset of Per-Page Checksum Table }
  lxDataPageOfs: Longint;             { Offset of Enumerated Data Pages }
  lxPreload    : Longint;             { Number of preload pages }
  lxNResTabOfs : Longint;             { Offset of Non-resident Names Table }
  lxNResTabSize: Longint;             { Size of Non-resident Name Table }
  lxNResSum    : Longint;             { Non-resident Name Table Checksum }
  lxAutoData   : Longint;             { Object # for automatic data object }
  lxDebugInfoOfs:Longint;             { Offset of the debugging information }
                                      { RELATIVE TO START OF EXE FILE}
  lxDebugLen   : Longint;             { The length of the debugging info. in bytes }
  lxInstPreload: Longint;             { Number of instance pages in preload section of .EXE file }
  lxInstDemand : Longint;             { Number of instance pages in demand load section of .EXE file }
  lxHeapSize   : Longint;             { Size of heap - for 16-bit apps }
  lxStackSize  : Longint;
  lxReserved   : array[1..lxResBytes] of Byte;
 end;                                 { Pad structure to 196 bytes }

{ Format of lxMFlags:                                                        }
{                                                                            }
{ Low word has the following format:                                         }
{                                                                            }
{ 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no                            }
{  |     |          | |     | |   |                                          }
{  |     |          | |     | |   +------- Per-Process Library Initialization}
{  |     |          | |     | +----------- No Internal Fixups for Module in .EXE}
{  |     |          | |     +------------- No External Fixups for Module in .EXE}
{  |     |          | +------------------- Incompatible with PM Windowing    }
{  |     |          +--------------------- Compatible with PM Windowing      }
{  |     |                                 Uses PM Windowing API             }
{  |     +-------------------------------- Module not Loadable               }
{  +-------------------------------------- Library Module                    }
const
 lxLibTerm      = $40000000;          { Per-Process Library Termination }
 lxNotP         = $00008000;          { Library Module - used as neNotP }
 lxNoLoad       = $00002000;          { Module not Loadable }
 lxAppMask      = $00000700;          { Application Type Mask }
 lxNoPMwin      = $00000100;          { Incompatible with PM Windowing }
 lxPMwin        = $00000200;          { Compatible with PM Windowing }
 lxPMapi        = $00000300;          { Uses PM Windowing API }
 lxNoIntFix     = $00000010;          { NO Internal Fixups in .EXE }
 lxNoExtFix     = $00000020;          { NO External Fixups in .EXE }
 lxLibInit      = $00000004;          { Per-Process Library Initialization }

{ Format of lxMFlags                                                       }
{                                                                          }
{ High word has the following format:                                      }
{ 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no                          }
{                                   | |                                    }
{                                   | +--- Protected memory library module }
{                                   +----- Device driver                   }
const
 lxEXE          = $00000000;         { .EXE module                     }
 lxDLL          = $00008000;         { Dynamic Link library            }
 lxPMDLL        = $00018000;         { Protected memory library module }
 lxPDD          = $00020000;         { Physical device driver          }
 lxVDD          = $00028000;         { Virtual device driver           }
 lxModType      = $00038000;         { Module type mask                }

{ RELOCATION DEFINITIONS - RUN-TIME FIXUPS }
type
 pOffset = ^tOffset;
 tOffset = record case byte of
  0 : (offset16 : Word16);
  1 : (offset32 : Longint);
 end;                                 { 16-bit or 32-bit offset }

{ ET + lxrrlc - Relocation item }
 pLXreloc = ^tLXreloc;
 tLXreloc = record                    { Relocation item }
  sType        : Byte;                { Source type - field shared with new_rlc }
  Flags        : Byte;                { Flag byte - field shared with new_rlc }
  sOffs        : Word16;              { Source offset }
  ObjMod       : Word16;              { Target object number or Module ordinal }
  Target       : record case Byte of
   0 : (intRef : Longint);
   1 : (extRef : record case byte of
         0 : (Proc : Longint);        { Procedure name offset }
         1 : (Ord  : Longint);        { Procedure ordinal }
        end);
  end;
  addFixup     : Longint;             { Additive fixup value }
  targetCount  : Longint;
  targets      : pWord16Array;
 end;

{ In 32-bit .EXE file run-time relocations are written as varying size }
{ records, so we need many size definitions.                           }
const
 rIntSize16    = 8;
 rIntSize32    = 10;
 rOrdSize      = 8;
 rNamSize16    = 8;
 rNamSize32    = 10;
 rAddSize16    = 10;
 rAddSize32    = 12;

{ Format of NR_STYPE(x)                                         }
{ 7 6 5 4 3 2 1 0  - bit no                                     }
{     | | | | | |                                               }
{     | | +-+-+-+--- Source type                                }
{     | +----------- Fixup to 16:16 alias                       }
{     +------------- List of source offset follows fixup record }
const
 nrSType       = $0F;               { Source type mask }
 nrSByte       = $00;               { lo byte (8-bits)}
 nrSSeg        = $02;               { 16-bit segment (16-bits) }
 nrSPtr        = $03;               { 16:16 pointer (32-bits) }
 nrSOff        = $05;               { 16-bit offset (16-bits) }
 nrPtr48       = $06;               { 16:32 pointer (48-bits) }
 nrOff32       = $07;               { 32-bit offset (32-bits) }
 nrSoff32      = $08;               { 32-bit self-relative offset (32-bits) }

 nrSrcMask     = $0F;               { Source type mask }
 nrAlias       = $10;               { Fixup to alias }
 nrChain       = $20;               { List of source offset follows }
                                    { fixup record, source offset field }
                                    { in fixup record contains number }
                                    { of elements in list }

{ Format of NR_FLAGS(x) and lxrFLAGS(x):                                  }
{ 7 6 5 4 3 2 1 0  - bit no                                               }
{ | | | | | | | |                                                         }
{ | | | | | | +-+--- Reference type                                       }
{ | | | | | +------- Additive fixup                                       }
{ | | | | +--------- New-type chained fixup  (OS/2 v4.0+?)                }
{ | | | +----------- 32-bit Target Offset Flag (1 - 32-bit; 0 - 16-bit)   }
{ | | +------------- 32-bit Additive Flag (1 - 32-bit; 0 - 16-bit)        }
{ | +--------------- 16-bit Object/Module ordinal (1 - 16-bit; 0 - 8-bit) }
{ +----------------- 8-bit import ordinal (1 - 8-bit;                     }
{                                          0 - NR32BITOFF toggles         }
{                                              between 16 and 32 bit      }
{                                              ordinal)                   }
const
 nrRtype       = $03;               { Reference type mask }
 nrRint        = $00;               { Internal reference }
 nrRord        = $01;               { Import by ordinal }
 nrRnam        = $02;               { Import by name }
 nrRent        = $03;               { Internal entry table fixup }
 nrAdd         = $04;               { Additive fixup }

 nrNewChain    = $08;               { New-type chains introduced in v4.0 }
 nr32bitOff    = $10;               { 32-bit Target Offset }
 nr32bitAdd    = $20;               { 32-bit Additive fixup }
 nr16objMod    = $40;               { 16-bit Object/Module ordinal }
 nr8bitOrd     = $80;               { 8-bit import ordinal }

{ OBJECT TABLE }

{ Object Table Entry }
type
 pObjTblRec = ^tObjTblRec;
 tObjTblRec = record                { Flat .EXE object table entry }
  oSize     : Longint;              { Object virtual size }
  oBase     : Longint;              { Object base virtual address }
  oFlags    : Longint;              { Attribute flags }
  oPageMap  : Longint;              { Object page map index }
  oMapSize  : Longint;              { Number of entries in object page map }
  oReserved : Longint;              { Reserved }
 end;

{ Format of oFlags                                                           }
{                                                                            }
{ High word of dword flag field is not used for now.                         }
{ Low word has the following format:                                         }
{ 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no                            }
{  |  |  |  |     | | | | | | | | | | |                                      }
{  |  |  |  |     | | | | | | | | | | +--- Readable Object                   }
{  |  |  |  |     | | | | | | | | | +----- Writeable Object                  }
{  |  |  |  |     | | | | | | | | +------- Executable Object                 }
{  |  |  |  |     | | | | | | | +--------- Resource Object                   }
{  |  |  |  |     | | | | | | +----------- Object is Discardable             }
{  |  |  |  |     | | | | | +------------- Object is Shared                  }
{  |  |  |  |     | | | | +--------------- Object has preload pages          }
{  |  |  |  |     | | | +----------------- Object has invalid pages          }
{  |  |  |  |     | | +------------------- Object is permanent and swappable }
{  |  |  |  |     | +--------------------- Object is permanent and resident  }
{  |  |  |  |     +----------------------- Object is permanent and long lockable}
{  |  |  |  +----------------------------- 16:16 alias required (80x86 specific)}
{  |  |  +-------------------------------- Big/Default bit setting (80x86 specific)}
{  |  +----------------------------------- Object is conforming for code (80x86 specific)}
{  +-------------------------------------- Object I/O privilege level (80x86 specific)}

const
 objRead       = $00000001;         { Readable object   }
 objWrite      = $00000002;         { Writeable object  }
 objExec       = $00000004;         { Executable object }
 objResource   = $00000008;         { Resource object   }
 objDiscard    = $00000010;         { object is Discardable }
 objShared     = $00000020;         { object is Shared }
 objPreload    = $00000040;         { object has preload pages  }
 objInvalid    = $00000080;         { object has invalid pages  }
 lnkNonPerm    = $00000600;         { object is nonpermanent - should be }
 objNonPerm    = $00000000;         { zero in the .EXE but LINK386 uses 6 }
 objPerm       = $00000100;         { object is permanent and swappable }
 objResident   = $00000200;         { object is permanent and resident }
 objContig     = $00000300;         { object is resident and contiguous }
 objDynamic    = $00000400;         { object is permanent and long locable }
 objTypeMask   = $00000700;         { object type mask }
 objAlias16    = $00001000;         { 16:16 alias required (80x86 specific) }
 objBigDef     = $00002000;         { Big/Default bit setting (80x86 specific) }
 objConform    = $00004000;         { object is conforming for code (80x86 specific)  }
 objIOPL       = $00008000;         { object I/O privilege level (80x86 specific) }

{ object Page Map entry }

type
 pObjMapRec = ^tObjMapRec;
 tObjMapRec = record                 { object Page Table entry }
  PageDataOffset : Longint;          { file offset of page }
  PageSize       : Word16;           { # bytes of page data }
  PageFlags      : Word16;           { Per-Page attributes }
 end;

const
 pgValid       = $0000;              { Valid Physical Page in .EXE }
 pgIterData    = $0001;              { Iterated Data Page }
 pgInvalid     = $0002;              { Invalid Page }
 pgZeroed      = $0003;              { Zero Filled Page }
 pgRange       = $0004;              { Range of pages }
 pgIterData2   = $0005;              { Iterated Data Page Type II }

{ RESOURCE TABLE }

{ tResource - Resource Table Entry }
type
 pResource = ^tResource;
 tResource = record                   { Resource Table Entry }
  resType : Word16;                   { Resource type }
  resName : Word16;                   { Resource name }
  resSize : Longint;                  { Resource size }
  resObj  : Word16;                   { Object number }
  resOffs : Longint;                  { Offset within object }
 end;

{ Iteration Record format for 'EXEPACK'ed pages. (DCR1346)  }
 pIterRec = ^tIterRec;
 tIterRec = record
  nIter    : Word16;                  { number of iterations }
  nBytes   : Word16;                  { number of bytes }
  IterData : Byte;                    { iterated data byte(s) }
 end;

{ ENTRY TABLE DEFINITIONS }

{ Entry Table bundle }
 pEntryTblRec = ^tEntryTblRec;
 tEntryTblRec = record
  Count   : Byte;                     { Number of entries in this bundle }
  BndType : Byte;                     { Bundle type }
  Obj     : Word16;                   { object number }
 end;                                 { Follows entry types }

 pEntry = ^tEntry;
 tEntry = record
  Flags   : Byte;                     { Entry point flags }
  Variant : record case byte of       { Entry variant }
   0 : (Offset : tOffset);            { 16-bit/32-bit offset entry }
   1 : (CallGate : record
         Offset   : Word16;           { Offset in segment }
         Selector : Word16;           { Callgate selector }
        end);
   2 : (Fwd : record                  { Forwarder }
         ModOrd : Word16;             { Module ordinal number }
         Value  : Longint;            { Proc name offset or ordinal }
        end);
  end;
 end;

{ Module format directive table }
type
 pDirTabRec = ^tDirTabRec;
 tDirTabRec = record
  DirN    : Word16;
  DataLen : Word16;
  DataOfs : Longint;
 end;

const
 dtResident = $8000;
 dtVerify   = $0001;
 dtLangInfo = $0002;
 dtCoProc   = $0003;
 dtThreadSt = $0004;
 dtCSetBrws = $0005;

const
 fixEnt16      = 3;
 fixEnt32      = 5;
 gateEnt16     = 5;
 fwdEnt        = 7;

{ BUNDLE TYPES }
const
 btEmpty       = $00;                 { Empty bundle }
 btEntry16     = $01;                 { 16-bit offset entry point }
 btGate16      = $02;                 { 286 call gate (16-bit IOPL) }
 btEntry32     = $03;                 { 32-bit offset entry point }
 btEntryFwd    = $04;                 { Forwarder entry point }
 btTypeInfo    = $80;                 { Typing information present flag }

{ Format for lxEflags                      }
{                                          }
{  7 6 5 4 3 2 1 0  - bit no               }
{  | | | | | | | |                         }
{  | | | | | | | +--- exported entry       }
{  | | | | | | +----- uses shared data     }
{  +-+-+-+-+-+------- parameter word count }
const
 lxExport      = $01;                 { Exported entry }
 lxShared      = $02;                 { Uses shared data }
 lxParams      = $F8;                 { Parameter word count mask }
 lxParamsShft  = 3;                   { Count of times to shift right }

{ Flags for forwarders only: }
const
 fwd_Ordinal   = $01;                 { Imported by ordinal }

{Entry table bundle element structure}
type
 pLXentryPoint = ^tLXentryPoint;
 tLXentryPoint = record case byte of
  btEmpty    : ();
  btEntry16  : (e16Flags : Byte; e16Ofs : Word16);
  btGate16   : (eGate16Flags : Byte; eGate16Ofs, eGate16Sel : Word16);
  btEntry32  : (e32Flags : Byte; e32Ofs : Longint);
  btEntryFwd : (eForwFlags : Byte; eForwModOrd : Word16; eForwOfs : Longint);
 end;

{Name table entry record used to keep name table in memory}
type
 pNameTblRec = ^tNameTblRec;
 tNameTblRec = record
  Name : pString;
  Ord  : Word16;
 end;

Implementation

end.
