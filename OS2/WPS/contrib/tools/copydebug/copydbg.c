#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <share.h>
/* ------------------------------------------------------------------------ */
/* Last 8 bytes of 16:16 or 32 file when CODEVIEW debugging info is present */
_Packed struct  _eodbug
        {
        unsigned short dbug;          /* 'NB' signature */
        unsigned short ver;           /* version        */
        unsigned long dfaBase;        /* size of codeview info */
        } eodbug;
#define         DBUGSIG         0x424E
#define TRANSFERSIZE 0x8000
char Buffer[TRANSFERSIZE];
int  TotalSize;
int  NewOffset=0;
unsigned long lfaBase;
main(int argc, char **argv,char ** envp)
{
   int ModuleFile;
   int DbgFile;
   if (argc==1) {
      printf("Syntax is COPYDBG  name.exe\n");
      printf("       or COPYDBG  name.dll\n");
      printf("It will generate a name.DBG file\n");
      printf("containing the Debug data only if any\n");
      exit(0);
   } /* endif */
   argc--;argv++;
   ModuleFile =sopen(argv[0],O_RDONLY|O_BINARY,SH_DENYNO);
   if (ModuleFile==-1) {
      perror("Error while opening file :");
      exit(1);
   }
    /* See if any CODEVIEW info */
    if ((TotalSize=lseek(ModuleFile,-8L,SEEK_END))==-1) {
        printf("Error %u seeking CodeView table in %s\n",errno,argv[0]);
        return(18);
    }

    if (read(ModuleFile,(void *)&eodbug,8)==-1) {
       printf("Error %u reading debug info from %s\n",errno,argv[0]);
       return(99);
    }
    if (eodbug.dbug!=DBUGSIG) {
       printf("\nNo CodeView information stored.\n");
       return(99);
    }
   strcpy(argv[0]+strlen(argv[0])-3,"DBG"); /* Build DBG File name */
   DbgFile =open(argv[0],O_CREAT |O_WRONLY | O_TRUNC|O_BINARY,S_IREAD | S_IWRITE);
   if (DbgFile==-1) {
      perror("Error while opening file :");
      exit(1);
   }
    if ((lfaBase=lseek(ModuleFile,-eodbug.dfaBase,SEEK_END))==-1L) {
      perror("Error seeking beginning of debug data :");
      return(18);
   }
   TotalSize+=8; /* Include the NB0x and offset */
   TotalSize-=lfaBase;
   while (TotalSize>0) {
      int ReadBytes;
      ReadBytes=read( ModuleFile ,(void *)Buffer,min(TRANSFERSIZE,TotalSize));
      if (ReadBytes>0) {
         TotalSize-=ReadBytes;
         write(DbgFile,(void *)Buffer,ReadBytes);
      } /* endif */
   } /* endwhile */
   close(DbgFile);
   close(ModuleFile);
}

