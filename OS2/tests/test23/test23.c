/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test23.c                                                      */
/*                                                                            */
/* Description: Test VioWrtTTY functions.                                     */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* VioWrtTTY                                                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/



/* include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_VIO
#include "os2.h"

#include "string.h"

/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

void main(void)
{
    char *str1 = "VioWrtTTY test";

    /* write some strings */
    VioWrtTTY(str1, strlen(str1), 0);
}
