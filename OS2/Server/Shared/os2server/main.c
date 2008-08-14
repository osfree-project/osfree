#include <io.h>
#include <cfgparser.h>
#include <modmgr.h>
#include <memmgr.h>

 /*********************************************************************
 * Here everything starts. This is the main function of the           *
 * OS/2 Server.                                                       *
 **********************************************************************/
int main(int argc, const char **argv)
{
    int rc;               // Return code
    void *addr;           // Pointer to CONFIG.SYS in memory
    int size;             // Size of CONFIG.SYS in memory

    io_printf("OS/2 Server started\n");

    // Initialize initial values from CONFIG.SYS
    rc=cfg_init_options();

    // Load CONFIG.SYS into memory
    rc=io_load_file(filename, &addr, &size);

    // Parse CONFIG.SYS in memory
    rc=cfg_parse_config(addr, size);

    // Remove CONFIG.SYS from memory
    rc=memmgr_free(addr, size);

    // Load and execute shell
    rc=modmgr_execute_module(options.protshell);

    // Clean up config data
    rc=cfg_cleanup();

    io_printf("OS/2 Server ended\n");

    return rc;
}
