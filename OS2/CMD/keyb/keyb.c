#define INCL_DOSERRORS
#include <osfree.h> /* system header */

#include <cmd_shared.h>         // Include file for cmd tools

#include <stdio.h>

APIRET syntax(void)
{
   cmd_ShowSystemMessage(cmd_MSG_KEYB_HELP,0L);
   return NO_ERROR;
}

//**************************************************************************
// Start of program
//**************************************************************************
int main (int argc, char* argv[], char* envp[])
{
  int argi;

    if (argc == 1)
    {
        return syntax();
    }

    for (argi = 1; argi < argc; argi++)
    {
        if ((argv[argi][0] == '-') || (argv[argi][0] == '/'))
        {
            switch (argv[argi][1])
            {
                case 'h':
                case 'H':
                case '?':
                case '-':
                    return syntax();


                default:
                    fprintf(stderr, "Unknown options '%s'.\n", argv[argi]);
                    return NO_ERROR;
            }
        }
    }

}
