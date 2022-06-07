/* interface.c  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCL_DOS
#define INCL_VIO
#define INCL_KBD
#include <os2.h>
#include "constant.h"
#include "display.h"
#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "edit.h"
#include "logging.h"

#include "interface.h"

HVIO hvio=0;                   // чисто текстовый режим
VIOMODEINFO ScreenData, OldScreenData;
VIOCURSORINFO Ins[2],Save, *mAcurs;  // Ins[0] - insert mode; Ins[1] - overwrite
int CELL_SIZE;    // размер ячейки экрана в байтах <attrib,smb>

HELP2 *HELP_SYSTEM=NULL;

HKBD hkbd=0;
KBDKEYINFO KeyInput;
CHF *Kbd_Chain_of_FLT=NULL;              // очередь клавиатурных фильтров

TIMES Times;           // служба отображения времени (цикл 1 сек)
KEY_BAR KeyBarHelp;      // служба поддержки key_bar на shift регистрах (цикл .5 сек)
/***************************************************************************************/
void Kbd_Wait()
{
   char str[80];

beg:
   KbdCharIn(&KeyInput,0,hkbd);
   if(KeyInput.chChar >= 32)
   sprintf(str,"KeyInput:fbStatus (%x) chScan (%x) chChar (%c=%x)",
          KeyInput.fbStatus, KeyInput.chScan,KeyInput.chChar,KeyInput.chChar);
   else
   sprintf(str,"KeyInput:fbStatus (%x) chScan (%x) chChar (%x)",
          KeyInput.fbStatus, KeyInput.chScan,KeyInput.chChar);
  LOG_EVENT( str )
//
   if (HELP_SYSTEM && Kbd_GetCode() == F1 && Kbd_GetType()==KBD_CNTRL)
      {
      Run_Help();
      goto beg;
      }
   if (Kbd_Chain_of_FLT)
      FLT_RunChain();
}

int Kbd_GetType()
{
  return (KeyInput.fbStatus==0x42 || (KeyInput.fbStatus==0x40 && (KeyInput.chChar < 0x20 || KeyInput.chChar==0x7f)))?KBD_CNTRL:KBD_ASCII;
}

int Kbd_GetCode()
{
   if (KeyInput.fbStatus==0x42 && (!KeyInput.chChar || KeyInput.chChar >=0x20))
      return  KeyInput.chScan;
   else
      return KeyInput.chChar;
}

int GetKeystroke2( void )
{    
    int code, type;
   for(;;) 
   {
     Kbd_Wait();
     type=Kbd_GetType();
     code=Kbd_GetCode();
     if (type==KBD_ASCII)
     {
         DosBeep(400,400);
         continue;
     }  else break;
   }   
   return code;
}      

void CHF_RemoveChain(CHF *chain)
{
CHF *pch, *tmp;
   if (!chain)
      return;
   else
      {
      pch=chain;
      while (pch)
         {
         tmp=pch;
         pch=pch->nxt;
         free(tmp);
         }
      }
}

void CHF_RunChain(CHF *chain)
{
CHF *pch;
int rc;
   rc=0;
   pch=chain;
   while (pch && !rc)
      {
      rc=pch->fun(pch->usd);
      pch=pch->nxt;
      }
}

void FLT_RunChain()
{
   CHF_RunChain(Kbd_Chain_of_FLT);
}

/******************************************************************/
/********************* Text area editing (MLE) ******************/
/******************************************************************/
// создание объекта редактирования типа MLE (без скролинга)
PMLE MLE_new(int width, int height,int blen,char *string, char *allow, char *deny, PFI finput)
{
PMLE pmle;
   if (!width || !height)
      return NULL;

   pmle=(PMLE)malloc(sizeof(MLE));
   pmle->width=width;
   pmle->height=height;
   pmle->blen=blen;
   pmle->buf=malloc(pmle->blen +1);
   memset(pmle->buf,0,pmle->blen +1);
   if (string)
      memcpy(pmle->buf,string,min(pmle->blen,strlen(string)));
   pmle->lux=0;
   pmle->luy=0;
   pmle->cursor=0;
   pmle->ins=1;                           // Insert mode default
   pmle->allow=allow;
   pmle->deny=deny;
   pmle->inp_filtr=finput;
   pmle->sp_func_key=NULL;
   return pmle;
}

void MLE_lastline(PMLE pmle)
{
int i;
   i= pmle->width * pmle->height - pmle->blen;
   if (i > 0)
      VioWrtNChar("▒", i, pmle->luy+pmle->height-1,pmle->lux + pmle->width -i, hvio);
}

void MLE_load(int row, int col, PMLE pmle, int fcol, int bcol)
{
int i, len;
   pmle->lux=col;
   pmle->luy=row;
   pmle->b_color=bcol;
   pmle->f_color=fcol;
   len=strlen(pmle->buf);
   Vid_ErasePart(row,col,pmle->width,pmle->height,fcol,bcol);
   i=0;
   while (len > 0)
      {
      VioWrtCharStr(pmle->buf + i*pmle->width,min(pmle->width,len),row+i,col,hvio);
      i++;
      len-=pmle->width;
      }
   MLE_lastline(pmle);
}


void MLE_reload(PMLE pmle)
{
int i, j, len;
int x,y;

   j=max(0,pmle->cursor-1);
   x=j % pmle->width;         // положение курсора
   y=j / pmle->width;         // в окне
   i=j-x;                     // позиция начало строки в буфере
   len=strlen(pmle->buf+i);

   while (len > pmle->width)
      {
      VioWrtCharStr(pmle->buf + i,pmle->width,pmle->luy+y,pmle->lux,hvio);
      i+=pmle->width;
      len-=pmle->width;
      y++;
      }
   if (len > 0)
      {
      Vid_EraseLine(pmle->luy + y,pmle->lux,pmle->width);
      VioWrtCharStr(pmle->buf + i,len,pmle->luy+y,pmle->lux,hvio);
      y++;
      }

   while(y < pmle->height)
      Vid_EraseLine(pmle->luy + y++,pmle->lux,pmle->width);

//   if (y >= pmle->height -1)
      MLE_lastline(pmle);
}

int MLE_edit_ascii(UCHAR code, PMLE pmle)
{
int i;
   if (pmle->ins && strlen(pmle->buf) >= pmle->blen)
      return -1;

   if (pmle->ins)
      {
      for (i=pmle->blen-1; i> pmle->cursor; i--)
         pmle->buf[i]=pmle->buf[i-1];
      pmle->buf[pmle->cursor]=code;
      }
   else
      pmle->buf[pmle->cursor]=code;

   if (pmle->cursor <pmle->blen-1)
         pmle->cursor++;
   return 1;
}

int MLE_edit_control(UCHAR code, PMLE pmle)
{
int x,y;
int rc;
int i,j,d;
   d=0;
   x=pmle->cursor % pmle->width;         // положение курсора
   y=pmle->cursor / pmle->width;         // в окне
   switch(code)
      {
      case INS:
         pmle->ins=1-pmle->ins;
         VioSetCurType(&Ins[pmle->ins],hvio);
         mAcurs =&Ins[pmle->ins];
         rc=2; // no reload
         break;
      case BS:
         if (!pmle->cursor)
            return -1;
         else
            strcpy(pmle->buf + pmle->cursor - 1,pmle->buf + pmle->cursor);
         d=1;
      case CULT:
         if (pmle->cursor)
            {
            pmle->cursor--;
            rc=2;
            }
         else
            rc=-1;
         break;
      case CURT:
         if (pmle->cursor < strlen(pmle->buf) && pmle->cursor < pmle->blen-1)
            {
            pmle->cursor++;
            rc=2;
            }
         else
            rc=-1;
         break;
      case CUUP:
         if (y)
            {
            pmle->cursor-=pmle->width;
            rc=2;
            }
         break;
      case CUDN:
         if (y < pmle->height - 1)
            {
            pmle->cursor+=pmle->width;
            rc=2;
            if (pmle->cursor >= strlen(pmle->buf))
               {
               pmle->cursor-=pmle->width;
               rc=-1;
               }
            }
         break;
      case CTRLCULT:
         if (pmle->cursor)
            pmle->cursor--;
         while (pmle->cursor && pmle->buf[pmle->cursor - 1] == 0x20)
               pmle->cursor--;
         while (pmle->cursor && pmle->buf[pmle->cursor-1] > 0x20)
               pmle->cursor--;
         rc=2;
         break;
      case CTRLCURT:
         i=strlen(pmle->buf);
         i=min(i,pmle->blen-1);
         while (pmle->cursor < i && pmle->buf[pmle->cursor] == 0x20)
               pmle->cursor++;
         while (pmle->cursor < i && pmle->buf[pmle->cursor] > 0x20)
            pmle->cursor++;
         while (pmle->cursor < i && pmle->buf[pmle->cursor] == 0x20)
            pmle->cursor++;
         rc=2;
         break;
      case ALTD:
         i=y*(pmle->width);
//         i=pmle->cursor-x;
         j=i+pmle->width;
         if (j < strlen(pmle->buf))
            strcpy(pmle->buf + i,pmle->buf + j);
         else
            pmle->buf[i]=0;
         pmle->cursor=i;
         d=1;
         break;
      case ALTK:
         i=y*(pmle->width);
         j=pmle->cursor-x+pmle->width;
         if (j < strlen(pmle->buf))
            strcpy(pmle->buf + pmle->cursor,pmle->buf + j);
         else
            pmle->buf[pmle->cursor]=0;
         d=1;
         break;
      case CTRLBS:
         i=pmle->cursor;
         rc=1;
         while (pmle->cursor && pmle->buf[--pmle->cursor] == 0x20);
         while (pmle->cursor && pmle->buf[--pmle->cursor] > 0x20);
         if (pmle->cursor)
             pmle->cursor++;
         if (pmle->cursor !=i)
            strcpy(pmle->buf + pmle->cursor,pmle->buf + i);
         break;
      case HOME:
         pmle->cursor=pmle->cursor - (pmle->cursor % pmle->width);
         rc=2;
         break;
      case END:
         j=max(pmle->cursor - x + pmle->width -1,0);
         i=strlen(pmle->buf);
         pmle->cursor =min (i, j);
         if (pmle->cursor == pmle->blen)
            pmle->cursor--;
         rc=2;
         break;
      case DEL:
         if (pmle->cursor < strlen(pmle->buf))
            {
            strcpy(pmle->buf + pmle->cursor,pmle->buf + pmle->cursor +1);
            d=1;
            }
         else
            rc=-1;
         break;
      case  CR: //was: CTRLCR:
         rc=0;
         break;
      default:
         return EDO_runCCKey((EDO *)pmle, code);

      }

   if (d)
      {
      rc=1;
      i=strlen(pmle->buf);
      while (i<pmle->blen)
         pmle->buf[i++]=0;
      }
   return rc;

}


int MLE_edit(PMLE pmle)
{
int K_TYPE;
int K_CODE;
int rc,i;
USHORT x,y;
   if (mAcurs !=&Ins[pmle->ins])
      {
      mAcurs =&Ins[pmle->ins];
      VioSetCurType(&Ins[pmle->ins],hvio);
      }


   rc=1;
   while (rc)
      {
      x=pmle->cursor % pmle->width;         // положение курсора
      y=pmle->cursor / pmle->width;         // в окне
      VioSetCurPos(pmle->luy+y,pmle->lux+x,hvio);

      Kbd_Wait();
      K_TYPE = Kbd_GetType();
      K_CODE = Kbd_GetCode();

      if (K_TYPE==KBD_ASCII)
         {
         if (EDO_allow((EDO *)pmle,K_CODE)!=FLG_YES && EDO_deny((EDO *)pmle,K_CODE)==FLG_YES)
            {
            DosBeep(500,50);
            continue;
            }
         else
            {
            K_CODE=EDO_filtr((EDO *)pmle,K_CODE);
            if (!K_CODE)
               {
               DosBeep(500,50);
               continue;
               }
            rc=MLE_edit_ascii(K_CODE,pmle);
            }
         }
      else
         if (K_CODE !=ESC)
            rc=MLE_edit_control(K_CODE,pmle);
         else
            rc=0;

      if (rc == 1)
         MLE_reload(pmle);
      else if (rc==-1)
         DosBeep(400,50);
      }
   return K_CODE;
}


/***************************************************************************************/
void Vid_Init(int mode)
{
   OldScreenData.cb=ScreenData.cb=sizeof(ScreenData);
   VioGetMode(&OldScreenData,hvio);
   memcpy(&ScreenData,&OldScreenData,sizeof(ScreenData));
   switch (mode)
      {
      case TEXT_25:
         ScreenData.row=25;
         ScreenData.vres=400;
         break;
      case TEXT_30:
         ScreenData.row=30;
         ScreenData.vres=480;
         break;
      }
VioSetMode(&ScreenData,hvio);
VioGetMode(&ScreenData,hvio);
CELL_SIZE=ScreenData.buf_length / (ScreenData.row * ScreenData.col);
VioGetCurType(&Save,hvio);
mAcurs=&Ins[1];
Ins[0].yStart=0;
Ins[0].cEnd=ScreenData.vres/ScreenData.row;
Ins[0].cx=1;
Ins[1].yStart=ScreenData.vres/ScreenData.row-2;
Ins[1].cEnd=ScreenData.vres/ScreenData.row;
Ins[1].cx=1;
VioSetCurType(&Ins[1],hvio);
}

void Vid_EnableBlink()
{
short req[19];
   req[0]=6;
   req[1]=2;
   req[2]=0;
   VioSetState(req,hvio);
}

void Vid_Reset()
{
  StopTimeService();
  StopKeyBarService();
  VioSetMode(&OldScreenData,hvio);
  VioSetCurType(&Save,hvio);
}

void Vid_Erase(int fcolor, int bcolor)
{
BYTE Attr[2];
   Attr[1]=MKATRB(bcolor,fcolor);
   Attr[0]=0x20;
   VioScrollUp(0,0,0xFFFF,0xFFFF,0xFFFF,(PBYTE)Attr,hvio);
   VioSetCurPos(0,0,hvio);
}

void Vid_EraseLine(int row, int col, int width)
{
   Vid_DrawLine(row, col, width,SPACE2);
}

void Vid_ErasePart(int row, int col, int width, int height, int fcolor, int bcolor)
{
BYTE Attr[2];
   Attr[1]=MKATRB(bcolor,fcolor);
   Attr[0]=0x20;
   VioScrollUp(row,col,row+height-1,col+width-1,height,(PBYTE)Attr,hvio);
}

void Vid_FillPart(int row, int col, int width, int height, int fcolor, int bcolor, int smb)
{
BYTE Attr[2];
   Attr[1]=MKATRB(bcolor,fcolor);
   Attr[0]=smb;
   VioScrollUp(row,col,row+height-1,col+width-1,height,(PBYTE)Attr,hvio);
}

/***************************************************************/
/***************    Линии, рамки, тени   ***********************/
/***************************************************************/
void Vid_DrawLine(int row, int col, int width, char *patern)
{
char *fill_smb;
   if (strlen(patern)==3)
      {
      width--;
      VioWrtNChar(&patern[2],1,row,col+width,hvio);
      VioWrtNChar(&patern[0],1,row,col,hvio);
      col++;
      width--;
      fill_smb= patern + 1;
      }
   else
      fill_smb= patern;

   VioWrtNChar(fill_smb,width,row,col,hvio);
}

void Vid_PutText(char **text,int row, int col, int yoffset, int xoffset, int width, int num_string)
{
int i, len;
char *str;
   for (i=0; i< num_string; i++)
      {
      str=text[i+yoffset];
      if (!str)
         return;
      len=strlen(str);
      if (len > xoffset)
         VioWrtCharStr(str+xoffset,min(width,len-xoffset),row+i,col,hvio);
      }
}



void StopTimeService(void)
{
   if (Times.service)
      {
      DosKillThread(Times.service);
      Times.service=0L;
      if (Times.add_service)
         CHF_RemoveChain(Times.add_service);
      }
}

void StopKeyBarService(void)
{
   if (KeyBarHelp.service)
      {
      KeyBarHelp.flag=0;
      DosKillThread(KeyBarHelp.service);
      KeyBarHelp.service=0L;
      ResetKeyBar();
      }
}

void ResetKeyBar()
{
   KeyBarHelp.status=0xff;
   KeyBarHelp.n_kbh=NULL;
   KeyBarHelp.s_kbh=NULL;
   KeyBarHelp.c_kbh=NULL;
   KeyBarHelp.a_kbh=NULL;
}

/***************************************************************/
/*******************      Редактирование     *******************/
/***************************************************************/
// установка дополнительных спец.ключей окончания редактирования и прочих операций
void EDO_addCCKey(PEDO obj,int key, PFI fun, void *usd)
{
CCK *cckTmp, *ct;
   cckTmp=(CCK *)malloc(sizeof(CCK));
   cckTmp->ckey=key;
   cckTmp->fun=fun;
   cckTmp->usd=usd;
   cckTmp->nxt=NULL;

   if (!obj->sp_func_key)
      obj->sp_func_key=cckTmp;
   else
      {
      ct=obj->sp_func_key;
      while (ct->nxt)
         ct=ct->nxt;
      ct->nxt=cckTmp;
      }
}

void EDO_delCCKey(PEDO obj)
{
CCK *ct, *ct1;
   ct=obj->sp_func_key;
   while (ct)
      {
      ct1=ct->nxt;
      free(ct);
      ct=ct1;
      }
   obj->sp_func_key=NULL;
}

int EDO_runCCKey(PEDO obj, int key)
{
CCK *ct;
int rc;
   ct=obj->sp_func_key;
   while (ct)
      {
      if (ct->ckey==key)
         {
         if (ct->fun)
            return ct->fun(obj, ct->usd);
         else
            return 0;                    // конец редактирования
         }
      else
         ct=ct->nxt;
      }
   return -1;
}

// проверка на разрешенные символы
int EDO_allow(PEDO obj, int key)
{
   if (obj->allow  && strchr(obj->allow,key))
      return FLG_YES;
   else
      return FLG_NO;
}

// проверка на запрещенные символы
int EDO_deny(PEDO obj, int key)
{
   if (obj->deny && strchr(obj->deny,key))
      return FLG_YES;
   else
      return FLG_NO;
}

// фильтрация символа на входе
int EDO_filtr(PEDO obj,int key)
{
   if (obj->inp_filtr)
      return obj->inp_filtr(key);
   else
      return key;
}

void Load_Help(HELP2 *htmp)
{
int n;
   if (htmp->text)
      Vid_PutText(htmp->text,htmp->luy,htmp->lux,
              htmp->y_offset,htmp->x_offset,htmp->width,htmp->height);
}

void Run_Help()
{
PAREA parea;
HELP2 *hTmp;
int code, type;
int renew;
   if (!HELP_SYSTEM->text)
      {
      DosBeep(500,200);
      DosBeep(500,300);
      return;
      }
   parea=Store_area(HELP_SYSTEM->luy, HELP_SYSTEM->lux, HELP_SYSTEM->width, HELP_SYSTEM->height);
   hTmp=HELP_SYSTEM;
   HELP_SYSTEM=NULL;

   renew=1;
   while (1)
      {
      if (renew)
         {
         Vid_ErasePart(hTmp->luy, hTmp->lux, hTmp->width, hTmp->height,
                       hTmp->f_color, hTmp->b_color);

         Load_Help(hTmp);
         renew=0;
         }

      Kbd_Wait();
      type=Kbd_GetType();
      code=Kbd_GetCode();
      if (type==KBD_ASCII)
         {
         DosBeep(400,400);
         continue;
         }
      else
         {
         switch(code)
            {
            case CUUP:
               if (hTmp->y_offset)
                  {
                  BYTE Attr[2];
                  Attr[0]=0x20;
                  Attr[1]=MKATRB(hTmp->b_color,hTmp->f_color);
                  hTmp->y_offset--;
                  VioScrollDn(hTmp->luy, hTmp->lux,hTmp->luy+hTmp->height-1,
                                                   hTmp->lux+hTmp->width-1 ,1,(PBYTE)Attr,hvio);
                  VioWrtCharStr(hTmp->text[hTmp->y_offset]+hTmp->x_offset,
                                min(strlen(hTmp->text[hTmp->y_offset]+hTmp->x_offset), hTmp->width),hTmp->luy,hTmp->lux,hvio);
//                  renew=1;
                  }
               continue;
            case CUDN:
               if (hTmp->y_offset + hTmp->height < hTmp->num_lines )
                  {
                  BYTE Attr[2];
                  Attr[0]=0x20;
                  Attr[1]=MKATRB(hTmp->b_color,hTmp->f_color);
                  hTmp->y_offset++;
                  VioScrollUp(hTmp->luy, hTmp->lux,hTmp->luy+hTmp->height-1,
                                                   hTmp->lux+hTmp->width-1 ,1,(PBYTE)Attr,hvio);
                  VioWrtCharStr(hTmp->text[hTmp->y_offset+hTmp->height-1]+hTmp->x_offset,
                                          min(strlen(hTmp->text[hTmp->y_offset+hTmp->height-1]+hTmp->x_offset),hTmp->width),
                                                   hTmp->luy+hTmp->height-1,hTmp->lux,hvio);
//                  renew=1;
                  }
               continue;
            case PGUP:
               if (hTmp->y_offset - hTmp->height >=0 )
                  {
                  hTmp->y_offset-=hTmp->height;
                  renew=1;
                  }
               continue;
            case PGDN:
               if (hTmp->y_offset + hTmp->height < hTmp->num_lines )
                  {
                  hTmp->y_offset=min(hTmp->y_offset + hTmp->height,
                                     hTmp->num_lines - hTmp->height);
                  renew=1;
                  }
               continue;
            case HOME:
               if (hTmp->x_offset || hTmp->y_offset)
                  {
                  hTmp->x_offset=0;
                  hTmp->y_offset=0;
                  renew=1;
                  }
               continue;
            case END:
               if (hTmp->y_offset != hTmp->num_lines - hTmp->height)
                  {
                  hTmp->y_offset=hTmp->num_lines - hTmp->height;
                  renew=1;
                  }
               continue;
            case CULT:
               if (hTmp->x_offset > 0)
                  {
                  hTmp->x_offset--;
                  renew=1;
                  }
               continue;
            case CURT:
               if (hTmp->x_offset + hTmp->width < hTmp->mw_lines)
                  {
                  hTmp->x_offset++;
                  renew=1;
                  }
               continue;
            case ESC:
               break;
            default:
               DosBeep(400,300);
               continue;
            }
         }
      break;
      }

   HELP_SYSTEM=hTmp;
   Restore_area(parea);
}

/******************************************************************/
/*********************  Сохранение экрана  ************************/
/******************************************************************/
PAREA Store_area(int row, int col, int width, int height)
{
int i;
unsigned short s;
PAREA tmp;
   tmp=(PAREA)malloc(sizeof(AREA));
   tmp->row=row;
   tmp->col=col;
   tmp->width=width;
   tmp->height=height;
   s=CELL_SIZE * width;                  // размер строки в байтах
   tmp->store=malloc(s * height);

   for (i=0; i < height; i++)
      VioReadCellStr(tmp->store + (i * s),&s,row+i,col,hvio);
  return tmp;
}

void Restore_area(PAREA parea)
{
int i,s;
   if (parea)
      {
      s=CELL_SIZE * parea->width;                  // размер строки в байтах
      for (i=0; i < parea->height; i++)
         VioWrtCellStr(parea->store + (i * s),s,parea->row+i,parea->col,hvio);
      }
}

void Free_area(PAREA parea)
{
   free(parea->store);
   free(parea);
   parea=NULL;
}
