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

extern unsigned long cur_addr;

struct line *string_list;
char   *string_table;
char   *string_table_end;
int    lines;
int    x0 = 0, y0 = 0;
char   apply = 0;
int    len = 0;


int getkey (void);
int checkkey (void);
void gotoxy(int x, int y);
int getxy(void);
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

int save_file (unsigned long addr)
{
  struct line *p;
  char   *q;

  for (p = string_list, q = (char *)addr; p->next;
       q += p->len + 2, p = p->next)
  {
    memmove(q, p->str, p->len);
    q[p->len] = '\r';
    q[p->len + 1] = '\n';
  }

  return (unsigned long)q - addr;
}

int get_user_input (int *x, int *y)
{
  int c;
  struct line *next_line, *prev_line, *mid_line;
  int x1, y1;
  char *p;

  for (;;)
  {
    c = getkey();

    switch (c)
    {
    case 0xe:    // down arrow
      if (*y < lines) ++*y;
      if (*y == y0 + SCREEN_HEIGHT) y0++;
      break;
    case 0x10:   // up arrow
      if (*y) --*y;
      if (*y == y0 - 1) y0--;
      break;
    case 0x6:    // right arrow
      ++*x;
      if (*x == x0 + SCREEN_WIDTH) x0++;
      break;
    case 0x2:    // left arrow
      if (*x) --*x;
      if (*x == x0 - 1) x0--;
      break;
    case 0x3:    // pgdn
      if (*y < lines) *y = min(*y + SCREEN_HEIGHT, lines);
      if (*y >= y0 + SCREEN_HEIGHT) y0 = min(y0 + SCREEN_HEIGHT, lines);
      break;
    case 0x7:    // pgup
      if (*y) *y = max(y0 - SCREEN_HEIGHT, 0);
      if (*y <= y0 - 1) y0 = max(y0 - SCREEN_HEIGHT, 0);
      break;
    case 0x1:    // home
      *x = 0;
      x0 = 0;
      break;
    case 0x5:    // end
      *x = line(*y)->len;
      x0 = max(*x - SCREEN_WIDTH + 1, 0);
      break;
    case 0x11b:  // esc
      gotoxy(0, SCREEN_HEIGHT);
      setcolor(0x71, 0x62);
      setcursor(0);
      printf("* Apply changes: Y(es)/N(o)/any key to Cancel?");
      c = getkey() & 0xff;
      if (c == 0x79 || c == 0x59) // 'Y' ('y')
      {
        apply = 1;
        return 0;
      }
      if (c == 0x6e || c == 0x4e) // 'N' ('n')
      {
        apply = 0;
        return 0;
      }
      gotoxy(0, SCREEN_HEIGHT);
      printf("                                               ");
      gotoxy(*x - x0, *y - y0);
      setcursor(1);
      break;
    case 0x1c0d: // enter
      prev_line = line(*y);
      /* allocate a new line structure */
      mid_line = string_list + lines + 1;
      next_line = prev_line->next;
      prev_line->next = mid_line;
      mid_line->prev = prev_line;
      mid_line->next = next_line;
      next_line->prev = mid_line;
      /* line break point */
      x1 = min(*x, prev_line->len);
      mid_line->len = prev_line->len - x1;
      memmove(string_table_end, prev_line->str + x1, prev_line->len - x1);
      mid_line->str = string_table_end;
      string_table_end += prev_line->len - x1;
      memmove(string_table_end, prev_line->str, x1);
      prev_line->len = x1;
      prev_line->str = string_table_end;
      string_table_end += x1;
      mid_line->dirty = 1;
      prev_line->dirty = 1;
      if (*y == y0 + SCREEN_HEIGHT) y0++;
      ++*y;
      *x = x0;
      lines++;
      break;
    case 0xe08:  // backspace
      next_line = string_list + lines + 1;
      mid_line = line(*y);
      x1 = min(*x, mid_line->len);
      if (!x1 && *x)
      {
        *x = 0;
        break;
      }
      if (!*x)
      {
        /* join a line with previous one */
        mid_line  = line(*y);
        prev_line = mid_line->prev;
        *x = prev_line->len;
        if (*x >= x0 + SCREEN_WIDTH) x0 = max(*x - SCREEN_WIDTH + 1, 0);
        (prev_line->prev)->next = next_line;
        next_line->prev = prev_line->prev;
        next_line->next = mid_line->next;
        (mid_line->next)->prev = next_line;
        next_line->len = prev_line->len + mid_line->len;
        memmove(string_table_end, prev_line->str, prev_line->len);
        p = string_table_end;
        string_table_end += prev_line->len;
        memmove(string_table_end, mid_line->str, mid_line->len);
        string_table_end += mid_line->len;
        if (*y) --*y;
        if (*y == y0 - 1) y0--;
      }
      else
      {
        prev_line = line(*y);
        (prev_line->prev)->next = next_line;
        next_line->prev = prev_line->prev;
        next_line->next = prev_line->next;
        (prev_line->next)->prev = next_line;
        next_line->len = prev_line->len - 1;
        x1 = min(*x, prev_line->len);
        memmove(string_table_end, prev_line->str, x1 - 1);
        p = string_table_end;
        string_table_end += x1 - 1;
        memmove(string_table_end, prev_line->str + x1, prev_line->len - x1);
        string_table_end += prev_line->len - x1;
        if (*x) --*x;
        if (*x == x0 - 1) x0--;
      }
      next_line->str = p;
      next_line->dirty = 1;
      lines++;
      break;
    case 0x4:    // del key
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
  char buf[SCREEN_WIDTH + 2];
  int xoff, line, l, oldcursor, cursor;
  struct line *s;

  setcolor(0x17, 0x26);
  gotoxy (0, 0);
  setcursor (0);

  for (line = 0, s = string_list; s->next; s = s->next, line++)
  {
    if (line < y0) continue;
    xoff = min(s->len, x0);
    l = min(s->len - xoff, SCREEN_WIDTH);
    memmove (buf, s->str + xoff, l);
    while (l < SCREEN_WIDTH) buf[l++] = ' ';
    buf[l] = '\0';
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
  setcolor(0x71, 0x62);
  for (l = 0; l < SCREEN_WIDTH; l++) buf[l] = ' ';
  buf[l] = '\0';
  printf(buf);
  gotoxy(x - x0, y - y0);
  setcursor(1);
}

void editor (char *cfg,             /* config file address */
             int length,               /* its length          */
             char force)            /* force starting the editor         */
{
  int  x = 0, y = 0;

  /* first available address aligned on page boundary */
  string_list = (struct line *)cur_addr;
  cur_addr += MAX_LINES * sizeof(struct line);
  string_table = string_table_end = (char *)cur_addr;

  /* Read file */
  read_file (cfg, length);

  /* if force or Alt-E pressed previously */
  if (force || (checkkey() >= 0 && getkey() == 0x1200))
  {
    /* x and y coordinates of cursor */
    cls();
    setcolorstate(COLOR_STATE_NORMAL);
    setcolor(0x17, 0x26);
    /* Editor main loop */
    do
    {
      draw_screen(x, y);
    } while (get_user_input(&x, &y));

    cls();
  }

  if (apply)
  {
    /* apply changes */
    cur_addr = (unsigned long)string_table_end;
    len = save_file(cur_addr);
  }
}
