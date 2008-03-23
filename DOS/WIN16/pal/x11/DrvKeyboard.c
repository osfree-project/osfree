/*
	@(#)DrvKeyboard.c	1.16
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
#include "windowsx.h"

#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "Log.h"
#include "DrvDP.h"
#include "DrvKeyboard.h"
#include "Driver.h"
#include "ctype.h"

/* static data */
static KEYSTATE KeyMap[256];

/* this maps X KeyCodes to Windows VK keys */
/* it is built by reverse engineering the  */
/* KeySyms to KeyCodes, and then setting   */
/* the VK key of the keycode, we do allow  */
/* more than one KeyCode to generate a VK  */
/* key, primarily for control and shifts.  */

static KEYTABLE VK2KeySym[] = {
	 { VK_BACK,XK_BackSpace} ,
	 { VK_TAB,XK_Tab} ,
	 { VK_CLEAR, XK_Clear} ,
	 { VK_RETURN, XK_Return} ,
	 { VK_RETURN, XK_KP_Enter} ,
	 { VK_SHIFT, XK_Shift_L} ,
	 { VK_SHIFT, XK_Shift_R} ,
	 { VK_CONTROL, XK_Control_L} ,
	 { VK_CONTROL, XK_Control_R} ,
	 { VK_MENU, XK_Menu} ,

	 { VK_PAUSE, XK_Pause} ,
	 { VK_CAPITAL, XK_Caps_Lock} ,

	 { VK_SPACE, XK_space} ,
	 { VK_PRIOR, XK_Prior} ,
	 { VK_NEXT, XK_Next} ,
	 { VK_END, XK_End} ,
	 { VK_HOME, XK_Home} ,

	 { VK_LEFT, XK_Left} ,
	 { VK_UP, XK_Up} ,
	 { VK_RIGHT, XK_Right} ,
	 { VK_DOWN, XK_Down} ,
	 { VK_SELECT, XK_Select} ,
	 { VK_SELECT, XK_Linefeed} ,

	 { VK_ALT,    XK_Alt_R} ,
	 { VK_ALT,    XK_Alt_L} ,
	 { VK_CANCEL, XK_Break} ,

	 { VK_PRINT, XK_Print} ,
	 { VK_EXECUTE, XK_Execute} ,

	 { VK_INSERT, XK_Insert} ,
	 { VK_DELETE, XK_Delete} ,
	 { VK_HELP, XK_Help} ,

	 { VK_0, XK_0} ,
	 { VK_1, XK_1} ,
	 { VK_2, XK_2} ,
	 { VK_3, XK_3} ,
	 { VK_4, XK_4} ,
	 { VK_5, XK_5} ,
	 { VK_6, XK_6} ,
	 { VK_7, XK_7} ,
	 { VK_8, XK_8} ,
	 { VK_9, XK_9} ,

/* strictly speaking not needed, only one case will be used */
	 { VK_A, XK_a} ,
	 { VK_B, XK_b} ,
	 { VK_C, XK_c} ,
	 { VK_D, XK_d} ,
	 { VK_E, XK_e} ,
	 { VK_F, XK_f} ,
	 { VK_G, XK_g} ,
	 { VK_H, XK_h} ,
	 { VK_I, XK_i} ,
	 { VK_J, XK_j} ,
	 { VK_K, XK_k} ,
	 { VK_L, XK_l} ,
	 { VK_M, XK_m} ,
	 { VK_N, XK_n} ,
	 { VK_O, XK_o} ,
	 { VK_P, XK_p} ,
	 { VK_Q, XK_q} ,
	 { VK_R, XK_r} ,
	 { VK_S, XK_s} ,
	 { VK_T, XK_t} ,
	 { VK_U, XK_u} ,
	 { VK_V, XK_v} ,
	 { VK_W, XK_w} ,
	 { VK_X, XK_x} ,
	 { VK_Y, XK_y} ,
	 { VK_Z, XK_z} ,

	 { VK_NUMPAD0, XK_KP_0} ,
	 { VK_NUMPAD1, XK_KP_1} ,
	 { VK_NUMPAD2, XK_KP_2} ,
	 { VK_NUMPAD3, XK_KP_3} ,
	 { VK_NUMPAD4, XK_KP_4} ,
	 { VK_NUMPAD5, XK_KP_5} ,
	 { VK_NUMPAD6, XK_KP_6} ,
	 { VK_NUMPAD7, XK_KP_7} ,
	 { VK_NUMPAD8, XK_KP_8} ,
	 { VK_NUMPAD9, XK_KP_9} ,

	 { VK_MULTIPLY, XK_KP_Multiply} ,
	 { VK_ADD, XK_KP_Add} ,
	 { VK_SEPARATOR, XK_KP_Separator} ,
	 { VK_SUBTRACT, XK_KP_Subtract} ,
	 { VK_DECIMAL, XK_KP_Decimal} ,
	 { VK_DIVIDE, XK_KP_Divide} ,

	 { VK_F1, XK_F1} ,
	 { VK_F2, XK_F2} ,
	 { VK_F3, XK_F3} ,
	 { VK_F4, XK_F4} ,
	 { VK_F5, XK_F5} ,
	 { VK_F6, XK_F6} ,
	 { VK_F7, XK_F7} ,
	 { VK_F8, XK_F8} ,
	 { VK_F9, XK_F9} ,
	 { VK_F10, XK_F10} ,
	 { VK_F11, XK_F11} ,
	 { VK_F12, XK_F12} ,
	 { VK_F13, XK_F13} ,
	 { VK_F14, XK_F14} ,
	 { VK_F15, XK_F15} ,
	 { VK_F16, XK_F16} ,

	 { VK_F13, XK_KP_F1} ,
	 { VK_F14, XK_KP_F2} ,
	 { VK_F15, XK_KP_F3} ,
	 { VK_F16, XK_KP_F4} ,

	 { VK_NUMLOCK, XK_Num_Lock} ,
	 { VK_OEM_SCROLL, XK_Scroll_Lock},  

	 { VK_OEM_1, XK_semicolon} ,
	 { VK_OEM_PLUS, XK_equal} ,
	 { VK_OEM_COMMA, XK_comma} ,
	 { VK_OEM_MINUS, XK_minus} ,
	 { VK_OEM_PERIOD, XK_period} ,
	 { VK_OEM_2, XK_slash} ,

	 { VK_OEM_3, XK_quoteleft} ,

	 { VK_OEM_4, XK_bracketleft} ,
	 { VK_OEM_5, XK_backslash} ,
	 { VK_OEM_6, XK_bracketright} ,
	 { VK_OEM_7, XK_quoteright} ,

	 { VK_CANCEL,XK_Cancel} ,
	 { VK_ESCAPE, XK_Escape} ,

	 { 0, 0 }
};

static XModifierKeymap ModifierKeymapCopy;

/* static procedures */
DWORD DrvKeyboardInit(LPARAM,LPARAM,LPVOID);
DWORD DrvKeyboardTab(void);
static DWORD DrvKeyboardDoNothing(LPARAM,LPARAM,LPVOID);
static DWORD DrvInitKeyboard(void);
DWORD DrvHandleKeyboardEvents(LPARAM, LPARAM, LPVOID);
static DWORD DrvGetKeyboardType(LPARAM,LPARAM,LPVOID);
static DWORD DrvGetKBCodePage(LPARAM,LPARAM,LPVOID);
static DWORD DrvKeyboardState(LPARAM,LPARAM,LPVOID);
static DWORD DrvKeyState(LPARAM,LPARAM,LPVOID);
static DWORD DrvButtonState(LPARAM,LPARAM,LPVOID);
static DWORD DrvGetKeyNameText(LPARAM,LPARAM,LPVOID);
static DWORD DrvMapVirtualKey(LPARAM,LPARAM,LPVOID);
static DWORD DrvGetChar(LPARAM,LPARAM,LPVOID);

static UINT MapShiftedKey(UINT);
static UINT KeysymToVKcode(UINT);
static UINT VKcodeToKeysym(UINT);

static TWINDRVSUBPROC DrvKeyboardEntryTab[] = {
	DrvKeyboardInit,
	DrvKeyboardDoNothing,
	DrvHandleKeyboardEvents,
	DrvGetKeyboardType,
	DrvGetKBCodePage,
	DrvKeyboardState,
	DrvKeyState,
	DrvButtonState,
	DrvGetKeyNameText,
	DrvMapVirtualKey,
	DrvGetChar
};

/* external library hook */
extern TWINLIBCALLBACKPROC lpLibCallback;

DWORD
DrvKeyboardInit(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	DWORD dwReturn;

	if (dwParam2)
	{
		dwReturn = DrvInitKeyboard();
		return dwReturn;
	}
	else
		return 0L;
}
		
DWORD
DrvKeyboardTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvKeyboardEntryTab[0] = DrvKeyboardInit;
        DrvKeyboardEntryTab[1] = DrvKeyboardDoNothing;
        DrvKeyboardEntryTab[2] = DrvHandleKeyboardEvents;
        DrvKeyboardEntryTab[3] = DrvGetKeyboardType;
        DrvKeyboardEntryTab[4] = DrvGetKBCodePage;
        DrvKeyboardEntryTab[5] = DrvKeyboardState;
        DrvKeyboardEntryTab[6] = DrvKeyState;
        DrvKeyboardEntryTab[7] = DrvButtonState;
        DrvKeyboardEntryTab[8] = DrvGetKeyNameText;
        DrvKeyboardEntryTab[9] = DrvMapVirtualKey;
        DrvKeyboardEntryTab[10] = DrvGetChar;
#endif
	return (DWORD)DrvKeyboardEntryTab;
}
static DWORD
DrvInitKeyboard(void)
{
    PRIVATEDISPLAY *dp = GETDP();
    XModifierKeymap *xmkp;
    int n;
    KeySym ks0,ks1;
    UINT vk,vku,vks;

    /* Get a copy of the current modifier keymap */
    xmkp = XGetModifierMapping(dp->display);
    if (xmkp)
    {
	ModifierKeymapCopy.max_keypermod = xmkp->max_keypermod;

	ModifierKeymapCopy.modifiermap = 
	    (KeyCode *) WinMalloc(sizeof(KeyCode) * xmkp->max_keypermod * 8);
	    
	memcpy(ModifierKeymapCopy.modifiermap, 
	       xmkp->modifiermap, 
	       sizeof(KeyCode) * xmkp->max_keypermod * 8);

	XFreeModifiermap(xmkp);
    }

    for(n=0;n<256;n++) {
	KeyMap[n].vk_state = KS_UP;

	ks0 = XKeycodeToKeysym(dp->display,n,KS_UNSHIFTED);
	ks1 = XKeycodeToKeysym(dp->display,n,KS_SHIFTED);

	if (ks0 == 0)
	    continue;

	if (ks1 & 0xff00) {
	    vku = KeysymToVKcode(ks0);
	    vks = KeysymToVKcode(ks1); 
	    if (vku && (vks == 0))
		vks = vku;
	    if (vks && (vku == 0))
		vku = vks;

	    KeyMap[n].virtual     = vku;
	    KeyMap[n].keypad      = vks;
			
	    KeyMap[vku].vkUnshifted = MapShiftedKey(vku);
	    KeyMap[vku].vkShifted   = MapShiftedKey(vku);

	    KeyMap[vks].vkUnshifted = MapShiftedKey(vks);
	    KeyMap[vks].vkShifted   = MapShiftedKey(vks);

	}
	else {
	    if (ks1) {
		/* map virtual key to wm_char */
		vk = KeysymToVKcode(ks0); 
		KeyMap[n].virtual     = vk;
		KeyMap[n].keypad     = vk;

		if (vk == 0)
		    continue;

		KeyMap[vk].vkUnshifted = ks0;
		KeyMap[vk].vkShifted   = ks1;
	    }
	    else {
		vk = KeysymToVKcode(ks0); 
		KeyMap[n].virtual     = vk;
		KeyMap[n].keypad      = vk;

		if(vk == 0)
		    continue;

		vku = MapShiftedKey(vk);
		KeyMap[vk].vkUnshifted = vku;
		KeyMap[vk].vkShifted   = vku;
	    }
	}
    }
    for(n = 0; n < 256; n++) 
    {
	KeyMap[n].vkShiftedCtrl = 0;
	KeyMap[n].vkUnshiftedCtrl = 0;

	if ((KeyMap[n].vkShifted & 0xe0) == 0x40) /* @,A...Z,[,\,],^,_ */
	{
	    KeyMap[n].vkShiftedCtrl = (KeyMap[n].vkShifted & ~0xe0);
	}

	if (isalpha(KeyMap[n].vkShifted) || 
	    (KeyMap[n].vkUnshifted & 0xe0) == 0x40)
	{
	    KeyMap[n].vkUnshiftedCtrl = (KeyMap[n].vkUnshifted & ~0xe0);
	}
    }

    /* set these according to better heuristics */
    dp->KeyBoardType    = 4;
    dp->KeyBoardSubType = 0;
    dp->KeyBoardFuncKeys = 12;

    return 1L;
}

static UINT
MapShiftedKey(UINT vk)
{
    UINT alt = 0;

    switch (vk) {
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
	    alt = VK_0 + vk - VK_NUMPAD0;
	    break;

	case VK_ADD:
	    alt = XK_plus;
	    break;

	case VK_SEPARATOR:
	    alt = XK_comma;
	    break;

	case VK_SUBTRACT:
	    alt = XK_minus;
	    break;

	case VK_DECIMAL:
	    alt = XK_period;
	    break;

	case VK_DIVIDE:
	    alt = XK_slash;
	    break;

	case VK_BACK:
	case VK_TAB:
	case VK_CLEAR:
	case VK_RETURN:
	case VK_SPACE:
	case VK_ESCAPE:
	    alt = vk;
	    break;
    }
    return alt;
}

static UINT
KeysymToVKcode(UINT keysym)
{
    KEYTABLE *kt = &VK2KeySym[0];

    for (; kt->ToCode; kt++)
	if (kt->ToCode == keysym)
	    return kt->FromCode;

    return 0;
}

static UINT
VKcodeToKeysym(UINT VKcode)
{
    KEYTABLE *kt = &VK2KeySym[0];

    for (; kt->FromCode; kt++)
	if (kt->FromCode == VKcode)
	    return kt->ToCode;

    return 0;
}

static DWORD
DrvGetKeyboardType(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    int fnKeybInfo = (int)dwParam1;
    PRIVATEDISPLAY *dp = GETDP();

    switch (fnKeybInfo) {
	case KT_TYPE:
	    return (DWORD)dp->KeyBoardType;

	case KT_SUBTYPE:
	    return (DWORD)dp->KeyBoardSubType;

	case KT_NUMFKEYS:
	    return (DWORD)dp->KeyBoardFuncKeys;

	default:
	    return 0L;
    }
}

static DWORD
DrvGetKBCodePage(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    PRIVATEDISPLAY *dp = GETDP();

    return (DWORD)dp->CodePage;
}

static DWORD
DrvKeyboardState(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    BOOL fSet = (BOOL)dwParam1;
    LPBYTE lpbKeyState = (LPBYTE)lpStruct;
    LPBYTE lpb = lpbKeyState;
    int n;

    if (fSet == 0) {
	for(n=0;n<256;n++)
	    if(KeyMap[n].vk_state & KS_DN)
		*lpb++ = 0x80;
	    else
		*lpb++ = 0;
    }
    else {
	for(n=0;n<256;n++)
	    if(*lpb++ & 0x80)
	  	KeyMap[n].vk_state = KS_DN;
	    else
	  	KeyMap[n].vk_state = KS_UP;
		
    }
    return 0;
}

static DWORD
DrvKeyState(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    int nVirtKey = (int)dwParam1;
    BOOL fAsync = (BOOL)dwParam2;
    DWORD retcode;
    char keys_return[32];
    UINT keysym;
    KeyCode keycode;
    int index;

    if (!fAsync)
	retcode = (KeyMap[nVirtKey].vk_state & (KS_DN | KS_LK)) ? 0x8000 : 0;
    else {
	PRIVATEDISPLAY *dp = GETDP();
	if (nVirtKey == VK_LBUTTON || nVirtKey == VK_MBUTTON ||
	    nVirtKey == VK_RBUTTON) {
	    Window window_return,child_return;
	    int root_x,root_y,win_x,win_y;
	    UINT mask,mask_return;;
	    XQueryPointer(dp->display,RootWindow(dp->display,dp->screen),
		&window_return,&child_return,
		&root_x, &root_y, &win_x, &win_y,
		&mask_return);
	    mask = 0;
	    if (nVirtKey == VK_LBUTTON)
		mask = Button1Mask;
	    else if (nVirtKey == VK_MBUTTON)
		mask = Button2Mask;
	    else if (nVirtKey == VK_RBUTTON)
		mask = Button3Mask;
	    retcode = (mask_return & mask)?0x8000:0;
	}
	else {
	    XQueryKeymap(dp->display,keys_return);
	    keysym = VKcodeToKeysym((UINT)nVirtKey);
	    keycode = XKeysymToKeycode(dp->display,keysym);
	    index = keycode / 8;
	    retcode = ((keys_return[index] >> (keycode % 8)) & 1)?
		0x8000:0;
	}
    }

    retcode |= (KeyMap[nVirtKey].vk_state & KS_LK) ? 1 : 0;

    return retcode;
}

static DWORD
DrvButtonState(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPMSG lpMsg = (LPMSG)lpStruct;
    switch (lpMsg->message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	    KeyMap[VK_LBUTTON].vk_state = KS_DN;
	    break;

	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
	    KeyMap[VK_MBUTTON].vk_state = KS_DN;
	    break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	    KeyMap[VK_RBUTTON].vk_state = KS_DN;
	    break;

	case WM_LBUTTONUP:
	    KeyMap[VK_LBUTTON].vk_state = KS_UP;
	    break;

	case WM_MBUTTONUP:
	    KeyMap[VK_MBUTTON].vk_state = KS_UP;
	    break;

	case WM_RBUTTONUP:
	    KeyMap[VK_RBUTTON].vk_state = KS_UP;
	    break;
    }

    if (KeyMap[VK_CONTROL].vk_state & KS_DN)
	lpMsg->wParam |= MK_CONTROL;

    if (KeyMap[VK_SHIFT].vk_state & KS_DN)
	lpMsg->wParam |= MK_SHIFT;

    return 0L;
}

static DWORD
DrvBuildKeyMessage(UINT keycode, LPMSG lpMsg)
{
    UINT vk;
    WORD wContext;

    if ( (KeyMap[VK_NUMLOCK].vk_state & KS_LK) &&
		!(KeyMap[VK_SHIFT].vk_state & KS_DN) )
	vk = KeyMap[keycode].keypad;
    else
	vk = KeyMap[keycode].virtual;

    if (vk == 0) {
	char *keyname = XKeysymToString(keycode);
	if (!keyname)
	    keyname = "unknown key";
	
	return 0L;
    }

    lpMsg->wParam = vk;
    if ( lpMsg->message == WM_KEYDOWN )
	KeyMap[vk].vk_state |= KS_DN;
    else
	KeyMap[vk].vk_state &= ~KS_DN;

    if(vk == VK_ALT) {
	return 0L;
    } 

    if (lpMsg->message == WM_KEYUP) {
	wContext = 0xc000;	/* key transition bits */
    }
    else {
	wContext = 0x0000;
    }
    wContext |= keycode;

    if (KeyMap[VK_ALT].vk_state & KS_DN) {
	wContext |= 0x2000;	/* alt key pressed bit */

	if (lpMsg->message == WM_KEYUP)
	    lpMsg->message = WM_SYSKEYUP;

	if (lpMsg->message == WM_KEYDOWN)
	    lpMsg->message = WM_SYSKEYDOWN;
    }
		
    lpMsg->lParam = MAKELONG(1,wContext);

    return (DWORD)vk;
}

static DWORD
DrvMapVirtualKey(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    UINT uKeyCode = (UINT)dwParam1;
    UINT fuMapType = (UINT)dwParam2;

    switch (fuMapType) {
	case 0:
	    return VKcodeToKeysym(uKeyCode);

	case 1:
	    return KeysymToVKcode(uKeyCode);

	case 2:
	    return uKeyCode;
    }

    return 0;
}

static void
DrvSynchronizeModifier(int modifier_idx, int is_on)
{
    XModifierKeymap *xmkp = &ModifierKeymapCopy;
    int i;
    unsigned int vk;			/* our virtual keycode */
    KeyCode kc;				/* X keycode */
    
    for (i = 0; i < xmkp->max_keypermod; i++)
    {
	kc = xmkp->modifiermap[i + modifier_idx * xmkp->max_keypermod];
	vk = KeyMap[kc].virtual;

	if (vk == VK_CAPITAL || vk == VK_NUMLOCK)
	{
	    if (is_on)
		KeyMap[vk].vk_state |= KS_LK;
	    else
		KeyMap[vk].vk_state &= ~KS_LK;
	}
	else if (vk)
	{
	    if (is_on)
		KeyMap[vk].vk_state |= KS_DN;
	    else
		KeyMap[vk].vk_state &= ~KS_DN;
	}
    }
}

void
DrvSynchronizeAllModifiers(unsigned int state)
{
    DrvSynchronizeModifier(0, (state & ShiftMask));
    DrvSynchronizeModifier(1, (state & LockMask));
    DrvSynchronizeModifier(2, (state & ControlMask));
    DrvSynchronizeModifier(3, (state & Mod1Mask));
    DrvSynchronizeModifier(4, (state & Mod2Mask));
    DrvSynchronizeModifier(5, (state & Mod3Mask));
    DrvSynchronizeModifier(6, (state & Mod4Mask));
    DrvSynchronizeModifier(7, (state & Mod5Mask));
}

DWORD
DrvHandleKeyboardEvents(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    XEvent *event = (XEvent *)lpStruct;
    PRIVATEDISPLAY *dp = GETDP();
    DWORD dwRet = 0L;
    MSG msg;

    /* 
     * The modifiers may have been changed when we weren't looking. 
     */
    DrvSynchronizeAllModifiers(event->xkey.state);

    if ( (event->type == KeyPress) || (event->type == KeyRelease) ) {
	dp->ServerTime = event->xkey.time;

	msg.hwnd    = 0;
	msg.message = (event->type==KeyPress) ? WM_KEYDOWN:WM_KEYUP;
	msg.time    = event->xkey.time;
	msg.pt.x    = event->xkey.x_root;
	msg.pt.y    = event->xkey.y_root;

	/*
	 *	Synthesize a KeyPress event to match the KeyRelease
	 *	event for CAPS and NUM lock keys.
	 */
	if ( KeyMap[event->xkey.keycode].virtual == VK_CAPITAL ||
	     KeyMap[event->xkey.keycode].virtual == VK_NUMLOCK ) {
	    if ( event->type == KeyPress )
		KeyMap[KeyMap[event->xkey.keycode].virtual].vk_state |= KS_LK;
	    else {
		KeyMap[KeyMap[event->xkey.keycode].virtual].vk_state &= ~KS_LK;
		msg.message = WM_KEYDOWN;

		if ( DrvBuildKeyMessage(event->xkey.keycode,&msg) )
		    lpLibCallback(TWINLIBCALLBACK_KEYBOARD, 0, 0, (LPVOID)&msg);
		    msg.message = WM_KEYUP;
	    }
	}

	if ( DrvBuildKeyMessage(event->xkey.keycode,&msg) )
	    dwRet = lpLibCallback(TWINLIBCALLBACK_KEYBOARD, 0, 0, (LPVOID)&msg);

	/*
	 *	Synthesize a KeyRelease event to match the KeyPress
	 *	event for CAPS and NUM lock keys.
	 */
	if ( event->type == KeyPress &&
	     ( KeyMap[event->xkey.keycode].virtual == VK_CAPITAL ||
	       KeyMap[event->xkey.keycode].virtual == VK_NUMLOCK ) ) {
	    msg.message = WM_KEYUP;

	    if ( DrvBuildKeyMessage(event->xkey.keycode,&msg) )
		lpLibCallback(TWINLIBCALLBACK_KEYBOARD, 0, 0, (LPVOID)&msg);
	}

    }

    return dwRet;
}

static DWORD
DrvGetKeyNameText(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LONG lParam = (LONG)dwParam1;
    int cbMaxKey = (int)dwParam2;
    LPSTR lpszBuffer = (LPSTR)lpStruct;
    PRIVATEDISPLAY *dp = GETDP();
    KeySym keysym;
    LPSTR lpKeyStr;

    keysym = XKeycodeToKeysym(dp->display,HIWORD(lParam),0);
    lpKeyStr = XKeysymToString(keysym);

    if (lpKeyStr) {
	strncpy(lpszBuffer,lpKeyStr,cbMaxKey-1);
	lpszBuffer[cbMaxKey] = '\0';
    }
    else
	lpszBuffer[0] = '\0';

    return 0L;
}

static DWORD
DrvGetChar(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    UINT vk = (UINT)dwParam1;
    UINT retChar;

    retChar = KeyMap[vk].vkUnshifted;

    if ( KeyMap[VK_CAPITAL].vk_state & KS_LK && isalpha(vk) ) 
    {
	if ( !(KeyMap[VK_SHIFT].vk_state & KS_DN) )
	    retChar = KeyMap[vk].vkShifted;
    }
    else if ( KeyMap[VK_SHIFT].vk_state & KS_DN )
    {
	retChar = KeyMap[vk].vkShifted;
    }

    if ( KeyMap[VK_CONTROL].vk_state & KS_DN )
    {
	retChar = 0;
	
	if ( KeyMap[VK_SHIFT].vk_state & KS_DN )
	{
	    if ( KeyMap[vk].vkShiftedCtrl )
	    {
		retChar = KeyMap[vk].vkShiftedCtrl;
	    }
	}
	else
	{
	    if ( KeyMap[vk].vkUnshiftedCtrl )
	    {
		retChar = KeyMap[vk].vkUnshiftedCtrl;
	    }
	}
    }

    return (DWORD)retChar;
}

static DWORD
DrvKeyboardDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

