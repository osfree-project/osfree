/*
 $Id: Fs_config.hpp,v 1.1 2003/06/15 17:45:47 evgen2 Exp $
*/
/* FreePM Server config */

class F_ServerConfig
{
   public:
/* Desktop defaults */
   char deviceName[80];
   int DesktopNx;
   int DesktopNy;
   int BytesPerPel;
   int DesktopBackColor;
   char DesktopBackgroundPicture[_MAX_PATH];
   char debugOptions[256];
   F_ServerConfig()
   {
      DesktopNx = 640;
      DesktopNy = 480;
      BytesPerPel = 3;
      DesktopBackColor = 0x606060;
      DesktopBackgroundPicture[0]=0;
      strcpy(deviceName,"PM");
      strcpy(debugOptions,"ALL,7");
   }
   int Read(char *fname);
   int Write(char *fname);
   int AnalizeRecodrRead(char *name, char *par);
   int ReadStr(FILE *fp, char *str, char *nameClass, char *name, char *par );

};
