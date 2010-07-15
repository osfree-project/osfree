unit common;

interface

uses
{$IFNDEF FPC} VpSysLow;
{$ELSE}       SysLow; {$ENDIF}

Type
  Str3 =        Array[0..2] Of Char;
  Str8 =        Array[0..7] Of Char;
  Str16 =       Array[0..15] Of Char;
  Possible_IFS = dtHDFAT..dtHDJFS ;

Const
  version       = 'osFree installer by osFree Project 2010';
//  Possible_IFS: ARRAY [1..4] of tDrivetype = (dtHDFAT,DThdfat32,dtHDHPFS,dtHDJFS);
  osFree: Str8  = '[osFree]' ;
  Sector0Len    = 512;
  BblockLen     = 8192;
//  8kLen         = 8192;
//  16kLen        = 16384;
  cdfs: Str16   = 'cdfs'#0#0#0#0#0#0#0#0#0#0#0#0;
  ntfs: Str16   = 'ntfs'#0#0#0#0#0#0#0#0#0#0#0#0;
  ext2: Str16   = 'ext2'#0#0#0#0#0#0#0#0#0#0#0#0;
  hpfs: Str16   = 'hpfs'#0#0#0#0#0#0#0#0#0#0#0#0;
  fat:  Str16   = 'fat'#0#0#0#0#0#0#0#0#0#0#0#0#0;
  jfs:  Str16   = 'jfs'#0#0#0#0#0#0#0#0#0#0#0#0#0;

Type
  Sector0Buf =  ARRAY [0..511] of Char;
  hdr =         Packed Record                 // Total size 60 bytes
                jmp:            Word;
                FS_Len:         Word;
                Preldr_Len:     Word;
                data2:          Array[0..7] Of Byte;
                PartitionNr:    Byte;
                data3:          Array[0..28] Of Byte;
                FS:             Str16;
                Data4:          Array[0..BblockLen-61] Of Char;
                End;


  BblockBuf =   Array[0..Bblocklen-1] of Char;
  FAT32Buf =    Packed Record
                // Bootsector (sector 0 = 512 bytes)
                jmp1:           Array[0..2] Of Byte;
                Oem_Id:         Str8;
                Bpb:            Array[0..78] Of Char;
                Boot_Code:      Array[0..410] Of Byte;
                FSD_LoadSeg:    Word;
                FSD_Entry:      Word;
                FSD_Len:        Byte;
                FSD_Addr:       Cardinal;
                Boot_End:       Word;
                Sector1:        Array[0..511] Of Byte;
                // ldr starts from sector 2
                jmp2:           Word;
                FS_Len:         Word;
                Preldr_Len:     Word;
                data2:          Array[0..7] Of Byte;
                PartitionNr:    Byte;
                data3:          Array[0..28] Of Byte;
                FS:             Str16;
                Data4:          Array[0..BblockLen-(61+1024)] Of Char;
                End;

  HPFSBuf =     Packed Record
                // Bootsector (sector 0 = 512 bytes)
                jmp1:           Array[0..2] Of Byte;
                Oem_Id:         Str8;
                Bpb:            Array[0..50] Of Char;
                Boot_Code:      Array[0..438] Of Byte;
                FSD_LoadSeg:    Word;
                FSD_Entry:      Word;
                FSD_Len:        Byte;
                FSD_Addr:       Cardinal;
                Boot_End:       Word;
                // Sector1:        Array[0..511] Of Byte;
                // ldr starts from sector 1
                jmp2:           Word;
                FS_Len:         Word;
                Preldr_Len:     Word;
                data2:          Array[0..7] Of Byte;
                PartitionNr:    Byte;
                data3:          Array[0..28] Of Byte;
                FS:             Str16;
                Data4:          Array[0..BblockLen-(61+512)] Of Char;
                End;

var
  Sector0:      Sector0Buf;
  i:            Integer;
  Drive1:       String;

implementation

{$IFDEF FPC}
{initialisation}
begin
{$ENDIF}
end.
