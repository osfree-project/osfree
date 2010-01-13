/*  Config.sys preprocessor
 *  and editor
 *
 */

#include <shared.h>
#include <term.h>

#pragma aux callback     "*"
#pragma aux u_msg        "*"

struct term_entry *t;
/* multiboot structure pointer */
extern struct multiboot_info *m;
extern char cfged;


/* first available free address */
extern unsigned long cur_addr;

void editor (char *cfg, int len, unsigned long free, char force);

int getkey (void)
{
  return t->getkey();
}

int checkkey (void)
{
  return t->checkkey();
}

void gotoxy(int x, int y)
{
  t->gotoxy(x, y);
}

int setcursor(int n)
{
  return t->setcursor(n);
}

void setcolorstate(int n)
{
  t->setcolorstate(n);
}

void setcolor(int x, int y)
{
  t->setcolor(x, y);
}

void cls(void)
{
  t->cls();
}

void u_msg (char *s)
{
}

void load_modules (void)
{

}

void callback(unsigned long addr,
              unsigned long size,
              char drvletter,
              struct term_entry *term)
{
  int i;
  char *cfg;

  t = term;
  cfg = (char *)addr;

  /* Preload needed files */
  load_modules ();

  /* Patch a boot drive letter */
  for (i = 0; i < size; i++)
  {
    if (cfg[i] == '@')
      cfg[i] = drvletter;
  }

  /* Start config.sys editor */
  printf ("Hello editor!\r\n");
  setcursor (1);
  editor (cfg, size, cur_addr, cfged);
}
