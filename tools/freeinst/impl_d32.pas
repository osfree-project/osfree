{**************************************
 *  System-dependent implementation   *
 *  of low-level functions for DPMI32 *
 **************************************}

unit Impl_D32

interface


procedure Open_Disk(Drive: PChar; var DevHandle: Hfile);
procedure Read_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Write_Disk(devhandle: Hfile; var buf; buf_len: Ulong);
procedure Close_Disk(DevHandle: Hfile);
procedure Lock_Disk(DevHandle: Hfile);
procedure Unlock_Disk(DevHandle: Hfile);

procedure Read_MBR_Sector(DriveNum: char; var MBRBuffer);
procedure Write_MBR_Sector(DriveNum: char; var MBRBuffer);
procedure Backup_MBR_Sector;
procedure Restore_MBR_Sector;

//procedure Fat32FSctrl(DevHandle: Hfile);
//procedure Fat32WriteSector(DevHandle: hfile; ulSector: ULONG; nSectors: USHORT; var buf);

implementation

type
  PRModeCall = ^TRModeCall;
  TRModeCall = record
    edi, esi, ebp, reserved, ebx, edx, ecx, eax: ULong;
    flags, es, ds, fs, gs, ip, cs, sp, ss: UShort;
  end;

  PSREGS = ^TSREGS;
  TSREGS = packed record
    es, cs, ss, ds, fs, gs: Word;
  end;

  PREGS  = ^TREGS;
  TREGS  = packed record
    case n of
      0:
        ax, bx, cx, dx: Word;
      1:
        al, ah, bl, bh, cl, ch, dl, dh: Byte;
      si, di: Word;
  end;

{*
 *  Performs a real mode interrupt from protected mode
 *  routines dpmi_rmode_intr and real_int86x are 'stolen' from A.Schulman's
 *  Undocumented DOS
 *}
function dpmi_rmode_intr(intno, flags, copywords: Word, rmc: PRModeCall): boolean; assembler; {$ASMMODE intel}
asm
  push di
  push bx
  push cx
  mov ax, 0300h             // simulate real mode interrupt
  mov bx, intno             // interrupt number, flags
  mov cx, copywords         // words to copy from pmode to rmode stack
  les di, rmc               // ES:DI = address of rmode call struct
  int 31h                   // call DPMI
  jc error
  mov ax, 1                 // return TRUE
  jmp short done
@error:
  mov ax, 0                 // return FALSE
@done:
  pop cx
  pop bx
  pop di
end;

function real_int86x(intno: integer; inregs, outregs: PREGS, sregs: PSREGS): integer;
var
  r: TRModeCall;
begin
  FillChar(@r, sizeof(r), 0);   { initialize all fields to zero: important! }
  r.edi := inregs^.di;
  r.esi := inregs^.si;
  r.ebx := inregs^.bx;
  r.edx := inregs^.dx;
  r.ecx := inregs^.cx;
  r.eax := inregs^.ax;
  r.flags := inregs^.cflag;
  r.es := sregs^.es;
  r.ds := sregs^.ds;
  r.cs := sregs^.cs;

  if not dpmi_rmode_intr(intno, 0, 0, @r) then
  begin
    outregs^.cflag := 1;          { error: set carry flag! }
    result := -1;
  end

  sregs^.es := r.es;
  sregs^.cs := r.cs;
  sregs^.ss := r.ss;
  sregs^.ds := r.ds;
  outregs^.ax := r.eax;
  outregs^.bx := r.ebx;
  outregs^.cx := r.ecx;
  outregs^.dx := r.edx;
  outregs^.si := r.esi;
  outregs^.di := r.edi;
  outregs^.cflag := r.flags and 1;  { carry flag }
  result := outregs^.ax;
end;

procedure Read_Sectors(Drive: char; var Buf; StartSec, Sectors: ULong);
begin

end;

{$IFDEF FPC}
{initialisation}
begin
{$ENDIF}
end.
