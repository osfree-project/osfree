void puts(char *s);

#pragma data_seg("MYDATA", "STACK")
#pragma code_seg("DGROUP", "STACK")
/*
   An even smaller test exe with no external dependencies, good enough to test only the
   loader.
   NOTE! There seems to be a bug of some sort inside the config parser, because I could not
   load a protshell exe of the name 'nullmini.exe', it only worked with the name 'nmini.exe'.
   What kind of problem does it have with null?
*/
#define INCL_DOSMISC
#include <os2.h>

APIRET APIENTRY DosPutMessage(HFILE hfile, ULONG cbMsg, PCHAR pBuf);
/*int _System DosPutMessage(unsigned int hfile, unsigned int cbMsg, char * pBuf);*/

void _System startup(void) {
    /*puts(msg); */
        /*char  msg[] = "I'm really small!\n";
        DosPutMessage(1, sizeof(msg), &msg);*/
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

wcc386 -bt=os2v2 -i/mnt/c/Projekt/open_watcom_src_1.6/rel2/h/os2 -s -g=DGROUP nullmini.c
wlink sys os2v2 name nullmini.exe f nullmini imp DosPutMessage MSG.5 op start=startup,st=32k,nod
*/
