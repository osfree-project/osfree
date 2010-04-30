#define INCL_GPILCIDS           /* Font functions               */

#include <os2.h>



BOOL  fSuccess;         /* success indicator                    */

HAB   hab;              /* anchor-block handle                  */

char  pszFilename[16];  /* Name of font resource file          */



/* Resource file is named 'TEST.FON' */

strcpy(pszFilename,"C:\\TEST.FON");



fSuccess = GpiLoadPublicFonts(hab, pszFilename);
