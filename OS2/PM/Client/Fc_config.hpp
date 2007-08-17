/*
 $Id: Fc_config.hpp,v 1.1 2003/06/15 17:39:55 evgen2 Exp $
*/
/* FreePM Client config */

class F_ClientConfig
{
   public:
   char ExternMachineName[_MAX_FNAME];
   char debugOptions[256];

   F_ClientConfig()
   { ExternMachineName[0] = 0;
     strcpy(debugOptions,"ALL,7");
   }
   int Read(char *fname);
   int Write(char *fname);
   int AnalizeRecodrRead(char *name, char *par);
   int ReadStr(FILE *fp, char *str, char *nameClass, char *name, char *par );

};
