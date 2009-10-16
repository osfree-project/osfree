/*  Config.sys preprocessor
 *  and editor
 *
 */

#pragma aux callback     "*"

void callback(unsigned long addr,
              unsigned long size,
              char drvletter)
{
  int i;
  char *cfg;

  cfg = (char *)addr;

  for (i = 0; i < size; i++)
  {
    if (cfg[i] == '!')
      cfg[i] = drvletter;
  }
}
