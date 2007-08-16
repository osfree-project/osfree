/* 
 $Id: F_session.cpp,v 1.2 2002/09/06 12:29:43 evgen2 Exp $ 
*/
/* FreePM_session.cpp */
/* functions of class FreePM_session */

#include "FreePM.hpp"
#include <malloc.h>

int FreePM_session::AddDesktop(int nx, int ny, int bytesPerPixel)
{  int i;

   FreePM_DeskTop *pDeskTop;
   pDeskTop = new FreePM_DeskTop(nx,ny, bytesPerPixel);
   if(numDeskTops == 0 && pDeskTops == NULL)
   {  numpDeskTopsAllocated = 16;
      pDeskTops = (FreePM_DeskTop * *) calloc(sizeof(FreePM_DeskTop *),numpDeskTopsAllocated);
      pDeskTops[numDeskTops++] = pDeskTop;
   } else {
      if(numDeskTops >= numpDeskTopsAllocated)
      {  numpDeskTopsAllocated += 16;
         pDeskTops = (FreePM_DeskTop * *) realloc((void *)pDeskTops, sizeof(FreePM_DeskTop *) * numpDeskTopsAllocated);
         for(i = 0; i < 16; i++) pDeskTops[numpDeskTopsAllocated-i-1] = NULL;
      }
      pDeskTops[numDeskTops++] = pDeskTop;
   }
   return numDeskTops-1;
}

int FreePM_session::DelDesktop(int ind)
{
    if(ind < 0 || ind >= numDeskTops)
           return 1;
    if(pDeskTops[ind] == NULL)
           return 2;
    delete pDeskTops[ind];
    pDeskTops[ind] = NULL;
    return 0;
}

