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
 	        WORD     TouchXLeft
          WORD     TouchYTop
          WORD     TouchXRight
          WORD     TouchYBottom
          WORD     LVBRowOff
          WORD     LVBColOff
          WORD     LVBWidth
          WORD     LVBHeight
          BYTE     LVBFormatID
          BYTE     LVBAttrCount
          } VDH_BUFUP;

USHORT EXPENTRY TextBufferUpdate(ENVIRONMENT FAR *Environment, VDH_BUFUP FAR *ParmBlock, ULONG Function)
{

}
