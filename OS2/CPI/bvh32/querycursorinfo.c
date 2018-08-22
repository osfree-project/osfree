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
          
USHORT EXPENTRY QueryCursorInfo(ENVIRONMENT FAR *Environment, VDH_CURSOR FAR *ParmBlock, ULONG Function)
{
}
