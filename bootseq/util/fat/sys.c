/*  Utility Filesystem funclions
 *  
 *
 */

#define  INCL_OS2DEFS
#include <os2.h>
#include <stdio.h>

short _Far16 _Pascal SYS(short argc, char * _Seg16 * _Seg16 argv, char * _Seg16 * _Seg16 envp) {
//APIRET APIENTRY SYS(LONG argc, char **argv, char **envp) {
    fprintf(stderr, "CHKDSK not implemented yet.\n");

    return 0;
}
