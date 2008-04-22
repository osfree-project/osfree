/*  Config parsing routines
 *  (c) osFree project,
 *  2003 Apr 21
 *  author valerius
 *
 *  These routines are common for loader
 *  and pre-loader. Config file parser
 *  reads a portion of file in buffer,
 *  retrieves a line from it. A line is
 *  built from several lines if there is
 *  continuation symbols at the end of
 *  current line. Also, backslashes are
 *  substituted by forward slashes, comments
 *  are deleted and for each line got this
 *  way, the routine is called, which expands
 *  variables.
 */

#include <shared.h>
#include "fsys.h"

#ifdef printf
#undef printf
#endif

#ifdef LOADER

#define printf grub_printf
#define expand_vars var_sprint

int var_sprint(char *buf, char *str);

#else

#define printf printmsg
#define expand_vars grub_strcpy

#endif

#define BUFSIZE 0x400
char buf[BUFSIZE];

int process_cfg_line(char *line);

void panic(char *msg, char *file)
{
  printf("\r\nFatal error: \r\n");
  printf(msg);
  printf(file);

  __asm {
    cli
    hlt
  }
}

char *wordend(char *s)
{
  char *p = s;

  while (*p && !grub_isspace(*p)) p++;
  return p;
}

int abbrev(char *s1, char *s2, int n)
{
  char *p = s1;
  char *q = s2;
  int  i = 1;

  while (*p++ == *q++ && *p != '\0') i++;
  if (i >= n) return 1;

  return 0;
}

/*  Strip leading and trailing
 *  spaces
 */
char *strip(char *s)
{
  char *p = s;
  int  i;

  i = grub_strlen(p) - 1;
  while (grub_isspace(p[i])) p[i--] = '\0'; // strip trailing spaces
  while (grub_isspace(*p)) p++;             // strip leading spaces

  return p;
}

/*  Delete ending CR
 *  and LF
 */
char *trim(char *s)
{
  int l, i;

  l = grub_index('\r', s);
  if (l) s[l - 1] = '\0';
  l = grub_index('\n', s);
  if (l) s[l - 1] = '\0';
  
  return s;
}

/*  Returns a next line from a buffer in memory
 *  and changes current position (*p)
 */
char *getline(char **p, int n)
{
  static char linebuf[BUFSIZE];
  int    i = 0;
  char   *q = *p;

  if (!q)
    panic("getline(): zero pointer: ", "*p");

  while (*q != '\r' && *q != '\n' && *q != '\0' && q - buf < n)
    linebuf[i++] = *q++;

  if (*q == '\r') linebuf[i++] = *q++;
  if (*q == '\n') linebuf[i++] = *q++;

  linebuf[i] = '\0';
  *p = q;

  return linebuf;
}

int process_cfg(char *cfg)
{
  // buffer for config file reading
  static char s[BUFSIZE];
  static int  s_pos = 0;
  // second buffer
  static char str[BUFSIZE];
  static int  str_pos = 0;
  char *p, *line, *q;
  char f, g;
  unsigned int size, sz;
  unsigned int rd;
  int bytes_read = 0;
  int i;

  if (u_open(cfg, &size)) {
    printf("Cannot open config file!\r\n");
    return 0;
  }
  u_close(); 

  //grub_memset(s, 0, sizeof(s));

  sz = size;
  while (sz) 
  {
    // read new buffer
    if (u_open(cfg, &size)) {
      printf("Cannot open config file!\r\n");
      return 0;
    }

    u_seek(bytes_read);

    rd = u_read(buf, sizeof(buf));
    sz -= rd;
    bytes_read += rd;

    u_close();

    if (sz && !rd)
    {
      printf("Can't read from config file!\r\n");
      return 0;
    }      

    f = 1;
    p = buf;
    while (*p && f) 
    {
      // read new line from current buffer
      line = getline(&p, rd);
      i = grub_strlen(line);
      if (s_pos + i > BUFSIZE) 
	panic("string s too long to fit in buffer!\r\n", cfg);
      grub_strcpy(s + s_pos, line);
      s_pos += i;
      f = (p - buf < rd);
      g = f || (!f && (buf[rd - 1] == '\n'));
      if (g)
      {
        // delete CR and LF symbols at the end
        line = strip(trim(s));
        // skip comments ";"
        i = grub_index(';', line);
        if (i) line[i - 1] = '\0';
        // skip comments "#"
        i = grub_index('#', line);
        if (i) line[i - 1] = '\0';
        // delete leading and trailing spaces
        line = strip(line);

	// line continuation symbol ('^' or '&') support
        i = grub_strlen(line);
        if (line[i - 1] == '^' || line[i - 1] == '&')
        {
          // the current line continued
          line[i - 1] = '\0';
          if (str_pos + i - 1 > BUFSIZE)
	    panic("string str too long to fit in buffer!\r\n", cfg);
	  grub_strcpy(str + str_pos, line);
          str_pos += i - 1;
          s[0] = '\0';
          s_pos = 0;

	  continue;
        }
        else
        {
          // the line ends
	  if (str_pos + i > BUFSIZE)
	    panic("string str too long to fit in buffer!\r\n", cfg);
	  grub_strcpy(str + str_pos, line);
	  str_pos = 0;
          s[0] = '\0';
          s_pos = 0;
        }

	// expand variables
	expand_vars(s, str);

	// process the line
        if (!process_cfg_line(s)) 
	  return -1;
      }
    }
  }

  return 1;
}
