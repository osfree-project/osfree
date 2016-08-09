/*  Output to system log
 *
 */

#include "kal.h"


APIRET APIENTRY DosLogWrite(PSZ s)
{
  return KalLogWrite(s);
}
