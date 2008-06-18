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
#include "term.h"

extern lip2_t *l;
extern bios_parameters_block *bpb;
extern FileTable ft;

struct multiboot_info *m;
struct term_entry *t;

void create_lip_module(lip2_t **l);
void multi_boot(void);

char *
skip_to (int after_equal, char *cmdline);

int (*process_cfg_line)(char *line);

//#pragma aux skip_to "*"

#pragma aux multi_boot     "*"
//#pragma aux kernel_func    "*"
//#pragma aux module_func    "*"
//#pragma aux modaddr_func   "*"
//#pragma aux lipmodule_func "*"
#pragma aux m              "*"
#pragma aux l              "*"

int kernel_func (char *arg, int flags);
int module_func (char *arg, int flags);
int modaddr_func (char *arg, int flags);
int lipmodule_func (char *arg, int flags);
int vbeset_func (char *arg, int flags);
int set_func (char *arg, int flags);
int varexpand_func (char *arg, int flags);

void panic(char *msg, char *file);
int  abbrev(char *s1, char *s2, int n);
char *strip(char *s);
char *trim(char *s);
char *wordend(char *s);

char *menu_items = (char *)MENU_BUF;
int  menu_cnt, menu_len = 0;
char *config_lines;
int  config_len = 0;
int  default_item = -1;
int  menu_timeout;

typedef struct script script_t;
// a structure corresponding to a 
// boot script or menu item
typedef struct script
{
  char *scr;      // a pointer to a script itself
  int  num;       // number of commands in boot script
  char *title;    // menu title
  script_t *next; // next item
  script_t *prev; // previous item
} script_t;

script_t *menu_first, *menu_last;

int process_cfg(char *cfg);

void init(lip2_t *l)
{

}

int 
process_cfg_line1(char *line)
{
  int n;
  int i;
  char *s, *p;
  char *title; // current menu item title
  static int section = 0;
  static script_t *sc = 0, *sc_prev = 0;

  if (!*line) return 1;

  if (!section && abbrev(line, "default", 7))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &n);
    default_item = n;
  }
  else if (!section && abbrev(line, "timeout", 7))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &menu_timeout);
  }
  else if (!section && abbrev(line, "set", 3))
  {
    line = strip(skip_to(0, line));
    if (set_func(line, 0x2))
    {
      printf("An error occured during execution of set_func\r\n");
      return 0;
    }
  }
  else if (!section && abbrev(line, "varexpand", 9))
  {
    line = strip(skip_to(0, line));
    if (varexpand_func(line, 0x2))
    {
      printf("An error occured during execution of set_func\r\n");
      return 0;
    }
  }
  else if (abbrev(line, "title", 5))
  {
    section++;
    s =  skip_to(1, line);
    p = menu_items;

    title = p + menu_len;
    while (p[menu_len++] = *s++) ;

    // add a script_t structure after previous menu item name
    sc = (script_t *)((char *)p + menu_len);
    sc->num = 0;
    sc->title = title;
    sc->prev = sc_prev;
    if (sc_prev) sc_prev->next = sc;
    sc->next = 0;
    menu_len += sizeof(script_t);
    if (section == 1) menu_first = sc;
    sc_prev = sc;
    menu_last = sc;
  }
  else if (section)
  {
    s = line;
    p = config_lines;
    // if this is a 1st command
    if (!sc->num) sc->scr = p + config_len;
    sc->num++;
    // copy a line to buffer
    while (p[config_len++] = *s++) ;    
  }
  else
  {
  }  

  //for (i = 0; i < 0xFFFFFF; i++) ;

  return 1;
}


int 
exec_line(char *line)
{
  int i;
  char *p, *s = line;
  //char *var, *val;
  //static int section = 0;
  //static int insection = 0;
  //static int sec_to_load;

  if (!*line) return 1;

  // change '\\' into '/'
  p = line;
  while (*p++)
    if (*p == '\\') *p = '/';

  //printf("%s\r\n", s);
  //t->getkey();

  if (abbrev(s, "modaddr", 7)) 
  {
    s = skip_to(1, s);
    if (modaddr_func(s, 0x2))
    {
      printf("An error occured during execution of modaddr_func\r\n");
      return 0;
    }
  }
  else if (abbrev(s, "kernel", 6))
  {
    s = skip_to(1, s);
    if (kernel_func(s, 0x2)) 
    {
      printf("An error occured during execution of kernel_func\r\n");
      return 0;
    }
  }
  else if (abbrev(s, "module", 6))
  {
    s = skip_to(1, s);
    if (abbrev(s, "--type", 6))
    {
      s = skip_to(1, s);
      if (abbrev(s, "lip", 3))
      {
        if (lipmodule_func(s, 0x2))
        {
          printf("An error occured during execution of lipmod_func\r\n");
          return 0;
        }
      }
      else if (abbrev(s, "bootsector", 10))
      {
        //if (bootsecmodule_func(s, 0x2))
        //{
        //  printf("An error occured during execution of lipmod_func\r\n");
        //  return 0;
        //}
      }
    }
    else
    if (module_func(s, 0x2))
    {
      printf("An error occured during execution of module_func\r\n");
      return 0;
    }
  }
  else if (abbrev(s, "vbeset", 6))
  {
    s = skip_to(1, s);
    if (vbeset_func(s, 0x2)) 
    {
      printf("An error occured during execution of vbeset_func\r\n");
      return 0;
    }
  }
  else
  {
  }

  return 1;
}


// get one digit from user
int
get_digit(void)
{
  int c = 0;

  while (c < '0' || c > '9')
  {
    c = (t->getkey() & 0xff);
  }
  
  c = c - '0';

  return c;
}

// get next menu item from user input
int
get_user_input(int *item)
{
  *item = 10 * get_digit() + get_digit();

  return 0;
}

// draw a menu with selected item
void draw_menu(int item)
{
  int i = 0, l;
  script_t *sc;
  char s[4];
  char str[4]; 
  char buf[0x100];
  #define MENU_WIDTH 56

  t->setcolorstate(COLOR_STATE_NORMAL);

  // background
  t->setcolor(0 | (3 << 4), 7 | (3 << 4));

  t->cls();          // clear screen
  t->setcolor(0 | (5 << 4), 7 | (3 << 4)); // 5 - normal (pink), 3 - highlighted (magenta)

  t->gotoxy(12, 5);
  l = 0;
  buf[l++] = 0xda;
  while (l < MENU_WIDTH) buf[l++] = 0xc4;
  buf[l++] = 0xbf; buf[l] = '\0';
  printf("%s", buf);

  sc = menu_first;
  while (sc)
  {
    t->gotoxy(12, 6 + i);

    sprintf(s, "%d", i + 1);
    l = grub_strlen(s);
    if (l == 1) grub_strcat(str, "0", s);
    if (l == 2) grub_strcpy(str, s);
    sprintf(buf, "%c %s. %s", 0xb3, str, sc->title);
    l = grub_strlen(buf);

    while (l < MENU_WIDTH) buf[l++] = ' ';
    buf[l++] = 0xb3; buf[l] = '\0';
    printf("%s", buf);

    sc = sc->next;
    i++;
  }

  t->gotoxy(12, 6 + i);
  l = 0;
  buf[l++] = 0xc0;
  while (l < MENU_WIDTH) buf[l++] = 0xc4;
  buf[l++] = 0xd9; buf[l] = '\0';
  printf("%s", buf);

  t->setcolor(7, 7);
}

int 
exec_menu(void)
{
  int cont = 1;
  int item = 0;

  while (cont)
  {
    draw_menu(item);
    cont = get_user_input(&item);
  }

  return item;
}

int
exec_script(char *script, int n)
{
  char *line = script;
  int  i;

  for (i = 0; i < n; i++)
  {
    if (!exec_line(line)) return 0;
    // next line
    while (*line++) ;
  }

  return 1;
}

int
exec_cfg(char *cfg)
{
  int rc;
  int item = -1; // menu item number
  char *line, *p;
  script_t *sc;

  config_lines = (char *)m->drives_addr + m->drives_length; // (char *)(0x100000);

  // exec global commands in config file
  // and copy config file to memory as
  // a string table (strings delimited by zeroes)
  // and make script_t structures list for 
  // boot scripts and menu items
  menu_len = 0; config_len = 0;  
  process_cfg_line = process_cfg_line1;
  rc = process_cfg(cfg);

/*
  line = config_lines; // config in memory
  do
  {

    printf("%s\r\n", line);
    
    while (*line++) ; // next line

  } while (line - config_lines < config_len);
*/
/*
  sc = menu_first;
  item = 0;
  while (sc)
  {
    if (item == 7)
    {
      printf("title: %s\r\n", sc->title);
      printf("num lines: %u\r\n", sc->num);

      p = sc->scr;
      for (rc = 0; rc < sc->num; rc++)
      {
        printf("%s\r\n", p);
        while (*p++) ;
      }  
    }
    sc = sc->next;
    item++;
  }
*/
/*
  sc = menu_last;
  while (sc)
  {
    printf("%s\r\n", sc->title);
    sc = sc->prev;
  }
*/

  // show a menu and let the user choose a menu item
  item = exec_menu();
  //printf("\r\n\r\n%d\r\n", item);

  sc = menu_first;
  t->cls();

  // find a menu item and execute corresponding script
  while (item)
  {
    item--;
    if (!item) rc = exec_script(sc->scr, sc->num);
    if (!rc) return 0;
    sc = sc->next;
  }

  //__asm {
  //  cli
  //  hlt
  //}

  return 1;
}

void 
KernelLoader(void)
{
  char *cfg = "/boot/freeldr/freeldr.cfg";
  int rc;

  printf("Kernel loader started.\r\n");

  if (!(rc = exec_cfg(cfg)))
  {
    printf("Error parsing loader config file!\r\n");
  }
  else if (rc == -1)
  {
    panic("Load error!", "");
  }
  else
  {
    //create_lip_module(&l);
    multi_boot();
  }
}

void cmain(void)
{
  /* Get mbi structure address from pre-loader */
  u_parm(PARM_MBI, ACT_GET, (unsigned int *)&m);
  // init terminal
  t = u_termctl(-1);

  KernelLoader();
}
