//
// common types
//

type
  ApiRet  = Longint;
  ApiRet16 = Word;
  SHandle = Word;
  LHandle = Longint;
  HPointer = LHandle;
  FHLock  = Longint;
  ULong   = Longint;
  Long    = Longint;
  Bool    = LongBool;
  UShort  = Word;
  PULong  = ^ULong;
  PLong   = ^Long;
  PFn     = Pointer;
  PUShort = ^UShort;
  PPChar  = ^PChar;

{ Quad-word }
  PQWord = ^QWord;
  QWord = record
   Lo: ULong;
   Hi: ULong;
 end;

  HIni                    = LHandle;
  tClipHack               = (clipInit, clipFailed, clipOk);
  tPMInit                 = (pmUntested, pmOK, pmFailed);

  HModule = LHandle;
  Pid     = LHandle;
  Tid     = LHandle;
  Sgid    = Word;

  PHmodule = ^HModule;
  PPid     = ^Pid;
  PTid     = ^Tid;
  HFile    = LongWord; //SHandle;
  pHFile   = ^HFile;

{ Common SUP types }
  Hab = LHandle;
  PHab = ^Hab;

const
{ cchMaxPath is the maximum fully qualified path name length including  }
{ the drive letter, colon, backslashes and terminating #0 symbol.       }
  cchMaxPath                    = 260;

{ cchMaxPathComp is the maximum individual path component name length   }
{ including a terminating #0 symbol.                                    }
  cchMaxPathComp                = 256;

const
{ indices for DosQuerySysInfo }
  qsv_Max_Path_Length           = 1;
  q_Max_Path_Length             = qsv_Max_Path_Length;
  qsv_Max_Text_Sessions         = 2;
  qsv_Max_Pm_Sessions           = 3;
  qsv_Max_Vdm_Sessions          = 4;
  qsv_Boot_Drive                = 5;      { 1=A, 2=B, etc. }
  qsv_Dyn_Pri_Variation         = 6;      { 0=Absolute, 1=Dynamic }
  qsv_Max_Wait                  = 7;      { seconds }
  qsv_Min_Slice                 = 8;      { milli seconds }
  qsv_Max_Slice                 = 9;      { milli seconds }
  qsv_Page_Size                 = 10;
  qsv_Version_Major             = 11;
  qsv_Version_Minor             = 12;
  qsv_Version_Revision          = 13;     { Revision letter }
  qsv_Ms_Count                  = 14;     { Free running millisecond counter }
  qsv_Time_Low                  = 15;     { Low dWord of time in seconds }
  qsv_Time_High                 = 16;     { High dWord of time in seconds }
  qsv_TotPhysMem                = 17;     { Physical memory on system }
  qsv_TotResMem                 = 18;     { Resident memory on system }
  qsv_TotAvailMem               = 19;     { Available memory for all processes }
  qsv_MaxPrMem                  = 20;     { Avail private mem for calling proc }
  qsv_MaxShMem                  = 21;     { Avail shared mem for calling proc }
  qsv_Timer_Interval            = 22;     { Timer interval in tenths of ms }
  qsv_Max_Comp_Length           = 23;     { max len of one component in a name }
{ OS/2 3.0+ only }
  qsv_Foreground_Fs_Session     = 24;     { Session ID of current fgnd FS session }
  qsv_Foreground_Process        = 25;     { Process ID of current fgnd process }
  qsv_Max                       = qsv_Foreground_Process;

type
  PTib2 = ^Tib2;                        { Thread Information Block (TIB)      }
  Tib2  = record                        { System specific portion             }
    Tib2_ulTid:           ULong;        { Thread I.D.                         }
    Tib2_ulPri:           ULong;        { Thread priority                     }
    Tib2_Version:         ULong;        { Version number for this structure   }
    Tib2_usMCCount:       Word;    { Must Complete count                 }
    Tib2_fMCForceFlag:    Word;    { Must Complete force flag            }
  end;

  PTib = ^Tib;
  Tib  = record                         { TIB Thread Information Block        }
    Tib_PExchain:         Pointer;      { Head of exception handler chain     }
    Tib_PStack:           Pointer;      { Pointer to base of stack            }
    Tib_PStackLimit:      Pointer;      { Pointer to end of stack             }
    Tib_PTib2:            PTib2;        { Pointer to system specific TIB      }
    Tib_Version:          ULong;        { Version number for this TIB structure}
    Tib_Ordinal:          ULong;        { Thread ordinal number               }
  end;

  PPib = ^Pib;
  Pib  = record                         { Process Information Block (PIB)     }
    Pib_ulPid:            ULong;        { Process I.D.                        }
    Pib_ulPPid:           ULong;        { Parent process I.D.                 }
    Pib_hMte:             ULong;        { Program (.EXE) module handle        }
    Pib_pchCmd:           PChar;        { Command line pointer                }
    Pib_pchEnv:           PChar;        { Environment pointer                 }
    Pib_flStatus:         ULong;        { Process' status bits                }
    Pib_ulType:           ULong;        { Process' type code                  }
  end;

const
{ File info levels&gml All listed API's }
  fil_Standard                  = 1;    { Info level 1, standard file info }
  fil_QueryEaSize               = 2;    { Level 2, return Full EA size     }
  fil_QueryEasFromList          = 3;    { Level 3, return requested EA's   }

{ File info levels: Dos...PathInfo only }
  fil_QueryFullName             = 5;    { Level 5, return fully qualified name of file }
{ DosFsAttach: Attact or detach }
  fs_Attach                     = 0;    { Attach file server }
  fs_Detach                     = 1;    { Detach file server }
  fs_SpoolAttach                = 2;    { Register a spooler device }
  fs_SpoolDetach                = 3;    { De-register a spooler device }

{ DosFsCtl: Routing type }
  fsctl_Handle                  = 1;    { File Handle directs req routing }
  fsctl_PathName                = 2;    { Path Name directs req routing }
  fsctl_FsdName                 = 3;    { FSD Name directs req routing }

{ defined FSCTL functions }
  fsctl_Error_Info              = 1;    { return error info from FSD }
  fsctl_Max_EaSize              = 2;    { Max ea size for the FSD }

{ DosQueryFSAttach: Information level types (defines method of query) }
const
  fsail_QueryName               = 1;    { Return data for a Drive or Device }
  fsail_DevNumber               = 2;    { Return data for Ordinal Device # }
  fsail_DrvNumber               = 3;    { Return data for Ordinal Drive # }

{ Item types (from data structure item "iType") }
  fsat_CharDev                  = 1;    { Resident character device }
  fsat_PseudoDev                = 2;    { Pusedu-character device }
  fsat_LocalDrv                 = 3;    { Local drive }
  fsat_RemoteDrv                = 4;    { Remote drive attached to FSD }

{ File System Drive Information&gml DosQueryFSInfo DosSetFSInfo }
{ FS Drive Info Levels }
const
  fsil_Alloc                    = 1;    { Drive allocation info (Query only) }
  fsil_VolSer                   = 2;    { Drive Volum/Serial info }

{ File time and date types }
(*
      typedef struct _FTIME
              {
              UINT   twosecs : 5;
              UINT   minutes : 6;
              UINT   hours   : 5;
              } FTIME;
*)

type
  PFTime = ^FTime;
  FTime = Word;

const
  mftTwoSecs                    = $001F;
  sftTwoSecs                    = 0;
  mftMinutes                    = $07E0;
  sftMinutes                    = 5;
  mftHours                      = $F800;
  sftHours                      = 11;

(*    typedef struct _FDATE
              {
              UINT   day     : 5;
              UINT   month   : 4;
              UINT   year    : 7;
              } FDATE;
*)

type
  PFDate = ^FDate;
  FDate = Word;

const
  mfdDay                        = $001F;
  sfdDay                        = 0;
  mfdMonth                      = $01E0;
  sfdMonth                      = 5;
  mfdYear                       = $FE00;
  sfdYear                       = 9;

type
  PVolumeLabel = ^VolumeLabel;
  VolumeLabel = String[12];


const
{ DosOpen/DosQFHandState/DosQueryFileInfo et al file attributes; also }
{ known as Dos File Mode bits... }
  file_Normal                   = $0000;
  file_ReadOnly                 = $0001;
  file_Hidden                   = $0002;
  file_System                   = $0004;
  file_Directory                = $0010;
  file_Archived                 = $0020;
  file_Ignore                   = $10000;       { ignore file attribute = in }
                                                { DosSetPath/File Info if    }
                                                { this bit is set            }
const
  msg_Net_Dev_Type_Invalid      = 66;
{*** File manager ***}

{ DosSetFilePtr file position codes }
  file_Begin                    = $0000; { Move relative to beginning of = file }
  file_Current                  = $0001; { Move relative to current fptr = position }
  file_End                      = $0002; { Move relative to end of = file }

{ DosFindFirst/Next Directory handle types }
  hdir_System                   =  1;    { Use system handle (= 1)         }
  hdir_Create                   = -1;    { Allocate a new, unused = handle }

{ DosCopy control bits; may be or'ed together }
  dcpy_Existing                 = $0001; { Copy even if target = exists }
  dcpy_Append                   = $0002; { Append to existing file, do not = replace }
  dcpy_FailEas                  = $0004; { Fail if EAs not supported on = target }


{ definitions for DosSearchPath control Word }
const
  dsp_ImpliedCur                = 1;    { current dir will be searched first }
  dsp_PathRef                   = 2;    { from env.variable }
  dsp_IgnoreNetErr              = 4;    { ignore net errs & continue search }

const
  info_Count_Partitionable_Disks = 1;   { # of partitionable disks }
  info_GetIoctlHandle            = 2;   { Obtain handle            }
  info_FreeIoctlHandle           = 3;   { Release handle           }
