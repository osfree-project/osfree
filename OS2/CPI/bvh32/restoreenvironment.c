typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD LogicalBufferSelector;
          } VDH_RESTORE;
          
USHORT EXPENTRY RestoreEnvironment(ENVIRONMENT FAR *Environment, VDH_RESTORE FAR *ParmBlock, ULONG Function)
{
}
