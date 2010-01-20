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

extern kernel_t kernel_type;
extern lip2_t *l;
extern bios_parameters_block *bpb;
extern FileTable ft;

extern struct variable_list_struct {
  char *name;
  char *value;
} variable_list[VARIABLES_MAX];

extern struct builtin *builtins[];

extern int menu_timeout;

struct multiboot_info *m;
struct term_entry *t;

unsigned char at_drive[16];

/* string table */
#define BUFLEN 0x800
char strtab[BUFLEN];
char *strpos = strtab;

static char state = 0;

int  default_item = -1;

/* menu colors */
int background_color = 0; // black
int foreground_color = 7; // white
int background_color_hl = 0;
int foreground_color_hl = 7;


int screen_bg_color = 0;
int screen_fg_color = 7;
int screen_bg_color_hl = 0;
int screen_fg_color_hl = 7;

int num_items = 0;
static int scrollnum = 0;

int exec_line(char *line);

void show_background_screen(void);
void draw_menu(int item, int shift);

void create_lip_module(lip2_t **l);
void multi_boot(void);

char *
skip_to (int after_equal, char *cmdline);

int (*process_cfg_line)(char *line);

//#pragma aux multi_boot     "*"
#pragma aux m              "*"
#pragma aux l              "*"

#pragma aux entry_addr "*"

extern entry_func entry_addr;

int width_func (char *arg, int flags);
int height_func (char *arg, int flags);
int boot_func (char *arg, int flags);
int exec_cfg(char *cfg, int menu_item, int menu_shift);

void panic(char *msg, char *file);
int  abbrev(char *s1, char *s2, int n);
char *strip(char *s);
char *trim(char *s);
char *wordend(char *s);

int item_save  = 0;
int shift_save = 0;

char *menu_items = (char *)MENU_BUF;
int  menu_cnt, menu_len = 0;
char *config_lines;
int  config_len = 0;

typedef struct script script_t;
typedef struct string string_t;
// a structure corresponding to a
// boot script or menu item
typedef struct script
{
  string_t *scr;  // a pointer to a script itself
  int  num;       // number of commands in boot script
  char *title;    // menu title
  script_t *next; // next item
  script_t *prev; // previous item
} script_t;

/* a string list structure */
typedef struct string
{
  char *s;        // a null-terminated string
  char last;      // whether it is the last string in series of strings
  string_t *next; // next list item
  string_t *prev; // previous list item
} string_t;

script_t *menu_first, *menu_last;

static int  section = 0;
static script_t *sc = 0, *sc_prev = 0;
static string_t *st = 0, *st_prev = 0;

char prev_cfg[0x100];
char curr_cfg[0x100];
int  menu_nest_lvl = 0;

char cmdbuf[0x200];
char path[] = "freeldr";
static int promptlen;

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

extern int menu_width;
extern int menu_height;

int process_cfg(char *cfg);

void init(lip2_t *l)
{

}

char *
macro_subst(char *path)
{
  char *p, *q, *r;
  char *macro = "()";
  char *s = strpos;
  int  l = strlen((char *)at_drive);
  int  m = strlen(macro);
  int  k;

  p = q = r = path - 1;
  while (1)
  {
    r = strstr(q + 1, macro);
    if (r) p = r; else p = q + 1 + strlen(q + 1);
    k = p - q - 1;

    grub_strncpy(s, q + 1, k);

    if (r) grub_strcat(s, s, (char *)at_drive);
    else break;

    s = s + l + k;
    q = p + m - 1;
  }
  s = strpos;
  strpos = s + strlen(s);

  return s;
}

int
process_cfg_line1(char *line)
{
  int    n;
  int    i, rc;
  char   *s, *p, *q;
  char   *title; // current menu item title
  struct builtin **b;

  if (!*line) return 1;

  if (abbrev(line, "title", 5))
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
    q = menu_items;
    st = (string_t *)((char *)q + menu_len);
    st->prev = st_prev;
    if (st_prev) st_prev->next = st;
    st->next = 0;
    menu_len += sizeof(string_t);
    // if this is a 1st command
    if (!sc->num) sc->scr = st;
    st_prev = st;
    st->s = p + config_len;
    st->last = 1;
    sc->num++;
    // copy a line to buffer
    while (p[config_len++] = *s++) ;
  }
  else
  {
    rc = exec_line(line);
    if (!rc)
      printf("Error occured during execution of %s\r\n", (*b)->name);

    return rc;
  }

  return 1;
}


int
exec_line(char *line)
{
  int i, rc;
  char *p, *s = line;
  struct builtin **b;

  if (!*line) return 1;

  // change '\\' into '/'
  p = line;
  while (*p++)
    if (*p == '\\') *p = '/';

  for (b = builtins; *b; b++)
  {
    if (abbrev(line, (*b)->name, strlen((*b)->name)))
    {
      s = skip_to(1, line);
      /* substitute macros, like '()' for a bootdrive */
      s = macro_subst(s);
      strpos = s;
      if (((*b)->func)(s, 0x2)) return 0;
      break;
    }
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
      }
      case 0x10:  // up arrow
      {
        --*item;
        if (*item == -1) *item = num_items;
        return 1;
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
      case 0x3: // pgdn
      {
        *item += menu_height - 1;
        if (*item >= num_items + 1) *item = 0;
        return 1;
      }
      case 0x7: // pgup
      {
        *item -= menu_height - 1;
        if (*item < 0) *item = num_items;
        return 1;
      }
      case 0x7400: // ctrl-right
      case 0x5:    // end
      {
        *shift += 10;
        if (*shift >= 2 * menu_width) *shift = menu_width;
        return 1;
      }
      case 0x7300: // ctrl-left
      case 0x1:    // home
      {
        *shift -= 10;
        if (*shift < 0) *shift = 0;
        return 1;
      }
      case 0x3e00: // F4
      case 0x1245: // E
      case 0x1265: // e
      {
        if (state == 0)
        {
          t->cls();
          state = 2;
        }
        return 0;
      }
      case 0x11b:  // esc
      {
        int ii;
        if (state == 0)
        {
          state++;
          t->cls();
        }
        return 0;
      }
      case 0x1c0d: // enter
      {
        menu_nest_lvl++;
        ++*item;

        return 0;
      }
      case 0xe08:  // backspace
      {
        /* return to the previous config */
        if (menu_nest_lvl > 0)
        {
          menu_nest_lvl--;
          state = 0;
          strcpy(curr_cfg, prev_cfg);
          memset(prev_cfg, 0, sizeof(prev_cfg));

          return -1;
        }
        else
          return 1;
      }
      default:
       {
         t->gotoxy(SCREEN_WIDTH - 7, SCREEN_HEIGHT - 2);
         t->setcolor((char)screen_fg_color    | ((char)screen_bg_color << 4),
                     (char)screen_fg_color_hl | ((char)screen_bg_color_hl << 4));
         printf("0x%x\r\n", c);
         return 1;
       }
    }
  }

  *shift = 0;

  return 1;
}

void show_background_screen(void)
{
  int i, j;
  char *s1 = "±±±±±±±±±±±±±± FreeLdr v.0.0.4, (c) osFree project, 2009 Oct 22 ±±±±±±±±±±±±±±";
  int  l, n;

  if (t->setcolor)
    t->setcolor((char)screen_fg_color    | ((char)screen_bg_color << 4),
                (char)screen_fg_color_hl | ((char)screen_bg_color_hl << 4));
  //t->cls();

  t->setcolorstate(COLOR_STATE_NORMAL);

  /* draw the frame */

  t->gotoxy(0, 0); t->putchar('É');
  for (i = 0; i < SCREEN_WIDTH - 2; i++) t->putchar('Í');
  t->gotoxy(SCREEN_WIDTH - 1, 0); t->putchar('»');

  for (i = 0; i < SCREEN_HEIGHT - 2; i++)
  {
    t->gotoxy(0,  1 + i); t->putchar('º');
    t->gotoxy(SCREEN_WIDTH - 1, 1 + i); t->putchar('º');
  }

  t->gotoxy(0, SCREEN_HEIGHT - 1); t->putchar('È');

  for (i = 0; i < SCREEN_WIDTH - 2; i++) t->putchar('Í');

  t->gotoxy(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1); t->putchar('¼');

  /* header line */
  t->gotoxy(1, 1);
  l = grub_strlen(s1);
  n = (SCREEN_WIDTH - 2 - l) / 2;
  for (i = 0; i < n; i++) t->putchar(' ');
  for (i = 0; i < l; i++) t->putchar(s1[i]);
  for (i = n + l; i < SCREEN_WIDTH - 2; i++) t->putchar(' ');

  /* the underline */
  t->gotoxy(0, 2);
  t->putchar('Ç');
  for (i = 0; i < SCREEN_WIDTH - 2; i++) t->putchar('Ä');
  t->putchar('¶');

  /* background pattern */
  for (i = 0; i < SCREEN_HEIGHT - 4; i++)
  {
    t->gotoxy(1, 3 + i);
    for (j = 0; j < SCREEN_WIDTH - 2; j++)
    {
      t->putchar('±');
    }
  }

  /* status line and help */
  t->gotoxy(0, SCREEN_HEIGHT - 3);
  t->putchar('Ç');
  for (i = 0; i < SCREEN_WIDTH - 2; i++) t->putchar('Ä');
  t->putchar('¶');

  t->gotoxy(1, SCREEN_HEIGHT - 2);
  printf(" F1 Help, Esc Console, F4 Edit ");
}

void
invert_colors(int *col1, int *col2)
{
  int col;

  col = *col1;
  *col1 = *col2;
  *col2 = col;

  if (t->setcolor)
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
  unsigned char buf[0x100];
  char *p;
  int  x0, y0;
  int  offset;

  //item--;

  //t->setcolorstate(COLOR_STATE_NORMAL);

  // background
  //t->setcolor(0 | (3 << 4), 7 | (3 << 4));
  // clear screen
  //t->cls();
  // 5 - normal (pink), 3 - highlighted (magenta)
  if (t->setcolor)
    t->setcolor((char)foreground_color    | ((char)background_color << 4),
                (char)foreground_color_hl | ((char)background_color_hl << 4));

  if (menu_width  > SCREEN_WIDTH  - 4) menu_width  = SCREEN_WIDTH  - 4;
  if (menu_height > SCREEN_HEIGHT - 8) menu_height = SCREEN_HEIGHT - 8;

  if (menu_height > num_items) menu_height = num_items + 1;

  x0 = (SCREEN_WIDTH  - menu_width)  / 2 - 1;
  y0 = (SCREEN_HEIGHT - menu_height) / 2 - 1;

  t->gotoxy(x0, y0);
  l = 0;
  buf[l++] = 0xda;
  while (l < menu_width) buf[l++] = 0xc4;
  buf[l++] = 0xbf; buf[l] = '\0';
  printf("%s", buf);

  grub_memset(buf, 0, sizeof(buf));

  if (item > menu_height - 2) scrollnum = (item - (menu_height - 1));
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

    t->gotoxy(x0, y0 + i + 1);

    printf("%c ", 0xb3);

    // show highlighted menu string in inverse color
    if (j == item) invert_colors(&foreground_color, &background_color);

    //sprintf(s, "%d", j);
    //l = grub_strlen(s);
    //if (l == 1) grub_strcat(str, " ", s);
    //if (l == 2) grub_strcpy(str, s);
    //sprintf(buf, "%s%s. %s", spc, str, sc->title);
    sprintf((char *)buf, "%s %s", spc, sc->title);

    p = (char *)buf + m;
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

  t->gotoxy(x0, y0 + i + 1);
  l = 0;
  buf[l++] = 0xc0;
  while (l < menu_width) buf[l++] = 0xc4;
  buf[l++] = 0xd9; buf[l] = '\0';
  printf("%s", buf);

  if (t->setcursor)
    t->setcursor(0);

  if (t->setcolor)
    t->setcolor(7, 7);
}

void showpath(void)
{
  printf("\r\n[%s] ", path);
  promptlen = strlen(path) + 3;
  t->setcursor(5);
}

char *getcmd(int key)
{
  int ch = key; // keycode
  int ind = 0;  // cmd line length
  int cur = 0;  // cursor pos in the cmd line
  int pos;      // cursor pos on the screen
  int p;

  memset(cmdbuf, 0, sizeof(cmdbuf));

  do
  {
    switch (ch)
    {
       case 0x0:    // reget char
         ch = t->getkey();
         continue;
       case 0x2:    // left arrow
         cur--;
         if (cur <= 0) cur = 0;
         pos = t->getxy();
         if ((pos >> 8) <= promptlen + 1) pos = ((promptlen + 1) << 8) | (pos & 0xff);
         t->gotoxy((pos >> 8) - 1, pos & 0xff);
         t->setcursor(5);
         ch = 0x0;
         continue;
       case 0x6:    // right arrow
         cur++;
         if (cur > ind) cur = ind;
         pos = t->getxy();
         if ((pos >> 8) < promptlen + 1) pos = ((promptlen + 1) << 8) | (pos & 0xff);
         if ((pos >> 8) >= promptlen + ind - 1) pos = ((promptlen + ind - 1) << 8) | (pos & 0xff);
         t->gotoxy((pos >> 8) + 1, pos & 0xff);
         t->setcursor(5);
         ch = 0x0;
         continue;
       case 0xe08:  // backspace
         pos = t->getxy();
         if ((pos >> 8) > promptlen)
         {
           cur--;
           ind--;
           t->gotoxy((pos >> 8) - 1, pos & 0xff);
           if ((pos >> 8) <= promptlen) pos = ((promptlen) << 8) | (pos & 0xff);
           if (cur <= 0) cur = 0;
           if (ind <= 0) ind = 0;
           p = cur;
           while (*(cmdbuf + p) = *(cmdbuf + p + 1)) p++;
           cmdbuf[p++] = '\0';
           t->gotoxy(promptlen, pos & 0xff);
           printf("%s ", cmdbuf);
           t->gotoxy((pos >> 8) - 1, pos & 0xff);
           t->setcursor(5);
         }
         ch = 0x0;
         continue;
       case 0x4:    // del key
         pos = t->getxy();
         if ((pos >> 8) < promptlen + ind)
         {
           ind--;
           p = cur;
           while (*(cmdbuf + p) = *(cmdbuf + p + 1)) p++;
           cmdbuf[p++] = '\0';
           t->gotoxy(promptlen, pos & 0xff);
           printf("%s ", cmdbuf);
           t->gotoxy((pos >> 8), pos & 0xff);
           t->setcursor(5);
         }
         ch = 0x0;
         continue;
       case 0x1:    // home key
         pos = t->getxy();
         cur = 0;
         t->gotoxy(promptlen, pos & 0xff);
         t->setcursor(5);
         ch = 0x0;
         continue;
       case 0x5:    // end key
         pos = t->getxy();
         cur = ind;
         t->gotoxy(promptlen + ind, pos & 0xff);
         t->setcursor(5);
         ch = 0x0;
         continue;
       case 0x7300: // ctrl-left
         pos = t->getxy();
         while (cur && !isspace(cmdbuf[cur--])) ; // move to the next space symbol to the left
         t->gotoxy(promptlen + cur, pos & 0xff);
         t->setcursor(5);
         ch = 0x0;
         continue;
       case 0x7400: // ctrl-right
         pos = t->getxy();
         while ((cur < ind) && !isspace(cmdbuf[cur++])) ; // move to the next space symbol to the right
         t->gotoxy(promptlen + cur, pos & 0xff);
         t->setcursor(5);
         ch = 0x0;
         continue;
       case 0x11b:  // esc key
         break;
       case 0x1c0d: // enter key
         cmdbuf[ind++] = '\0';
         return cmdbuf;
       default:
         printf("%c", ch);
         t->setcursor(5);
         cmdbuf[ind++] = (char)(ch & 0xff);
         ch = t->getkey();
         cur = ind;
         continue;
    }
    break;
  } while (1);

  return NULL;
}

int exec_cmd(char *cmd)
{
  return exec_line(cmd);
}

void cmdline(int item, int shift)
{
  int  ii;
  int  rc;
  char exitflag = 0;
  char *cmd;
  int  ch = 0;

  //printf("cmdline!\r\n");

  while (1)
  {
    showpath();
    ch = t->getkey();
    if (ch == 0x11b) break; // esc
    if (cmd = getcmd(ch))
    {
      printf("\r\n%s\r\n", cmd);
      rc = exec_cmd(cmd);
      if (!rc)
        printf("Error occured during execution of a command\r\n");
    }
    else
      break;
  }

  t->cls();
  state = 3; // exit
}

void menued(int item, int shift)
{
  int  ch;
  int  n;
  int  i;
  char *p;
  script_t *sc;

  n = item + 1;
  sc = menu_first;

  // find a menu item and execute corresponding script
  while (n)
  {
    n--;
    if (!n)
    { /* (sc->scr, sc->num) */
      p = sc->scr->s;
      for (i = 0; i < sc->num; i++)
      {
        printf("%s\r\n", p);
        while (*p++) ;
      }
    }
    sc = sc->next;
  }

  while (1)
  {
    ch = t->getkey();
    if (ch == 0x11b) break; // esc
    switch (ch)
    {
      default:
        printf("%c", ch & 0xff);
    }
  }

  t->cls();
  state = 0;
}


int
exec_menu(int item, int shift)
{
  int t = 0;
  //int cont = 1;  // continuation flag
  //int item;      // selected menu item
  //int shift = 0; // horiz. scrolling menu shift

  //item = default_item;

  for (;;)
  {
    switch (state)
    {
      case 0: // menu
        show_background_screen();
        do {
          draw_menu(item, shift);
        }   while ((t = get_user_input(&item, &shift)) && (t != -1));
        if (state) continue;    // if we got here by pressing Esc key
        if (t == -1)            // exit to the previous menu
          item |= 0x80000000;
        break;                  // otherwise, if Enter key pressed
      case 1: // cmd line
        cmdline(item, shift);
        continue;
      case 2: // menu editor
        menued(item, shift);
        continue;
      case 3: // exit to exec_cfg
        item |= 0x40000000;
        state = 0;
        break;
      default:
        break;
    }
    break;
  }

  return item;
}

int
exec_script(script_t *script)
{
  string_t *line = script->scr;
  int  i, rc;
  char buf[0x800];
  char last;
  int  l;


  for (i = 0; i < script->num; i++)
  {
    *buf = '\0';
    /* concat lines */
    do {
      grub_strcat(buf, buf, line->s);
      last = line->last;
      // next line
      line = line->next;
    } while (!last);

    rc = exec_line(buf);
    if (!rc)
      printf("Error occured during execution of a command\r\n");

    if (rc != 1) return rc;
  }

  return 1;
}

void
init_vars(void)
{
  memset((char *)menu_items, 0, menu_len);
  memset((char *)config_lines, 0, config_len);

  menu_first = menu_last = 0;
  num_items = 0;

  menu_len = 0;
  config_len = 0;

  section = 0;
  sc = 0;
  sc_prev = 0;

  /* set default menu width and height */
  width_func("68", 2);
  height_func("15", 2);
}

int
exec_cfg(char *cfg, int menu_item, int menu_shift)
{
  int i;
  int rc;
  int itm;
  int item; // menu item number
  int shift;
  char *line, *p;
  script_t *scr;
  char buf[0x100];
  char *ccfg;
  unsigned long drv;

  strcpy(buf, cfg);
  init_vars();

  rc = process_cfg(buf);

  if (!rc)           // can't read config file
  {
    printf("\r\nError opening/reading config file: %s\r\n", buf);
    state = 1;       // go to command line
  }
  else if (rc == -1) // something went wrong during global commands execution
  {
    printf("\r\nError processing config file: %s\r\n", buf);
    state = 1;       // go to command line
  }

  /* If there are items, then show a menu and choose an item */
  if (num_items)
  {
    // starting point 0 instead of 1
    num_items--;

    if (menu_item) item = menu_item;
    if (!item) item = default_item;
    if (item > num_items) item = num_items;

    shift = menu_shift;

  restart_menu:

    //ccfg = curr_cfg;
    // show a menu and let the user choose a menu item
    item = exec_menu(item, shift);

    if (item & 0x80000000) // return to the previous menu
    {
      item &= ~0x80000000; // clear flag in upper bits
      return 1;
    }

    if (item & 0x40000000)
    {
      item &= ~0x40000000; // clear flag in upper bits
      return 0;
    }

    item--;

    scr = menu_first;
    t->cls();

    itm = item + 1;
    // find a menu item and execute corresponding script
    while (itm)
    {
      itm--;
      if (!itm)
      {
        ccfg = curr_cfg;
        if (!exec_script(scr))
        {
          printf("Loading failed, press any key...\r\n");
          kernel_type = KERNEL_TYPE_NONE; /* invalidate */
          errnum = 0;
          drv = 0xff;
          u_parm(PARM_BUF_DRIVE, ACT_SET, (unsigned *)&drv);
          u_parm(PARM_ERRNUM, ACT_SET, (unsigned *)&errnum);
          t->getkey();
          goto restart_menu;
        }
        if (strcmp(ccfg, curr_cfg)) // configfile issued (determined by cfg change)
        {
          //item = item_save;
          //shift = shift_save;
          return 2;
        }
      }
      else
        scr = scr->next;
    }
  }

  /* launch a multiboot kernel */
  t->setcursor(1);
  boot_func(0, 2);

  return 0;
}

void
KernelLoader(char **script)
{
  char *cfg = "/boot/loader/boot.cfg";
  char **cmd;
  int item = 0;
  int shift = 0;
  int rc;
  int i, n = 0;

  printf("\r\nKernel loader started.\r\n");

  config_lines = (char *)m->drives_addr + m->drives_length;

  // exec global commands in config file
  // and copy config file to memory as
  // a string table (strings delimited by zeroes)
  // and make script_t structures list for
  // boot scripts and menu items
  // clear variable store

  memset(variable_list, 0, sizeof(variable_list));
  item_save = shift_save = 0;

  process_cfg_line = process_cfg_line1;

  menu_len = 0; config_len = 0;

  memset(prev_cfg, 0, sizeof(prev_cfg));
  memset(curr_cfg, 0, sizeof(curr_cfg));

  while (1)
  {
    if (!*curr_cfg) strcpy(curr_cfg, cfg);

    if (script)
    {
      for (cmd = script; *cmd; cmd++)
        if (!exec_line(*cmd))
        {
          printf("Boot script failed, press any key...\r\n");
          t->getkey();
          rc = -1;
          break;
        }
      script = 0;
    }
    else
      rc = exec_cfg(curr_cfg, item, shift);

    switch (rc)
    {
    case 0: // script execution is successful, but no kernel loaded
      continue;
    case 1: // return to the previous menu
      continue;
    case 2: // configfile
      item_save = item;
      shift_save = shift;
      item = 0;
      shift = 0;
      continue;
    default:
      item = 0;
      shift = 0;
      continue;
    }
  }
}

void
cmain(char **script)
{
  /* Get mbi structure address from pre-loader */
  u_parm(PARM_MBI, ACT_GET, (unsigned int *)&m);
  /* get a boot drive value */
  u_parm(PARM_AT_DRIVE, ACT_GET, (unsigned int *)at_drive);
  /* init terminal */
  t = u_termctl(-1);
  KernelLoader(script);
}
