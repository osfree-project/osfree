/*
 $Id: Fs_config.cpp,v 1.1 2003/06/15 17:45:47 evgen2 Exp $
*/
/* Fs_config.cpp */
/* Server config: write/read and analyze server's ini file
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Fs_config.hpp"


int F_ServerConfig::Read(char *fname)
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

int F_ServerConfig::ReadStr(FILE *fp, char *str, char *nameClass, char *name, char *par )
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

int F_ServerConfig::AnalizeRecodrRead(char *name, char *par)
{   static char *lsNames[]=
     { "debugOptions","device",
        "DesktopNx", "DesktopNy","DesktopBackColor","DesktopBackgroundPicture",
        NULL

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
        case 0: /* Debug Options */
           sscanf(par,"%s",&debugOptions);
          break;
        case 1: /* Device: Memory,PM, gradd, etc.*/
           sscanf(par,"%s",&deviceName);
          break;
        case 2:  /* DesktopNx */
           sscanf(par,"%i",&DesktopNx);
         break;
        case 3:  /* DesktopNy */
           sscanf(par,"%i",&DesktopNy);
         break;
        case 4:  /* DesktopBackColor */
           sscanf(par,"%x",&DesktopBackColor);
         break;
        case 5:
           sscanf(par,"%s", DesktopBackgroundPicture);
         break;

   }
    return 0;
}


