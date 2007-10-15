// **********************************************************************
//
// Copyright (c) 2001 - 2002 MT Tools.
//
// All Rights Reserved
//
// MT_DORB is based in part on the product DORB,
// written by Shadrin Victor
//
// See Readme.txt for contact information
//
// **********************************************************************
unit idldb;

interface

uses
  classes,parser,ir_int,ir,sysutils;

type
  TPseudoRepoId = class
  private
    FFullId: string;
    FSuffix: string;
    FPrefix: string;
    FContainedId: string;
    function GetFullId: string;
    procedure SetFullId(const Value: string);
    procedure SetContainedId(const Value: string);
    procedure SetPrefix(const Value: string);
    procedure SetSuffix(const Value: string);
  public
    property ContainedId: string read FContainedId write SetContainedId;
    property Prefix: string read FPrefix write SetPrefix;
    property Suffix: string read FSuffix write SetSuffix;
    property FullId: string read GetFullId write SetFullId;
  end;

   TDB = class
   private
     FRepoidFilesnames : TStringList;
     FCurrentPrefixes : TStringList;
     FPrefixes : TStringList;
     FPrefixeNames : TStringList;
     FVersions : TStringList;
     FVersionNames : TStringList;
     FIDs : TStringList;
     FIDNames : TStringList;
     FScopeNames : TStringList;
     FIdToPseudoIdMap: TStringList;
   public
     constructor Create();
     destructor Destroy(); override;
     procedure register_repoid(node: TNode; id: string);
     function repoid_defined(id: string): Boolean;
     function repoid_file_name(id: string): string;
     procedure register_anonymous(node: TNode; t : IIDLType);
     function pseudo_repoid(t: IIDLType): string;
     procedure enter_scope(name: string);
     procedure leave_scope();
     procedure switch_prefix(val: string);
     procedure set_prefix(const name, val: string);
     function get_prefix(const absolute_name: string): string;
     function get_id_prefix: string;
     procedure set_version(name: string; val: string);
     procedure enter_include();
     procedure leave_include();
     procedure set_repoids(rep: IRepository);
     procedure set_repo_id(name,id: string);
     function get_current_scope(): string;
     procedure gen_pseudo_repoid(t: IIDLType; pseudo_id: TPseudoRepoId);
   end;

implementation

{ TDB }
uses
  orb_int;

constructor TDB.Create;
begin
  FRepoidFilesnames := TStringList.Create;
  FCurrentPrefixes := TStringList.Create;
  FPrefixes := TStringList.Create;
  FPrefixeNames := TStringList.Create;
  FVersions := TStringList.Create;
  FVersionNames := TStringList.Create;
  FIDs := TStringList.Create;
  FIDNames := TStringList.Create;
  FScopeNames := TStringList.Create;
  FIdToPseudoIdMap := TStringList.Create;
  FCurrentPrefixes.Add('');
end;

destructor TDB.Destroy;
var
  i: Integer;
begin
  FRepoidFilesnames.free;
  FVersions.Free;
  FVersionNames.Free;
  FIDs.Free;
  FIDNames.Free;
  FCurrentPrefixes.Free;
  FPrefixes.Free;
  FPrefixeNames.Free;
  FScopeNames.Free;
  for i := 0 to FIdToPseudoIdMap.Count - 1 do
    FIdToPseudoIdMap.Objects[i].Free;
  FIdToPseudoIdMap.Free;
  inherited;
end;

procedure TDB.enter_include;
begin
  FScopeNames.Add('');
  FCurrentPrefixes.Add('');
end;

procedure TDB.enter_scope(name: string);
begin
  FScopeNames.Add(name);
  FCurrentPrefixes.Add(FCurrentPrefixes[FCurrentPrefixes.count-1]);
end;

function TDB.get_id_prefix: string;
var
  i : integer;
begin
  result := FCurrentPrefixes[FCurrentPrefixes.Count-1];
  for i := 0 to pred(FScopeNames.count) do
    begin
      if result <> '' then result := result + '_';
      result := result + FScopeNames[i];
    end;
  if result <> '' then result := result + '_';
end;

function TDB.get_prefix(const absolute_name: string): string;
var
  i, longest_match, len: integer;
begin
  longest_match := 0;
  result := '';
  for i := 0 to FPrefixeNames.Count - 1 do begin
    len := Length(FPrefixeNames[i]);
    if ((StrLComp(PChar(FPrefixeNames[i]), PChar(absolute_name), len) = 0) and
	    ((Length(absolute_name) = len) or
	    (absolute_name[len + 1] = ':') or
	    ((len = 2) and (absolute_name[1] = ':') and (absolute_name[2] = ':')) and
	    (len > longest_match))) then begin
      longest_match := len;
      result := FPrefixes[i];
    end;
  end;
  if longest_match = 0 then
    result := FCurrentPrefixes[FCurrentPrefixes.Count-1];
  for i := 0 to pred(FScopeNames.count) do begin
    if FScopeNames[i] = '' then continue;
    if result <> '' then
      result := result + '/';
    result := result {+ '/'} + FScopeNames[i];
    //result := FScopeNames[i] + '/' + result;
  end;
  if result <> '' then
    result := result + '/';
  //if FCurrentPrefixes[FCurrentPrefixes.Count-1] <> '' then
    //result := FCurrentPrefixes[FCurrentPrefixes.Count-1] + '/' + result;
end;

procedure TDB.leave_include;
begin
  FScopeNames.Delete(FScopeNames.Count-1);
  FCurrentPrefixes.Delete(FCurrentPrefixes.Count-1);
end;

procedure TDB.leave_scope;
begin
  FScopeNames.Delete(FScopeNames.Count-1);
  FCurrentPrefixes.Delete(FCurrentPrefixes.Count-1);
end;

function TDB.pseudo_repoid(t: IIDLType): string;
var
  pseudo_id: TPseudoRepoId;
begin
  pseudo_id := TPseudoRepoId.Create;
  try
    gen_pseudo_repoid(t, pseudo_id);
    result := pseudo_id.FullId;
  finally
    pseudo_id.Free;
  end;
end;

procedure TDB.register_anonymous(node: TNode; t: IIDLType);
var
  pseudo_id: TPseudoRepoId;
begin
  if node.top_level then exit;
  pseudo_id := TPseudoRepoId.Create;
  gen_pseudo_repoid(t, pseudo_id);

  FRepoidFilesnames.Values[pseudo_id.FullId] := node.file_name;

  if pseudo_id.ContainedId <> '' then
    FIdToPseudoIdMap.AddObject(pseudo_id.ContainedId, pseudo_id)
  else
    pseudo_id.Free;
end;

procedure TDB.register_repoid(node: TNode; id: string);
begin
  if not node.top_level then
    FRepoidFilesnames.Values[id] := node.file_name;
end;

function TDB.repoid_defined(id: string): Boolean;
begin
  result := FRepoidFilesnames.Values[id] <> '';
end;

function TDB.repoid_file_name(id: string): string;
begin
  result := FRepoidFilesnames.Values[id]
end;

procedure TDB.switch_prefix(val: string);
begin
  FCurrentPrefixes[FCurrentPrefixes.Count-1] := val;
end;

procedure TDB.set_repoids(rep: IRepository);
var
  i, j : integer;
  c: IContained;
  pseudoId: TPseudoRepoId;
  oldId, fname, newId, newPseudoId: string;
begin
  for i := 0 to Pred(FVersions.count) do begin
    c := rep.lookup(FVersionNames[i]);
    if c <> nil then begin
      c.version(FVersions[i]);
      oldId := c.id();
      fname := repoid_file_name(oldId);
      if fname <> '' then begin
        //this is a total hack
        //replace last 3 character!!! we can get problematic with version of next format 6.66
        newId := Copy(oldId, 1, Length(oldId) - 3) + FVersionNames[i];
        FRepoidFilesnames.Values[newId] := fname;
      end;
    end;
  end;
  for i := 0 to Pred(FIDs.count) do begin
    c := rep.lookup(FIDNames[i]);
    if c <> nil then begin
      oldId := c.id();
      fname := repoid_file_name(oldId);
      if fname <> '' then
        FRepoidFilesnames.Values[FIDs[i]] := fname;
      c.id(FIDs[i]);
    end;

    // Update pseudo_ids of anonymous types that depend on the updated id
    for j := 0 to FIdToPseudoIdMap.Count - 1 do
      if FIdToPseudoIdMap[j] = oldId then begin
        pseudoId := TPseudoRepoId(FIdToPseudoIdMap.Objects[j]);
        newPseudoId := pseudoId.Prefix + FIDs[0] + pseudoId.Suffix;
        if not repoid_defined(newPseudoId) then
          FRepoidFilesnames.Values[newPseudoId] := repoid_file_name(pseudoId.FullId);
      end;
  end;
end;

procedure TDB.set_repo_id(name, id: string);
begin
  FIDs.add(id);
  FIDNames.Add(name);
end;

procedure TDB.set_version(name, val: string);
begin
  FVersionNames.Add(name);
  FVersions.Add(val);
end;

procedure TDB.set_prefix(const name, val: string);
begin
  FPrefixeNames.Add(name);
  FPrefixes.Add(val);
end;

function TDB.get_current_scope: string;
var
  i: Integer;
begin
  result := '::';
  for i := 0 to FScopeNames.Count - 1 do
    if FScopeNames[i] <> '' then
      result := result + FScopeNames[i] + '::';
end;

procedure TDB.gen_pseudo_repoid(t: IIDLType; pseudo_id: TPseudoRepoId);
var
  con: IContained;
  seq: ISequenceDef;
  arr : IArrayDef;
  pr : IPrimitiveDef;
begin
  con := TContained._narrow(t as IORBObject);
  if con <> nil then
    begin
      pseudo_id.ContainedId := con.id;
      exit;
    end;
  seq := TSequenceDef._narrow(t as IORBObject);
  if seq <> nil then
    begin
      gen_pseudo_repoid(seq.element_type_def, pseudo_id);
      pseudo_id.Prefix := 'sequence_';
      pseudo_id.Suffix := '_' + inttostr(seq.bound);
      //result := 'sequence_'+pseudo_repoid(seq.element_type_def)+'_'+inttostr(seq.bound) ;
      exit;
    end;
  arr := TArrayDef._narrow(t as IORBObject);
  if arr <> nil then
    begin
      gen_pseudo_repoid(seq.element_type_def, pseudo_id);
      pseudo_id.Prefix := 'array_';
      pseudo_id.Suffix := '_' + inttostr(arr.length);
      //result := 'array_'+pseudo_repoid(arr.element_type_def)+'_'+inttostr(arr.length) ;
      exit;
    end;
  pr := TPrimitiveDef._narrow(t as IORBObject);
  if pr <> nil then
    begin
      pseudo_id.FullId := 'primitive_'+inttostr(ord(pr.kind));
      exit;
    end;
  Assert(False, 'Not implemented');
  // IFixedDef
  // IStringDef
  // IWStringDef
end;

{ TPseudoRepoId }

function TPseudoRepoId.GetFullId: string;
begin
  Result := FFullId;
end;

procedure TPseudoRepoId.SetContainedId(const Value: string);
begin
  FContainedId := Value;
  FFullId := Value;
end;

procedure TPseudoRepoId.SetFullId(const Value: string);
begin
  FFullId := Value;
end;

procedure TPseudoRepoId.SetPrefix(const Value: string);
begin
  FPrefix := Value;
  FFullId := FPrefix + FFullId;
end;

procedure TPseudoRepoId.SetSuffix(const Value: string);
begin
  FSuffix := Value;
  FFullId := FFullId + FSuffix;
end;

end.
