/*
	@(#)DrvFloodFill.c	1.6
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "windows.h"
#include "DrvDC.h"
#include "DeviceData.h"

DWORD	DrvFloodFillImage(XImage *,int,int,DWORD,BOOL);
LPVOID WinCalloc( unsigned int , unsigned int );

static BOOL bDoSurface = FALSE;
static BOOL bMemOK = TRUE;
static int nStackLevel = 0;
static DWORD pixStart, pixPaint;
static int X,Y;
static LPDRIVERDC lpDriverDC;

typedef struct tagStackNode {
	struct tagStackNode *next;
	int	xStart,xEnd;
	int	y;
	int	FillDir;
} StackNode;

static StackNode *pStack;

#define FDUp	0
#define FDDown	1

#define PIX_STACK_MAX_DEPTH 1000

#define StackAlloc(n)   (StackNode *)WinCalloc(1, sizeof(StackNode))
#define StackFree(p)	WinFree((LPSTR)p)

DWORD DrvMakePixel(COLORREF, LPDRIVERDC);

static void
Push( int xStart, int xEnd, int y, int nFillDir )
{
   if (!bMemOK)
   	return;

   if (nStackLevel > PIX_STACK_MAX_DEPTH) {
   	bMemOK = FALSE;
   	return;
   }

   if (pStack) {
   	StackNode *p = StackAlloc(1);
   	p->next   = pStack;
   	pStack = p;

   } else {
   	pStack = StackAlloc(1);
   	/* note: next init-ed to NULL */
   }
   pStack->xStart = xStart;
   pStack->xEnd = xEnd;
   pStack->y = y;
   pStack->FillDir = nFillDir;

   nStackLevel++;
}

static void
Pop( StackNode *node )
{
   if (!bMemOK)
   	return; 

   if ((nStackLevel <= 0) || (!pStack)) {
   	bMemOK = FALSE;
   	return;
   }
   *node = *pStack;
   StackFree(pStack);
   pStack = node->next;
   nStackLevel--;
}

#define DrawLine(xStart,xEnd,y)		{ \
    		XFillRectangle(lpDriverDC->dp->display, \
				lpDriverDC->Drawable,   \
				lpDriverDC->gc,		\
				X+xStart,Y+y,		\
				xEnd-xStart+1,1); }

#define PixelMatch(im,x,y)	((XGetPixel(im,x,y) != pixStart) ^ bDoSurface)
#define PaintPixel(im,x,y)	XPutPixel(im,x,y,pixPaint)

static void
FillLine( XImage *image, int xStart, int y, StackNode *node )
{
    int      x;

    for(x = xStart+1; x < image->width; x++) {
	if (PixelMatch(image,x,y)) 
	    PaintPixel(image,x,y);
	else
	    break;
    }
    node->xEnd = x - 1;

    for(x = xStart; x >= 0; x--) {
	if (PixelMatch(image,x,y)) 
	    PaintPixel(image,x,y);
	else
	    break;
    }
    node->xStart = x + 1;
    node->y = y;
    DrawLine(node->xStart,node->xEnd,y);
}

static void
ProcessNode(XImage *im, StackNode *pNode)
{
    int x, xStart, xEnd, xNewStart;
    int y = pNode->y;

    xStart = pNode->xStart;
    xEnd = pNode->xEnd;

    do {
	if (y < 0 || y >= im->height)
	    break;
	/* go left */
	for (x = xStart; x >= 0; x--) {
	    if (!PixelMatch(im,x,y)) 
		break;
	}
	xNewStart = min(xStart,x + 1);
	if ((xStart - xNewStart) >= 2) {
	    Push( xNewStart, xStart, ((pNode->FillDir == FDUp) ? y+1 : y-1),
			(pNode->FillDir == FDUp) ? FDDown : FDUp);
	}

	if (xStart == xNewStart) {
	    for (x = xStart; x <= xEnd; x++) {
		if (PixelMatch(im,x,y)) {
		    xNewStart = x;
		    break;
		}
	    }
	    if (x > xEnd)
		break;	/* out of the main DO - WHILE loop */
	}

	/* go right */
	for (x = xNewStart; x < im->width; x++) {
	    if (PixelMatch(im,x,y)) 
	        PaintPixel(im,x,y);
	    else
		break;
	}
	if ((--x - xEnd) >= 2) {
	    Push( xEnd+1, x, ((pNode->FillDir == FDUp) ? y+1 : y-1),
		(pNode->FillDir == FDUp) ? FDDown : FDUp);
	}
	if ((xEnd - x) >= 2) {
	    Push( x+1, xEnd, y, pNode->FillDir);
	}

	xEnd = x;
	xStart = xNewStart;

	if (xEnd >= xStart)
	    DrawLine(xStart,xEnd,y);

	(pNode->FillDir == FDUp) ? y-- : y++;

    } while ((xEnd - xStart) >= 0);
}

DWORD
DrvExtFloodFill(LPDRIVERDC lpddc, LPLSDS_PARAMS lpStruct, LPRECT lpClipRect)
{
    int nStartX, nStartY;
    int nWidth,nHeight;
    LSDE_FLOODFILL *lpff = &lpStruct->lsde.floodfill;
    XImage *image;
    StackNode node;

    nStartX = lpff->nXStart;
    if (nStartX < lpClipRect->left || nStartX >= lpClipRect->right)
	return 0;

    nStartX -= lpClipRect->left;
    X = lpClipRect->left;

    nStartY = lpff->nYStart;
    if (nStartY < lpClipRect->top || nStartY >= lpClipRect->bottom)
	return 0;
    nStartY -= lpClipRect->top;
    Y = lpClipRect->top;

    nWidth = lpClipRect->right - lpClipRect->left;
    nHeight = lpClipRect->bottom - lpClipRect->top;

    lpDriverDC = lpddc;
    if (lpddc->ForeColorPixel != lpddc->BrushColorPixel) {
	XSetForeground(lpddc->dp->display,lpddc->gc,
			lpddc->BrushColorPixel);
	lpddc->ForeColorPixel = lpddc->BrushColorPixel;
    }
    image = XGetImage(lpddc->dp->display,lpddc->Drawable,
		lpClipRect->left,lpClipRect->top,
		nWidth,nHeight,
		(DWORD)-1,ZPixmap);
    if (image == NULL)
	return 0;

    bDoSurface = (lpff->fuFillType == FLOODFILLSURFACE) ? TRUE : FALSE;

    pixStart = DrvMakePixel(lpff->cr, lpddc);
    pixPaint = (bDoSurface) ? (DWORD)(255 - (BYTE)pixStart) : pixStart;
    pStack = NULL;
    FillLine(image, nStartX, nStartY, &node);
    Push(node.xStart,node.xEnd,nStartY-1,FDUp);
    Push(node.xStart,node.xEnd,nStartY+1,FDDown);
    while (nStackLevel > 0) {	/* while stack is not empty */
	Pop(&node);
	ProcessNode(image,&node);
    }
    XDestroyImage(image);
    lpDriverDC = NULL;
    return 0;
}
