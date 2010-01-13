/*  A small embedded
 *  config file editor
 *
 */

#include <shared.h>
#include <term.h>

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 24

#define MAX_LINES 8192

/* a string list structure */
struct line
{
  char *str;
  int  len;
  int  dirty;
  struct line *next;
  struct line *prev;
};

struct line *string_list;
char   *string_table;
char   *string_table_end;
int    lines;
int    x0 = 0, y0 = 0;

int getkey (void);
int checkkey (void);
void gotoxy(int x, int y);
int setcursor(int n);
void setcolorstate(int n);
void setcolor(int x, int y);
void cls(void);

int max (int x, int y)
{
  if (x <= y)
    return y;
  else
    return x;
}

int min (int x, int y)
{
  if (x <= y)
    return x;
  else
    return y;
}

/* returns n'th line of the list */
struct line *line (int n)
{
  int i = n;
  struct line *s = string_list;

  while (i--) s = s->next;
  return s;
}

void read_file (char *cfg, int len)
{
  struct line *s;
  char *p, *line;
  int  n;

  /* Read file */
  s = string_list;
  lines = 0;
  line = cfg;

  for (;;)
  {
    s->str = line;
    for (p = line, n = 0; *p != '\r' && *p != '\n'; p++, n++) ;
    if (*p == '\r') p++;
    if (*p == '\n') p++;
    s->len = n;
    s->dirty = 0;

    if (lines)
      s->prev = s - 1;
    else
      s->prev = 0;

    if (line - cfg < len)
      s->next  = s + 1;
    else
    {
      s->next  = 0;
      break;
    }

    line = p;
    lines++;
    s++;
  }
}

int get_user_input (int *x, int *y)
{
  int c;

  for (;;)
  {
    c = getkey();

    switch (c)
    {
    case 0xe:    // down arrow
      if (*y < lines) ++*y;
      if (*y >= y0 + SCREEN_HEIGHT + 1) y0++;
      break;
    case 0x10:   // up arrow
      if (*y) --*y;
      if (*y <= y0 - 1) y0--;
      break;
    case 0x6:    // right arrow
      if (*x < (line(*y))->len) ++*x;
      if (*x >= x0 + SCREEN_WIDTH + 1)  x0++;
      break;
    case 0x2:    // left arrow
      if (*x) --*x;
      if (*x <= x0 - 1) x0--;
      break;
    case 0x3:    // pgdn
      if (*y < lines) *y = min(*y + SCREEN_HEIGHT, lines);
      if (*y >= y0 + SCREEN_HEIGHT + 1) y0 = min(y0 + SCREEN_HEIGHT, lines);
      if (*y < y0) *y = y0;
      break;
    case 0x7:    // pgup
      if (*y) *y = max(y0 - SCREEN_HEIGHT, 0);
      if (*y <= y0 - 1) y0 = max(y0 - SCREEN_HEIGHT, 0);
      if (*y > y0) *y = y0;
      break;
    case 0x11b:  // esc
      gotoxy(0, SCREEN_HEIGHT);
      setcolor(0x71, 0x71);
      printf("* Exit (y/n)?");
      c = getkey() & 0xff;
      if (c == 0x79 || c == 0x59) return 0;
      gotoxy(0, SCREEN_HEIGHT);
      printf("             ");
      gotoxy(*x, *y);
      break;
    default:
      continue;
    }
    break;
  }

  return 1;
}

void draw_screen (int x, int y)
{
  char buf[82];
  int xoff, line, l;
  struct line *s;

  gotoxy (0, 0);
  setcolor(0x17, 0x17);
  for (line = 0, s = string_list; s->next; s = s->next, line++)
  {
    if (line < y0) continue;
    xoff = min(s->len, x0);
    l = min(s->len - xoff, SCREEN_WIDTH);
    memmove (buf, s->str + xoff, l);
    while (l < SCREEN_WIDTH) buf[l++] = ' ';
    buf[l] = '\0';
    setcursor(0);
    printf (buf);
    if (line - y0 > SCREEN_HEIGHT - 2) break;
    printf ("\r\n");
  }

  if (!s->next)
  {
    for (l = 0; l < SCREEN_WIDTH; l++) buf[l] = ' ';
    buf[l] = '\0';

    while (line - y0 < SCREEN_HEIGHT)
    {
      printf ("%s\r\n", buf);
      line++;
    }
  }

  gotoxy(0, SCREEN_HEIGHT);
  setcolor(0x71, 0x71);
  for (l = 0; l < SCREEN_WIDTH; l++) buf[l] = ' ';
  buf[l] = '\0';
  printf(buf);
  gotoxy(x, y);
  setcursor(1);
}

void editor (char *cfg,             /* config file address */
             int len,               /* its length          */
             unsigned long first,   /* first available address after mbi */
             char force)            /* force starting the editor         */
{
  char *curr_addr;
  int  x = 0, y = 0;
  /* first available address aligned on page boundary */
  curr_addr = (char *)((first + 0xfff) & 0xfffff000);
  string_list = (struct line *)curr_addr;
  curr_addr += MAX_LINES * sizeof(struct line);
  string_table = curr_addr;

  /* Read file */
  read_file (cfg, len);

  /* if force or Alt-E pressed previously */
  if (force || (checkkey() >= 0 && getkey() == 0x1200))
  {
    /* x and y coordinates of cursor */
    setcolorstate(COLOR_STATE_NORMAL);
    setcolor(0x17, 0x17);
    /* Editor main loop */
    do
    {
      draw_screen(x, y);
    } while (get_user_input(&x, &y));
  }

  cls();
}
