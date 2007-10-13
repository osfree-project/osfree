/* MS data types dumping */

#include <os2.h>

#include <stdio.h>
#include <string.h>
#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#include "$$TYPES.H"
#include "MAPTYPES.H"
#pragma pack(pop)

extern void *pvFile;
extern BYTE bDebugFormat; /* debug format */
extern ULONG ulDebugOff;
extern ULONG ulModules;
extern MODULE *pModules;


void ProcessHLLTypes(ULONG ulModule,FILE *dmp);
void ProcessMsTypes(ULONG ulModule,FILE *dmp);

void PrintTypeIndex (USHORT n,FILE *dmp);
UCHAR *GetField( UCHAR *buffer, void *FieldValue, UINT *FieldLength );

BOOL ProcessTypes()
{
  ULONG i;
  FILE *dmp;
  printf("+++ Processing data types\n");

  dmp=fopen("types.dat","a"); // !!

  for (i=0;i<ulModules;i++)
  {
   if ((pModules[i].TypeDefs==0)||(pModules[i].TypeLen==0)) continue;

   fprintf(dmp,"\n\nmodule number: 0x%08x (%u)\n",i+1,i+1);

   ProcessMsTypes(i,dmp);
//   ProcessHLLTypes(i,dmp);

  }; //end: for (i=0;i<ulModules;i++)

  fclose(dmp);

  printf("--- Processed\n");
  return TRUE;
};

void ProcessMsTypes(ULONG ulModule,FILE *dmp)
{
  int RecIDLen=1;
  UCHAR *endOffset;
  Trec *pType;
  USHORT chNameLen;
  CHAR szName[256];
  USHORT chRecLen;
  UCHAR *ptr;

   pType=(Trec*)((UCHAR*)pvFile+ulDebugOff+pModules[ulModule].TypeDefs+1);
   endOffset=(UCHAR*)((UCHAR*)pType+pModules[ulModule].TypeLen);

  while ((UCHAR*)pType<endOffset)
  {
    chRecLen=pType->RecLen;

    switch( pType->RecType )
    {
     case T_NULL: /* Null-type? skipping... */
      break;
     case T_STRUCT:
      {
        MS_STRUCT *pMSStruct;
        UINT   FieldLength;
        UINT   BitLen = 0;
        USHORT members=0;

        fprintf(dmp,"  Structure\n");
        ptr=((UCHAR*)pType+sizeof(Trec));
        ptr = GetField( ptr, (void *)&BitLen,
                             &FieldLength );
        fprintf(dmp,"   Size:     %d\n",BitLen/8);
        ptr = GetField( ptr, (void *)&(members),
                             &FieldLength );
        fprintf(dmp,"   Members:  %d\n",members);

        pMSStruct = (MS_STRUCT *)ptr;

        fprintf(dmp,"   TypeListIndex: ");
        PrintTypeIndex(pMSStruct->TypeListIndex,dmp);
        fprintf(dmp,"\n   NameListIndex: ");
        PrintTypeIndex(pMSStruct->NameListIndex,dmp);

            chNameLen=pMSStruct->NameLen;
            memset(szName,0,256);
            strncpy(szName,&pMSStruct->Name[0],chNameLen);
            fprintf(dmp,"\n    Name: %s\n",szName);

      };  
      break;
     case T_LIST:
     {
        UCHAR FIDIndex = *((UCHAR *)pType + 3);

        fprintf(dmp,"  List\n");
        ptr=((UCHAR*)pType+sizeof(Trec)+1);
        switch( FIDIndex )
        {
         case 0x83: /* type list */
         {
          int i;
          LONG lRecLen=chRecLen-2;
          fprintf(dmp,"   type...:\n");
          while (lRecLen>0)
          {
            if (*((USHORT*)ptr)==0 )
              fprintf(dmp,"     VOID\n");
            else
            {
             fprintf(dmp,"      %d\n",*((USHORT*)ptr));
            };
             ptr=((UCHAR*)ptr+3);
             lRecLen-=3;
          }; // while (lRecLen>0)
         }; //end case type list
         break;
         case 0x82: /* name list */
         {
          USHORT  AdjustLen;
          LONG lRecLen=chRecLen-2;
          USHORT usNameLen;
          UINT    OffsetLen;
          ULONG   Offset;

          fprintf(dmp,"   name...:\n");
          while (lRecLen>0)
          {
            usNameLen=*ptr;

            if (usNameLen==0) break;

            ptr++;
            lRecLen--;

            memset(szName,0,256);
            strncpy(szName,ptr,usNameLen);
            fprintf(dmp,"      %s\n",szName);

            ptr+=usNameLen;
            lRecLen-=usNameLen;

            Offset = 0;
            AdjustLen = 0;
            if( *ptr & 0x80 )
              AdjustLen = 1;

            ptr = GetField( ptr, (void *)&Offset, &OffsetLen );

              lRecLen -= (OffsetLen + AdjustLen);

              ptr++;
              lRecLen--;
		
          }; //while lRecLen>0
         }; //end case name list
         break;
        };
     };
     break;
     case T_PTR:/* Pointer. */
     {    
        /* WARNING: we do not support recognizing 16:16 from 0:32 pointers (yet) */
        MS_POINTER *pMSPointer = (MS_POINTER *)((UCHAR *)pType + 3);
        USHORT usNameLen=pMSPointer->NameLen;	
        fprintf(dmp,"  Pointer:\n");
        fprintf(dmp,"     TypeIndex:\n",pMSPointer->TypeIndex);
            memset(szName,0,256);
            strncpy(szName,pMSPointer->Name,usNameLen);
            fprintf(dmp,"      %s\n",szName);

     };
     break;

     case T_PROC:     /* Procedure.                 (0x75) */
     case T_ENTRY:    /* Entry.                     (0x53) */
     case T_FUNCTION: /* Function.                  (0x54) */
     {
        #define  ARGSRTOL   0x01
        #define  CALLERPOPS 0x02
        #define  FUNC32BIT  0x04
        #define  FARCALL    0x08

        UCHAR  CallingConv;
        MS_PROC *pMSProc  = (MS_PROC *)((UCHAR *)pType + 3);

        fprintf(dmp,"  Procedure:\n");

        if( pMSProc->FID_ReturnType == FLDID_VOID )
        {
          UCHAR *RecordPtr = (UCHAR *)&(pMSProc->ReturnType);

          CallingConv = *RecordPtr;
          fprintf(dmp,"   Retrun type:    VOID\n");
          fprintf(dmp,"   NumParams:      %d\n",*(RecordPtr + 1));
          fprintf(dmp,"   ParamListIndex: %d\n",*(USHORT *)(RecordPtr + 3));
        }
        else
        {
          CallingConv = pMSProc->CallConv;
          fprintf(dmp,"   Retrun type:    %d\n",pMSProc->ReturnType);
          fprintf(dmp,"   NumParams:      %d\n",pMSProc->NumParams);
          fprintf(dmp,"   ParamListIndex: %d\n",pMSProc->ParmListIndex);
        };

        switch( CallingConv )
        {
          case 0x63:
            fprintf(dmp,"   ARGSRTOL\n");
            fprintf(dmp,"   CALLERPOPS\n");
            break;

          case 0x96:
            fprintf(dmp,"   FARCALL\n");
            break;

          default:
            break;
        }

     };
     break;

     case T_BITFLD: /* Bit Field. */
     {
      #define SIGNED_FLAG     0x02
      MS_BITFLD *pMSBitfld  = (MS_BITFLD *)((UCHAR *)pType + 3);

        fprintf(dmp,"  Bitfield:\n");
        fprintf(dmp,"   Offset: %d\n",pMSBitfld->Offset);
        fprintf(dmp,"   BitSize: %d\n",pMSBitfld->BitSize);

        switch( pMSBitfld->BaseType )
        {
          case 0x7D:
          case 0x7C:
            if( pModules[ulModule].DbgFormatFlags.Typs == TYPE103_CL386 )
          fprintf(dmp,"   UINT/ULONG\n");
            break;
          case 0x6F:
          fprintf(dmp,"   UCHAR\n");
            break;
        };

     };
     break;

     case T_ARRAY:
     {
        MS_ARRAY *pMSArray;
        UINT   FieldLength;
        UINT   BitLen = 0;

        fprintf(dmp,"  Array\n");
        ptr=((UCHAR*)pType+3);
        ptr = GetField( ptr, (void *)&BitLen,
                             &FieldLength );

        if( FieldLength > 1 )
          FieldLength++;

        fprintf(dmp,"   Size:             %d\n",BitLen/8);
        pMSArray = (MS_ARRAY *)ptr;
        fprintf(dmp,"   Element type:     %d\n",pMSArray->ElemType);

     if( pType->RecLen > (FieldLength + 5) )
        {

        chNameLen=pMSArray->NameLen;	
            memset(szName,0,256);
            strncpy(szName,pMSArray->Name,chNameLen);
            fprintf(dmp,"      %s\n",szName);
      };
     };
     break;

     default:
        fprintf(dmp,"  Unknown rec type 0x%2.2x (%d)\n",pType->RecType,pType->RecType);
    }; //end: switch( pType->RecType )

    pType = (Trec *)((UCHAR *)pType + pType->RecLen + 2 + RecIDLen);

  }; //end:   while ((UCHAR*)pType<endOffset)

};

void ProcessHLLTypes(ULONG ulModule,FILE *dmp)
{
  int RecIDLen;
  UCHAR *endOffset;
  Trec *pType;
  USHORT chNameLen;
  CHAR szName[256];
  UCHAR chRecLen;


//  RecIDLen = 1; /* I'm not sure should we use thios or 0... */

/*  if( (pModule->DbgFormatFlags.Typs == TYPE104_HL03) ||
      (pModule->DbgFormatFlags.Typs == TYPE104_HL04)
    )*/
    RecIDLen = 0;

   pType=(Trec*)((UCHAR*)pvFile+ulDebugOff+pModules[ulModule].TypeDefs);
   endOffset=(UCHAR*)((UCHAR*)pType+pModules[ulModule].TypeLen);

  while ((UCHAR*)pType<endOffset)
  {
    chRecLen=pType->RecLen;

    switch( pType->RecType )
    {
/*     case T_PTR:
     {
       HL_POINTER *pPtr = (HL_POINTER *)((UCHAR *)pType+3);
       fprintf(dmp,"  Pointer\n");
       fprintf(dmp,"    TypeIndex:     0x%x (%d)\n",pPtr->TypeIndex,pPtr->TypeIndex);
       fprintf(dmp,"    TypeQual:      0x%x (%d)\n",pPtr->TypeQual,pPtr->TypeQual);
       fprintf(dmp,"    FID_TypeIndex: 0x%x (%d)\n",pPtr->FID_TypeIndex,pPtr->FID_TypeIndex);       fprintf(dmp,"    FID_Name:      0x%x (%d)\n",pPtr->FID_Name,pPtr->FID_Name);
        PrintTypeIndex(pPtr->TypeIndex,dmp);
            chNameLen=pPtr->NameLen;
            memset(szName,0,256);
//            if ((sizeof(HL_USERDEF)+chNameLen)>chRecLen)
//              chNameLen=chRecLen-sizeof(HL_USERDEF);
            strncpy(szName,&pPtr->Name[0],chNameLen);
            fprintf(dmp,"    Name: (%d) %s\n",chNameLen,szName);

     };
     break;*/
/*     case T_TYPDEF:
     {
      HL_USERDEF *pDef=((HL_USERDEF*)pType+3);

       fprintf(dmp,"  TypeDef\n");
       fprintf(dmp,"    TypeIndex:     0x%x (%d)\n",pDef->TypeIndex,pDef->TypeIndex);
       fprintf(dmp,"    TypeQual:      0x%x (%d)\n",pDef->TypeQual,pDef->TypeQual);
       fprintf(dmp,"    FID_TypeIndex: 0x%x (%d)\n",pDef->FID_TypeIndex,pDef->FID_TypeIndex);
       fprintf(dmp,"    FID_Name:      0x%x (%d)\n",pDef->FID_Name,pDef->FID_Name);



            chNameLen=pDef->NameLen;
            memset(szName,0,256);
//            if ((sizeof(HL_USERDEF)+chNameLen)>chRecLen)
//              chNameLen=chRecLen-sizeof(HL_USERDEF);
            strncpy(szName,&pDef->Name[0],chNameLen);
            fprintf(dmp,"    Name: (%d) %s\n",chNameLen,szName);
      
     };
     break;*/
     case T_STRUCT:
     {
       HL_STRUCT *pStruct = ((HL_STRUCT *)pType+3);
       fprintf(dmp,"  Structrure\n");
       fprintf(dmp,"    Size:                 %u\n",pStruct->ByteSize);
       fprintf(dmp,"    N:                    %d\n",pStruct->NumMembers);
       fprintf(dmp,"    NameListIndex:        %d\n",pStruct->NameListIndex);
       fprintf(dmp,"    TypeListIndex:        %d\n",pStruct->TypeListIndex);
       fprintf(dmp,"    FID_NameListIndex:    %d\n",pStruct->FID_NameListIndex);
       fprintf(dmp,"    FID_TypeListIndex:    %d\n",pStruct->FID_TypeListIndex);

        PrintTypeIndex(pStruct->NameListIndex,dmp);
        PrintTypeIndex(pStruct->TypeListIndex,dmp);

            chNameLen=pStruct->NameLen;
            memset(szName,0,256);
            if ((sizeof(HL_STRUCT)+chNameLen)>chRecLen)
              chNameLen=chRecLen-sizeof(HL_STRUCT);
            strncpy(szName,&pStruct->Name[0],chNameLen);
            fprintf(dmp,"    Name: %s\n",szName);

     };
     break;

     default:
        fprintf(dmp,"  Unknown rec type 0x%2.2x (%d)\n",pType->RecType,pType->RecType);
    }; //end: switch( pType->RecType )

    pType = (Trec *)((UCHAR *)pType + pType->RecLen + 2 + RecIDLen);

  }; //end:   while ((UCHAR*)pType<endOffset)
};


void PrintTypeIndex (USHORT n,FILE *dmp)
{

  typedef struct recsymnames
    {
      int     code;
      char    name[20];
    };

static struct recsymnames
       PrimTypeNames[9] = {
           0x80,   "CHAR",
           0x81,   "SHORT",
           0x82,   "LONG",
           0x84,   "UCHAR",
           0x85,   "USHORT",
           0x86,   "ULONG",
           0x88,   "FLOAT",
           0x89,   "DOUBLE",
           0x97,   "VOID"
  };

  UCHAR *cp;
  USHORT baseN, primN, i;

  baseN = (n & 0x0060) >> 5;
  primN = n & 0xFF9F;

  for (i=0; i<9; i++)
    {
      if (primN == PrimTypeNames[i].code)
        {
          fprintf(dmp, "%s", PrimTypeNames[i].name);
          switch (baseN)
            {
              case 0x01:
                fprintf(dmp, "* NEAR");
                break;
              case 0x02:
                fprintf(dmp, "* FAR");
                break;
              case 0x03:
                fprintf(dmp, "* HUGE");
                break;
            }
          return;
        }
    }

  fprintf(dmp, "%u", n);
}

UCHAR *GetField( UCHAR *buffer, void *FieldValue, UINT *FieldLength )
{
  if( *buffer & 0x80 )
  {
    switch( *buffer )
    {
      case 0x88:
        *((char *)FieldValue) = *((char *)(buffer + 1));
        *FieldLength = 1;
        return( buffer + 2 );

      case 0x8B:
        *((UCHAR *)FieldValue) = *(buffer + 1);
        *FieldLength = 1;
        return( buffer + 2 );

      case 0x85:
        *((SHORT *)FieldValue) = *((USHORT *)(buffer + 1));
        *FieldLength = 2;
        return( buffer + 3 );

      case 0x89:
        *((USHORT *)FieldValue) = *((USHORT *)(buffer + 1));
        *FieldLength = 2;
        return( buffer + 3 );

      case 0x86:
        *((LONG *)FieldValue) = *((LONG *)(buffer + 1));
        *FieldLength = 4;
        return( buffer + 5 );

      case 0x8A:
        *((ULONG *)FieldValue) = *((ULONG *)(buffer + 1));
        *FieldLength = 4;
        return( buffer + 5 );
    }
  }
  else
  {
    *((UCHAR *)FieldValue) = *buffer;
    *FieldLength = 1;
    return( buffer + 1 );
  }
  return(0);
}
