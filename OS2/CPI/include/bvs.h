// Function numbers for Base Video System main call
#define FN_GETPHYSBUF      0
#define FN_GETBUF          1
#define FN_SHOWBUF         2
#define FN_GETCURPOS       3
#define FN_GETCURTYPE      4
#define FN_GETMODE         5
#define FN_SETCURPOS       6
#define FN_SETCURTYPE      7
#define FN_SETMODE         8
#define FN_READCHARSTR     9
#define FN_READCELLSTR     10
#define FN_WRTNCHAR        11
#define FN_WRTNATTR        12
#define FN_WRTNCELL        13
#define FN_WRTCHARSTR      14
#define FN_WRTCHARSTRATT   15
#define FN_WRTCELLSTR      16
#define FN_WRTTTY          17
#define FN_SCROLLUP        18
#define FN_SCROLLDN        19
#define FN_SCROLLLF        20
#define FN_SCROLLRT        21
#define FN_SETANSI         22
#define FN_GETANSI         23
#define FN_PRTSC           24
#define FN_SCRLOCK         25
#define FN_SCRUNLOCK       26
#define FN_SAVREDRAWWAIT   27
#define FN_SAVREDRAWUNDO   28
#define FN_POPUP           29
#define FN_ENDPOPUP        30
#define FN_PRTSCTOGGLE     31
#define FN_MODEWAIT        32
#define FN_MODEUNDO        33
#define FN_GETFONT         34
#define FN_GETCONFIG       35
#define FN_SETCP           36
#define FN_GETCP           37
#define FN_SETFONT         38
#define FN_GETSTATE        39
#define FN_SETSTATE        40

typedef
union
{
struct 	{
			PVIOPHYSBUF pvioPhysBuf; 
		} GetPhysBuf;
struct 	{
			PULONG pLVB;
			PUSHORT pcbLVB;
			HVIO hvio; 
		} GetBuf;
struct	{
			USHORT offLVB;
			USHORT cb;
			HVIO hvio;
		} ShowBuf;
struct	{
			USHORT * Row;
			USHORT * Column;
			HVIO Handle;
		} GetCurPos;
struct	{
			VIOCURSORINFO * CursorInfo;
			HVIO Handle;
		} GetCurType;
struct	{
			VIOMODEINFO * ModeInfo;
			HVIO Handle;
		} GetMode;
struct	{
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} SetCurPos;
struct	{
			PVIOCURSORINFO CursorInfo;
			HVIO Handle;
		} SetCurType;
struct	{
			PVIOMODEINFO ModeInfo;
			HVIO hvio;
		} SetMode;
struct	{
			CHAR * CellStr;
			USHORT * Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} ReadCharStr;
struct	{
			CHAR * CellStr;
			USHORT * Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} ReadCellStr;
struct	{
			PCHAR Char;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtNChar;
struct	{
			PBYTE Attr;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtNAttr;
struct	{
			PBYTE Cell;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtNCell;
struct	{
			PCHAR Str;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtCharStr;
struct	{ 
			PCHAR Str;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			PBYTE pAttr;
			HVIO Handle;
		} WrtCharStrAtt;
struct	{
			PCHAR CellStr;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtCellStr;
struct	{ 
			USHORT TopRow;
			USHORT LeftCol;
			USHORT BotRow;
			USHORT RightCol;
			USHORT Lines;
			PBYTE Cell;
			HVIO Handle;
		} ScrollUp;
struct	{
			USHORT TopRow;
			USHORT LeftCol;
			USHORT BotRow;
			USHORT RightCol;
			USHORT Lines;
			PBYTE Cell;
			HVIO Handle;
		} ScrollDn;
struct	{
			USHORT usTopRow;
			USHORT usLeftCol;
			USHORT usBotRow;
			USHORT usRightCol;
			USHORT cbCol;
			PBYTE pCell;
			HVIO hvio;
		} ScrollLf;
struct	{ 
			USHORT usTopRow;
			USHORT usLeftCol;
			USHORT usBotRow;
			USHORT usRightCol;
			USHORT cbCol;
			PBYTE pCell;
			HVIO hvio;
		} ScrollRt;
struct	{ 
			USHORT Ansi;
			HVIO Handle;
		} SetAnsi;
struct	{ 
			USHORT * Ansi;
			HVIO Handle;
		} GetAnsi;
struct	{ 
			HVIO hvio;
		} PrtSc;
struct	{
			USHORT fWait;
			PUCHAR pfNotLocked;
			HVIO hvio;
		} ScrLock;
struct	{
			HVIO hvio;
		} ScrUnLock;
struct	{
			USHORT usRedrawInd;
			PUSHORT pNotifyType;
		} SavRedrawWait;
struct	{
			USHORT usOwnerInd;
			USHORT usKillInd;
		} SavRedrawUndo;
struct	{ 
			PUSHORT pfWait;
			HVIO hvio;
		} PopUp;
struct	{ 
			HVIO hvio;
		} EndPopUp;
struct	{ 
			HVIO hvio;
		} PrtScToggle;
struct	{
			USHORT usReqType;
			PUSHORT pNotifyType;
		} ModeWait;	
struct	{ 
			USHORT usOwnerInd;
			USHORT usKillInd;
		} ModeUndo;
struct	{
			PVIOFONTINFO pviofi;
			HVIO hvio;
		} GetFont;
struct	{
			USHORT ConfigId;
			VIOCONFIGINFO * vioin;
			HVIO hvio;
		} GetConfig;
struct	{ 
			USHORT IdCodePage;
			HVIO Handle;
		} SetCp;
struct	{ 
			USHORT * IdCodePage;
			HVIO Handle;
		} GetCp;
struct	{ 
			PVIOFONTINFO pviofi;
			HVIO hvio;
		} SetFont;
struct	{ 
			PVOID pState;
			HVIO Handle;
		} GetState;
struct	{ 
			PVOID pState;
			HVIO Handle;
		} SetState;
struct	{ 
			PCHAR Str;
			USHORT Count;
			HVIO Handle;
		} WrtTTY;
} ARGS, * PARGS;
