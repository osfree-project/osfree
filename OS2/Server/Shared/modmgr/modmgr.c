#include "io.h"
#include "execlx.h"
#include "loadobjlx.h"
#include "modmgr.h"
#include "cfgparser.h"
#include "dynlink.h"
#include "native_dynlink.h"

#if 0
typedef
  struct
  {
    void * FormatStruct;
  } IXFModule;

unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ifxModule)
{
  struct LX_module *lx_exe_mod;

  lx_exe_mod = (struct LX_module *) malloc(sizeof(struct LX_module));

  ixfModule->FormatStruct=(void *)lx_exe_mod;

  if(load_lx_stream((char*)addr, size, lx_exe_mod)) /* A file from a buffer.*/
  {
    load_dll_code_obj_lx(lx_exe_mod); /* Load all objects in dll.*/
  }
}

unsigned long IXFFixupModule(IXFModule module)
{
  do_fixup_code_data_lx((struct LX_module *)ixfModule->FormatStruct);
  /* Apply fixups. */
}

unsigned long ModLoadModule(const char *    pszName,
                            unsigned long   cbName,
                            const char *    pszModname,
                            unsigned long * phmod)
{
  struct module_rec * new_module_el;
  #define buf_size 4096
  char buf[buf_size+1];
  char *p_buf = (char *) &buf;
  void * addr;
  unsigned long size;
  unsigned long rc;

  find_module_path(name, p_buf); /* Searches for module name and returns the full path in
                           the buffer p_buf. */

  rc=io_load_file(p_buf, &addr, &size);

  // Load module
  rc=IXFLoadModule(addr, size, &ixfModule);

  // Register in module list
  new_module_el = register_module(name, ixfModule);
  new_module_el->load_status = LOADING;

  // Fixup module
  rc=IXFFixupModule(ixfModule);

  new_module_el->load_status = DONE_LOADING;
  return lx_exe_mod;
}

#endif

        /* Loads a module name which proc needs. */
void * load_module(char * name) {
        struct LX_module *lx_exe_mod;
        FILE *f;
        struct module_rec * new_module_el;
        #define buf_size 4096
        char buf[buf_size+1];
        char *p_buf = (char *) &buf;

        find_module_path(name, p_buf); /* Searches for module name and returns the full path in
                                                                        the buffer p_buf. */

        lx_exe_mod = (struct LX_module *) malloc(sizeof(struct LX_module));
        io_printf("load_module: '%s' \n", p_buf);
        f = fopen(p_buf, "rb");  /* Open file in read only binary mode, in case this code
                                          will be compiled on OS/2 or on windows. */

        /* Load LX file from buffer. */
        /* if(load_lx_stream((char*)lx_buf, pos, &lx_exe_mod)) { */


        /* Load LX file from ordinary disk file. */
        if(f && load_lx(f, lx_exe_mod)) {     /* Load LX header.*/
                load_dll_code_obj_lx(lx_exe_mod); /* Load all objects in dll.*/
                new_module_el = register_module(name, lx_exe_mod);
                new_module_el->load_status = LOADING;
                        /* A risk for cycles here. Need to check if a dll is already loading,
                           indirect recursion.*/
                do_fixup_code_data_lx(lx_exe_mod); /* Apply fixups. */
                new_module_el->load_status = DONE_LOADING;
                return lx_exe_mod;
        }
        free(lx_exe_mod);
        if(f)
                fclose(f);
        io_printf("load_module: Load error!!! of %s in %s\n", name, p_buf);
        return 0;
}

int modmgr_execute_module(char * filename)
{
  int rc;
  int do_start;
  void * lx_buf;
  int pos;

  char* native_libpath[] =
  {
            "c:\\os2\\dll"
  };

  /* The structure for loaded LX exe. */
  struct LX_module lx_exe_mod ;

//  set_path(options.libpath, 0); /* Zero-based number in path.*/

  set_libpath(options.libpath, 0); /* Zero-based number in libpath.*/

  set_native_libpath(native_libpath, 0); /* Zero-based number i native_libpath.*/

  // Load executable into memory
  rc=io_load_file(filename, &lx_buf, &pos);

  if(load_lx_stream((char*)lx_buf, pos, &lx_exe_mod)) /* A file from a buffer.*/
  {
    /* Creates an simple process(keeps info about it, does not start to execute). */
    struct t_processlx * tiny_process = processlx_create(&lx_exe_mod);


    /* Reads the actual objects from the file, code- and dataobjects.*/
    /* load_code_data_obj_lx(&lx_exe_mod, tiny_process); */
    load_dll_code_obj_lx(&lx_exe_mod); /* Use same routine as the one is loading dlls.*/

    /* Register the exe with the module table. With the complete path. */
    register_module(filename, (void *) &lx_exe_mod);

    do_start = 0; /* A check to make sure the loading of the file succeeded. */

    /* Applies fixups to the loaded objects. */
    if(do_fixup_code_data_lx(&lx_exe_mod))
            do_start = 1;

    /* Print info about used memory loaded modules. */
    print_used_mem(&tiny_process->root_mem_area);
    print_detailed_module_table();

    /* Starts to execute the process. */
    if(do_start)
            exec_lx(&lx_exe_mod, tiny_process);
    processlx_destroy(tiny_process); /* Removes the process.
             Maybe use garbage collection here? Based on reference counter?
                And when the counter reeches zero, release process. */
  } else {
    io_printf("load_lx returns an error !!! \n");
  }

  return 0;
}
