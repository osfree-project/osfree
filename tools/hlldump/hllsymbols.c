/* private symbols dumping */

#include <os2.h>

#include <stdio.h>

#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#include "mapsyms.h"
#pragma pack(pop)

void ProcessType104_CL386Syms(ULONG ulModule,FILE *dmp);
void ProcessMs16Syms(ULONG ulModule,FILE *dmp);
void ProcessHlSyms(ULONG ulModule,FILE *dmp);


UCHAR  GetRecordType( UCHAR *Buffer, MODULE *pModule );
USHORT  GetRecordLength( UCHAR *Buffer, MODULE *pModule );
void PrintRegType (UINT n,FILE *dmp);

extern void *pvFile;
extern BYTE bDebugFormat; /* debug format */
extern ULONG ulDebugOff;
extern ULONG ulModules;
extern MODULE *pModules;

BOOL ProcessSymbols()
{
  ULONG i;
  FILE *dmp;

  printf("+++ Processing symbols\n");

  dmp=fopen("symbols.dat","a"); // !!

  for (i=0;i<ulModules;i++)
  {
   /* skip modules with no symbols or zero-length symbol area */
   if ((pModules[i].Symbols==0) || (pModules[i].SymLen==0)) continue;

   fprintf(dmp,"\n\nmodule number: 0x%08x (%u)\n",i+1,i+1);

   ProcessHlSyms(i,dmp);
/*   switch(pModules[i].DbgFormatFlags.Syms)
   {
    case TYPE104_C211:
    case TYPE104_C600:
      ProcessMs16Syms(i,dmp);
      break;                                               

    case TYPE104_CL386:                                    
      ProcessType104_CL386Syms(i,dmp);
      break;                                               

    case TYPE104_HL01:                                     
    case TYPE104_HL02:                                     
    case TYPE104_HL03:                                     
    case TYPE104_HL04:
      MapHLLSyms( RawTable, IntSymTable, mptr );         
      break;                                             
    default:
      fprintf(dmp,"    unsupported (yet) symbol format (%d)\n",pModules[i].DbgFormatFlags.Syms);
   };
*/
  }; //end: for (i=0;i<ulModules;i++)

  fclose(dmp);

  printf("--- Processed\n");
  return TRUE;
};



/* the following is as is, not finished.. */
void ProcessType104_CL386Syms(ULONG ulModule,FILE *dmp)
{
  SSrec *pType=(SSrec*)((UCHAR*)pvFile+ulDebugOff+pModules[ulModule].Symbols);
  LONG lLength=pModules[ulModule].SymLen;

  /* while there are data... */
  while( lLength > 0 )
  {
   fprintf(dmp,"  Symbols record type:   0x%2.2X (%d)",pType->RecType,pType->RecType);
   fprintf(dmp,",  length: 0x%2.2X (%d)\n",pType->RecLen,pType->RecLen);
   lLength-=pType->RecLen;
   switch (pType->RecType)
   {
     
     default: /* we just skip symbol record */
        {
        SSReg32 *reg32ptr = (SSReg32 *)pType;                      
        pType = (SSrec *)((UCHAR *)pType + reg32ptr->RecLen + 1);   
        };
        break;                                                         

   }; //end:  switch (pType->RecType)
  }; //end:  while( ulLength > 0 )

};

void ProcessMs16Syms(ULONG ulModule,FILE *dmp)
{
  SSrec *pType=(SSrec*)((UCHAR*)pvFile+ulDebugOff+pModules[ulModule].Symbols);
  LONG lLength=pModules[ulModule].SymLen;
  UCHAR chNameLen;
  UCHAR szName[256];

  UCHAR *test;

  /* while there are data... */
  while( lLength > 0 )
  {
   test=(UCHAR*)pType;
   fprintf(dmp,"  Symbols record type:   0x%2.2X (%d)",pType->RecType,pType->RecType);
   fprintf(dmp,"  length: 0x%2.2X (%d)\n",pType->RecLen,pType->RecLen);
   lLength-=pType->RecLen;
   switch (pType->RecType)
   {
     case SSPROC:
     {
      SSProc16 * pProc16=(SSProc16 *)pType;

      fprintf(dmp,"    procedure");
      memset(szName,0,256);
      chNameLen=pProc16->Name[0];
      strncpy(szName,&pProc16->Name[1],chNameLen);
      fprintf(dmp,"  %d  %s\n",chNameLen,szName);
       pType = (SSrec *)((UCHAR *)pType + pProc16->RecLen + 1);
     };
     break;

     default: /* we just skip symbol record */
        {
        SSReg16 *reg16ptr = (SSReg16 *)pType;                      
        pType = (SSrec *)((UCHAR *)pType + reg16ptr->RecLen + 1);   
        };
        break;                                                         

   }; //end:  switch (pType->RecType)
  }; //end:  while( ulLength > 0 )
};

void ProcessHlSyms(ULONG ulModule,FILE *dmp)
{
  SSrec *pType=(SSrec*)((UCHAR*)pvFile+ulDebugOff+pModules[ulModule].Symbols);
  LONG lLength=pModules[ulModule].SymLen;
  UCHAR chRecLen,chRecType;

  SHORT chNameLen;
  CHAR szName[256];

  while( lLength > 0 )
  {
    chRecLen     = GetRecordLength( (UCHAR *)pType, &pModules[ulModule] );
    chRecType = GetRecordType( (UCHAR *)pType, &pModules[ulModule] );
    lLength -= (chRecLen + 1);

    switch (chRecType)
    {
      case SSPROC:
      case SSENTRY:
      case SSPROCCPP:
      case SSMEMFUNC:
           {
            SSProcHLL *pProc=(SSProcHLL*)((UCHAR*)pType+2);
            fprintf(dmp,"  Proc:");
            switch (chRecType)
            {
             case SSPROC:
              fprintf(dmp," (PROCEDURE)\n");
             break;
             case SSENTRY:
              fprintf(dmp," (SECONDENTRY)\n");
             break;
             case SSPROCCPP:
              fprintf(dmp," (CPPPROCEDURE)\n");
             break;
             case SSMEMFUNC:
              fprintf(dmp," (MEMBERFUNC)\n");
             break;
            };

        fprintf(dmp,"   Offset=%lu  Type=%u (0x%.4X)  Bytes=%lu\n",
        pProc->ProcOffset, pProc->TypeIndex, pProc->TypeIndex, pProc->ProcLen);
        fprintf(dmp,
                    "              BodyOff=%u  EpilogOff=%u",
                    pProc->DebugStart, pProc->DebugEnd);

            chNameLen=pProc->Name[0];
            memset(szName,0,256);
            if ((sizeof(SSProcHLL)+chNameLen-1)>chRecLen)
              chNameLen=chRecLen-sizeof(SSProcHLL)-1;
          
            strncpy(szName,&pProc->Name[1],chNameLen);
            fprintf(dmp," Name: %s\n",szName);

           };
          break;
      case SSREG:
          {
            SSRegHLL *pReg=(SSRegHLL*)((UCHAR*)pType+2);
            fprintf(dmp,"  SSreg:\n");
            fprintf(dmp, "   Type=%u (0x%.4X)  RegNum=",
                    pReg->TypeIndex, pReg->TypeIndex);

            PrintRegType(pReg->RegNum,dmp);

            chNameLen=pReg->Name[0];
            memset(szName,0,256);
            if ((sizeof(SSRegHLL)+chNameLen-1)>chRecLen)
              chNameLen=chRecLen-sizeof(SSRegHLL)-1;
            strncpy(szName,&pReg->Name[1],chNameLen);
            fprintf(dmp," Name: %s\n",szName);
         };
          break;

      case SSCHGDEF:
          {
            SSChgDefHLL *pChgDef=(SSChgDefHLL*)((UCHAR*)pType+2);
            fprintf(dmp,"  ChDefSeg:\n");
            fprintf(dmp,"    Seg=%u  Reserved=%u\n",pChgDef->SegNum,pChgDef->Reserved);
          };
          break;

      case SSDEF:
          {
           SSDefHLL *pDef=(SSDefHLL*)((UCHAR*)pType+2);
            fprintf(dmp,"  Def:\n");
                  fprintf(dmp, "   FrameOff=%ld  Type=%u (0x%.4X)",
                    pDef->FrameOffset, pDef->TypeIndex, pDef->TypeIndex);

            chNameLen=pDef->Name[0];
            memset(szName,0,256);
            if ((sizeof(SSDefHLL)+chNameLen-1)>chRecLen)
              chNameLen=chRecLen-sizeof(SSDefHLL)-1;
            if (chNameLen<0) chNameLen=0;
            strncpy(szName,&pDef->Name[1],chNameLen);
            fprintf(dmp," Name: %s\n",szName);
          };
          break;
      case SSBEGIN:
          {
           SSBeginHLL *pBegin=(SSBeginHLL*)((UCHAR*)pType+2);
           fprintf(dmp,"  Beginining:\n");
           fprintf(dmp, "   Offset=%lu  Bytes=%lu\n", pBegin->BlockOffset,
                    pBegin->BlockLen);
          };
          break;
      case SSVAR:
      case SSVARCPP:
          {
            SSVarHLL *pVar=(SSVarHLL*)((UCHAR*)pType+2); //+adjust+1
            fprintf(dmp,"  Variable: ");

            switch( chRecType )
            {
             case SSVAR:
               fprintf(dmp,"(normal)\n");;
             break;
             case SSVARCPP:
               fprintf(dmp,"(cpp)\n");;
             break;
            };
 
            fprintf(dmp, "   Addr=%4.4hX:%8.8lX  Type=%u (0x%.4X)",
                 pVar->ObjectNum, pVar->Offset, pVar->TypeIndex, pVar->TypeIndex);

            chNameLen=pVar->Name[0];
            memset(szName,0,256);
            if ((sizeof(SSVarHLL)+chNameLen-1)>chRecLen)
              chNameLen=chRecLen-sizeof(SSVarHLL)-1;
            strncpy(szName,&pVar->Name[1],chNameLen);
            fprintf(dmp," Name: %s\n",szName);
          };
          break;

      case SSUSERDEF:
          {
           SSUserDefHLL *pDef=(SSUserDefHLL*)(SSVarHLL*)((UCHAR*)pType+2); //+adjust+1
           fprintf(dmp,"   UserDef\n");
                  fprintf(dmp, "   Type=%u (0x%.4X)",
                    pDef->TypeIndex, pDef->TypeIndex);

            chNameLen=pDef->Name[0];
            memset(szName,0,256);
            if ((sizeof(SSUserDefHLL)+chNameLen-1)>chRecLen)
              chNameLen=chRecLen-sizeof(SSUserDefHLL)-1;
            strncpy(szName,&pDef->Name[1],chNameLen);
            fprintf(dmp," Name: %s\n",szName);
          };
          break;
      case SSEND:
          fprintf(dmp,"  End...\n");
          break;
      default:
          fprintf(dmp,"  Uknown rec type 0x%2.2x (%d)\n",chRecType,chRecType);
    };
        pType = (SSrec *)((UCHAR *)pType + chRecLen + 1 );    

  }; //END:    while( lLength > 0 )
};

UCHAR  GetRecordType( UCHAR *Buffer, MODULE *pModule )                  /*809*/
{
  UCHAR RecordType;

  /***************************************************************************/
  /* - Set the default record type.                                          */
  /* - Check to see if the 7th bit in the first byte is set and the symbol   */
  /*   type is HL03. If so calculate the record type.                        */
  /* - Return the record type.                                               */
  /***************************************************************************/
  RecordType = *(Buffer + 1);
  if( *Buffer & 0x80 )
  {
//    if( (pModule->DbgFormatFlags.Syms == TYPE104_HL03) ||
//        (pModule->DbgFormatFlags.Syms == TYPE104_HL04)
//      )
      RecordType = *(Buffer + 2);
  }
  return( RecordType );
};

USHORT  GetRecordLength( UCHAR *Buffer, MODULE *pModule )               /*809*/
{
  USHORT RecordLength;

  /***************************************************************************/
  /* - Set the default record length.                                        */
  /* - Check to see if the 7th bit in the first byte is set and the symbol   */
  /*   type is HL03. If so calculate the record length.                      */
  /* - Return the record length.                                             */
  /***************************************************************************/
  RecordLength = *Buffer;
  if( *Buffer & 0x80 )
  {
//    if( (pModule->DbgFormatFlags.Syms == TYPE104_HL03) ||
//        (pModule->DbgFormatFlags.Syms == TYPE104_HL04)
//      )
      RecordLength = ((*Buffer & 0x7F) << 8) + *(Buffer + 1);
  }
  return( RecordLength );
};


void PrintRegType (UINT n,FILE *dmp)
{

  typedef struct recsymnames
    {
      int     code;
      char    name[20];
    };

static struct recsymnames
       RegTypeNames[43] = {
           0x00,   "AL",
           0x01,   "CL",
           0x02,   "DL",
           0x03,   "BL",
           0x04,   "CH",
           0x05,   "DH",
           0x06,   "BH",
           0x07,   "AL",
           0x08,   "AX",
           0x09,   "CX",
           0x0A,   "DX",
           0x0B,   "BX",
           0x0C,   "SP",
           0x0D,   "BP",
           0x0E,   "SI",
           0x0F,   "DI",
           0x10,   "EAX",
           0x11,   "ECX",
           0x12,   "EDX",
           0x13,   "EBX",
           0x14,   "ESP",
           0x15,   "EBP",
           0x16,   "ESI",
           0x17,   "EDI",
           0x18,   "SS",
           0x19,   "CS",
           0x1A,   "SS",
           0x1B,   "DS",
           0x1C,   "FS",
           0x1D,   "GS",
           0x20,   "DX:AX",
           0x21,   "ES:BX",
           0x22,   "IP",
           0x23,   "FLAGS",
           0x24,   "EFLAGS",
           0x80,   "ST(0)",
           0x81,   "ST(1)",
           0x82,   "ST(2)",
           0x83,   "ST(3)",
           0x84,   "ST(4)",
           0x85,   "ST(5)",
           0x86,   "ST(6)",
           0x87,   "ST(7)"
  };

  UINT   i;

  for (i=0; i<43; i++)
    {
      if (n == RegTypeNames[i].code)
        {
          fprintf(dmp, "%s", RegTypeNames[i].name);
          return;
        }
    }

  fprintf(dmp, "%u", n);
}
