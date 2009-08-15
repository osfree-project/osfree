#define INCL_DOSMISC
#include <os2.h>

void main(void)
{
  char  msg[] = "I'm really small!\n";
  DosPutMessage(1, sizeof(msg), &msg);
}
