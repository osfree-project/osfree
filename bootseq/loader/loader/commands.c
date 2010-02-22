/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <lip.h>
#include <filesys.h>
#include <shared.h>
#include <serial.h>

#include "fsd.h"
#include "fsys.h"

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

/* The type of kernel loaded.  */
kernel_t kernel_type;
extern grub_error_t errnum;
extern struct builtin *builtins[];
extern struct term_entry *t;

/* The address for Multiboot command-line buffer.  */
static char *mb_cmdline;

#pragma aux l "*"
//#pragma aux t "*"

//#pragma aux kernel_func    "*"
//#pragma aux module_func    "*"
//#pragma aux modaddr_func   "*"
//#pragma aux lipmodule_func "*"

//#pragma aux skip_to "*"

extern lip2_t *l;
void create_lip_module(lip2_t **l);
int create_bs_module(char *arg);

extern int default_item;
int  menu_timeout;

/* menu colors */
extern int background_color; // black
extern int foreground_color; // white
extern int background_color_hl;
extern int foreground_color_hl;

extern int screen_bg_color;
extern int screen_fg_color;
extern int screen_bg_color_hl;
extern int screen_fg_color_hl;

extern char prev_cfg[0x100];
extern char curr_cfg[0x100];

extern int num_items;
extern int item_save  = 0;
extern int shift_save = 0;

int exec_cfg(char *cfg, int menu_item, int menu_shift);

int abbrev(char *s1, char *s2, int n);
int lipmodule_func (char *arg, int flags);

/* menu width and height */
#define MENU_WIDTH    56
#define MENU_HEIGHT   10
int menu_width  = MENU_WIDTH;
int menu_height = MENU_HEIGHT;

int slot;

#define VARIABLE_STORE_SIZE 1024
char variable_store[VARIABLE_STORE_SIZE];
unsigned int variable_store_actpos = 0; /* Points to the next free entry */
struct variable_list_struct {
  char *name;
  char *value;
} variable_list[VARIABLES_MAX];

//static
void var_show(void)
{
  int i = 0;

  for (; i < VARIABLES_MAX; i++)
    if (variable_list[i].name)
      {
        grub_printf("%s = %s\n",
                    variable_list[i].name,
                    variable_list[i].value);
      }
}

//static
int var_get_index(char *var)
{
  int i = 0;

  if (!*var)
    return -1;

  for (; i < VARIABLES_MAX; i++)
    if (variable_list[i].name &&
        grub_strcmp(variable_list[i].name, var) == 0)
      return i;

  return -1;
}

char *var_get(char *var)
{
  int i;

  if ((i = var_get_index(var)) == -1)
    return NULL;

  return variable_list[i].value;
}

//static
char *var_alloc_mem(unsigned int len)
{
  if (VARIABLE_STORE_SIZE < variable_store_actpos + len
      || !len)
  {
    printf("len = %d, variable_store_actpos = 0x%x\r\n", len, variable_store_actpos);
    return NULL;
  }

  variable_store_actpos += len;
  return &variable_store[variable_store_actpos - len];
}

/* Maybe we're implementing "unset" later? */
//static
int var_get_free_var(void)
{
  int i = 0;

  for (; i < VARIABLES_MAX; i++)
    if (variable_list[i].name == NULL)
      return i;

  return -1;
}

//static inline
char *skip_ws(char *s)
{
  while (isspace(*s))
    s++;
  return s;
}

//static
int var_sprint_once(char *buf, char *str)
{
  char *b = buf;

  while (*str)
    {
      if (*str == '$' && *(str + 1) == '(')
        {
          /* Found start of variable */
          char *end_var;
          char *c = str + 2;

          end_var = c;
          while (*end_var && *end_var != ')')
            end_var++;

          if (*end_var == ')')
            {
              char *val;

              /* Copy variable name into var */

              str = end_var + 1;

              *end_var = 0;
              if ((val = var_get(c)))
                {
                  /* All ok, we got it */
                  /* Copy to buf */
                  while (*val)
                    *b++ = *val++;

                  *end_var = ')';

                  continue;
                }
              /* else
                grub_printf("Unknown variable: %var!\n", var); */

              *end_var = ')';
            }
        }

      *b++ = *str++;
    }

  *b = 0;

  return b - buf;
}

int var_sprint(char *buf, char *str)
{
  int i = 10;
  /* Waste some stack here... */
  //const int buffer_size = 1000;
  #define buffer_size 1000
  char buffer[buffer_size];

  grub_strcpy(buf, str);

  /* We're not checking any return values here or any array overflows... :( */
  do
    {
      if (grub_strlen(buf) > buffer_size)
        {
          grub_printf("Buffer overflow: %s(%d)\n", __FILE__, __LINE__);
          while (1) {}
        }

      grub_strcpy(buffer, buf);

      var_sprint_once(buf, buffer);
    }
  while (--i && grub_strcmp(buf, buffer));

  if (!i)
    {
      grub_printf("Possible loop in var_sprintf!\n");
      //getkey();
    }

  return grub_strlen(buf);
}

/* Use our own buffer instead of a supplied one and
 * return the pointer to the buffer and not the bytes
 * processed. */
/* We try to detect buffer overruns... */
static char var_sprint_buffer[1500];
static const long var_sprint_magic = 0x14233241;
char *var_sprint_buf(char *str, int *bytes)
{
  *(long *)(var_sprint_buffer + sizeof(var_sprint_buffer) - sizeof(var_sprint_magic)) = var_sprint_magic;

  *bytes = var_sprint(var_sprint_buffer, str);

  if (*(long *)(var_sprint_buffer + sizeof(var_sprint_buffer) - sizeof(var_sprint_magic)) != var_sprint_magic)
    {
      grub_printf("Possible buffer overrun: %s(%d)\n", __FILE__, __LINE__);
      while (1) {}
    }
  return var_sprint_buffer;
}

/* This function just updates the pointer to the value, this value has to
 * be inside the variable_store */
//static
int var_set_no_copy(char *name, char *val)
{
  int i;

  if (val < variable_store || val >= (variable_store + VARIABLE_STORE_SIZE))
    return 1;

  i = var_get_index(name);

  if (i == -1)
    {
      /* The variable doesn't exist yet, so we have a new variable */
      char *a = name;

      /* Some sanity check */
      while (*a)
        {
          if (*a == '(' || *a == ')' || *a == ' ')
            return 1;
          a++;
        }


      if ((i = var_get_free_var()) == -1)
        return 1;

      if ((a = var_alloc_mem(grub_strlen(name) + 1)) == NULL)
        return 1;

      grub_strcpy(a, name);
      variable_list[i].name = a;
    }

  variable_list[i].value = val;

  return 0; /* Ok */
}

/* XXX: this is a bit code duplication with var_set_no_copy,
 *      so if someone has some free time, feel free to join this
 */
int var_set(char *name, char *value, int parse)
{
  int i;

  if (parse)
    value = var_sprint_buf(value, &i);

  i = var_get_index(name);

  if (i == -1)
    {
      /* The variable doesn't exist yet, so we have a new variable */
      char *a = name;

      /* Some sanity check */
      while (*a)
        {
          if (*a == '(' || *a == ')' || *a == ' ')
            return 1;
          a++;
        }

      if ((i = var_get_free_var()) == -1)
        return 1;

      if ((a = var_alloc_mem(grub_strlen(name) + 1)) == NULL)
        return 1;

      grub_strcpy(a, name);
      variable_list[i].name = a;

      if ((a = var_alloc_mem(grub_strlen(value) + 1)) == NULL)
        return 1;

      grub_strcpy(a, value);
      variable_list[i].value = a;
    }
  else
    {
      /* Variable already exists */
      if (grub_strlen(variable_list[i].value) >= grub_strlen(value))
        {
          /* We can just replace the value but we're potentially
           * loosing space */
          grub_strcpy(variable_list[i].value, value);
        }
      else
        {
          /* New value is longer then the old one, we need to
           * allocate a new place and drop the old one (i.e. wasting it) */
          char *a;
          if ((a = var_alloc_mem(grub_strlen(value) + 1)) == NULL)
            return 1;

          grub_strcpy(a, value);

          variable_list[i].value = a;
        }
    }

  //grub_printf("Saved %s=\"%s\" in slot %d.\n", variable_list[i].name, variable_list[i].value, i + 1);
  //getkey();

  return 0; /* Ok */
}

/* kernel */
int
kernel_func (char *arg, int flags)
{
  int len;
  kernel_t suggested_type = KERNEL_TYPE_NONE;
  unsigned long load_flags = 0;

#ifndef AUTO_LINUX_MEM_OPT
  load_flags |= KERNEL_LOAD_NO_MEM_OPTION;
#endif

  /* Deal with GNU-style long options.  */
  while (1)
    {
      /* If the option `--type=TYPE' is specified, convert the string to
         a kernel type.  */
      if (grub_memcmp (arg, "--type=", 7) == 0)
        {
          arg += 7;

          if (grub_memcmp (arg, "netbsd", 6) == 0)
            suggested_type = KERNEL_TYPE_NETBSD;
          else if (grub_memcmp (arg, "freebsd", 7) == 0)
            suggested_type = KERNEL_TYPE_FREEBSD;
          else if (grub_memcmp (arg, "openbsd", 7) == 0)
            /* XXX: For now, OpenBSD is identical to NetBSD, from GRUB's
               point of view.  */
            suggested_type = KERNEL_TYPE_NETBSD;
          else if (grub_memcmp (arg, "linux", 5) == 0)
            suggested_type = KERNEL_TYPE_LINUX;
          else if (grub_memcmp (arg, "biglinux", 8) == 0)
            suggested_type = KERNEL_TYPE_BIG_LINUX;
          else if (grub_memcmp (arg, "multiboot", 9) == 0)
            suggested_type = KERNEL_TYPE_MULTIBOOT;
          else
            {
              errnum = ERR_BAD_ARGUMENT;
              u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
              return 1;
            }
        }
      /* If the `--no-mem-option' is specified, don't pass a Linux's mem
         option automatically. If the kernel is another type, this flag
         has no effect.  */
      else if (grub_memcmp (arg, "--no-mem-option", 15) == 0)
        load_flags |= KERNEL_LOAD_NO_MEM_OPTION;
      else
        break;

      /* Try the next.  */
      arg = skip_to (0, arg);
    }

  len = grub_strlen (arg);

  /* Reset MB_CMDLINE.  */
  mb_cmdline = (char *) MB_CMDLINE_BUF;
  if (len + 1 > MB_CMDLINE_BUFLEN)
    {
      errnum = ERR_WONT_FIT;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      return 1;
    }

  /* Copy the command-line to MB_CMDLINE.  */
  grub_memmove (mb_cmdline, arg, len + 1);
  kernel_type = load_image (arg, mb_cmdline, suggested_type, load_flags);
  if (kernel_type == KERNEL_TYPE_NONE)
  {
    return 1;
  }

  mb_cmdline += len + 1;
  return 0;
}

static struct builtin builtin_kernel =
{
  "kernel",
  kernel_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "kernel [--no-mem-option] [--type=TYPE] FILE [ARG ...]",
  "Attempt to load the primary boot image from FILE. The rest of the"
  " line is passed verbatim as the \"kernel command line\".  Any modules"
  " must be reloaded after using this command. The option --type is used"
  " to suggest what type of kernel to be loaded. TYPE must be either of"
  " \"netbsd\", \"freebsd\", \"openbsd\", \"linux\", \"biglinux\" and"
  " \"multiboot\". The option --no-mem-option tells GRUB not to pass a"
  " Linux's mem option automatically."
};

/* module */
int
module_func (char *arg, int flags)
{
  int len = grub_strlen (arg);

  if (abbrev(arg, "--type", 6))
  {
    arg = skip_to(1, arg);
    if (abbrev(arg, "lip", 3))
    {
      if (lipmodule_func(arg, 0x2))
      {
        printf("An error occured during execution of lipmod\r\n");
        return 1;
      }
      return 0;
    }
  }

  switch (kernel_type)
    {
    case KERNEL_TYPE_MULTIBOOT:
      if (mb_cmdline + len + 1 > (char *) MB_CMDLINE_BUF + MB_CMDLINE_BUFLEN)
        {
          errnum = ERR_WONT_FIT;
          u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
          return 1;
        }
      grub_memmove (mb_cmdline, arg, len + 1);
      if (! load_module (arg, mb_cmdline))
        return 1;
      mb_cmdline += len + 1;
      break;

    //case KERNEL_TYPE_LINUX:
    //case KERNEL_TYPE_BIG_LINUX:
    //  if (! load_initrd (arg))
    //    return 1;
    //  break;

    default:
      errnum = ERR_NEED_MB_KERNEL;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      return 1;
    }

  return 0;
}

static struct builtin builtin_module =
{
  "module",
  module_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "module FILE [ARG ...]",
  "Load a boot module FILE for a Multiboot format boot image (no"
  " interpretation of the file contents is made, so users of this"
  " command must know what the kernel in question expects). The"
  " rest of the line is passed as the \"module command line\", like"
  " the `kernel' command."
};

/* modaddr */
int
modaddr_func (char *arg, int flags)
{
  int addr;

  switch (kernel_type)
    {
    case KERNEL_TYPE_MULTIBOOT:
      if (safe_parse_maxint(&arg, (long *)&addr))
        {
          set_load_addr(addr);
          break;
        }

      /* else fallthrough */

    default:
      errnum = ERR_NEED_MB_KERNEL;
      u_parm(PARM_ERRNUM, ACT_SET, (unsigned int *)&errnum);
      return 1;
    }

  return 0;
}

static struct builtin builtin_modaddr =
{
  "modaddr",
  modaddr_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "modaddr ADDRESS",
  "Set the load address for the next Multiboot module to ADDRESS"
};

/* lipmod */
int
lipmodule_func (char *arg, int flags)
{
  create_lip_module(&l);
  return 0;
}

static struct builtin builtin_lipmodule =
{
  "lipmodule",
  lipmodule_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "lipmodule",
  "Create a module with LIP table in it"
};

/* bs mod */

int
bsmodule_func (char *arg, int flags)
{
  if (!create_bs_module(arg))
    return 1;

  return 0;
}

static struct builtin builtin_bsmodule =
{
  "bsmodule",
  bsmodule_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "bsmodule <file>",
  "Create a module with bootsector and fix BPB table in it,"
  "so correct hiddensectors and boot drive letter are set."
};


/* vbeset MODE */
int
vbeset_func (char *arg, int flags)
{
#ifndef GRUB_UTIL
  int mode_number;
  int pmif_segoff, pmif_len;
  struct vbe_controller controller;
  struct vbe_mode mode;

  if (kernel_type != KERNEL_TYPE_MULTIBOOT)
    {
      grub_printf("Multiboot kernel must be loaded before vbeset command\n");
      errnum = MAX_ERR_NUM;
      return 1;
    }

  safe_parse_maxint (&arg, (long *)&mode_number);

  if (vbe_mode_set(&controller, mode_number,
                   &mode, (unsigned int *)&pmif_segoff,
                   (unsigned int *)&pmif_len) == -1)
    return 1;

  create_vbe_module(&controller, sizeof(struct vbe_controller),
                    &mode, sizeof(struct vbe_mode),
                    mode_number, pmif_segoff, pmif_len, controller.version);

  /* mode setting was successful */
  return 0;
#else
  errnum = ERR_BAD_ARGUMENT;
  return 1;
#endif
}

static struct builtin builtin_vbeset =
{
  "vbeset",
  vbeset_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "vbeset [MODE]",
  "Set the VBE mode MODE. If no MODE is given, switch back to text mode."
};

//static
int
set_func(char *arg, int flags)
{
  char *a = arg;
  char *variable, *value;
  char *end_variable, end_variable_val;
  char *end_value, end_value_val;
  char end_char;
  int i = 0, parse = 0, emptysetonly = 0;

  a = skip_ws(a);

  if (!*a)
    {
      var_show();
      return 0;
    }

  variable = a;
  //while (*a && *a != ':' && *a != '=' && *a != ' ')
  while (*a && *a != ':' && *a != '=' && *a != '?' && *a != ' ')
    a++;
  end_variable_val = *a;
  end_variable = a;

  a = skip_ws(a);

  if (*a == ':')
    {
      parse = 1;
      a++;
    }
  if (*a == '?')
    {
      emptysetonly = 1;
      a++;
    }

  if (*a != '=')
    goto bad_arg;

  a++;
  a = skip_ws(a);

  end_char = ' ';
  if (*a == '"')
    {
      end_char = *a;
      a++;
    }
  /* XXX: add/fix \" and \\ stuff */
  value = a;
  while (*a && *a != end_char)
    a++;
  end_value_val = *a;
  end_value = a;

  *end_value = *end_variable = 0;

  if ((emptysetonly && var_get_index(variable) == -1) ||
      !emptysetonly)
    i = var_set(variable, value, parse);

  *end_value    = end_value_val;
  *end_variable = end_variable_val;

  return i;

bad_arg:
  //grub_printf("%s: ERR_BAD_ARGUMENT\n", __func__);
  //getkey();
  errnum = ERR_BAD_ARGUMENT;
  return 1;
}

static struct builtin builtin_set =
{
  "set",
  set_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "set var=val",
  "Set a variable to a value."
};

#define TOGGLES 24
#define MAX_BLOCKS 24
#define MAX_VAR_PER_BLOCK 5

struct toggle_data_struct {
  int key; /* int because of F-keys !? */
  int current_block;
  char nr_blocks;
  struct {
    char nr_vars;
    struct {
      int var;          /* index of variable_list */
      char *value;      /* pointer to value */
    } var[MAX_VAR_PER_BLOCK];
  } block[MAX_BLOCKS];
} toggle_data[TOGGLES];
int toggles_used = 0;

char toggle_trigger_init_done;

//static
int get_toggle_slot_for_key(int key)
{
  int i = 0;

  for (; i < TOGGLES; i++)
    if (toggle_data[i].key == key)
      return i;

  return -1;
}

int toggle_print_status(int x, int y)
{
  /* Basically print all variable which are on the left side on toggles */
  int printed[VARIABLES_MAX];
  int printed_something = 0;
  int i, tg, b, v, dummy;
  //const int ylines = 6;
  #define ylines 6
  int xpos[ylines];
  int dy = 0;

  for (i = 0; i < ylines; i++)
    xpos[i] = x;

  for (i = 0; i < VARIABLES_MAX; i++)
    printed[i] = 0;

  for (tg = 0; tg < toggles_used; tg++)
    {
      for (b = 0; b < toggle_data[tg].nr_blocks; b++)
        {
          for (v = 0; v < toggle_data[tg].block[b].nr_vars; v++)
            {
              i = toggle_data[tg].block[b].var[v].var;
              if (!printed[i])
                {
                  int len;
                  char *vals;

                  t->gotoxy(xpos[dy], y + dy);

                  /* don't use printf here since we need the lengths
                   * of the printed string and we don't want to use another
                   * buffer for sprintf
                   */
                  grub_putstr(variable_list[i].name);
                  grub_putchar('=');
                  len = grub_strlen(variable_list[i].name) + 1;

                  vals = var_sprint_buf(variable_list[i].value, &dummy);
                  grub_putstr(vals);
                  len += grub_strlen(vals);

                  xpos[dy] += len + 2;

                  if (++dy == ylines) {
                    dy = 0;
                  }

                  printed[i] = printed_something = 1;
                }
            }
        }
    }

  return printed_something;
}

//static
int toggle_do_block(int slot, int block_nr)
{
  int v;

  if (slot >= toggles_used || block_nr >= toggle_data[slot].nr_blocks)
    return 0;

  /* Set all variables in th block block_nr to their new values */
  for (v = 0; v < toggle_data[slot].block[block_nr].nr_vars; v++)
    {
      int l = toggle_data[slot].block[block_nr].var[v].var;

      var_set_no_copy(variable_list[l].name, toggle_data[slot].block[block_nr].var[v].value);
    }

  return 1; /* Ok */
}

//static
int toggle_find_slot(int key)
{
  int i = 0;

  for (; i < toggles_used; i++)
      if (toggle_data[i].key == key)
        return i;

  return -1;
}

int toggle_do_key(int key)
{
  int slot;

  if ((slot = toggle_find_slot(key)) == -1)
    return 0;

  /* Proceed to the next block */
  if (++toggle_data[slot].current_block == toggle_data[slot].nr_blocks)
    toggle_data[slot].current_block = 0;

  toggle_do_block(slot, toggle_data[slot].current_block);

  return 1;
}

int process_var(int bl, int var, char *start, char *end)
{
    char *p, origvar, *origvarp, origval, *v;

    //grub_printf("VAR(%d, %d) %s [%d]\n", bl, var, start, end-start+1);

    start = skip_ws(start);
    p = start;
    while (p < end && *p != '=' && !isspace(*p))
      p++;

    origvar = *p;
    origvarp = p;

    p = skip_ws(p);
    if (*p != '=')
      return 1;

    p = skip_ws(p+1);

    // value is now from p to end (both inclusive)

    origval = *(end + 1);
    *(end + 1) = 0;
    *origvarp = 0;

    /* See if we already have the same value in that toggle
     * (in case we're entering a menu multiple time we would
     *  allocate memory multiple time) */

    if (toggle_data[slot].block[bl].var[var].value &&
        var_get_index(start) != -1 &&
        !grub_strcmp(toggle_data[slot].block[bl].var[var].value, p))
      {
      }
    else
      {
        /* Allocate space for the value and hang it in */
        if ((v = var_alloc_mem(grub_strlen(p) + 1)) == NULL)
          return 1;
        grub_strcpy(v, p);

        var_set_no_copy(start, v);

        if ((toggle_data[slot].block[bl].var[var].var = var_get_index(start)) == -1)
          return 1; /* internal error */
        toggle_data[slot].block[bl].var[var].value = v;
      }

    *origvarp = origvar;
    *(end + 1) = origval;

    //grub_printf("slot=%d block=%d var=%d %d=%s\n", slot, bl, var, toggle_data[slot].block[bl].var[var].var, v);

    if (var >= toggle_data[slot].block[bl].nr_vars)
      toggle_data[slot].block[bl].nr_vars = var + 1;

    return 0;
}

int process_block(int bl, char *start, char *end)
{
    char *p = start;
    int var = 0;

    //grub_printf("BL(%d) %s [%d]\n", bl, start, end-start+1);

    while (p <= end)
      {
        if (*p == ',' || p == end)
          {
            if (var == MAX_VAR_PER_BLOCK)
              return ERR_WONT_FIT;

            if (process_var(bl, var, start, (p == end) ? p : (p-1)))
              return ERR_BAD_ARGUMENT;

            var++;
            start = p + 1;
          }
        p++;
      }

    return 0;
}

//static
int
toggle_func(char *arg, int flags)
{
  int  key, block = 0, i;
  char *a = arg, *eb;
  int command;
  enum {
    COMMAND_SET,
    COMMAND_SELECT,
    COMMAND_TRIGGER,
  };


  /* parse args */
  /* proceed to next arg */
  while (*a && *a == ' ')
    a++;

  /* Get command */
  {
    eb = a;

    while (!isspace(*eb))
      eb++;

    if (!isspace(*eb))
      goto bad_arg;

    //grub_printf("trigger: processing line: %s\n", a);

    *eb = 0;

    if (!grub_strcmp(a, "set"))
      command = COMMAND_SET;
    else if (!grub_strcmp(a, "select"))
      command = COMMAND_SELECT;
    else if (!grub_strcmp(a, "trigger"))
      command = COMMAND_TRIGGER;
    else
      {
        grub_printf("toggle: Unknown command!\n");
        goto bad_arg;
      }

    *eb = ' ';
    a = eb + 1;
  }

  a = skip_ws(a);

  if (command == COMMAND_SET)
    {

      if (!*a || !*(a+1))
        goto bad_arg;

      key = *a;
      a++;

      //grub_printf("key: %c\n", key);

      /* Find slot for key */
      slot = get_toggle_slot_for_key(key);
      if (slot == -1)
        {
          /* Get next free toggle_data slot */
          slot = toggles_used;
          if (slot == TOGGLES)
            goto wont_fit;
          toggle_data[slot].key = key;
          toggles_used++;
        }

      /* Reset current slot */
      toggle_data[slot].nr_blocks = toggle_data[slot].current_block = 0;
      for (i = 0; i < MAX_VAR_PER_BLOCK; i++)
        toggle_data[slot].block[i].nr_vars = 0;

      while (*a)
        {
          int ret;

          a = skip_ws(a);

          if (*a == 0)
            break;

          if (*a != '{')
            goto bad_arg;
          /* find the correspondig '}' */
          eb = a++;
          while (*eb && *eb != '}')
            eb++;
          if (*eb != '}')
            goto bad_arg;
          /* Now we have the block between a and eb-1 */
          ret = process_block(block, a, eb-1);
          if (ret == ERR_WONT_FIT)
            goto wont_fit;
          if (ret)
            goto bad_arg;

          a = eb + 1;

          block++;
          if (*a) {
            if (block == MAX_BLOCKS)
              goto wont_fit;
          }
        }
      toggle_data[slot].nr_blocks = block;

      /* finally, set all vars from the first block */
      toggle_do_block(slot, 0);
    }
  else if (command == COMMAND_SELECT)
    {
      while (*a)
        {
          int k, b, s;

          /* there's something between a and eb-1 now*/
          /* *a is a key and *(a+1) == '=' */
          k = *a++;

          if (*a++ != '=')
            goto bad_arg;

          if (!safe_parse_maxint(&a, (long *)&b))
            goto bad_arg;

          if ((s = toggle_find_slot(k)) == -1)
            goto bad_arg;

          if (!toggle_do_block(s, b))
            goto bad_arg;

          toggle_data[s].current_block = b;

          a = skip_ws(a);
        }
    }
  else if (command == COMMAND_TRIGGER)
    {
      char *vr, *vre, *vl, *vle, *vrval;
      char o;
      int cmp;
      // remaining a(rg) format: VAR==VAL "command"
      //grub_printf("%s: trigger\n");

      /* Find the '=' */
      vr = a;
      while (*a && *a != ' ' && *a != '=')
        a++;

      if (*a != '=' || *(a+1) != '=')
        goto bad_arg;

      vre = a;

      if (vr == vre)
        goto bad_arg;

      a += 2;
      vl = a;

      while (!isspace(*a))
        a++;

      vle = a;

      if (vl == vle)
        goto bad_arg;


      *vre = 0;
      vrval = var_get(vr);
      *vre = '=';

      if (!vrval)
        goto bad_arg;

      o = *vle;
      *vle = 0;
      cmp = grub_strcmp(vrval, vl);
      *vle = o;

      a = skip_ws(a);

      if (!cmp)
        toggle_func(a, 0);
    }

  return 0;

wont_fit:
  grub_printf("%s: ERR_WONT_FIT\n", __func__);
  //getkey();
  errnum = ERR_WONT_FIT;
  return 1;

bad_arg:
  grub_printf("%s: ERR_BAD_ARGUMENT\n", __func__);
  //getkey();
  errnum = ERR_BAD_ARGUMENT;
  return 1;
}

int detect_vmware(void)
{
#ifndef GRUB_UTIL
  char *start = (char *)0xc0000;
  int size = 16 << 10;
  int i = 0, p = 0;
  char *s = "VMware, Inc.";

  while (i < size)
  {
    if (*(start + i) == s[p])
    {
      p++;
      if (!s[p])
        return 1;
    }
    else
      p = 0;
    i++;
  }
#endif
  return 0;
}

void toggle_trigger_init(void)
{
  if (toggle_trigger_init_done)
    return;
  toggle_trigger_init_done = 1;

  var_set("TT_VMWARE", (detect_vmware()) ? "1" : "0", 0);
}

static struct builtin builtin_toggle =
{
  "toggle",
  toggle_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "toggle [command] args...",
  "Doc me."
};

static char var_expand_line_edit = 1;

char is_var_expand(void)
{
  return var_expand_line_edit;
}

//static
int
varexpand_func(char *arg, int flags)
{
  static const char *on_vals[]  = { "on",  "true",  "1" };
  static const char *off_vals[] = { "off", "false", "0" };
  int i;

  arg = skip_ws(arg);

  for (i = 0; i < sizeof(on_vals) / sizeof(on_vals[0]); i++)
    if (!grub_memcmp(arg, on_vals[i], sizeof(*on_vals[i])))
      {
        var_expand_line_edit = 1;
        goto out;
      }
  for (i = 0; i < sizeof(off_vals) / sizeof(off_vals[0]); i++)
    if (!grub_memcmp(arg, off_vals[i], sizeof(*off_vals[i])))
      {
        var_expand_line_edit = 0;
        goto out;
      }

  if (*arg)
    {
      grub_printf("Unknown argument: %s\n", arg);
      return 1;
    }

  var_expand_line_edit = !var_expand_line_edit;

out:
  grub_printf("Expansion is %s\n", var_expand_line_edit ? "on" : "off");
  return 0;
}

static struct builtin builtin_varexpand =
{
  "varexpand",
  varexpand_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "varexpand [on|off]",
  "Expand lines in line editing? Default is on. It toggles without arguments."
};

int
root_func(char *arg, int flags)
{
  unsigned int size;

  return u_open(arg, &size);
}

static struct builtin builtin_root =
{
  "root",
  root_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "root [DEVICE [HDBIAS]]",
  "Set the current \"root device\" to the device DEVICE, then"
  " attempt to mount it to get the partition size (for passing the"
  " partition descriptor in `ES:ESI', used by some chain-loaded"
  " bootloaders), the BSD drive-type (for booting BSD kernels using"
  " their native boot format), and correctly determine "
  " the PC partition where a BSD sub-partition is located. The"
  " optional HDBIAS parameter is a number to tell a BSD kernel"
  " how many BIOS drive numbers are on controllers before the current"
  " one. For example, if there is an IDE disk and a SCSI disk, and your"
  " FreeBSD root partition is on the SCSI disk, then use a `1' for HDBIAS."
};

int
boot_func(char *arg, int flags)
{

  switch (kernel_type)
  {
  case KERNEL_TYPE_NONE:
    return 1;
  case KERNEL_TYPE_OS2LDR:
    return_to_preldr();
  case KERNEL_TYPE_MULTIBOOT:
    ;
  default:
    ;
  }

  multi_boot();

  return 0;
}

static struct builtin builtin_boot =
{
  "boot",
  boot_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "boot",
  "Transfer control to the multiboot kernel."
};

int
default_func(char *arg, int flags)
{
  int n;
  safe_parse_maxint(&arg, (long *)&n);
  default_item = n;
  return 0;
}

static struct builtin builtin_default =
{
  "default",
  default_func,
  BUILTIN_HELP_LIST,
  "default <menuitem>",
  "Set default menuitem."
};

int
timeout_func(char *arg, int flags)
{
  safe_parse_maxint(&arg, (long *)&menu_timeout);
  return 0;
}

static struct builtin builtin_timeout =
{
  "timeout",
  timeout_func,
  BUILTIN_HELP_LIST,
  "timeout <interval>",
  "Set menu timeout."
};

int
color_func(char *arg, int flags)
{
  int color;

  safe_parse_maxint(&arg, (long *)&color);
  background_color    = color & 0xff;
  foreground_color    = (color >> 8)  & 0xff;
  background_color_hl = (color >> 16) & 0xff;
  foreground_color_hl = (color >> 24) & 0xff;
  return 0;
}

static struct builtin builtin_color =
{
  "color",
  color_func,
  BUILTIN_HELP_LIST,
  "color <color>",
  "Set menu colors. 4-byte integer. Lowest byte is menu"
  "background; second byte is menu foreground. 3rd byte"
  "is a highlighted background color. 4th byte is a"
  "highlighted foreground color."
};

int
width_func(char *arg, int flags)
{
  safe_parse_maxint(&arg, (long *)&menu_width);
  return 0;
}

static struct builtin builtin_width =
{
  "width",
  width_func,
  BUILTIN_HELP_LIST,
  "width <menu width>",
  "Set menu width."
};

int
height_func(char *arg, int flags)
{
  safe_parse_maxint(&arg, (long *)&menu_height);
  return 0;
}

static struct builtin builtin_height =
{
  "height",
  height_func,
  BUILTIN_HELP_LIST,
  "height <menu height>",
  "Set menu heigt."
};

int
screen_func(char *arg, int flags)
{
  int screen;

  safe_parse_maxint(&arg, (long *)&screen);
  screen_bg_color    = screen & 0xff;
  screen_fg_color    = (screen >> 8)  & 0xff;
  screen_bg_color_hl = (screen >> 16) & 0xff;
  screen_fg_color_hl = (screen >> 24) & 0xff;
  return 0;
}

static struct builtin builtin_screen =
{
  "screen",
  screen_func,
  BUILTIN_HELP_LIST,
  "screen <screen colors>",
  "Set screen colors. 4-byte integer. Lowest byte is screen"
  "background; second byte is screen foreground. 3rd byte"
  "is a highlighted background color. 4th byte is a"
  "highlighted foreground color."
};

int
os2ldr_func(char *arg, int flags)
{
  char *s, *se, *r;
  char bf[0x200];
  int  n;

  kernel_type = KERNEL_TYPE_OS2LDR;
  printf("Loading OS/2\r\n");

  if (s = strstr(arg, "--prefix"))
  {
    s  = skip_to(1, s); // pointer to beginning of the prefix
    se = skip_to(1, s); // pointer to the end of the prefix
    r = se - 1;
    if (*r == ' ') se--;
    grub_strncpy(bf, s, se - s);
    n = se - s;
    bf[n] = '\0';
    u_parm(PARM_PREFIX, ACT_SET, (unsigned int *)bf);
  }

  if (s = strstr(arg, "--ldrlen"))
  {
    s = skip_to(1, s);
    safe_parse_maxint(&s, (long *)&n);
    u_parm(PARM_LDRLEN, ACT_SET, (unsigned int *)&n);
  }

  return 0;
}

static struct builtin builtin_os2ldr =
{
  "os2ldr",
  os2ldr_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "os2ldr",
  "Return control to os2ldr"
};

int
dlat_func(char *arg, int flags)
{
  return get_dlat_info(arg);
}

static struct builtin builtin_dlat =
{
  "dlat",
  dlat_func,
  BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "dlat <partition>",
  "Get/set LVM DLAT info"
};

/* serial */
int
serial_func (char *arg, int flags)
{
  t = u_termctl(2);

  return t->startup(arg);
}

static struct builtin builtin_serial =
{
  "serial",
  serial_func,
  BUILTIN_MENU | BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "serial [--unit=UNIT] [--port=PORT] [--speed=SPEED] [--word=WORD] [--parity=PARITY] [--stop=STOP] [--device=DEV]",
  "Initialize a serial device. UNIT is a digit that specifies which serial"
  " device is used (e.g. 0 == COM1). If you need to specify the port number,"
  " set it by --port. SPEED is the DTE-DTE speed. WORD is the word length,"
  " PARITY is the type of parity, which is one of `no', `odd' and `even'."
  " STOP is the length of stop bit(s). The option --device can be used only"
  " in the grub shell, which specifies the file name of a tty device. The"
  " default values are COM1, 9600, 8N1."
};

/* write */
int
write_func (char *arg, int flags)
{
  int  addr;
  unsigned long val;
  char *str, *s, *p;

  safe_parse_maxint(&arg, (long *)&addr);
  p = (char *)addr;
  str = skip_to(0, arg);

  if (*str == '"') /* it's a string */
  {
    /* next '"' occurence position */
    if (s = strstr(str + 1, "\""))
    {
      memmove(p, str + 1, s - str - 1);
      p[s - str - 1] = '\0';
      return 0;
    }
  }
  else
  {
    if (safe_parse_maxint(&str, (long *)&val))
    {
      *((unsigned long *)p) = val;
      return 0;
    }
  }

  return 1;
}

static struct builtin builtin_write =
{
  "write",
  write_func,
  BUILTIN_MENU | BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "write <addr> <string>",
  "write a string or hex number at specified phys address."
};

int
configfile_func (char *arg, int flags)
{
  char buf[0x100];
  int  rc;

  if (!*arg) *arg = 'q'; // something != NULL

  strcpy(prev_cfg, curr_cfg);
  strcpy(buf, arg);
  strcpy(curr_cfg, buf);

  return 0;
}

static struct builtin builtin_configfile =
{
  "configfile",
  configfile_func,
  BUILTIN_MENU | BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "configfile <config file>",
  "Load and execute a config file."
};

int
print_func(char *arg, int flags)
{
  printf("%s\r\n", var_sprint_buf(arg, &flags));
  return 0;
}

static struct builtin builtin_print =
{
  "print",
  print_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "print \"string with vars\"",
  "Print a string which may contain variables. Variables are enclosed"
  " in $( and ) (like \"make\")."
};

static struct builtin builtin_echo =
{
  "echo",
  print_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "echo \"string with vars\"",
  "Alias for \"print\"."
};

int
prompt_func(char *arg, int flags)
{
  printf("%s\r\n", var_sprint_buf(arg, &flags));
  printf("Press any key when ready...\r\n");
  t->getkey();
  return 0;
}

static struct builtin builtin_prompt =
{
  "prompt",
  prompt_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "Output a message and wait for a key",
  "Output a message and wait for a key"
};

int
help_func(char *arg, int flags)
{
  struct builtin **b;
  char flag = 0;
  char str[0x100];
  char buf[0x800];
  char *p;
  int  x = 0, y = 0;

  if (!*arg)
  {
    *buf = '\0';
    for (b = builtins; *b; b++)
    {
      grub_sprintf(str, "%s\r\n", (*b)->short_doc);
      grub_strcat(buf, buf, str);
    }
    x = 0; y = 0;
    t->cls();
    p = buf;
    while (*p && ((p - buf) < (strlen(buf) / 2)))
    {
      while (*p && ((p - buf) < (strlen(buf) / 2)) && x < SCREEN_WIDTH / 2 - 1)
      {
        t->putchar(*p++); x++;
      }
      x = 0; y++;
      t->gotoxy(x, y);
    }
/*    x = SCREEN_WIDTH / 2 + 1; y = 0;
    for (; *p; p++)
    {
      t->putchar(*p); x++;
      if ((p - buf) % (SCREEN_WIDTH / 2 - 1) == 0)
      {
        x = SCREEN_WIDTH / 2 + 1; y++;
        t->gotoxy(x, y);
      }
    }   */
  }
  else
  {
    for (b = builtins; *b; b++)
      if (abbrev(arg, (*b)->name, strlen((*b)->name)))
      {
        flag = 1;
        break;
      }
    if (flag)
    {
      printf("\r\n%s\r\n", arg);
      printf("%s\r\n", (*b)->long_doc);
    }
    else
      printf("No such command!\r\n");
  }

  return 0;
}

static struct builtin builtin_help =
{
  "help",
  help_func,
  BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
  "Get help",
  "Outputs a long help screen for a command, if a name is"
  "specified. Otherwise, it outputs a short description"
  "for each command"
};

struct builtin *builtins[] = {
  &builtin_kernel,
  &builtin_module,
  &builtin_modaddr,
  &builtin_lipmodule,
  &builtin_bsmodule,
  &builtin_vbeset,
  &builtin_set,
  &builtin_toggle,
  &builtin_varexpand,
  &builtin_root,
  &builtin_boot,
  &builtin_default,
  &builtin_timeout,
  &builtin_color,
  &builtin_width,
  &builtin_height,
  &builtin_screen,
  &builtin_os2ldr,
  &builtin_dlat,
  &builtin_serial,
  &builtin_write,
  &builtin_configfile,
  &builtin_print,
  &builtin_echo,
  &builtin_prompt,
  &builtin_help,
  0
};
