#include <ctype.h>
#include <string.h>
#include <malloc.h>

#include "minicmd.h"
#include "env.h"

// environment array
char **envp = NULL;

void env_create(void)
{
  PTIB ptib; PPIB ppib;
  char *env;
  char **pEnv;
  int  cntVars = 0;
  int  bufsize, i;

  // get initial environment from PIB
  DosGetInfoBlocks(&ptib, &ppib);
  env = ppib->pib_pchenv;

  bufsize = 0;

  // get the number of all env. strings
  while (env[bufsize])
  {
    bufsize += strlen(env + bufsize) + 1;
    cntVars ++;
  }

  bufsize = 0;

  // create env. array
  pEnv = (char **)malloc( (cntVars + 1) * sizeof(char *) );

  for (i = 0; i < cntVars; i ++)
  {
    pEnv[i] = env + bufsize;
    bufsize += strlen(env + bufsize) + 1;
  }

  pEnv[cntVars] = NULL;
  envp = pEnv;
}

void env_var_add(char *var)
{
  char **pEnv  = envp;
  char varname[128];
  char varname2[128];
  int  cntVars, i, len, len2;
  char *p, *s;

  if ( (p = strchr(var, '=')) == NULL)
  {
    // display variable
    for (cntVars = 0; pEnv[cntVars]; cntVars ++)
    {
      len = strlen(var);
      strncpy(varname2, pEnv[cntVars], len);
      varname2[len] = '\0';

      if (!stricmp(var, varname2))
        break; // found
    }

    // no such variable
    if (pEnv[cntVars] == NULL)
      return;

    VioWrtTTY(pEnv[cntVars], strlen(pEnv[cntVars]), 0);
    VioWrtTTY("\r\n", 2, 0);
    return;
  }

  // uppercase the var name
  for (s = var; s != p; s ++)
    *s = toupper(*s);

  len = p - var;
  strncpy(varname, var, len);
  varname[len] = '\0';

  // search for 'varname' in environment
  for (cntVars = 0; pEnv[cntVars]; cntVars ++)
  {
    strncpy(varname2, pEnv[cntVars], len);
    varname2[len] = '\0';

    if (!stricmp(varname2, varname))
      break; // found
  }

  if (!strcmp(p, "="))
  {
    // if 'var' does not exist in enironment
    if (! pEnv[cntVars])
      return;

    // unset variable

    // free var. memory
    free(pEnv[cntVars]);

    // compress the array
    for (i = cntVars; pEnv[i]; i++)
      pEnv[i] = pEnv[i + 1];

    pEnv = realloc(pEnv, (i + 1) * sizeof(void *));
    return;
  }

  if (pEnv[cntVars])
  {
    // found
    // the var already exists
    pEnv[cntVars] = realloc(pEnv[cntVars], strlen(var) + 1);
    strcpy(pEnv[cntVars], var);
  }
  else
  {
    // not found
    for (cntVars = 0; pEnv[cntVars]; cntVars ++) ;

    // add new variable
    pEnv = realloc(pEnv, (cntVars + 2) * sizeof(char *));
    pEnv[cntVars] = (char *)malloc( strlen(var) + 1 );
    strcpy(pEnv[cntVars], var);
    pEnv[cntVars + 1]     = NULL;
  }

  envp = pEnv;
}

char *env_get(void)
{
  char **pEnv = envp;
  char *env;
  int  cntVars = 0;
  int  bufsize = 0;
  int  i;

  // count the common env. strings length
  for (cntVars = 0; pEnv[cntVars]; cntVars++)
    bufsize += strlen(pEnv[cntVars]) + 1;

  env = (char *)malloc( bufsize + 1 );

  bufsize = 0;

  for (i = 0; i < cntVars; i ++)
  {
    strcpy(env + bufsize, pEnv[i]);
    bufsize += strlen(pEnv[i]) + 1;
  }

  env[bufsize] = '\0';

  return env;
}

void env_free(char *pEnv)
{
  free(pEnv);
}

void env_destroy(void)
{
  char **env = envp;

  while (*env)
    free(*env ++);

  free(envp);
  envp = NULL;
}

int set_cmd(int argc, char **argv)
{
  int  cntVars;
  char *var, *p;
  char varname[128];
  char **pEnv = envp;

  if (argc > 2)
    return 1;

  if (argc == 1)
  {
    // print env
    for (cntVars = 0; pEnv[cntVars]; cntVars ++)
    {
      var = pEnv[cntVars];
      VioWrtTTY(var, strlen(var), 0);
      VioWrtTTY("\r\n", 2, 0);
    }

    return 0;
  }

  env_var_add(argv[1]);

  return 0;
}

int env_cmd(int argc, char **argv)
{
  if (argc > 1)
    return 1;

  set_cmd(argc, argv);

  return 0;
}
