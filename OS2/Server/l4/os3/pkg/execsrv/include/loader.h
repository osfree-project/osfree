/*  Loader/os2exec types and structs
 *
 *
 */

#ifndef __OS2EXEC_LOADER_H__
#define __OS2EXEC_LOADER_H__
 
#include <l4/sys/types.h>
 
typedef 
struct
{
  l4_addr_t    ip;
  l4_addr_t    sp;
  l4_addr_t    sp_limit;
  l4_uint32_t  hmod;
} os2exec_module_t;

#endif
