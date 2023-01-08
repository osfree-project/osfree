{

     osFree Turbo Professional Copyright (C) 2022 osFree

     All rights reserved.

     Redistribution  and  use  in  source  and  binary  forms, with or without
modification, are permitted provided that the following conditions are met:

     *  Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
     *  Redistributions  in  binary  form  must  reproduce the above copyright
notice,   this  list  of  conditions  and  the  following  disclaimer  in  the
documentation and/or other materials provided with the distribution.
     * Neither the name of the osFree nor the names of its contributors may be
used  to  endorse  or  promote  products  derived  from  this software without
specific prior written permission.

     THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN  NO  EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES  (INCLUDING,  BUT  NOT  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES;  LOSS  OF  USE,  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}

unit tpmenu;

interface

uses
  crt,
  tpcrt,
  tpwindow;

const
  LotusFrame = #255#255#255#255#255#255; {Tag denotes unframed submenus}
  NoFrame = LotusFrame;      {Synonym for LotusFrame}

type
  Orientation =              {Which direction scrolling proceeds}
  (Vertical, Horizontal, NoOrient);

  MenuCharSet = set of Char; {User-defined keys to exit menu selection}

  MenuKey = LongInt;         {What a menu selection returns as identification}

  MenuColorType =            {Colors used by the menu system}
  (FrameColor,               {Color used for menu frames}
    HeaderColor,             {Color used for header string within frame}
    BodyColor,               {Color used for body of menu}
    SelectColor,             {Color used for the selected item}
    HiliteColor,             {Color used for the pick character of each item}
    HelpColor,               {Color used for help row, if any}
    DisabledColor,           {Color used for temporarily disabled items}
    ShadowColor              {Color used for menu shadows}
    );
  MenuColorArray = array[MenuColorType] of Byte;

  Menu = ^MenuRec;
  ItemP = ^ItemRec;
  SubMenuP = ^SubMenuRec;
  MenuStackP = ^MenuStackRec;

  ItemRec =                  {28 bytes+name+help}
    record
      DisplayPos : Byte;     {Offset from top left corner of menu for display}
      SelectPos : Byte;      {Byte in string to highlight and cause selection, 0 for none}
      Key : MenuKey;         {Key returned when item is selected}
      Name : Pointer;        {Pointer to string to display for item}
      Help : Pointer;        {Pointer to string to display for item help}
      Next : ItemP;          {Pointer to next item in list}
      Prev : ItemP;          {Pointer to previous item in list}
      Sub : SubMenuP;        {Pointer to submenu, nil if none}
      OnHeap : Boolean;      {True if name/help is allocated on heap}
      Enabled : Boolean;     {True if item may be selected by user}
    end;

  ItemList =
    record                   {12 bytes}
      First : ItemP;         {First item in menu}
      Last : ItemP;          {Last item in menu}
      Current : ItemP;       {Current item in menu}
    end;

  MenuStackRec =
    record                   {8 bytes}
      Top : SubMenuP;        {Points to active submenu}
      Next : MenuStackP;     {Remainder of the stack}
    end;

  SubMenuRec =               {44 bytes+header+screen buffers}
    record
      XL, YL : Byte;         {Upper left corner of window frame}
      XH, YH : Byte;         {Actual bottom right corner of window frame}
      YHelp : Byte;          {Row where a help line starts}
      Orient : Orientation;  {Horizontal or vertical scroll}
      Frame : FrameArray;    {Characters for frame}
      Colors : MenuColorArray; {Colors for parts of menu}

      LotusStyle : Boolean;  {True for menus without frames, ala Lotus}
      Header : Pointer;      {Title string for frame}
      Items : ItemList;      {Linked list of entries}
      HelpCovers : BufP;     {Points to buffer for screen covered by help}

      Draw : WinDrawRec;     {Hook into TPWINDOW}
    end;

  MenuRec =
    record                   {50 bytes}
      Root : SubMenuP;       {Root of menu}
      Active : SubMenuP;     {Currently active submenu}
      Stack : MenuStackP;    {Points to stack of active menus}
      UserFunc : Pointer;    {Points to user-supplied function}
      SelectKeys : MenuCharSet; {User-defined keys to perform selection}
      Visible : Boolean;     {True when menus are onscreen}
      SubPending : Boolean;  {True when submenu will pop down automatically}
    end;

function NewMenu(SelectKeys : MenuCharSet; UserFunc : Pointer) : Menu;
  {-Initialize a new menu system by returning a pointer to a new menu}

procedure SubMenu(XLP, YLP, YhelpP : Byte;
                  OrientP : Orientation;
                  FrameP : FrameArray;
                  ColorsP : MenuColorArray;
                  HeaderStr : string
                  );
  {-Add a submenu to currently active item of currently active submenu
    of currently active menu}

procedure MenuItem(NameStr : string; {Name of item}
                   DisplayPosP : Byte; {Offset from upper left corner of menu for item}
                   SelectPosP : Byte; {Position within namestr to hilite and select from}
                   KeyP : MenuKey; {Key to return when item is selected}
                   HelpStr : string {Help string for item}
                   );
  {-Add an item to currently active submenu of currently active menu.
    Name space is allocated on heap}

procedure ResetMenu(Mnu : Menu);
  {-Set all selections to first item}

function MenuChoice(Mnu : Menu; var SelectKey : Char) : MenuKey;
  {-Display menu system, let user browse it, return menukey of selected item,
    return keystroke used to select item, leave menu on screen}

implementation

var
  CurrentMenu : Menu;           {Menu currently being built}

function NewMenu(SelectKeys : MenuCharSet; UserFunc : Pointer) : Menu;
  {-Initialize a new menu system by returning a pointer to a new menu}
begin
  GetMem(CurrentMenu, SizeOf(MenuRec));
  FillChar(CurrentMenu^, SizeOf(MenuRec), 0);
  CurrentMenu^.SelectKeys:=SelectKeys;
  CurrentMenu^.UserFunc:=UserFunc;
  Result:=CurrentMenu;
end;

procedure SubMenu(XLP, YLP, YhelpP : Byte;
                  OrientP : Orientation;
                  FrameP : FrameArray;
                  ColorsP : MenuColorArray;
                  HeaderStr : string
                  );
  {-Add a submenu to currently active item of currently active submenu
    of currently active menu}
var
  SM: SubMenuP;
begin
  GetMem(SM, SizeOf(SubMenuRec));
  FillChar(SM^, SizeOf(SubMenuRec), 0);
  with SubMenuP(SM)^, Draw do
  begin
    XL:=XLP;
    YL:=YLP;
    XL1:=XLP;
    YL1:=YLP;
    Orient:=OrientP;
    Frame:=FrameP;
    LotusStyle:=(Frame=LotusFrame);
    Framed:=not LotusStyle;
    if Framed then
    begin
      Inc(XL1);
      Inc(YL1);
    end;
    Colors:=ColorsP;
    GetMem(Header, Length(HeaderStr)+1);
    Move(HeaderStr, Header^, Length(HeaderStr)+1);
    YHelp:=YHelpP;
    FAttr:=Colors[FrameColor];
    WAttr:=Colors[BodyColor];
    HAttr:=Colors[HeaderColor];
    SAttr:=Colors[ShadowColor];
  end;
  CurrentMenu^.Root:=SM;
  CurrentMenu^.Active:=SM;
end;

procedure MenuItem(NameStr : string; {Name of item}
                   DisplayPosP : Byte; {Offset from upper left corner of menu for item}
                   SelectPosP : Byte; {Position within namestr to hilite and select from}
                   KeyP : MenuKey; {Key to return when item is selected}
                   HelpStr : string {Help string for item}
                   );
  {-Add an item to currently active submenu of currently active menu.
    Name space is allocated on heap}
var
  I: ItemP;
  H: Byte;
begin
  GetMem(I, SizeOf(ItemRec));
  FillChar(I^, SizeOf(ItemRec), 0);
  with I^ do
  begin
    DisplayPos:=DisplayPosP;
    SelectPos:=SelectPosP;
    Key:=KeyP;
    GetMem(Name, Length(NameStr)+1);
    Move(NameStr, Name^, Length(NameStr)+1);
    GetMem(Help, Length(HelpStr)+1);
    Move(HelpStr, Help^, Length(HelpStr)+1);
    OnHeap:=True;
    Enabled:=True;
  end;

  with CurrentMenu^.Active^, Draw, Items do
  begin
    if Current = nil then begin
      I^.Prev := nil;
      I^.Next := nil;
      First := I;
      Last := I;
    end else begin
      I^.Prev := Current;
      I^.Next := Current^.Next;
      if Current^.Next <> nil then
        Current^.Next^.Prev := I;
      Current^.Next := I;
      if Current = Last then
        Last := I;
    end;
    Current:=I;
    H:=XL+Length(NameStr);
    if Framed then H:=H+2;
    if XH<H then XH:=H;
    H:=YL+DisplayPosP-1;
    if Framed then H:=H+2;
    if YH<H then YH:=H;
    
  end;
end;

procedure ResetMenu(Mnu : Menu);
  {-Set all selections to first item}
begin
  with Mnu^.Root^ do
  begin
    Items.Current := Items.First;
    // @todo got to all submenus here and reset them too
  end;
end;

procedure PushSubMenu(Mnu: Menu; SubMnu: SubMenuP);
  {-Put submenu onto active stack of the menu}
var
  P : MenuStackP;
begin
  GetMem(P, SizeOf(MenuStackRec));
  with Mnu^ do
  begin
    if Root=nil then Root:=SubMnu;
    Active:=SubMnu;
    P^.Top:=SubMnu;
    P^.Next:=Stack;
    Stack:=P;
  end;
end;

procedure DrawItem(SubMnu: SubMenuP; Item: ItemP; UserFunc: Pointer);
var
  S: ShortString;
  OldTextAttr: Byte;
begin
  with SubMnu^, Draw do
  begin
    GoToXY(XL1, YL1+Item^.DisplayPos-1);
    if Item^.Name = nil then
      S[0] := #0
    else
      S:=string(Item^.Name^);
    OldTextAttr:=TextAttr;
    If Item=Items.Current then
      TextAttr:=Colors[SelectColor]
    else
      TextAttr:=Colors[BodyColor];
    Write(S);
    While WhereX<XH do Write(' ');
    TextAttr:=OldTextAttr;
  end;
end;

procedure DrawSubMenu(SubMnu : SubMenuP; UserFunc : Pointer);
  {-Draw a submenu on-screen}
var
  Item: ItemP;
begin
  with SubMnu^, Draw do
  begin
    FrameChars:=Frame;
    if Framed then FrameWindow(XL, YL, XH, YH, Colors[FrameColor], Colors[FrameColor], '');

    Item := Items.First;
    while Item <> nil do begin
      DrawItem(SubMnu, Item, UserFunc);
      Item := Item^.Next;
    end;
  end;
end;

function MenuChoice(Mnu: Menu; var SelectKey: Char) : MenuKey;
  {-Display menu system, let user browse it, return menukey of selected item,
    return keystroke used to select item, leave menu on screen}
var
  K: Char;
begin
  PushSubMenu(Mnu, Mnu^.Root);
  DrawSubMenu(Mnu^.Root, nil);

  repeat
    k:=readkey;
    case k of
    #13: // enter
    begin
      Result:=Mnu^.Active^.Items.Current^.Key;
    end;
    #0:
    begin
      k:=readkey;
      case k of
      #77, #80: // rg, dn
      begin
        with Mnu^.Active^.Items do
        begin
          if Current=Last then
            Current:=First
          else
            Current:=Current^.Next;
        end;
      end;
      #75, #72: // lf, up
      begin
        with Mnu^.Active^.Items do
        begin
          if Current=First then
            Current:=Last
          else
            Current:=Current^.Prev;
        end;
      end;
      end;
    end;
    end;
    DrawSubMenu(Mnu^.Active, nil);
  until K in [#13];
end;

end.