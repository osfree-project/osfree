typedef struct
          {
          WORD Parmlength;
          WORD Flags;
          WORD LogicalBufferSelector;
          } VDH_INITENV;


USHORT EXPENTRY InitializeEnvironment(ENVIRONMENT FAR *Environment, VDH_INITENV FAR *ParmBlock, ULONG Function)
{
}
