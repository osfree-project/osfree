/*
 *
 *
 */

#define  INCL_OS2DEFS
#include <os2.h>
#include <stdio.h>

//int pascal CHKDSK(INT iArgc, PSZ rgArgv[], PSZ rgEnv[])

short _Far16 _Pascal CHKDSK(short argc, char * _Seg16 * _Seg16 argv, char * _Seg16 * _Seg16 envp) {
//APIRET APIENTRY CHKDSK(LONG argc, char **argv, char **envp) {
    fprintf(stderr, "CHKDSK not implemented yet.\n");

    return 0;
}

short _Far16 _Pascal FORMAT(short argc, char * _Seg16 * _Seg16 argv, char * _Seg16 * _Seg16 envp) {
//APIRET APIENTRY FORMAT(LONG argc, char **argv, char **envp) {
    fprintf(stderr, "FORMAT not implemented yet.\n");

    return 0;
}

short _Far16 _Pascal RECOVER(short argc, char * _Seg16 * _Seg16 argv, char * _Seg16 * _Seg16 envp) {
//APIRET APIENTRY RECOVER(LONG argc, char **argv, char **envp) {
    fprintf(stderr, "RECOVER not implemented yet.\n");

    return 0;
}
