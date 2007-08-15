/* os2_config.h v 0.0 14/12/2002 EK */

#ifndef __os2_mesa_config_h__
#define __os2_mesa_config_h__

#include "F_config.hpp"
#include "F_def.hpp"

//#define INCL_DOSPROCESS
#define F_INCL_DOSPROCESS
#define F_INCL_DOSDATETIME
#define F_INCL_WIN
   #include "F_OS2.hpp"
//   #include <os2.h>

  #define WINGDIAPI 
  #define WINAPI
#define CALLBACK

#define CDECL __cdecl

#define USE_IEEE
#ifndef __i386__
	#define __i386__
#endif

#define __OS2PM__

/* Warning! use only if you know what are you doing */
/* Optlink is VAC specific and is a bit faster,     
   but it may be not compatble with user compiler
*/
#define USE_OPTLINK

#if defined(USE_OPTLINK)
 #define GLAPIENTRY    _Optlink
 #define GLCALLBACK    _Optlink
 #define GLUTAPIENTRY  _Optlink
 #define GLUTCALLBACK  _Optlink
#else
 #define GLAPIENTRY    APIENTRY
 #define GLCALLBACK    APIENTRY
 #define GLUTAPIENTRY  APIENTRY
 #define GLUTCALLBACK  APIENTRY
#endif

#define GLCALLBACKP *
#define GLCALLBACKPCAST *
#define GLWINAPI
#define GLWINAPIV

#define GLAPI extern

#define DD3D_APIENTRY  _System


#endif
   //__os2_mesa_config_h__