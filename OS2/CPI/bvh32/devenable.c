#include "bvh.h"

/*
**
** Table of supported BVH entry points
**
*/

#define MaxFn 26

ENTRYPOINT *BVHEntryPoint[MaxFn] =
{
  &TextBufferUpdate,       //1
  &InitializeEnvironment,  //2
  &SaveEnvironment,        //3
  &RestoreEnvironment,     //4
  &QueryConfigInfo,        //5
  &DBCSDisplayInfo,        //6
  UNSUPPORTED_FUNCTION,    //7
  UNSUPPORTED_FUNCTION,    //8
  &QueryCursorInfo,        //9
  &SetCursorInfo,          //10
  UNSUPPORTED_FUNCTION,    //11
  UNSUPPORTED_FUNCTION,    //12
  &QueryMode,              //13
  &SetMode,                //14
  UNSUPPORTED_FUNCTION,    //15
  UNSUPPORTED_FUNCTION,    //16
  UNSUPPORTED_FUNCTION,    //17
  UNSUPPORTED_FUNCTION,    //18
  &QueryVariableInfo,      //19
  &SetVariableInfo,        //20
  UNSUPPORTED_FUNCTION,    //21
  UNSUPPORTED_FUNCTION,    //22
  UNSUPPORTED_FUNCTION,    //23
  &QueryLVBInfo,           //24
  &QueryVideoState,        //25
  &SetVideoState           //26
};

APIRET APIENTRY Dev32Enable(PPARM2 Parameter2,
                            PPARM1 Parameter1,
                            ULONG Subfunction)
{
  ULONG i;
  APIRET rc = 0;

  for (i=0; (i<=MaxFn) && (i<((PPARM1)Parameter1)->TableSize); i++)
  {
    if (BVHEntryPoint[i] != UNSUPPORTED_FUNCTION)
      ((PPARM2)Parameter2)->CallVectorTable[i+0x100] = BVHEntryPoint[i];
  }
  return rc;
}
