/* LVM_Cli.c */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_VIO
#define INCL_DOSPROCESS
#include <os2.h>

#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "strings.h"

#include "logging.h"
#include "LVM_Cli.h"



int Quit( char *str)
{
  printf("todo:%s\n", __FUNCTION__);
  LOG_EVENT( "Quit" )
  exit(1);
  return 0;
}

int ParkCursor(void)
{
  HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
  int rc;
  rc = VioSetCurPos(0, 0, VioHandle);
//??
  return 0;
}


char *SaveString(char *str)
{
 // printf("todo:%s\n", __FUNCTION__);
  return strdup(str);
}

int SetBootmanOptions(ADDRESS  volume_handle)
{
  printf("todo:%s\n", __FUNCTION__);
  return 0;
}

uint ConstructBootmanPanels ( uint   row,  uint   column)
{
  printf("todo:%s\n", __FUNCTION__);
  return 0;
}

int  DisplayAttributes (int row, int column, int row_length, int  number_of_rows, int attr)
{
  HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
  int i,r,rc;
  char attr2[2], *pattr;
  pattr = &attr2[0];
  for (r = row; r < row+number_of_rows; r++)
  {  attr2[1] = attr;
     attr2[0] = 0;
     rc = VioWrtNCell(pattr, row_length, r, column,VioHandle);
     if(rc)
     {   printf("Error in %s VioWrtCellStr rc=%i\n", __FUNCTION__,rc);
        return NULL;
     }
  }

  return 0;
}

int  DisplayString (char *string, int row, int column, int len )
{
    APIRET    rc;         /*  Return code. */
    HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
    char str[160];
    int l;
    l = strlen(string);
    if(len >= sizeof(str)-1) len = sizeof(str)-1;
    memset(str, ' ', len);
    strncpy(str,string,l);
    rc = VioWrtCharStr(str, len, row, column, VioHandle);
    if(rc)
    {   printf("Error in %s VioWrtCharStr rc=%i, r=%i,c=%i,text=%s\n", __FUNCTION__,rc, row,column,string);
        return 1;
    }

//printf(">%s< %i %i\n",str,row,column);
  return 0;
}

int  DisplayStringWithAttribute (char *string, int row, int column, int len, int attr )
{
  HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
  char attr2[2];
  int rc;
  char str[160];
  int l;
  l = strlen(string);
  if(len >= sizeof(str)-1) len = sizeof(str)-1;
  memset(str, ' ', len);
  strncpy(str,string,l);
  attr2[0] = attr;

   rc = VioWrtCharStrAtt(str, len, row, column, &attr2, VioHandle);

  return 0;
}

int  DisplayChars(char ch, int row, int column, int row_length, int b )
{
  HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
  int i,r,l,rc;
  char attr[2];
  attr[0] = ch;

  rc = VioWrtNChar(attr, row_length, row, column, VioHandle);
  if(rc)
   {   printf("Error in %s VioWrtNChar rc=%i\n", __FUNCTION__,rc);
      return rc;
   }

  return 0;
}


int  DisplayCharsWithAttribute(char ch, int row, int column, int row_length, int  number_of_rows, int attr )
{
  HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
  int i,r,l,rc;
  char attr2[2];
  attr2[0] = ch;
  attr2[1] = attr;

  for(i=0; i<number_of_rows;i++)
  { r = row+i;
    rc = VioWrtNCell(&attr2, row_length, r, column,VioHandle);
  }
  return 0;
}

uint ReverseAttributes ( uint row, uint column,uint max_string_length, uint bb )
{
  HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
  int i,r,rc;
  unsigned short l;
  char attr2[2], *pmem, ch;
  pmem = calloc(2, max_string_length);
  l = max_string_length*2;
  rc = VioReadCellStr(pmem, &l, row,column, VioHandle);
  for (i = 0; i < l; i+=2)
  { ch = pmem[i+1];
    pmem[i+1] = (ch>>4)|((ch&0xf)<<4);
  }
  rc = VioWrtCellStr(pmem, l, row, column,VioHandle);
  free(pmem);

  return 0;
}

void * SaveDisplayBlock (int row, int column, int row_length, int  number_of_rows)
{
  APIRET    rc;         /*  Return code. */
  HVIO      VioHandle=0;/*  VIO presentation-space handle. */
  PCH       CellStr;    /*  Cell string buffer. */
  USHORT     Length;     /*  Length of cell string buffer. */
  int i,r;
  unsigned short l;

  Length = row_length * number_of_rows *2;
  CellStr = calloc(Length,1);
  for(i=0; i<number_of_rows;i++)
  { r = row+i;
    l = row_length*2;
    rc = VioReadCellStr(&CellStr[i*row_length*2], &l, r,column, VioHandle);
    if(rc)
     {   printf("Error in %s VioReadCellStr rc=%i\n", __FUNCTION__,rc);
        return NULL;
     }
  }

  return CellStr;
}

void * RestoreDisplayBlock(int row, int column, int row_length, int  number_of_rows, char *buf)
{
  APIRET    rc;          /*  Return code. */
  HVIO      VioHandle=0; /*  VIO presentation-space handle. */
  USHORT    Length;      /*  Length of cell string buffer. */
  int i,r;

  Length = row_length * number_of_rows *2;
  for(i=0; i<number_of_rows;i++)
  {  r = row+i;
     rc = VioWrtCellStr(&buf[i*row_length*2], row_length*2, r, column,VioHandle);
     if(rc)
     {   printf("Error in %s VioReadCellStr rc=%i\n", __FUNCTION__,rc);
        return NULL;
     }
  }
  free(buf);
  return NULL;
}

uint InitializePanels ( uint default_attribute, uint exit_attribute )
{
  HVIO      VioHandle=0;  /*  VIO presentation-space handle. */
  int i,r,col,rc, attr;
  char attr2[2];

  GetScreenRows ( );
// printf("todo:%s %i %i\n", __FUNCTION__,Screen_rows,Screen_columns);

  for(attr=0; attr < Screen_rows*Screen_columns; attr++)
  {  r = attr/Screen_columns;
     col = attr%Screen_columns;
     attr2[0] = (32+attr)%256;
     attr2[1] = attr;
     rc = VioWrtNCell(&attr2, 1, r, col,VioHandle);
  }

  return 0;
}

void
ReInitializePanels ( uint default_attribute, uint exit_attribute )
{ printf("todo:%s\n", __FUNCTION__);
}

/*
uint DataEntryPanel ( panel_t  *panel )
{ printf("todo:%s\n", __FUNCTION__);
  return 0;
}
*/

uint InputName (struct _panel *Partition_name_panel, char *Partition_name_input,int Partition_name_width, char *partition_name )
{
    uint   key = ESCAPE;
    int l, i, rc;
    char ch;
//    bool    finished = FALSE,

    MESSAGE_BAR_PANEL ( Enter_name_message_line );
    l = Partition_name_width;
    i = strlen(Partition_name_input);
    if(l > i) l = i;
    for(i=0; i<l; i++)
    {  ch = partition_name[i];
       if(ch == 0) break;
       (*Partition_name_panel->input_line)[i] = ch;
    }

//partition_name-> ? , Partition_name_width
    key = DataEntryPanel ( Partition_name_panel );
    if(key == '\r')
    {
      l = Partition_name_width;
      i = strlen(Partition_name_input);
      if(l > i) l = i;
      i = strlen((*Partition_name_panel->input_line));
      if(i < l) l = i;
      if(l > i) l = i;
      for(i=0; i<l; i++)
      {  ch = (*Partition_name_panel->input_line)[i];
         if(ch == 0) break;
         Partition_name_input[i] = ch;
      }
    }
//Partition_name_input <- ?
    return key;

  return 0;
}

uint GetScreenRows ( void )
{
  VIOMODEINFO    ModeData;   /*  Mode characteristics. */
  HVIO            VioHandle=0;  /*  VIO presentation-space handle. */
  APIRET          rc;         /*  Return code. */

  ModeData.cb = sizeof(VIOMODEINFO);

  rc = VioGetMode(&ModeData, VioHandle);
  if(rc)
  {   printf("Error in %s VioGetMode rc=%i\n", __FUNCTION__,rc);
      return 25;
  }
  Screen_rows    =  ModeData.row;
  Screen_columns =  ModeData.col;

  return ModeData.row;
}

uint GetScreenColumns ( void )
{
  VIOMODEINFO    ModeData;   /*  Mode characteristics. */
  HVIO            VioHandle=0;  /*  VIO presentation-space handle. */
  APIRET          rc;         /*  Return code. */

  ModeData.cb = sizeof(VIOMODEINFO);

  rc = VioGetMode(&ModeData, VioHandle);
  if(rc)
  {   printf("Error in %s VioGetMode rc=%i\n", __FUNCTION__,rc);
      return 25;
  }
  Screen_rows    =  ModeData.row;
  Screen_columns =  ModeData.col;

  return Screen_columns;
}



void *AllocateOrQuit ( uint n, uint size )
{ void *pmem;
  pmem = calloc(n, size);
  if(pmem == NULL)
  {
    printf("Error in %s calloc(%i,%i) return NULL\n", __FUNCTION__,n,size);
    exit(1);
  }
  return pmem;
}

void *ReallocStringArray (void * item, uint n )
{ void *pmem;
  char **p;
  n++;
  if(item == NULL)
  {  // if(n == 0) return NULL;
      pmem = calloc(n, sizeof(char *));
  } else {
      pmem = realloc(item,n * sizeof(char *));
  }
  if(pmem == NULL)
  {
      printf("Error in %s calloc(%i,%i) return NULL\n", __FUNCTION__,n,sizeof(char *));
      exit(1);
  }
  p = (char **)pmem;
  p[n-1] = NULL;
  return pmem;
}

int GetKeystroke2( void );

int GetKeystroke ( void )
{  int rc,rc2,i;
   int isKey;
   char str[80];
   rc =GetKeystroke2( );
   sprintf(str,"Key read: %i (%x)",rc, rc);
  LOG_EVENT( str )

   return  rc;
/*

   for(i=0;i<1000;i++)
   { isKey = 0;
     if(_kbhit())
     {  rc = _getch();
        if(_kbhit())
        { rc = _getch();
         if(rc){ rc += 500; isKey = 1; }
        }
        break;
     } else {
          DosSleep(100);
     }
  }
  return rc;
*/
}
