/*  Loader/os2exec types and structs
 *
 *
 */

#ifndef __OS2EXEC_LOADER_H__
#define __OS2EXEC_LOADER_H__

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct
{
  void          *ip;
  void          *sp;
  void          *sp_limit;
  unsigned long hmod;
  char          exeflag;
  char          path[0x100];
} os2exec_module_t;

#ifdef __cplusplus
  }
#endif

#endif
