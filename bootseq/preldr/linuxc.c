/*
 *
 */

#pragma aux m     "*"
#pragma aux l     "*"

#include <lip.h>

struct multiboot_info *m;
lip2_t *l;

void init(void)
{

}

void cmain(void)
{
  //unsigned int size;
  //l->u_open("os2krnl", &size); 
  #define VIDEO_BUF 0xb8000
  char *errmsg = "Hello from multiboot kernel!";
  int  msglen  = 28;

  __asm {
start:
        cld
        mov   esi, errmsg
        mov   edi, VIDEO_BUF
        mov   ecx, msglen
        mov   ah,  02h // attribute
loop1:
        lodsb          // symbol
        stosw
        loop  loop1    // copy a string to video buffer

        cli
        hlt
  }
}
