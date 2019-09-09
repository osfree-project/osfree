/*
 *      BinLib.c
 *
 *      @(#)binlib2.c	1.3 11/5/96 16:35:33 /users/sccs/src/samples/bindemo2/s.binlib2.c
 *
 *      Dynamic Linking Sample Application
 *
 *      Demonstrates the following:
 *
 *      - encapsulating the binary interface to the windows DLL
 *        from the calling application
 *
 */

#include "windows.h"
#include "Endian.h"
#include "OEM.h"
#include "binlib2.h"

#include "kerndef.h"
#include "BinTypes.h"
#include "Log.h"
#include "DPMI.h"
#include "Resources.h"
#include "Module.h"
 
extern ENV *envp_global;

BOOL FAR PASCAL cnb_cdtInit(int FAR *, int FAR *);
BOOL FAR PASCAL cnb_cdtDrawExt(HDC , int , int , int , int , int , int , DWORD );
BOOL FAR PASCAL cnb_cdtDraw(HDC , int , int , int , int , DWORD );
void FAR PASCAL cnb_cdtTerm();

HINSTANCE hLibInst;	/* Handle to the DLL */

BOOL LoadCardsDll();
void AddOEMTable(char *,HSMT_OEMENTRYTAB *);

/* Pointer to the library-functions we will use */
BOOL (FAR PASCAL *lfp_cdtInit)(int FAR *pdxCard, int FAR *pdyCard) = NULL;
BOOL (FAR PASCAL *lfp_cdtDrawExt)(HDC hdc, int x, int y, int carddx, int carddy, int cd, int mode, DWORD rgbBgnd) =NULL;
BOOL (FAR PASCAL *lfp_cdtDraw)(HDC hdc, int x, int y, int cd, int mode, DWORD rgbBgnd) =NULL;
void (FAR PASCAL *lfp_cdtTerm)() = NULL;
extern void invoke_binary();

/* Binary interface to cards.dll */
HSMT_OEMENTRYTAB OEM_tab_cards[] =
{       /* 000 */       { "", 0 },
        /* 001 */       { "CDTINIT",    (long int (*)())cnb_cdtInit },
        /* 002 */       { "CDTDRAW",    (long int (*)())cnb_cdtDraw },
        /* 003 */       { "CDTDRAWEXT", (long int (*)())cnb_cdtDrawExt },
        /* 004 */       { "CDTTERM",    (long int (*)())cnb_cdtTerm }
};


/* 
 * This routine is called on loading of cards.so
 * This will map the above binary interface to cards.dll, when
 * that library is loaded.
 */
void
Binlib_LibMain()
{
        AddOEMTable("cards", OEM_tab_cards);
}

int FAR PASCAL _export
WEP()
{ 
  if(hLibInst)
    FreeLibrary(hLibInst);
  return 0;
}

/* These functions are the interface to the DLL. They make sure the Library
 *  is loaded and the address of the function in the DLL is known.
*/  
BOOL FAR PASCAL cdtInit(int FAR *pdxCard, int FAR *pdyCard)
{
  if(LoadCardsDll())
  {
    if(!lfp_cdtInit)
    {
      lfp_cdtInit= (BOOL (FAR PASCAL *)(int FAR *, int FAR *)) GetProcAddress(hLibInst,"cdtInit");
    }
    if(lfp_cdtInit)
    {
      return (*lfp_cdtInit)(pdxCard, pdyCard);
    } 
  }
  return FALSE;
}

BOOL FAR PASCAL cdtDrawExt(HDC hdc, int x, int y, int carddx, int carddy, int cd, int mode, DWORD rgbBgnd)
{
  if(LoadCardsDll())
  {
    if(!lfp_cdtDrawExt)
    {
      lfp_cdtDrawExt= (BOOL (FAR PASCAL *)(HDC, int, int, int, int, int, int, DWORD)) GetProcAddress(hLibInst,"cdtDrawExt");
    }
    if(lfp_cdtDrawExt)
    {
      return (*lfp_cdtDrawExt)(hdc, x, y, carddx, carddy, cd, mode, rgbBgnd); 
    }
  }
  return FALSE;
}

BOOL FAR PASCAL cdtDraw(HDC hdc, int x, int y, int cd, int mode, DWORD rgbBgnd)
{
  if(LoadCardsDll())
  {
    if(!lfp_cdtDraw)
    {
      lfp_cdtDraw= (BOOL (FAR PASCAL *)(HDC, int, int, int, int, DWORD)) GetProcAddress(hLibInst,"cdtInit");
    }
    if(lfp_cdtDraw)
    {
      return (*lfp_cdtDraw)(hdc, x, y, cd, mode, rgbBgnd); 
    }
  }
  return FALSE;
}

void FAR PASCAL cdtTerm()
{
  if(LoadCardsDll())
  {
    if(!lfp_cdtTerm)
    {
      lfp_cdtTerm= (void (FAR PASCAL *)()) GetProcAddress(hLibInst,"cdtInit");
    }
    if(lfp_cdtTerm)
    {
      (*lfp_cdtTerm)(); 
    }
  }
}


/* 
 * Interface routine for the cards.dll init routine
 * passed pointers to two integers
 *
 * the interface does the following...
 *    maps 2 words to the dos address space
 *    pushes them on the stack, as two far pointers
 *    calls the underlying routine
 *    sets the 'real' values from the intermediate values
 *    (note: x,y are intel format, must use a macro to convert to native)
 *    frees the mapping selectors
 *    returns the result
 */

BOOL FAR PASCAL 
cnb_cdtInit(int FAR *pdxCard, int FAR *pdyCard)
{
    WORD wSel0=0,wSel1=0;
    WORD x,y;

    if(!hLibInst)
    {
      hLibInst=LoadLibrary("cards.dll");
      if(hLibInst < HINSTANCE_ERROR)
         hLibInst = 0;
    }
    if(hLibInst)
    {
      envp_global->reg.sp -= DWORD_86 + DWORD_86;

      wSel0 = ASSIGNSEL(&y, 2);
      PUTDWORD(envp_global->reg.sp,MAKELP(wSel0,0));

      wSel1 = ASSIGNSEL(&x, 2);
      PUTDWORD(envp_global->reg.sp+4,MAKELP(wSel1,0));

      invoke_binary();

      /* don't forget to take care of the endian issue: use GETWORD !!!*/
      *pdxCard = GETWORD(&x);
      *pdyCard = GETWORD(&y);

      FreeSelector(wSel0);
      FreeSelector(wSel1);

      return envp_global->reg.ax;
    }
    return 0;
}

/*
 *  Interface to cdtDrawExt
 *  push enough space for arguments
 *  push the arguments, no special handling required
 *  invoke the binary code
 *  return the result code
 */

BOOL FAR PASCAL 
cnb_cdtDrawExt(HDC hdc, int x, int y, int dx, int dy, int cd, int mode, DWORD rgbBgnd)
{
    envp_global->reg.sp -= 7 * WORD_86 + DWORD_86;
    PUTDWORD(envp_global->reg.sp,rgbBgnd);
    PUTWORD(envp_global->reg.sp+4,mode);
    PUTWORD(envp_global->reg.sp+6,cd);
    PUTWORD(envp_global->reg.sp+8,dy);
    PUTWORD(envp_global->reg.sp+10,dx);
    PUTWORD(envp_global->reg.sp+12,y);
    PUTWORD(envp_global->reg.sp+14,x);
    PUTWORD(envp_global->reg.sp+16,hdc);

    invoke_binary();

    return envp_global->reg.ax;
}

/*
 *  Interface to cdtDraw
 *  push enough space for arguments
 *  push the arguments, no special handling required
 *  invoke the binary code
 *  return the result code
 */

BOOL FAR PASCAL 
cnb_cdtDraw(HDC hdc, int x, int y, int cd, int mode, DWORD rgbBgnd)
{
    envp_global->reg.sp -= 5 * WORD_86 + DWORD_86;
    PUTDWORD(envp_global->reg.sp,rgbBgnd);
    PUTWORD(envp_global->reg.sp+4,mode);
    PUTWORD(envp_global->reg.sp+6,cd);
    PUTWORD(envp_global->reg.sp+8,y);
    PUTWORD(envp_global->reg.sp+10,x);
    PUTWORD(envp_global->reg.sp+12,hdc);

    invoke_binary();

    return envp_global->reg.ax;
}

/*
 *  Interface to cdtTerm
 *  no special handling required
 *  invoke the binary code
 */
void FAR PASCAL 
cnb_cdtTerm()
{
        invoke_binary();
}

/* This function tries to load cards.dll if necessary and returns 0 if the
 *  Library cannot be loaded
*/
BOOL LoadCardsDll()
{
  if(!hLibInst)
  { 
    hLibInst=LoadLibrary("cards.dll");
    if(hLibInst < HINSTANCE_ERROR)
      hLibInst = 0;
  }
  return hLibInst; 
}
    
