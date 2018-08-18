typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD LogicalBufferSelector;
          } VDH_SAVE;

USHORT EXPENTRY SaveEnvironment(ENVIRONMENT FAR *Environment, VDH_SAVE FAR *ParmBlock, ULONG Function)
{
}
