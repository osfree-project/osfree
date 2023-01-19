{$A-,B-,D+,G-,I-,O-,P-,Q-,R-,S-,T-,V-,X+}
{$ifndef fpc}
{$E-,F-,L+,N-,Y+,Use32+}
{$else}
{$mode objfpc}
{$H-}
{$endif}
{様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様}
{ Streams                                                                    }
{ Portable source code (tested on DOS and OS/2)                              }
{ Copyright (c) 1996 by Andrew Zabolotny, FRIENDS software                   }
{様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様}
Unit Streams;

Interface uses MiscUtil;

const
 steOK               = 0;
 steNoSuchFile       = 1;
 steCreateError      = 2;
 steInvalidFormat    = 3;
 steInvalidOpenMode  = 4;
 steReadError        = 5;
 steWriteError       = 6;
 steNoMemory         = 7;
 steSeekError        = 8;
 steNotApplicable    = 9;

type
 pStream = ^tStream;
 tStream = object(tObject)
  Error       : Word16;
  function    Name : string; virtual;
  function    Put(var Data; bytes : word) : word; virtual;
  function    Get(var Data; bytes : word) : word; virtual;
  procedure   Skip(bytes : longint); virtual;
  procedure   Seek(newPos : longint); virtual;
  function    GetPos : longint; virtual;
  function    Size : longint; virtual;
  function    EOS : boolean; virtual;
  procedure   PutStr(var S : string);
  function    GetStr : string;
  procedure   PutZTstr(S : pChar);
  function    GetZTstr : pChar;
  function    CopyFrom(var S : tStream; bytes : longint) : longint;
 end;

 pFilter = ^tFilter;
 tFilter = object(tStream)
  ChainStream : pStream;
  constructor Create(Chain : pStream);
  function    Name : string; virtual;
  function    Put(var Data; bytes : word) : word; virtual;
  function    Get(var Data; bytes : word) : word; virtual;
  procedure   Skip(bytes : longint); virtual;
  function    EOS : boolean; virtual;
 end;

const
 stmReadOnly         = $0000; { ---- ---- ---- -000 }
 stmWriteOnly        = $0001; { ---- ---- ---- -001 }
 stmReadWrite        = $0002; { ---- ---- ---- -010 }
 stmAccessMask       = $0007; { ---- ---- ---- -111 }
 stsDenyReadWrite    = $0010; { ---- ---- -001 ---- }
 stsDenyWrite        = $0020; { ---- ---- -010 ---- }
 stsDenyRead         = $0030; { ---- ---- -011 ---- }
 stsDenyNone         = $0040; { ---- ---- -100 ---- }
 stfNoInherit        = $0080; { ---- ---- 1--- ---- }
 stfNo_Locality      = $0000; { ---- -000 ---- ---- }
 stfSequential       = $0100; { ---- -001 ---- ---- }
 stfRandom           = $0200; { ---- -010 ---- ---- }
 stfRandomSequential = $0300; { ---- -011 ---- ---- }
 stfNoCache          = $1000; { ---1 ---- ---- ---- }
 stfFailOnError      = $2000; { --1- ---- ---- ---- }
 stfWriteThrough     = $4000; { -1-- ---- ---- ---- }
 stfDASD             = $8000; { 1--- ---- ---- ---- }
type
 pFileStream = ^tFileStream;
 tFileStream = object(tStream)
  F           : File;
  constructor Create(const fName : string; openMode : Word);
  function    Name : string; virtual;
  function    Put(var Data; bytes : word) : word; virtual;
  function    Get(var Data; bytes : word) : word; virtual;
  procedure   Skip(bytes : longint); virtual;
  procedure   Seek(newPos : longint); virtual;
  function    GetPos : longint; virtual;
  function    Size : longint; virtual;
  function    EOS : boolean; virtual;
  function    GetTime : longint; virtual;
  procedure   SetTime(Time : longint); virtual;
  function    GetAttr : longint;
  procedure   SetAttr(Attr : longint);
  procedure   Truncate;
  procedure   Free; virtual;
  destructor  Erase;
 end;

Implementation uses Dos, Strings;

{$ifndef DOS}
function MaxAvail : longint;
begin
 MaxAvail := High(longint);
end;
{$endif}

function tStream.Name : string;
begin
 Name := '';
end;

function tStream.Get(var Data; bytes : word) : word;
begin
 Get := 0;
 if Error = steOK
  then Error := steNotApplicable;
end;

function tStream.Put(var Data; bytes : word) : word;
begin
 Put := 0;
 if Error = steOK
  then Error := steNotApplicable;
end;

procedure tStream.Skip(bytes : longint);
var
 buff  : Pointer;
 bsz,I : Word;
begin
 if Error = steOK
  then begin
        Seek(GetPos + bytes);
        if Error <> steOK
         then begin
               Error := steOK;
               bsz := minL(minL(maxAvail, $FFF0), bytes);
               GetMem(buff, bsz);
               if buff <> nil
                then begin
                      While (Error = steOK) and (bytes > 0) do
                       begin
                        I := minL(bytes, bsz);
                        Dec(bytes, Get(buff^, I));
                       end;
                      FreeMem(buff, bsz);
                     end
                else Error := steNoMemory;
              end;
       end;
end;

procedure tStream.Seek(newPos : longint);
begin
 if Error = steOK
  then Error := steNotApplicable;
end;

function tStream.GetPos : longint;
begin
 GetPos := -1;
 if Error = steOK
  then Error := steNotApplicable;
end;

function tStream.Size : longint;
begin
 Size := -1;
 if Error = steOK
  then Error := steNotApplicable;
end;

function tStream.EOS : boolean;
begin
 EOS := TRUE;
 if Error = steOK
  then Error := steNotApplicable;
end;

procedure tStream.PutStr(var S : string);
begin
 Put(S, succ(length(S)));
end;

function tStream.GetStr : string;
var
 S : string;
begin
 S := '';
 Get(S[0], 1);
 Get(S[1], length(S));
 GetStr := S;
end;

procedure tStream.PutZTstr(S : pChar);
var
 I : SmallWord;
begin
 I := strLen(S);
 Put(I, sizeOf(I));
 Put(S^, I);
end;

function tStream.GetZTstr : pChar;
var
 I : SmallWord;
 S : pChar;
begin
 Get(I, sizeOf(I));
 if Error = steOK
  then begin
        GetMem(S, succ(I));
        Get(S^, I);
        pByteArray(S)^[I] := 0;
       end
  else S := nil;
 GetZTstr := S;
end;

function tStream.CopyFrom(var S : tStream; bytes : longint) : longint;
var
 Buff : Pointer;
 bSz  : Word;
 i,rc : longint;
begin
 CopyFrom := 0;
 bSz := minL($FFF0, maxAvail);
 GetMem(Buff, bSz);
 if Buff = nil then begin Error := steNoMemory; exit; end;
 rc := 0;
 While (not S.EOS) and (bytes <> 0) and (Error = steOK) do
  begin
   if bytes = -1
    then i := bSz
    else i := minL(bytes, bSz);
   i := S.Get(Buff^, i);
   Put(Buff^, i);
   if bytes <> -1
    then Dec(bytes, i);
   Inc(rc, i);
  end;
 FreeMem(Buff, bSz);
 CopyFrom := rc;
end;

constructor tFilter.Create(Chain : pStream);
begin
 inherited Create;
 ChainStream := Chain;
end;

function tFilter.Name : string;
begin
 if ChainStream <> nil
  then Name := ChainStream^.Name
  else Name := inherited Name;
end;

function tFilter.Get(var Data; bytes : word) : word;
begin
 if Error = steOK
  then if ChainStream <> nil
        then begin
              Get := ChainStream^.Get(Data, bytes);
              Error := ChainStream^.Error;
             end
        else Get := inherited Get(Data, bytes)
  else Get := 0;
end;

function tFilter.Put(var Data; bytes : word) : word;
begin
 if Error = steOK
  then if ChainStream <> nil
        then begin
              Put := ChainStream^.Put(Data, bytes);
              Error := ChainStream^.Error;
             end
        else Put := inherited Put(Data, bytes)
  else Put := 0;
end;

procedure tFilter.Skip(bytes : longint);
begin
 if Error = steOK
  then if (ChainStream <> nil)
        then begin
              ChainStream^.Skip(bytes);
              Error := ChainStream^.Error;
             end
        else inherited Skip(bytes);
end;

function tFilter.EOS : boolean;
begin
 if ChainStream <> nil
  then begin
        EOS := ChainStream^.EOS;
        Error := ChainStream^.Error;
       end
  else EOS := inherited EOS;
end;

constructor tFileStream.Create(const fName : string; openMode : Word);
label
 fCreate;
var
 oldMode : Integer;
begin
 inherited Create;
 Assign(F, fName);
 oldMode := FileMode;
 FileMode := openMode;
 case openMode and stmAccessMask of
  stmReadOnly,
  stmReadWrite : begin
                  Reset(F, 1);
                  if ioResult <> 0
                   then if openMode and stmAccessMask = stmReadWrite
                         then goto fCreate
                         else Error := steNoSuchFile;
                 end;
  stmWriteOnly : begin
fCreate:          Rewrite(F, 1);
                  if ioResult <> 0
                   then Error := steCreateError;
                 end;
  else Error := steInvalidOpenMode;
 end;
 FileMode := oldMode;
end;

function tFileStream.Name : string;
begin
{$ifndef FPC}
 Name := strPas(FileRec(F).Name);
{$else}
 Name := FileRec(F).Name;
{$endif}
end;

function tFileStream.Put(var Data; bytes : word) : word;
var
 L : Word;
begin
 Put := 0;
 if Error = steOK
  then begin
        blockWrite(F, Data, bytes, L);
        if ioResult <> 0 then Error := steWriteError;
        Put := L;
       end;
end;

function tFileStream.Get(var Data; bytes : word) : word;
var
 L : Word;
begin
 Get := 0;
 if Error = steOK
  then begin
        blockRead(F, Data, bytes, L);
        if ioResult <> 0 then Error := steReadError;
        Get := L;
       end;
end;

procedure tFileStream.Skip(bytes : longint);
begin
 if Error = steOK
  then begin
        inOutRes := 0;
        System.Seek(F, filePos(F) + bytes);
        if ioResult <> 0 {not a random-access file}
         then inherited Skip(bytes);
       end;
end;

function tFileStream.GetPos : longint;
begin
 if Error = steOK
  then begin
        inOutRes := 0;
        GetPos := FilePos(F);
        if ioResult <> 0 then Error := steSeekError;
       end
  else GetPos := -1;
end;

procedure tFileStream.Seek(newPos : longint);
begin
 if Error = steOK
  then begin
        System.Seek(F, newPos);
        if ioResult <> 0 then Error := steSeekError;
       end;
end;

function tFileStream.Size : longint;
begin
 if Error = steOK
  then begin
        inOutRes := 0;
        Size := System.FileSize(F);
        if ioResult <> 0 then Error := steNotApplicable;
       end
  else Size := -1;
end;

function tFileStream.EOS : boolean;
begin
 if Error = steOK
  then begin
        inOutRes := 0;
        EOS := System.EOF(F);
        if ioResult <> 0 then Error := steNotApplicable;
       end
  else EOS := TRUE;
end;

function tFileStream.GetTime : longint;
var
 L : longint;
begin
 if Error = steOK
  then begin
        GetFTime(F, L);
        GetTime := L;
        if ioResult <> 0 then Error := steNotApplicable;
       end
  else GetTime := 0;
end;

procedure tFileStream.SetTime(Time : longint);
begin
 if (Error = steOK) and (Time <> 0)
  then begin
        SetFTime(F, Time);
        if ioResult <> 0 then Error := steNotApplicable;
       end;
end;

function tFileStream.GetAttr : longint;
var
 W : {$ifdef fpc}Word16{$else}word{$endif};
begin
 if Error = steOK
  then begin
        GetFAttr(F, W);
        GetAttr := W;
        if ioResult <> 0 then Error := steNotApplicable;
       end
  else GetAttr := 0;
end;

procedure tFileStream.SetAttr(Attr : longint);
begin
 if (Error = steOK) and (Attr <> 0)
  then begin
        SetFAttr(F, Attr);
        if ioResult <> 0 then Error := steNotApplicable;
       end;
end;

procedure tFileStream.Truncate;
begin
 if Error = steOK
  then begin
        System.Truncate(F);
        if ioResult <> 0 then Error := steNotApplicable;
       end;
end;

procedure tFileStream.Free;
begin
 inOutRes := 0;
 Close(F);
 inOutRes := 0;
end;

destructor tFileStream.Erase;
begin
 Free;
 System.Erase(F);
 inOutRes := 0;
end;

end.
