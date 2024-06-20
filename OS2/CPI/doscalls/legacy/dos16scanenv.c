#include "kal.h"


APIRET16 APIENTRY16     DOS16SCANENV(char * _Seg16 pszEnvVar, char * _Seg16 * _Seg16 ppszResult)
{
  return DosScanEnv(pszEnvVar, ppszResult);
}
