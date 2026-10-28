/*! @file Fc_config.cpp
    @brief Client-side configuration reader for fpm.ini.

    Reads the FreePM client configuration file (fpm.ini) and stores
    the settings in the global F_ClientConfig instance. Recognised
    keys: debugOptions, remote, backend.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Fc_config.hpp"


/*! @brief Read an fpm.ini file and fill this configuration object.

    Opens the file, parses it line by line via ReadStr(), and applies
    each recognised key through AnalizeRecodrRead(). A missing file is
    not an error: the object keeps its defaults.

    @param [in] fname  Path to the configuration file.
    @return 0 on success (including "file not found"), 1 if the file
            could not be opened.
    @retval 0  On success (including "file not found").
    @retval 1  If the file could not be opened. */
int F_ClientConfig::Read(char *fname)
{  int id,rc;
   FILE *fp;
   char str[256],nameClass[128],name[128],par[128];
   fp = fopen(fname,"r");
   if(fp == NULL)
         return 1;
   do
   {  rc = ReadStr(fp, str,nameClass,name,par);
      if(!rc)
      { //  if(!strcmpi(nameClass,"prg"))
                 AnalizeRecodrRead(name,par);
        //  else
      }
   } while(!rc);

   fclose(fp);


   return 0;
}

/*! @brief Read one logical line from the configuration file.

    Skips empty lines and lines whose first non-space character is
    ';' (comment). Splits "name=value" at '='; if the name contains a
    dot, the part before the dot is treated as a class name.

    @param [in]  fp         Open file.
    @param [in]  str        Buffer for the raw line (at least 128 bytes).
    @param [out] nameClass  Class name (empty if no dot in name).
    @param [out] name       Key name.
    @param [out] par        Value string.
    @return 0 if a line was parsed, 1 on EOF or a skipped line.
    @retval 0  A line was parsed.
    @retval 1  EOF or a line that was skipped (empty, comment, or
               missing '='). */
int F_ClientConfig::ReadStr(FILE *fp, char *str, char *nameClass, char *name, char *par )
{  int i,iscomment,l;
   char *pstr;
/* читаем строку */
M: pstr= fgets(str,128,fp);
   if(pstr == NULL) return 1; //EOF
/* игнорируем строки нулевой длины */
   l = strlen(str);
   if(str[l-1] == '\n')
   {  str[--l] = 0;
   }
   if(l == 0) goto M;
/* игнорируем комментарии */
   iscomment = 0;
   for(i=0;i<l;i++)
   { if(str[i] > 32)
     {   if(str[i] == ';') iscomment = 1;
         break;
     }
   }
   if(iscomment) goto M;
   pstr = strstr(str,"=");
   if(pstr == NULL)
        goto M; // игнорируем строки без "="
   *pstr = 0;
   strcpy(par,pstr+1);      // читаем параметры
   pstr = strstr(str,".");  // ищем точку для определения имени класса
   if(pstr == NULL)
   {
//  строка без "."
      nameClass[0] = 0;
      sscanf(str,"%s",name);
   } else {
      *pstr = 0;
      sscanf(str,"%s",nameClass);
      sscanf(pstr+1,"%s",name); // читаем имя параметра
   }
   return 0;
}

/*! @brief Apply one parsed key to this configuration object.

    Recognised keys: "debugOptions", "remote", "backend". Unknown
    keys are silently ignored.

    @param [in] name  Key name.
    @param [in] par   Value string.
    @return 0 if the key was recognised and applied, 1 otherwise.
    @retval 0  A recognised key was applied.
    @retval 1  The key is not recognised. */
int F_ClientConfig::AnalizeRecodrRead(char *name, char *par)
{   static char *lsNames[]=
     { "debugOptions","remote","backend",NULL
     };
      int i,i1,is=0,npar=0,rc;

     for(i=0;lsNames[i];i++)
     {   if(!strcmp(name,lsNames[i]))
         {  is = 1;
            npar = i;
            break;
         }
     }
     if(!is) return 1;
     switch(npar)
     {
        case 0:
           sscanf(par,"%s",&debugOptions);
          break;
        case 1:  /* remote */
           sscanf(par,"%s", ExternMachineName);
         break;
        case 2:  /* backend */
           sscanf(par,"%s", BackendName);
         break;
   }
    return 0;
}
