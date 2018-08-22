typedef USHORT (APIENTRY FAR ENTRYPOINT)(VOID FAR *, VOID FAR *, ULONG);
typedef USHORT (APIENTRY FAR * FAR * ENTRYPOINTTABLE)(VOID FAR *, VOID FAR *, ULONG);

typedef struct _PARM1
        {
            ULONG       EngineVersion;
            ULONG       TableSize;
        } PARM1, FAR * PPARM1;

typedef struct _PARM2
        {
            ULONG               FAR * Flags;
            ENTRYPOINTTABLE     CallVectorTable;
        } PARM2, FAR * PPARM2;

/*
**
** Table of BVH entry points supported
**
*/


ENTRYPOINT FAR *BVHEntryPoint[MaxFn] =
{
  &TextBufferUpdate,
  &InitializeEnvironment,
  &SaveEnvironment,
  &RestoreEnvironment,
  &QueryConfigInfo,
  &DBCSDisplayInfo,
  UNSUPPORTED_FUNCTION,
  UNSUPPORTED_FUNCTION,
  &QueryCursorInfo,
  &SetCursorInfo,
  UNSUPPORTED_FUNCTION,
  UNSUPPORTED_FUNCTION,
  &QueryMode,
  &SetMode,
  UNSUPPORTED_FUNCTION,
  UNSUPPORTED_FUNCTION,
  UNSUPPORTED_FUNCTION,
  UNSUPPORTED_FUNCTION,
  &QueryVariableInfo,
  &SetVariableInfo,
  UNSUPPORTED_FUNCTION,
  UNSUPPORTED_FUNCTION,
  UNSUPPORTED_FUNCTION,
  &QueryLVBInfo,
  &QueryVideoState,
  &SetVideoState
};

USHORT APIENTRY DevEnable(PPARM2 Parameter2,
                          PPARM1 Parameter1,
                          ULONG Subfunction)
{
  ULONG i;
  USHORT rc = 0;


        for (i=0; (i<=MaxFn) && (i<((PPARM1)Parameter1)->TableSize); i++)
        {
          if (BVHEntryPoint[i] != UNSUPPORTED_FUNCTION)
            ((PPARM2)Parameter2)->CallVectorTable[i+0x100] = BVHEntryPoint[i];
        }
  return rc;
}



