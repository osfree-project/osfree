/*  Config file preprocessor
 *
 */

#include <shared.h>
#include <fsys.h>

extern int cur_addr;
extern struct multiboot_info *m;

#pragma aux m "*"

int module_func (char *arg, int flags);

/* loads a file and returns its length */
int load_file(char *file, char *at)
{
  unsigned int size;

  if (u_open(file, &size))
    return 0;

  size = u_read(at, size);
  u_close();

  return size;
}

void subst_include(char *to, char *from, int *len)
{
  char *line  = from;
  char *s, *p = to, *q;
  int l, n, m = *len;

  do {
    /* process a line */
    if (strstr(line, "!include ") == line)
    {
      q = line;
      line += strlen("!include ");
      /* skip spaces */
      while (*line == ' ') line++;
      if (*line == '"') line++;
      for (s = line; *s != '\r' && *s != '\n' && *s != '"'; s++) ;
      if (*s == '"') *s++ = '\0';
      if (*s == '\r' || *s == '\n') *s++ = '\0';
      if (*s == '\r' || *s == '\n') *s++ = '\0';
      n = s - q; /* string length */
      l = load_file(line, p);
      *len += l - n; p += l;
      line = s;
    }
    else
    {
      /* copy string as is */
      while (*line != '\r' && *line != '\n') *p++ = *line++;
      *p++ = '\r'; *p++ = '\n';
      if (*line == '\r' || *line == '\n') line++;
      if (*line == '\r' || *line == '\n') line++;
    }
  } while (line - from < m);
}

void subst_if(char *to, char *from, int *len)
{
  char *line = from;
  char *p    = to;
  char *s, *r;
  char var[24];
  char val[24];
  int  m     = *len;
  int  count = 0;
  int  skip = 0, flag = 0;

  do {
    /* process a line */
    r = line;
    if (strstr(line, "!ifeq ") == line)
    {
      count++;
      if (count == 1)
      {
        line += strlen("!ifeq ");
        while (*line == ' ') line++;
        s = var;
        while (*line != ' ') *s++ = *line++;
        *s = '\0';
        while (*line == ' ') line++;
        s = val;
        while (*line != ' ' && *line != '\r' && *line != '\n') *s++ = *line++;
        *s = '\0';
        flag = !strcmp(var_get(var), val);
        while (*line == ' ') line++;
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        *len -= line - r;
        skip = !flag;
      }
      else
      {
        while (*line != '\r' && *line != '\n') *p++ = *line++;
        *p++ = '\r'; *p++ = '\n';
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        continue;
      }
    }
    else if (strstr(line, "!else") == line)
    {
      if (count == 1)
      {
        skip = flag;
        line += strlen("!else");
        while (*line == ' ') line++;
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        *len -= line - r;
      }
      else if (skip)
      {
        while (*line != '\r' && *line != '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        *len -= line - r;
      }
      else
      {
        while (*line != '\r' && *line != '\n') *p++ = *line++;
        *p++ = '\r'; *p++ = '\n';
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
      }
      continue;
    }
    else if (strstr(line, "!endif") == line)
    {
      if (count > 1)
      {
        while (*line != '\r' && *line != '\n') *p++ = *line++;
        *p++ = '\r'; *p++ = '\n';
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
      }
      else if (skip)
      {
        while (*line != '\r' && *line != '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        *len -= line - r;
      }
      else
      {
        line += strlen("!endif");
        while (*line == ' ') line++;
        *len -= line - r;
      }

      if (count == 1)
        skip = flag = 0;

      count--;
    }
    else
    {
      if (skip)
      {
        while (*line != '\r' && *line != '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        if (*line == '\r' || *line == '\n') line++;
        *len -= line - r;
        continue;
      }

      /* copy string as is */
      while (*line != '\r' && *line != '\n') *p++ = *line++;
      *p++ = '\r'; *p++ = '\n';
      if (*line == '\r' || *line == '\n') line++;
      if (*line == '\r' || *line == '\n') line++;
    }
  } while (line - from < m);
}

int preprocess(char *file)
{
  int rc = 1, ret;
  char *from, *p;
  char *to;
  int len;

  struct mod_list *mll;

  /* Load a text file as is */
  ret = module_func(file, 2);
  if (ret)
    return 1;

  mll = (struct mod_list *)m->mods_addr;

  from = (char *)mll[m->mods_count - 1].mod_start;
  len  = (char *)mll[m->mods_count - 1].mod_end - from;

  /* resolve '!include' directives */
  to = (char *)((cur_addr + 0xFFF) & 0xFFFFF000);

  do {
    subst_include(to, from, &len);
    from = to; to = (char *)(((int)to + len + 0xFFF) & 0xFFFFF000);
  } while ((p = strstr(from, "!include ")) && (p[-1] == '\n'));

  do {
    subst_if(to, from, &len);
    from = to; to = (char *)(((int)to + len + 0xFFF) & 0xFFFFF000);
  } while ((p = strstr(from, "!if")) && (p[-1] == '\n'));

  /* fix mod_start and mod_end to point to 'expanded' file */
  mll[m->mods_count - 1].mod_start = (int)from;
  mll[m->mods_count - 1].mod_end   = (int)from + len;

  cur_addr = (int)to;

  return 0;
}
