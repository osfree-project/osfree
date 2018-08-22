typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD DBCSTableLength;
          WORD DBCS Table Offset;
          } VDH_DBCS;

      
USHORT EXPENTRY DBCSDisplayInfo(ENVIRONMENT FAR *Environment, VDH_DBCS FAR *ParmBlock, ULONG Function)
{
}
