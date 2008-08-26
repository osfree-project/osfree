/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    Or see <http://www.gnu.org/licenses/>
*/
#define INCL_ERRORS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#include <os2.h>

#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#include <cfgparser.h>
#include <processlx.h>
#include <modlx.h>
#include <execlx.h>
#include <modmgr.h>
#include <ixfmgr.h>
#include <io.h>


//#define INCL_DOSPROCESS
//#include <bsetib.h>


/*ULONG     pib_ulpid;      Process identifier.
  ULONG     pib_ulppid;     Parent process identifier.
  ULONG     pib_hmte;       Module handle of executable program.
  PCHAR     pib_pchcmd;     Command line pointer.
  PCHAR     pib_pchenv;     Environment pointer.
  ULONG     pib_flstatus;   Process' status bits.
  ULONG     pib_ultype;     Process' type code. */

/* Creates a process for an LX-module. */
struct t_processlx * processlx_create(struct LX_module * lx_m) {

        struct t_processlx * c = (struct t_processlx *) malloc(sizeof(struct t_processlx));
        c->lx_pib   = (struct _PIB*) malloc(sizeof(PIB));
        c->main_tid = (struct _TIB*) malloc(sizeof(TIB));

    if (c != NULL) {
                c->pid = 1;
                c->code_mmap = 0;
                c->stack_mmap = 0;
                c->lx_mod = lx_m;

                c->lx_pib->pib_ulpid = 1;
                c->lx_pib->pib_ulppid = 0;
                c->lx_pib->pib_hmte = (ULONG) lx_m;
                c->lx_pib->pib_pchcmd = "";
                c->lx_pib->pib_pchenv = "";

                init_memmgr(&c->root_mem_area); /* Initialize the memory registry. */
                /* Registrate base invalid area. */
                alloc_mem_area(&c->root_mem_area, (void*) 1, 0xfffd);
                /* Make sure the the lower 64 kb address space is marked as used. */
        }
    return(c);
}

void processlx_destroy(struct t_processlx * proc) {
        /* Avmappar filen. */
        struct o32_obj * kod_obj = (struct o32_obj *) get_code(proc->lx_mod);
        int ret_munmap = munmap((void*) kod_obj->o32_base,
                        kod_obj->o32_size);


        if(ret_munmap != 0)
                io_printf("Error at unmapping of fh: %p\n", proc->lx_mod->fh);
        else
                io_printf("\nUnmapping fh: %p\n\n", proc->lx_mod->fh);

        free(proc->lx_pib);
        free(proc->main_tid);
        free(proc);
}

unsigned long find_path(char * name, char * full_path_name);


APIRET APIENTRY PrcExecuteModule(char * filename)
{
  int rc;
  void * addr;
  unsigned long size;
  IXFModule * ixfModule;
  struct t_processlx * tiny_process;
  #define buf_size 4096
  char buf[buf_size+1];
  char *p_buf = (char *) &buf;

  // Ok. No module found. Try to load file
  // (consider fully-qualified name specified)
//  rc=io_load_file(filename, &addr, &size);
//  if (rc)
//  {
    // Searches for module name and returns the full path in the buffer p_buf.
    rc=find_path(filename, p_buf);
    if (!rc) rc=io_load_file(p_buf, &addr, &size);
//  }
  if (rc) return rc;

  ixfModule = (IXFModule *) malloc(sizeof(IXFModule));

  rc=IXFIdentifyModule(addr, size, ixfModule);
  if (rc)
  {
    free(ixfModule);
    return rc;
  }

  // Load module
  rc=IXFLoadModule(addr, size, ixfModule);
  if (rc)
  {
    free(ixfModule);
    return rc;
  }

  /* Creates an simple process(keeps info about it, does not start to execute). */
  tiny_process = processlx_create((struct LX_module *)(ixfModule->FormatStruct));

  /* Register the exe with the module table. With the complete path. */
  /* @todo Is we really need to register executable??? Don't see any reason */
  // @todo extract filename only because can be fullname with path
  register_module(filename, ixfModule);

  // Fixup module
  rc=IXFFixupModule(ixfModule);
  if (rc)
  {
    free(ixfModule);
    return rc;
  }

  /* Print info about used memory loaded modules. */
  print_used_mem(&tiny_process->root_mem_area);

  /* Starts to execute the process. */
  exec_lx((struct LX_module *)(ixfModule->FormatStruct), tiny_process);

  processlx_destroy(tiny_process); /* Removes the process.
             Maybe use garbage collection here? Based on reference counter?
                And when the counter reeches zero, release process. */

  return NO_ERROR;
}


struct STR_SAVED_TOKENS_ {
        char *str_next_saved_tokens;
        char str_ch_saved_tokens;
};

typedef struct STR_SAVED_TOKENS_ STR_SAVED_TOKENS;


static char *nxtToken = 0;              /* pointer to previous scanned string */
static char ch;                                         /* previous token delimiter */

char *StrTokenize(char *str, const char * const token)
{

        if(!str) {
                if((str = nxtToken) == 0                /* nothing to do */
                 || (*str++ = ch) == 0)               /* end of string reached */
                        return( nxtToken = 0);
        }

        if(!token || !*token)                   /* assume all the string */
                nxtToken = 0;
        else {
                nxtToken = str - 1;
                while(!strchr(token, *++nxtToken));
                ch = *nxtToken;
                *nxtToken = 0;
        }

        return( str);
}

void StrTokSave(STR_SAVED_TOKENS *st)
{

        if(st) {
                st->str_next_saved_tokens = nxtToken;
                st->str_ch_saved_tokens = ch;
        }

}

void StrTokRestore(STR_SAVED_TOKENS *st)
{

        if(st) {
                nxtToken = st->str_next_saved_tokens;
                ch = st->str_ch_saved_tokens;
        }

}


#define StrTokStop() (void)StrTokenize(0, 0)


unsigned long find_path(char * name, char * full_path_name)
{
  FILE *f;
  char *p;
  #define buf_size 4096
  char buf[buf_size+1];
  char *path = (char *) &buf;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;

  cfg_getenv("PATH", &path);

  p = path - 1;

  StrTokSave(&st);
  if((p = StrTokenize((char*)path, ";")) != 0) do if(*p)
  {
    p_buf = full_path_name;
    p_buf[0] = 0;
    strcat(p_buf, p);
    strcat(p_buf, "\\");
    strcat(p_buf, name);
    f = fopen(p_buf, "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
    if(f)
    {
      StrTokStop();
      return NO_ERROR;
    }
  } while((p = StrTokenize(0, ";")) != 0);
  StrTokRestore(&st);

  return ERROR_FILE_NOT_FOUND;
}
