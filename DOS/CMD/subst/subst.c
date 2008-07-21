#define INCL_VDM
#include <os2vdm.h> /* system header */
#include <stdio.h>

//int main(void)
//{
//  CHAR path[1024]="C:\\OS2";
//  VdmSubstSet('H', path);
//}

APIRET syntax(void)
{
//   cmd_ShowSystemMessage(cmd_MSG_DISKCOPY_HELP,0L);
//   return NO_ERROR;
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
                    return 0;//NO_ERROR;
            }
        }
    }

}
