#ifndef __OS3_PATH_H__
#define __OS3_PATH_H__

#ifdef __cplusplus
  extern "C" {
#endif

char *get_fname(const char *s_path);
unsigned int find_path(const char *name, char *full_path_name);
unsigned int find_module_path(const char *name, char *full_path_name);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_PATH_H__ */
