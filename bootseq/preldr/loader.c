/*
 *
 *
 */

#include <lip.h>
#include <types.h>
#include <loader.h>
#include <bpb.h>

#include <shared.h>

#include "fsys.h"
#include "fsd.h"

extern lip2_t *l;
extern bios_parameters_block *bpb;
extern FileTable ft;

struct multiboot_info *m;

#pragma aux kernel_func  "*"
#pragma aux module_func  "*"
#pragma aux modaddr_func "*"
#pragma aux m            "*"

int kernel_func (char *arg, int flags);
int module_func (char *arg, int flags);
int modaddr_func (char *arg, int flags);

void init(lip2_t *l)
{

}

void KernelLoader(void)
{
  printf("KernelLoader started");

  if (kernel_func("(cd)/l4ka/kickstart",0x2))
     printf("An error occured during execution of kernel_func\r\n");

  //modaddr_func("0x02000000",0x2);

  if (module_func("(cd)/l4ka/pistachio",0x2))
     printf("An error occured during execution of module_func\r\n");

  if (module_func("(cd)/l4ka/sigma0",0x2))
     printf("An error occured during execution of module_func\r\n");

  if (module_func("(cd)/l4ka/pingpong",0x2))
     printf("An error occured during execution of module_func\r\n");
}

void cmain(void)
{
  printf("Hello from loader!\r\n");

  /* Get mbi structure address from pre-loader */
  u_parm(PARM_MBI, ACT_GET, (unsigned long *)&m);

  KernelLoader();
}
