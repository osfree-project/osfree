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

int num_items = 0;
int scrollnum = 0;

void create_lip_module(lip2_t **l);
void multi_boot(void);

char *
skip_to (int after_equal, char *cmdline);

int (*process_cfg_line)(char *line);

#pragma aux multi_boot     "*"
#pragma aux m              "*"
#pragma aux l              "*"

#pragma aux entry_addr "*"

extern entry_func entry_addr;

int root_func (char *arg, int flags);
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

/* menu colors */
int background_color = 0; // black
int foreground_color = 7; // white
int background_color_hl = 0;
int foreground_color_hl = 7;


int screen_bg_color = 0;
int screen_fg_color = 7;
int screen_bg_color_hl = 0;
int screen_fg_color_hl = 7;


/* menu width and height */
#define MENU_WIDTH  56
#define MENU_HEIGHT 10
int menu_width  = MENU_WIDTH;
int menu_height = MENU_HEIGHT;

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
  else if (!section && abbrev(line, "menubg", 6))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &background_color);
  }
  else if (!section && abbrev(line, "menufg", 6))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &foreground_color);
  }
  else if (!section && abbrev(line, "menubghl", 8))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &background_color_hl);
  }
  else if (!section && abbrev(line, "menufghl", 8))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &foreground_color_hl);
  }
  else if (!section && abbrev(line, "width", 5))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &menu_width);
  }
  else if (!section && abbrev(line, "height", 6))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &menu_height);
  }
  else if (!section && abbrev(line, "screenbg", 8))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &screen_bg_color);
  }
  else if (!section && abbrev(line, "screenfg", 8))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &screen_fg_color);
  }
  else if (!section && abbrev(line, "screenbghl", 10))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &screen_bg_color_hl);
  }
  else if (!section && abbrev(line, "screenfghl", 10))
  {
    line = skip_to(1, line);
    safe_parse_maxint(&line, &screen_fg_color_hl);
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
      printf("An error occured during execution of varexpand_func\r\n");
      return 0;
    }
  }
  else if (abbrev(line, "title", 5))
  {
    section++;
    num_items++;
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

  if (!*line) return 1;

  // change '\\' into '/'
  p = line;
  while (*p++)
    if (*p == '\\') *p = '/';

  if (abbrev(s, "modaddr", 7))
  {
    s = skip_to(1, s);
    if (modaddr_func(s, 0x2))
    {
      printf("An error occured during execution of modaddr_func\r\n");
      return 0;
    }
  }
/*  else if (abbrev(s, "root", 4))
  {
    s = skip_to(1, s);
    if (root_func(s, 0x2))
    {
      printf("An error occured during execution of kernel_func\r\n");
      return 0;
    }
  } */
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

// get next menu item from user input
int
get_user_input(int *item, int *shift)
{
  int c;

  // scan code
  for (;;)
  {
    c = t->getkey();
    //printf("0x%x ", c);
    switch (c)
    {
      case 0xe:   // down arrow
      {
        ++*item;
        if (*item == num_items + 1) *item = 0;
        return 1;

       /* if (t->checkkey() == -1)
          return 1;
        else
          continue; */
      }
      case 0x10:  // up arrow
      {
        --*item;
        if (*item == -1) *item = num_items;
        return 1;

       /* if (t->checkkey() == -1)
          return 1;
        else
          continue; */
      }
      case 0x6:   // right arrow
      {
        if (*shift >= -menu_width) --*shift;
        return 1;
      }
      case 0x2:   // left arrow
      {
        if (*shift < 0) ++*shift;
        return 1;
      }
      case 0x1c0d: // enter
      {
        ++*item;
        return 0;
      }
      default:
        ;
    }
  }

  *shift = 0;

  return 1;
}

void show_background_screen(void)
{
  int i;
  char *s1 = "--== FreeLdr ver. 0.0.2. ==--";
  char *s2 = "(c) osFree project, 2008 Jun 23. licensed under GNU GPL v.2";
  int  l, n;

  t->setcolor((char)screen_fg_color    | ((char)screen_bg_color << 4),
              (char)screen_fg_color_hl | ((char)screen_bg_color_hl << 4));
  t->cls();

  t->setcolorstate(COLOR_STATE_NORMAL);

  /* draw the frame */

  t->gotoxy(1, 0);
  t->putchar(0xda);
  for (i = 0; i < 80 - 4; i++) t->putchar(0xc4);
  t->putchar(0xbf);


  for (i = 0; i < 25 - 2; i++)
  {
    t->gotoxy(1,  1 + i); t->putchar(0xb3);
    t->gotoxy(78, 1 + i); t->putchar(0xb3);
  }

  t->gotoxy(1, 24);
  t->putchar(0xc0);

  for (i = 0; i < 80 - 4; i++) t->putchar(0xc4);

  t->gotoxy(78, 24);
  t->putchar(0xd9);

  t->setcolor(0x75, 7);

  /* header line */
  t->gotoxy(4, 0);
  l = grub_strlen(s1);
  n = (80 - 8 - l) / 2;
  for (i = 0; i < n; i++) t->putchar(' ');
  for (i = 0; i < l; i++) t->putchar(s1[i]);
  for (i = n + l; i < 80 - 8; i++) t->putchar(' ');

  /* footer line */
  t->gotoxy(4, 24);
  for(i = 0; i < 80 - 8; i++) t->putchar(' ');

  /* copyright */
  t->gotoxy(5, 24);
  printf("%s", s2);

  t->setcolor((char)foreground_color    | ((char)background_color << 4),
              (char)foreground_color_hl | ((char)background_color_hl << 4));
}

void
invert_colors(int *col1, int *col2)
{
  int col;

  col = *col1;
  *col1 = *col2;
  *col2 = col;

  t->setcolor((char)foreground_color | ((char)background_color << 4),
              (char)foreground_color_hl | ((char)background_color_hl << 4));
}

// draw a menu with selected item shifted
// by 'shift' symbols to right/left
void draw_menu(int item, int shift)
{
  int i = 0, j, l, k, m;
  script_t *sc;
  char s[4];
  char str[4];
  char spc[0x80];
  char buf[0x100];
  char *p;

  //t->setcolorstate(COLOR_STATE_NORMAL);

  // background
  //t->setcolor(0 | (3 << 4), 7 | (3 << 4));
  // clear screen
  //t->cls();
  // 5 - normal (pink), 3 - highlighted (magenta)
  t->setcolor((char)foreground_color    | ((char)background_color << 4),
              (char)foreground_color_hl | ((char)background_color_hl << 4));

  t->gotoxy(12, 5);
  l = 0;
  buf[l++] = 0xda;
  while (l < menu_width) buf[l++] = 0xc4;
  buf[l++] = 0xbf; buf[l] = '\0';
  printf("%s", buf);

  grub_memset(buf, 0, sizeof(buf));

  if (item + 2 > menu_height) scrollnum = item + 1 - menu_height;
  if (item == num_items + 1)  scrollnum = 0;
  if (!item) scrollnum = 0;

  sc = menu_first;

  if (shift > 0)
  {
    for (i = 0; i < shift; i++) spc[i] = ' ';
    spc[i] = '\0';
    m = 0;
  }
  else
  {
    spc[0] = '\0';
    m = -shift;
  }

  for (k = 0; k < scrollnum; k++) sc = sc->next;

  for (i = 0; i < menu_height; i++)
  {
    j = scrollnum + i;

    t->gotoxy(12, 6 + i);

    printf("%c ", 0xb3);

    // show highlighted menu string in inverse color
    if (j == item) invert_colors(&foreground_color, &background_color);

    sprintf(s, "%d", j);
    l = grub_strlen(s);
    if (l == 1) grub_strcat(str, " ", s);
    if (l == 2) grub_strcpy(str, s);
    sprintf(buf, "%s%s. %s", spc, str, sc->title);

    p = buf + m;
    l = grub_strlen(p);

    while (l > menu_width - 3) p[l--] = '\0';
    while (l < menu_width - 3) p[l++] = ' ';
    p[l] = '\0';

    printf("%s", p);

    grub_memset(buf, 0, sizeof(buf));

    // show highlighted menu string in inverse color
    if (j == item) invert_colors(&foreground_color, &background_color);

    printf(" %c", 0xb3);

    sc = sc->next;
    //i++;
  }

  t->gotoxy(12, 6 + i);
  l = 0;
  buf[l++] = 0xc0;
  while (l < menu_width) buf[l++] = 0xc4;
  buf[l++] = 0xd9; buf[l] = '\0';
  printf("%s", buf);

  t->setcursor(0);

  t->setcolor(7, 7);
}

int
exec_menu(void)
{
  //int cont = 1;  // continuation flag
  int item;      // selected menu item
  int shift = 0; // horiz. scrolling menu shift

  item = default_item;

  do {
    draw_menu(item, shift);
  }   while (get_user_input(&item, &shift));

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

  // starting point 0 instead of 1
  num_items--;

  // show screen header, border and status line
  show_background_screen();

  // show a menu and let the user choose a menu item
  item = exec_menu();

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

  return 1;
}

void
KernelLoader(void)
{
  char *cfg = "/boot/loader/boot.cfg";
  int rc;

  printf("Kernel loader started.\r\n");

  // exec the config file
  if (!(rc = exec_cfg(cfg)))
  {
    printf("Error processing loader config file!\r\n");
  }
  else if (rc == -1)
  {
    panic("Load error!", "");
  }
  else
  {
    // launch a multiboot kernel

    //printf("entry_addr=0x%x", entry_addr);
    //printf("&mbi=0x%x", m);

    //__asm {
    //  cli
    //  hlt
    //}

    multi_boot();
  }
}

void
cmain(void)
{
  /* Get mbi structure address from pre-loader */
  u_parm(PARM_MBI, ACT_GET, (unsigned int *)&m);
  // init terminal
  t = u_termctl(-1);

  //printf("!!!\r\n");
  //__asm {
  //  cli
  //  hlt
  //}

  KernelLoader();
}
