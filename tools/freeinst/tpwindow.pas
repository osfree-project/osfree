Unit tpwindow;

interface

uses
  crt,
  tpcrt;

type
  ShadowType = (SmallShadow, BigShadow);

const
  Shadow : Boolean = False;  {True to make shadowed windows}
  ShadowMode : ShadowType = BigShadow; {Kind of shadow to draw}
  ShadowAttr : Byte = $07;   {Attribute to apply to shadow}

  Explode : Boolean = False; {True to make exploding windows}
  ExplodeDelay : Word = 15;  {Milliseconds per stage of explosion}
  SoundFlagW : Boolean = True; {True to make sound during explosions}

type
  WindowPtr = Pointer;       {Generic type of a window}
  WindowP = ^WindowRec;      {Detailed type of a window}

  BufferArray = array[0..$7FF0] of Char; {Will hold screen image}
  BufP = ^BufferArray;

  SaveRec =
    record
      WMin : Word;           {Window coordinates}
      WMax : Word;
      CS : Byte;             {Cursor scan lines}
      CE : Byte;
      CX : Byte;             {Absolute cursor position}
      CY : Byte;
      Attr : Byte;           {TextAttr}
    end;

  WinDrawRec =               {Used while drawing and undrawing window}
    record
      Framed : Boolean;      {True to draw frame around window}
      Exploding : Boolean;   {True if window displays and erases in stages}
      Shadowed : Boolean;    {True to draw shadow around window}
      Noisy : Boolean;       {True to make noise while exploding}
      ExploDelay : Word;     {Milliseconds per stage of explosion}
      ShadowM : ShadowType;  {Type of shadow to draw}

      XL1, YL1 : Byte;       {Overall window coordinates (frame included)}
      XH1, YH1 : Byte;

      FAttr : Byte;          {Attribute for frame}
      WAttr : Byte;          {Attribute for window contents}
      HAttr : Byte;          {Attribute for header}
      SAttr : Byte;          {Attribute for window shadow}

      Covers : BufP;         {Points to buffer of what window covers}
      BufSize : Word;        {Size of screen buffers}

      Shadows : BufP;        {Points to buffer of what shadow covers}
      ShadowSize : Word;     {Size of buffer for shadow region}
    end;
  WinDrawPtr = ^WinDrawRec;

  WindowRec =                {Details of a window}
    record                   {74 bytes}
      Draw : WinDrawRec;     {Used while drawing and erasing window}

      XL, YL : Byte;         {Turbo window coordinates (no frame included)}
      XH, YH : Byte;

      HeaderP : ^string;     {Stores frame title, nil if none}
      Frame : FrameArray;    {Frame characters for this window}
      Current : SaveRec;     {Values to restore when this window is displayed}
      Previous : SaveRec;    {Values to restore when this window is erased}

      Holds : BufP;          {Points to buffer of what window holds if Save is True}

      Clear : Boolean;       {True to clear window when it is displayed}
      Save : Boolean;        {True to save contents when window is erased}
      Active : Boolean;      {True if window is currently on screen}
      DisplayedOnce : Boolean; {True if window displayed at least once}

      UFrame : FrameArray;   {Frame to draw when unselected}
      UFAttr : Byte;         {Attribute for unselected frame}
      UHAttr : Byte;         {Attribute for unselected header}
      FrameDiff : Boolean;   {True if select and unselect frames differ}
    end;

function MakeWindow
  (var W : WindowPtr;        {Window identifier returned}
    XLow, YLow : Byte;       {Window coordinates, including frame if any}
    XHigh, YHigh : Byte;     {Window coordinates, including frame if any}
    DrawFrame : Boolean;     {True to draw a frame around window}
    ClearWindow : Boolean;   {True to clear window when displayed}
    SaveWindow : Boolean;    {True to save window contents when erased}
    WindowAttr : Byte;       {Video attribute for body of window}
    FrameAttr : Byte;        {Video attribute for frame}
    HeaderAttr : Byte;       {Video attribute for header}
    Header : string          {Title for window}
    ) : Boolean;             {Returns True if successful}
  {-Allocate and initialize, but do not display, a new window}

procedure DisposeWindow(W : WindowPtr);
  {-Deallocate heap space for specified window}

function DisplayWindow(W : WindowPtr) : Boolean;
  {-Display the specified window, returning true if successful}

implementation

//////////////////////////////////////////////////////////////////////////
// Private functions

procedure SaveState(var State: SaveRec);
  {-Get settings for the current screen state}
begin
  with State do begin
//    CS:=CursorStartLine;
//    CE:=CursorEndLine;
//    CX:=WhereXAbs;
//    CY:=WhereYAbs;
    Attr:=TextAttr;
    WMin:=WindMin;
    WMax:=WindMax;
  end;
end;

/////////////////////////////////////////////////////////////////////////
// Public functions

function MakeWindow
  (var W : WindowPtr;        {Window identifier returned}
    XLow, YLow : Byte;       {Window coordinates, including frame if any}
    XHigh, YHigh : Byte;     {Window coordinates, including frame if any}
    DrawFrame : Boolean;     {True to draw a frame around window}
    ClearWindow : Boolean;   {True to clear window when displayed}
    SaveWindow : Boolean;    {True to save window contents when erased}
    WindowAttr : Byte;       {Video attribute for body of window}
    FrameAttr : Byte;        {Video attribute for frame}
    HeaderAttr : Byte;       {Video attribute for header}
    Header : string          {Title for window}
    ) : Boolean;             {Returns True if successful}
  {-Allocate and initialize, but do not display, a new window}
begin
  Result:=false;

  // @Todo add check of free memory
  GetMem(W, SizeOf(WindowRec));
  With WindowP(W)^, Draw do
  begin
    if DrawFrame then
    begin
      XL:=XLow+1;
      YL:=YLow+1;
      XH:=XHigh-1;
      YH:=YHigh-1;
      XL1:=XLow;
      YL1:=YLow;
      XH1:=XHigh;
      YH1:=YHigh;
      Frame:=FrameChars;
      Framed:=True;
    end else begin
      XL:=XLow;
      YL:=YLow;
      XH:=XHigh;
      YH:=YHigh;
      XL1:=XLow;
      YL1:=YLow;
      XH1:=XHigh;
      YH1:=YHigh;
      Frame:='      ';
      Framed:=False;
    end;
    FAttr:=FrameAttr;
    HAttr:=HeaderAttr;
    WAttr:=WindowAttr;
    Clear:=ClearWindow;
    HeaderP:=nil;

    if Header<>'' then begin
      GetMem(HeaderP, Length(Header)+1);
      HeaderP^:=Header;
    end;

  end;
end;

procedure DisposeWindow(W : WindowPtr);
  {-Deallocate heap space for specified window}
begin
  FreeMem(W, SizeOf(WindowRec));
end;

function DisplayWindow(W : WindowPtr) : Boolean;
  {-Display the specified window, returning true if successful}
begin
  Result:=False;
  With WindowP(W)^, Draw do
  begin
    SaveState(Previous);

    If Framed then FrameWindow(XL1, YL1, XH1, YH1, FAttr, HAttr, HeaderP^);
    Window(XL,YL,XH,YH);
    TextAttr:=WAttr;
    if Clear then ClrScr;
  end;
end;

end.
