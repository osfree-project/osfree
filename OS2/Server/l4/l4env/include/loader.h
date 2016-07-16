/*  Loader/os2exec types and structs
 *
 *
 */

#ifndef __OS2EXEC_LOADER_H__
#define __OS2EXEC_LOADER_H__

#ifdef __cplusplus
  extern "C" {
#endif
 
#include <l4/sys/types.h>
 
typedef 
struct
{
  l4_addr_t     ip;
  l4_addr_t     sp;
  l4_addr_t     sp_limit;
  l4_uint32_t   hmod;
  char          path[0x100];
} os2exec_module_t;

#ifdef __cplusplus
  }
#endif

#endif
