/* HL03 & 109_32 type lines dumping  */

#include <os2.h>

#include <stdio.h>

#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#pragma pack(pop)

extern void *pvFile;
extern BYTE bDebugFormat; /* debug format */
extern ULONG ulDebugOff;
extern ULONG ulModules;
extern MODULE *pModules;

/* local functions */

void ProcessHL03Lines(ULONG ulModule,FILE *dmp);
void Process109_32Lines(ULONG ulModule,FILE *dmp);



BOOL ProcessLines()
{
  ULONG i;
  FILE *dmp;
  printf("+++ Processing line numbers\n");

  dmp=fopen("lines.dat","a"); // !!

  for (i=0;i<ulModules;i++)
  {
   if ((pModules[i].LineNums==0) || (pModules[i].LineNumsLen==0)) continue;

   fprintf(dmp,"\n\nmodule number: 0x%08x (%u)\n",i+1,i+1);

   switch(pModules[i].DbgFormatFlags.Lins)
   {
     case TYPE10B_HL03:
       ProcessHL03Lines(i,dmp);
     break;
     case TYPE109_32:
       Process109_32Lines(i,dmp);
     break;
     default:
      fprintf(dmp,"    unsupported (yet) line format (%d)\n",pModules[i].DbgFormatFlags.Lins);
   };

  }; //end: for (i=0;i<ulModules;i++)

  fclose(dmp);

  printf("--- Processed\n");
  return TRUE;
};

void ProcessHL03Lines(ULONG ulModule,FILE *dmp)
{
  UCHAR *pLineNumberTable=((UCHAR*)pvFile+ulDebugOff+pModules[ulModule].LineNums);
  UCHAR *pLineNumberTableEnd=pLineNumberTable+pModules[ulModule].LineNumsLen;
  ULONG i,j;
  PHL03FIRSTENTRY pFirstEntry;
  PLINE_NUMBER_TABLE_ENTRY_HL03 pLineEntry;
  PFILE_NAME_TABLE_ENTRY_HL03 pFileEntry;
  static char szName[256];
  UCHAR *fileName;
  UCHAR chNameLen;

  pFirstEntry=(PHL03FIRSTENTRY)pLineNumberTable;
  pLineEntry=(PLINE_NUMBER_TABLE_ENTRY_HL03)((UCHAR*)pFirstEntry +sizeof(HL03FIRSTENTRY));

  fprintf(dmp,"  First entry:\n");
  fprintf(dmp,"    Line Number       0x%04x (%d)\n"
              "    Entry type        0x%04x (%d)\n"
              "    reserved          0x%04x (%d)\n"
              "    Number of entries 0x%04x (%d)\n"
              "    Object            0x%04x (%d)\n"
              "    Size of FileName Table 0x%08x (%u)\n",
              pFirstEntry->LineNumber, pFirstEntry->LineNumber,
              pFirstEntry->EntryType,pFirstEntry->EntryType,
              pFirstEntry->Reserved,pFirstEntry->Reserved,
              pFirstEntry->NumEntries,pFirstEntry->NumEntries,
              pFirstEntry->SegNum,pFirstEntry->SegNum,
              pFirstEntry->FileNameTableSize,pFirstEntry->FileNameTableSize);

  fprintf(dmp,"   Line info\n");
 
  for (i=0;i<pFirstEntry->NumEntries;i++)
    fprintf(dmp,"       LineNumber=%4d (0x%03x)  FileIndex=%4d (0x%04x)  Offset=0x%08x (%u)\n",
           pLineEntry[i].LineNumber,pLineEntry[i].LineNumber,
           pLineEntry[i].SourceFileIndex,pLineEntry[i].SourceFileIndex,
           pLineEntry[i].Offset,pLineEntry[i].Offset);

  pFileEntry=(PFILE_NAME_TABLE_ENTRY_HL03)((UCHAR*)pLineEntry+
    pFirstEntry->NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL03));

  fprintf(dmp,"   File info");

  fprintf(dmp,"(StartOfSource: %u, Records: %u, Files: %u)\n",
      pFileEntry->StartOfSource,
      pFileEntry->NumberOfSourceRecords,
      pFileEntry->NumberOfSourceFiles);

  fileName=((UCHAR*)pLineEntry+
    pFirstEntry->NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL03)+sizeof(FILE_NAME_TABLE_ENTRY_HL03)-1);
 for (j=0;j<pFileEntry->NumberOfSourceFiles;j++)
 {
  chNameLen=*fileName++;
  memset(szName,0,256);
  strncpy(szName,fileName,chNameLen);
  fprintf(dmp," %u, %s\n",j+1,szName);
  fileName+=chNameLen;
 };

};

void Process109_32Lines(ULONG ulModule,FILE *dmp)
{
  UCHAR *pLineNumberTable=((UCHAR*)pvFile+ulDebugOff+pModules[ulModule].LineNums);
  UCHAR *pLineNumberTableEnd=pLineNumberTable+pModules[ulModule].LineNumsLen;
  ULONG i,j;
  PFIRST_ENTRY_109_32 pFirstEntry;
  PLINE_NUMBER_TABLE_ENTRY_109_32 pLineEntry;

  static char szName[256];
  UCHAR *fileName=((UCHAR*)pLineNumberTable+1);
  CHAR chNameLen=*pLineNumberTable;

  pLineNumberTable+=chNameLen+1;

  pFirstEntry=(PFIRST_ENTRY_109_32)pLineNumberTable;
  pLineEntry=(PLINE_NUMBER_TABLE_ENTRY_109_32)((UCHAR*)pFirstEntry +sizeof(FIRST_ENTRY_109_32));


  fprintf(dmp,"  First entry:\n");
  fprintf(dmp,"    Number of entries 0x%04x (%d)\n"
              "    Object            0x%04x (%d)\n",
              pFirstEntry->NumEntries,pFirstEntry->NumEntries,
              pFirstEntry->SegNum,pFirstEntry->SegNum);
  

  fprintf(dmp,"   Line info\n");
 
  for (i=0;i<pFirstEntry->NumEntries;i++)
    fprintf(dmp,"       LineNumber=%4d (0x%2x) FileIndex=%4d (0x%04x) Offset=0x%08x (%u)\n",
           pLineEntry[i].LineNumber,pLineEntry[i].LineNumber,
           1,1,
           pLineEntry[i].Offset,pLineEntry[i].Offset);


  fprintf(dmp,"   File info\n");

  memset(szName,0,256);
  strncpy(szName,fileName,chNameLen);
  fprintf(dmp,"    1, %s\n",szName);

};
