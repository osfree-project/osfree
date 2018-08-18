
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
          
USHORT EXPENTRY QueryVariableInfo(ENVIRONMENT FAR *Environment, VDH_VARIABLE FAR *ParmBlock, ULONG Function)
{
}
