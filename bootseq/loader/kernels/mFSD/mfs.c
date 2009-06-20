/*
 *
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>                // From the "Developer Connection Device Driver Kit" version 2.0

//#include <ifs.h>

int far pascal MFS_CHGFILEPTR(
    unsigned long  offset,              /* offset       */
    unsigned short type                 /* type         */
) {
    return NO_ERROR;
}

int far pascal MFS_CLOSE(void) {
    return NO_ERROR;
}

int far pascal MFS_INIT(
    void far *bootdata,                 /* bootdata     */
    char far *number,                   /* number io    */
    long far *vectorripl,               /* vectorripl   */
    void far *bpb,                      /* bpb          */
    unsigned long far *pMiniFSD,        /* pMiniFSD     */
    unsigned long far *dump             /* dump address */
) {
    return NO_ERROR;
}

int far pascal MFS_OPEN(
    char far *name,                     /* name         */
    unsigned long far *size             /* size         */
) {
    return NO_ERROR;
}

int far pascal _loadds MFS_READ(
    char far *data,             /* data         */
    unsigned short far *length  /* length       */
) {
    return NO_ERROR;
}

int far pascal _loadds MFS_TERM(void) {

    //printf("**** MFS_TERM");

    return NO_ERROR;
}
