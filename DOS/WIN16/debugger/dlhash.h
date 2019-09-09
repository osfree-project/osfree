#include <link.h>

struct dyn_elf{
  struct elf_resolve * dyn;
  struct dyn_elf * next_handle;  /* Used by dlopen et al. */
  struct dyn_elf * next;
};
 
struct elf_resolve{
  /* These entries must be in this order to be compatible with the interface used
     by gdb to obtain the list of symbols. */
  char * loadaddr;
  char * libname;
  unsigned int dynamic_addr;
  struct elf_resolve * next;
  struct elf_resolve * prev;
  /* Nothing after this address is used by gdb. */
  enum {elf_lib, elf_executable,program_interpreter, loaded_file} libtype;
  struct dyn_elf * symbol_scope;
  unsigned short usage_count;
  unsigned short int init_flag;
  unsigned int nbucket;
  unsigned int * elf_buckets;
  /*
   * These are only used with ELF style shared libraries
   */
  unsigned int nchain;
  unsigned int * chains;
  unsigned int dynamic_info[24];

  unsigned int dynamic_size;
  unsigned int n_phent;
  struct elf_phdr * ppnt;
};

#if 0
/*
 * The DT_DEBUG entry in the .dynamic section is given the address of this structure.
 * gdb can pick this up to obtain the correct list of loaded modules.
 */

struct r_debug{
  int r_version;
  struct elf_resolve * link_map;
  unsigned long brk_fun;
  enum {RT_CONSISTENT, RT_ADD, RT_DELETE};
  unsigned long ldbase;
};
#endif

#define COPY_RELOCS_DONE 1
#define RELOCS_DONE 2
#define JMP_RELOCS_DONE 4
#define INIT_FUNCS_CALLED 8

extern struct dyn_elf     * _dl_symbol_tables;
extern struct elf_resolve * _dl_loaded_modules;

extern struct elf_resolve * _dl_check_hashed_files(char * libname);
extern struct elf_resolve * _dl_add_elf_hash_table(char * libname, 
					       char * loadaddr, 
					       unsigned int * dynamic_info, 
					       unsigned int dynamic_addr, 
					       unsigned int dynamic_size);
extern int _dl_linux_dynamic_link(void);

#ifdef __mc68000__
/* On m68k constant strings are referenced through the GOT. */
/* XXX Requires load_addr to be defined. */
#define SEND_STDERR(X)				\
  { const char *__s = (X);			\
    if (__s < (const char *) load_addr) __s += load_addr;	\
    _dl_write (2, __s, _dl_strlen (__s));	\
  }
#else
#define SEND_STDERR(X) _dl_write(2, X, _dl_strlen(X));
#endif
extern int _dl_write(int, const char *, int);
extern int _dl_fdprintf(int, const char *, ...);
extern char * _dl_library_path;
extern char * _dl_not_lazy;
extern char * _dl_strdup(const char *);
/*extern inline int _dl_symbol(char * name);*/
unsigned long _dl_elf_hash(const char * name);

#if 0
extern inline int _dl_symbol(char * name)
{
  if(name[0] != '_' || name[1] != 'd' || name[2] != 'l' || name[3] != '_')
    return 0;
  return 1;
}
#endif

#define DL_ERROR_NOFILE 1
#define DL_ERROR_NOZERO 2
#define DL_ERROR_NOTELF 3
#define DL_ERROR_NOTMAGIC 4
#define DL_ERROR_NOTDYN 5
#define DL_ERROR_MMAP_FAILED 6
#define DL_ERROR_NODYNAMIC 7
#define DL_WRONG_RELOCS 8
#define DL_BAD_HANDLE 9
#define DL_NO_SYMBOL 10

