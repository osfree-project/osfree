/* if s is a string of type "var = val" then this
 * function returns var
 */
char *var(char *s)
{
  int i;

  i = grub_index('=', s);
  if (i) grub_strncpy(lb, s, i - 1);
  lb[i - 1] = '\0';

  return lb;
}

/* if s is a string of type "var = val" then this
 * function returns val
 */
char *val(char *s)
{
  int i, l;

  i = grub_index('=', s);
  l = grub_strlen(s) - i;
  if (i) grub_strncpy(lb, s + i, l);
  lb[l] = '\0';

  return lb;
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

/*  Add a string to the string table
 *  and return its address
 */
char *
stradd(char *s)
{
  int  k;
  char *l;

  k = grub_strlen(s);

  if (strtab_pos + k < STRTAB_LEN) {
    l = strtab + strtab_pos;
    grub_strcpy(l, s);
    l[k] = '\0';
    strtab_pos += k + 1;
  } else {
    /* no space in buffer */
    return 0;
  }

  return l;
}

/*  Returns a next line from a file in memory
 *  and changes current position (*p)
 */
char *getline(char **p)
{
  int  i = 0;
  char *q = *p;
  char *s;

  if (!q)
    panic("getline(): zero pointer: ", "*p");

  while (*q != '\n' && *q != '\r' && *q != '\0' && i < 512)
    linebuf[i++] = *q++;

  if (*q == '\r') q++;
  if (*q == '\n') q++;

  linebuf[i] = '\0';
  *p = q;

  s = strip(linebuf);

  if (!*s)
    return s;

  /* skip comments */
  i = grub_index(';', s);
  if (i) s[i - 1] = '\0';

  s = strip(s);

  /* if empty line, get new line */
  if (!*s)
    return getline(p);

  return s;
}
