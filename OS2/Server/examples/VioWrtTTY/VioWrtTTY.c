#include <l4/log/l4log.h>
#include <stdio.h>
#include <l4/os2/os2server.h>

int main(void)
{
  printf("Call of os2server_VioWrtTTY\n");
  os2server_VioWrtTTY("test", 4, 0);
  return 0;
}
