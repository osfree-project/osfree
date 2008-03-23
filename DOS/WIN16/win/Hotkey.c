/*    
	Hotkey.c	1.3
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

#include "windows.h"
#include "kerndef.h"
#include "Hotkey.h"

typedef struct _HOTKEY
{
  HWND hwnd;
  int id;
  UINT fsModifiers;
  UINT vk;
  struct _HOTKEY *next;
} HOTKEY;

HOTKEY *pFirstHotKey = NULL;

/*
 * CheckHotKey: Determines whether a given key message is a hotkey message
 * 
 * If the message is for a window and a key that we have a hotkey for, send a
 * WM_HOTKEY message instead.  We return TRUE if we sent (or possibly didn't send...)
 * our own message and the WM_KEY{UP,DOWN} message shouldn't be sent.
 * 
 * Additionally, every time we get a WM_KEYDOWN message (even if it's a repeat key),
 * we should send a WM_HOTKEY message.  We can just ignore it if it's a WM_KEYUP message
 * (even though we don't want a WM_KEYUP message to be sent to the app).
 */
BOOL CheckHotKey(LPMSG lpMsg)
{
  HOTKEY *pHotKey = pFirstHotKey;
  HOTKEY *pFound = NULL;
  int nExtra = 0, nNew;

  while (pHotKey)
    {
      /* Is this hotkey our key? */
      if ((!pHotKey->hwnd || (lpMsg->hwnd == pHotKey->hwnd)) &&
	  (pHotKey->vk == lpMsg->wParam) &&
	  (pHotKey->fsModifiers & MOD_SHIFT   ? GetKeyState(VK_SHIFT)   & 0x8000 : TRUE) &&
	  (pHotKey->fsModifiers & MOD_CONTROL ? GetKeyState(VK_CONTROL) & 0x8000 : TRUE) &&
	  (pHotKey->fsModifiers & MOD_ALT     ? GetKeyState(VK_MENU)    & 0x8000 : TRUE))
	{
	  /* Okay, we found one, how specific is it? */
	  nNew = 0;
	  if (GetKeyState(VK_SHIFT) & 0x8000)
	    nNew++;
	  if (GetKeyState(VK_CONTROL) & 0x8000)
	    nNew++;
	  if (GetKeyState(VK_MENU) & 0x8000)
	    nNew++;
	  if (pHotKey->fsModifiers & MOD_SHIFT)
	    nNew--;
	  if (pHotKey->fsModifiers & MOD_CONTROL)
	    nNew--;
	  if (pHotKey->fsModifiers & MOD_ALT)
	    nNew--;

	  /* nNew now holds the number of modifier keys down that were not originally specified */
	  if (!pFound || nNew < nExtra)
	    {
	      nExtra = nNew;
	      pFound = pHotKey;
	    }
	}

      /* No, loop */
      pHotKey = pHotKey->next;
    }

  /* We didn't find anything, give up */
  if (!pFound)
    return FALSE;

  /* Ignore WM_KEYUP messages with hotkeys */
  if (lpMsg->message == WM_KEYUP || lpMsg->message == WM_SYSKEYUP)
    return TRUE;

  /* We _did_ find something, send a hotkey message */
  if (pFound->hwnd)
    SendMessage(pFound->hwnd, WM_HOTKEY, pFound->id,
		MAKELPARAM((((GetKeyState(VK_SHIFT)   & 0x8000) ? MOD_SHIFT   : 0) &
			    ((GetKeyState(VK_CONTROL) & 0x8000) ? MOD_CONTROL : 0) &
			    ((GetKeyState(VK_MENU)    & 0x8000) ? MOD_ALT     : 0)),
			   lpMsg->wParam));
  else
    /* PostThreadMessage(GetCurrentThreadId(), WM_HOTKEY, pHotKey->id, */
    PostAppMessage(GetCurrentTask(), WM_HOTKEY, pFound->id,
		   MAKELPARAM((((GetKeyState(VK_SHIFT)   & 0x8000) ? MOD_SHIFT   : 0) &
			       ((GetKeyState(VK_CONTROL) & 0x8000) ? MOD_CONTROL : 0) &
			       ((GetKeyState(VK_MENU)    & 0x8000) ? MOD_ALT     : 0)),
			      lpMsg->wParam));
  return TRUE;
}

BOOL WINAPI RegisterHotKey(HWND hwnd, int id, UINT fsModifiers, UINT vk)
{
  HOTKEY **ppNextHotKey, *pHotKey, *pNewHotKey;

  /* Check that this hot key doesn't conflict with any others */
  ppNextHotKey = &pFirstHotKey;
  pHotKey = pFirstHotKey;
  while (pHotKey)
    {
      ppNextHotKey = &pHotKey->next;
      if (pHotKey->id == id)
	return FALSE; /* Same hotkey identifier */
      if ((pHotKey->vk == vk) &&
	  (pHotKey->fsModifiers == fsModifiers))
	return FALSE; /* Same keystroke */
      pHotKey = pHotKey->next;
    }

  pNewHotKey = (HOTKEY *)WinMalloc(sizeof(HOTKEY));
  if (!pNewHotKey)
    return FALSE;
  pNewHotKey->hwnd = hwnd;
  pNewHotKey->id = id;
  pNewHotKey->fsModifiers = fsModifiers;
  pNewHotKey->vk = vk;
  pNewHotKey->next = NULL;
  *ppNextHotKey = pNewHotKey;

  return TRUE;
}

BOOL WINAPI UnregisterHotKey(HWND hwnd, int id)
{
  HOTKEY **ppNextHotKey, *pHotKey;

  ppNextHotKey = &pFirstHotKey;
  pHotKey = pFirstHotKey;
  while (pHotKey)
    {
      if ((pHotKey->hwnd == hwnd) &&
	  (pHotKey->id == id))
	{
	  *ppNextHotKey = pHotKey->next;
	  WinFree((char *)pHotKey);
	  return TRUE;
	}
      ppNextHotKey = &pHotKey->next;
      pHotKey = pHotKey->next;
    }

  return FALSE;
}
