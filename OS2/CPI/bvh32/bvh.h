#include <os2.h>

#define ENVIRONMENT VOID
#define UNSUPPORTED_FUNCTION NULL

typedef APIRET (APIENTRY ENTRYPOINT)(VOID *, VOID *, ULONG);
typedef APIRET (APIENTRY ** ENTRYPOINTTABLE)(VOID *, VOID *, ULONG);

typedef struct _PARM1
        {
            ULONG       EngineVersion;
            ULONG       TableSize;
        } PARM1, * PPARM1;

typedef struct _PARM2
        {
            ULONG             * Flags;
            ENTRYPOINTTABLE     CallVectorTable;
        } PARM2, * PPARM2;

typedef struct
          {
          WORD     Parmlength;
          WORD     Flags;
          DWORD    ApplicationDataArea;
          DWORD    SecondaryDataArea;
          WORD     Index;
          WORD     StartingRow;
          WORD     StartingColumn;
          WORD     SecondaryRow;
          WORD     SecondaryColumn;
          WORD     RepeatFactor;
          WORD     LogicalBufferSelector;
          WORD     TouchXLeft;
          WORD     TouchYTop;
          WORD     TouchXRight;
          WORD     TouchYBottom;
          WORD     LVBRowOff;
          WORD     LVBColOff;
          WORD     LVBWidth;
          WORD     LVBHeight;
          BYTE     LVBFormatID;
          BYTE     LVBAttrCount;
          } VDH_BUFUP;

typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD DBCSTableLength;
          WORD DBCSTableOffset;
          } VDH_DBCS;

typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD LogicalBufferSelector;
          } VDH_INITENV;

typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD LogicalBufferSelector;
          } VDH_SAVE;

typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD LogicalBufferSelector;
          } VDH_RESTORE;

typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD BlinkBackgroundIntensity;
          WORD OverscanborderColor;
          WORD ScanLine;
          WORD VideoEnable;
          DWORD  DisplayMask;
          WORD CodePage;
          WORD ScrollableRectangleLeft;
          WORD ScrollableRectangleTop;
          WORD ScrollableRectangleRight;
          WORD ScrollableRectangleBottom;
          WORD ScreenRows;
          WORD ScreenCols;
          } VDH_VARIABLE;

typedef struct
          {
          WORD       Parmlength;
          WORD       Flags;
          DWORD      ModeDataStructureFarAddress;
          } VDH_MODE;

typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD Row;
          WORD Column;
          WORD TopCursorScanLine;
          WORD BottomCursorScanLine;
          WORD CursorWidth;
          WORD CursorAttribute;
          } VDH_CURSOR;
          

typedef struct
          {
          WORD         Length;
          WORD         Flags;
          } VDH_CONFIG;
          
typedef struct
          {                                                      
          WORD  Length;                                             
          WORD  Flags;                                              
          DWORD   OEMData;      
          } VDH_STATE;

APIRET APIENTRY InitializeEnvironment(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY RestoreEnvironment(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY SaveEnvironment(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY SetCursorInfo(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY SetMode(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY DBCSDisplayInfo(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY SetVideoState(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY QueryVideoState(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY TextBufferUpdate(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY SetVariableInfo(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY QueryVariableInfo(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY QueryMode(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY QueryCursorInfo(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY QueryConfigInfo(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
APIRET APIENTRY QueryLVBInfo(ENVIRONMENT *Environment, VOID *ParmBlock, ULONG Function);
