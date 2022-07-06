{$A-,B-,D+,G-,I-,O-,P-,Q-,R-,S-,T-,V-,X+}
{$ifndef fpc}{$E-,F-,L+,N-,Y+,Use32+}{$endif}
{様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様}
{ Collections (dynamic arrays) ver. 1.1                                      }
{ Portable source code (tested on DOS and OS/2)                              }
{ Original copyright (c) Borland International :-)                           }
{ Version history:                                                           }
{ 1.0 Major modifications by Andrew Zabolotny, FRIENDS software              }
{ 1.1 Function tFakeStream.Get bugfixed by Max Alekseyev                     }
{様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様}
Unit Collect;

Interface uses miscUtil, Strings, Streams;

const
{$ifDef use32}
 MaxCollectionSize = (512*1024) div sizeOf(Pointer);
{$else}
 MaxCollectionSize = (64*1024-8) div sizeOf(Pointer);
{$endIf}
type
 pCollection = ^tCollection;
 tCollection = object(tObject)
  Items       : pPointerArray;
  Count       : Integer;
  Limit       : Integer;
  Delta       : Integer;
  constructor Create(ALimit, ADelta : Integer);
  constructor Load(var S : tStream);
  constructor Clone(C : pCollection);
  destructor  Destroy; virtual;
  function    At(Index : Integer): Pointer;
  procedure   AtDelete(Index : Integer);
  procedure   AtFree(Index : Integer);
  function    AtInsert(Index : Integer; Item : Pointer) : boolean;
  procedure   AtReplace(Index : Integer; Item : Pointer);
  procedure   Delete(Item : Pointer);
  procedure   DeleteAll;
  function    FirstThat(Test : Pointer) : Pointer;
  procedure   ForEach(Action : Pointer);
  procedure   FreeAll; virtual;
  procedure   FreeItem(Item : Pointer); virtual;
  function    GetItem(var S : tStream) : Pointer; virtual;
  function    IndexOf(Item : Pointer) : Integer; virtual;
  function    Insert(Item : Pointer) : Integer; virtual;
  function    LastThat(Test : Pointer) : Pointer;
  procedure   PutItem(var S : tStream; Item : Pointer); virtual;
  procedure   SetLimit(ALimit : Integer); virtual;
  procedure   Store(var S : tStream);
 end;

 pSortedCollection = ^tSortedCollection;
 tSortedCollection = object(tCollection)
  Duplicates  : Boolean;
  constructor Create(ALimit, ADelta : Integer);
  constructor Load(var S : tStream);
  function    Compare(Key1, Key2 : Pointer) : Integer; virtual;
  function    IndexOf(Item : Pointer) : Integer; virtual;
  function    Insert(Item : Pointer) : Integer; virtual;
  function    KeyOf(Item : Pointer) : Pointer; virtual;
  function    Search(Key : Pointer; var Index : Integer) : Boolean; virtual;
  procedure   Store(var S : tStream);
 end;

 pStringCollection = ^tStringCollection;
 tStringCollection = object(tSortedCollection)
  function    Compare(Key1, Key2 : Pointer) : Integer; virtual;
  procedure   FreeItem(Item : Pointer); virtual;
  function    GetItem(var S : tStream) : Pointer; virtual;
  procedure   PutItem(var S : tStream; Item : Pointer); virtual;
 end;

 pZTstrCollection = ^tZTstrCollection;
 tZTstrCollection = object(tSortedCollection)
  function    Compare(Key1, Key2 : Pointer) : Integer; virtual;
  procedure   FreeItem(Item : Pointer); virtual;
  function    GetItem(var S : tStream) : Pointer; virtual;
  procedure   PutItem(var S : tStream; Item : Pointer); virtual;
 end;

 pValueCollection = ^tValueCollection;
 tValueCollection = object(tCollection)
  procedure   FreeItem(Item : Pointer); virtual;
 end;

Implementation

type
{fake stream object for cloning collections}
 pFakeStream = ^tFakeStream;
 tFakeStream = object(tStream)
  DataStorage : pByteArray;
  DataPtr,
  DataSize    : word;
  function    Put(var Data; bytes : word) : word; virtual;
  function    Get(var Data; bytes : word) : word; virtual;
  destructor  Destroy; virtual;
 end;

function tFakeStream.Put;
var
 nP : pByteArray;
begin
 GetMem(nP, DataSize + bytes);
 Move(DataStorage^, nP^, DataSize);
 Move(Data, nP^[DataSize], bytes);
 FreeMem(DataStorage, DataSize);
 Inc(DataSize, bytes);
 DataStorage := nP;
 DataPtr := 0;
 Put := bytes;
end;

function tFakeStream.Get;
var
 I : word;
begin
 I := minI(DataSize - DataPtr, bytes);
 Move(DataStorage^[DataPtr], Data, I);
 Inc(DataPtr, I);
 if DataPtr >= DataSize
  then begin
        FreeMem(DataStorage, DataSize);
        DataStorage := nil;
        DataSize := 0;
        DataPtr := 0;
       end;
 Get := I;
end;

destructor tFakeStream.Destroy;
begin
 if DataSize > 0
  then FreeMem(DataStorage, DataSize);
 inherited Destroy;
end;

constructor tCollection.Create;
begin
 inherited Create;
 Items := nil;
 Count := 0;
 Limit := 0;
 Delta := ADelta;
 SetLimit(ALimit);
end;

constructor tCollection.Load;
var
 C,I : Integer;
begin
 S.Get(Count, SizeOf(Integer) * 3);
 Items := nil;
 C := Count;
 I := Limit;
 Count := 0;
 Limit := 0;
 SetLimit(I);
 Count := C;
 for I := 0 to C - 1 do AtReplace(I, GetItem(S));
end;

constructor tCollection.Clone;
var
 I : Integer;
 S : pFakeStream;
begin
 inherited Create;
 New(S, Create);
 if S = nil then Fail;
 Delta := C^.Delta;
 SetLimit(C^.Limit);
 For I := 0 to pred(C^.Count) do
  begin
   C^.PutItem(S^, C^.At(I));
   AtInsert(I, GetItem(S^));
  end;
 Dispose(S, Destroy);
end;

destructor tCollection.Destroy;
begin
 FreeAll;
 inherited Destroy;
end;

function tCollection.At;
begin
 if (Index < Count)
  then At := Items^[Index]
  else At := nil;
end;

procedure tCollection.AtDelete;
begin
 if (Index < Count)
  then begin
        Move(Items^[succ(Index)], Items^[Index], sizeOf(Pointer) * (Count - succ(Index)));
        Dec(Count);
       end;
end;

function tCollection.AtInsert;
begin
 AtInsert := FALSE;
 if Index <= Count
  then begin
        if Count >= Limit
         then SetLimit(Limit + Delta);
        if Count < Limit
         then begin
               Move(Items^[Index], Items^[succ(Index)], sizeOf(Pointer) * (Count - Index));
               Items^[Index] := Item;
               Inc(Count);
               AtInsert := TRUE;
              end;
       end;
end;

procedure tCollection.AtReplace;
begin
 if (Index < Count)
  then Items^[Index] := Item;
end;

function tCollection.FirstThat;
label
 Found;
var
 I : Integer;
 P : Pointer;
begin
 For I := 0 to pred(Count) do
  begin
   P := At(I);
   if level2call(Test, P)
    then Goto Found;
  end;
 P := nil;
Found:
 FirstThat := P;
end;

function tCollection.LastThat;
label
 Found;
var
 I : Integer;
 P : Pointer;
begin
 For I := pred(Count) downto 0 do
  begin
   P := At(I);
   if level2call(Test, P)
    then Goto Found;
  end;
 P := nil;
Found:
 LastThat := P;
end;

procedure tCollection.ForEach;
var
 I : Integer;
begin
 For I := pred(Count) downto 0 do
  level2call(Action, At(I));
end;

function tCollection.IndexOf;
var
 I : Integer;
begin
 For I := 0 to pred(Count) do
  if Item = At(I)
   then begin
         IndexOf := I;
         exit;
        end;
 IndexOf := -1;
end;

procedure tCollection.AtFree;
var
 Item : Pointer;
begin
 Item := At(Index);
 AtDelete(Index);
 FreeItem(Item);
end;

procedure tCollection.Delete;
begin
 AtDelete(IndexOf(Item));
end;

procedure tCollection.DeleteAll;
begin
 Count := 0;
end;

procedure tCollection.FreeAll;
var
 I : Integer;
begin
 for I := 0 to pred(Count) do FreeItem(At(I));
 DeleteAll; SetLimit(0);
end;

procedure tCollection.FreeItem;
begin
 Dispose(pObject(Item), Destroy);
end;

function tCollection.GetItem;
var P : Pointer;
begin
 if S.Get(P, sizeOf(P)) = sizeOf(P)
  then GetItem := P
  else GetItem := nil;
end;

procedure tCollection.PutItem;
begin
 S.Put(Item, sizeOf(Item));
end;

function tCollection.Insert;
begin
 Insert := Count;
 if not AtInsert(Count, Item)
  then Insert := -1;
end;

procedure tCollection.SetLimit;
var
 AItems : pPointerArray;
begin
 if ALimit < Count then ALimit := Count;
 if ALimit > MaxCollectionSize then ALimit := MaxCollectionSize;
 if ALimit <> Limit
  then begin
        if ALimit = 0
         then AItems := nil
         else begin
               GetMem(AItems, ALimit * SizeOf(Pointer));
               if AItems = nil then Exit;
               if (Count <> 0) and (Items <> nil)
                then Move(Items^, AItems^, Count * SizeOf(Pointer));
              end;
        if Limit <> 0 then FreeMem(Items, Limit * SizeOf(Pointer));
        Items := AItems;
        Limit := ALimit;
       end;
end;

procedure tCollection.Store;

procedure DoPutItem(P : Pointer); far;
begin
 PutItem(S, P);
end;

begin
 S.Put(Count, SizeOf(Integer) * 3);
 ForEach(@DoPutItem);
end;

constructor tSortedCollection.Create;
begin
 inherited Create(ALimit, ADelta);
 Duplicates := False;
end;

constructor tSortedCollection.Load;
begin
 inherited Load(S);
 S.Get(Duplicates, SizeOf(Duplicates));
end;

function tSortedCollection.Compare;
begin
 Compare := Integer(Key1 = Key2);
end;

function tSortedCollection.IndexOf(Item: Pointer): Integer;
var
 I : Integer;
begin
 IndexOf := -1;
 if Search(KeyOf(Item), I)
  then begin
        if Duplicates
         then while (I < Count) and (Item <> Items^[I]) do Inc(I);
        if I < Count then IndexOf := I;
       end;
end;

function tSortedCollection.Insert;
var
 I : Integer;
begin
 if (not Search(KeyOf(Item), I)) or Duplicates
  then if AtInsert(I, Item)
        then Insert := I
        else Insert := -1
  else Insert := -1;
end;

function tSortedCollection.KeyOf;
begin
 KeyOf := Item;
end;

function tSortedCollection.Search;
var
 L,H,I,C : Integer;
begin
 Search := False;
 L := 0;
 H := Count - 1;
 while L <= H do
  begin
   I := (L + H) shr 1;
   C := Compare(KeyOf(Items^[I]), Key);
   if C < 0
    then L := I + 1
    else begin
          H := I - 1;
          if C = 0
           then begin
                 Search := True;
                 if not Duplicates then L := I;
                end;
         end;
  end;
 Index := L;
end;

procedure tSortedCollection.Store;
begin
 inherited Store(S);
 S.Put(Duplicates, SizeOf(Duplicates));
end;

function tStringCollection.Compare;
var
 Res : Integer;
begin
 Res := MemCmp(pString(Key1)^[1], pString(Key2)^[1],
         MinI(length(pString(Key1)^), length(pString(Key2)^)));
 if Res = 0
  then if length(pString(Key1)^) < length(pString(Key2)^)
        then Res := -1
        else
       if length(pString(Key1)^) > length(pString(Key2)^)
        then Res := +1
        else Res := 0;
 Compare := Res;
end;

procedure tStringCollection.FreeItem;
begin
 DisposeStr(Item);
end;

function tStringCollection.GetItem;
begin
 GetItem := NewStr(S.GetStr);
end;

procedure tStringCollection.PutItem;
begin
 S.PutStr(pString(Item)^);
end;

function tZTstrCollection.Compare;
begin
 Compare := StrComp(Key1, Key2);
end;

procedure tZTstrCollection.FreeItem;
begin
 StrDispose(Item);
end;

function tZTstrCollection.GetItem;
begin
 GetItem := S.GetZTstr;
end;

procedure tZTstrCollection.PutItem;
begin
 S.PutZTstr(Item);
end;

procedure tValueCollection.FreeItem;
begin
end;

end.

