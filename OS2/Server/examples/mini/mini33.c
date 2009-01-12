void puts(char *s);

#pragma data_seg("MYDATA", "STACK")
#pragma code_seg("DGROUP", "STACK")



#define INCL_DOSMISC
#include <os2.h>

APIRET APIENTRY DosPutMessage(HFILE hfile, ULONG cbMsg, PCHAR pBuf);
/*int _System DosPutMessage(unsigned int hfile, unsigned int cbMsg, char * pBuf);*/

void _System startup(void) {
    /*puts(msg); */
        char  msg[] = "I'm really small!\n";
        DosPutMessage(1, sizeof(msg), &msg);
}

/*
APIRET APIENTRY DosPutMessage(HFILE hfile, ULONG cbMsg, PCHAR pBuf);
 ordinal: MSG.5
unsigned int DosPutMessage(unsigned int hfile, unsigned int cbMsg, char * pBuf)
*/

/*
This is built with the following commands (old form with stars, non working):

**** wcc386 -s -g=DGROUP mini3.c  ****
**** wlink sys os2v2 name .exe f mini3 imp puts_ LIBCM.362 op start=startup,st=32k,nod ***
****  -g=DGROUP

wcc386 -bt=os2v2 -i/mnt/c/Projekt/open_watcom_src_1.6/rel2/h/os2 -s -g=DGROUP mini33.c
wlink sys os2v2 name mini33.exe f mini33 imp DosPutMessage MSG.5 op start=startup,st=32k,nod
*/
