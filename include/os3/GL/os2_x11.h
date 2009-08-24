/* os2_x11.h v 0.1 11/03/2000 EK */

#ifndef __os2_x11_h__
#define __os2_x11_h__

#define INCL_GPI
#define INCL_WIN

#include "os2_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (DD3D_APIENTRY FN_DD3DI_MESAINITHAB) (HAB  proghab,int param);
typedef FN_DD3DI_MESAINITHAB *PFN_DD3DI_MESAINITHAB;

#if defined(DINAMIC_LOAD)
 FN_DD3DI_MESAINITHAB OS2_WMesaInitHab;
//extern  void  APIENTRY OS2_WMesaInitHab(HAB  proghab,int useDive);
#else
 extern PFN_DD3DI_MESAINITHAB OS2_WMesaInitHab;
#endif

#ifdef __cplusplus
}
#endif

#endif
