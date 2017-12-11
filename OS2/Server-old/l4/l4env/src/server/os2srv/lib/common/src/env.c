/* dice includes   */
#include <dice/dice.h>

#include <l4/os3/cfgparser.h>
#include <l4/os3/io.h>

#include <l4/os2srv/os2server-client.h>

extern l4_threadid_t os2srv;

unsigned long cfg_Getenv(char *name, char **value);
unsigned long cfg_Getopt(const char *name, int *is_int, int *value_int, char **value_str);

unsigned long cfg_Getenv(char *name, char **value)
{
  CORBA_Environment env = dice_default_environment;
  unsigned long rc;
  io_log("name=%s", name);
  rc = os2server_cfg_getenv_call(&os2srv, name, value, &env);
  io_log("path=%s", *value);
  return rc;
}


unsigned long cfg_Getopt(const char *name, int *is_int, int *value_int, char **value_str)
{
  CORBA_Environment env = dice_default_environment;
  return os2server_cfg_getopt_call (&os2srv, name, is_int, value_int,
                                    value_str, &env);
}
