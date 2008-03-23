/*    
	OEMTab.c	2.7
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
#include "print.h"

#include "Endian.h"
#include "OEM.h"

extern long int cnb_AdvancedSetupDialog();
extern long int cnb_Control();
extern long int cnb_ColorInfo();
extern long int cnb_Enable();
extern long int cnb_EnumDFonts();
extern long int cnb_EnumObj();
extern long int cnb_Output();
extern long int cnb_RealizeObject();
extern long int cnb_DeviceMode();
extern long int cnb_ExtTextOut();
extern long int cnb_GetCharWidth();
extern long int cnb_ExtDeviceMode();
extern long int cnb_DeviceCapabilities();
extern long int cnb_StretchDIBits();
extern long int cnb_BitBlt();
extern long int cnb_StretchBlt();
extern long int cnb_Pixel();

HSMT_OEMENTRYTAB OEM_tab_PRINTER[] =
{	/* 000 */	{ "", 0 },
	/* 001 */	{ "BITBLT", cnb_BitBlt },
	/* 002 */	{ "COLORINFO", cnb_ColorInfo },
	/* 003 */	{ "CONTROL", cnb_Control },
	/* 004 */	{ "DISABLE", 0 },
	/* 005 */	{ "ENABLE", cnb_Enable },
	/* 006 */	{ "ENUMDFONTS", cnb_EnumDFonts },
	/* 007 */	{ "ENUMOBJ", cnb_EnumObj },
	/* 008 */	{ "OUTPUT", cnb_Output },
	/* 009 */	{ "PIXEL", cnb_Pixel },
	/* 00a */	{ "REALIZEOBJECT", cnb_RealizeObject },
	/* 00b */	{ "STRBLT", 0 },
	/* 00c */	{ "SCANLR", 0 },
	/* 00d */	{ "DEVICEMODE", cnb_DeviceMode },
	/* 00e */	{ "EXTTEXTOUT", cnb_ExtTextOut },
	/* 00f */	{ "GETCHARWIDTH", cnb_GetCharWidth },
	/* 010 */	{ "DEVICEBITMAP", 0 },
	/* 011 */	{ "FASTBORDER", 0 },
	/* 012 */	{ "SETATTRIBUTE", 0 },
	/* 013 */	{ "", 0 },
	/* 014 */	{ "", 0 },
	/* 015 */	{ "DIBTODEVICE", 0 },
	/* 016 */	{ "", 0 },
	/* 017 */	{ "", 0 },
	/* 018 */	{ "", 0 },
	/* 019 */	{ "", 0 },
	/* 01a */	{ "", 0 },
	/* 01b */	{ "STRETCHBLT", cnb_StretchBlt },
	/* 01c */	{ "STRETCHDIB", cnb_StretchDIBits },
	/* 01d */	{ "", 0 },
	/* 01e */	{ "", 0 },
	/* 01f */	{ "", 0 },
	/* 020 */	{ "", 0 },
	/* 021 */	{ "", 0 },
	/* 022 */	{ "", 0 },
	/* 023 */	{ "", 0 },
	/* 024 */	{ "", 0 },
	/* 025 */	{ "", 0 },
	/* 026 */	{ "", 0 },
	/* 027 */	{ "", 0 },
	/* 028 */	{ "", 0 },
	/* 029 */	{ "", 0 },
	/* 02a */	{ "", 0 },
	/* 02b */	{ "", 0 },
	/* 02c */	{ "", 0 },
	/* 02d */	{ "", 0 },
	/* 02e */	{ "", 0 },
	/* 02f */	{ "", 0 },
	/* 030 */	{ "", 0 },
	/* 031 */	{ "", 0 },
	/* 032 */	{ "", 0 },
	/* 033 */	{ "", 0 },
	/* 034 */	{ "", 0 },
	/* 035 */	{ "", 0 },
	/* 036 */	{ "", 0 },
	/* 037 */	{ "", 0 },
	/* 038 */	{ "", 0 },
	/* 039 */	{ "", 0 },
	/* 03a */	{ "", 0 },
	/* 03b */	{ "", 0 },
	/* 03c */	{ "", 0 },
	/* 03d */	{ "", 0 },
	/* 03e */	{ "", 0 },
	/* 03f */	{ "", 0 },
	/* 040 */	{ "", 0 },
	/* 041 */	{ "", 0 },
	/* 042 */	{ "", 0 },
	/* 043 */	{ "", 0 },
	/* 044 */	{ "", 0 },
	/* 045 */	{ "", 0 },
	/* 046 */	{ "", 0 },
	/* 047 */	{ "", 0 },
	/* 048 */	{ "", 0 },
	/* 049 */	{ "", 0 },
	/* 04a */	{ "", 0 },
	/* 04b */	{ "", 0 },
	/* 04c */	{ "", 0 },
	/* 04d */	{ "", 0 },
	/* 04e */	{ "", 0 },
	/* 04f */	{ "", 0 },
	/* 050 */	{ "", 0 },
	/* 051 */	{ "", 0 },
	/* 052 */	{ "", 0 },
	/* 053 */	{ "", 0 },
	/* 054 */	{ "", 0 },
	/* 055 */	{ "", 0 },
	/* 056 */	{ "", 0 },
	/* 057 */	{ "", 0 },
	/* 058 */	{ "", 0 },
	/* 059 */	{ "", 0 },
	/* 05a */	{ "EXTDEVICEMODE", cnb_ExtDeviceMode },
	/* 05b */	{ "DEVICECAPABILITIES", cnb_DeviceCapabilities},
	/* 05c */	{ "", 0 },
	/* 05d */	{ "ADVANCEDSETUPDIALOG", cnb_AdvancedSetupDialog },
	/* 05e */	{ "", 0 },
	/* 05f */	{ "", 0 },
	/* end */	{ 0, 0 }
};

extern long int cnb_CTL3DV2_Stub();
extern long int cnb_CTL3DV2_Ctl3dRegister();
extern long int cnb_CTL3DV2_Ctl3dColorChange();
extern long int cnb_CTL3DV2_Ctl3dSubclassCtl();
extern long int cnb_CTL3DV2_Ctl3dSubclassDlg();
extern long int cnb_CTL3DV2_Ctl3dSubclassDlgEx();
extern long int cnb_CTL3DV2_Ctl3dCtlColor();
extern long int cnb_CTL3DV2_Ctl3dCtlColorEx();
extern long int cnb_CTL3DV2_Ctl3dDlgFramePaint();
#define	cnb_CTL3DV2_Ctl3dUnregister cnb_CTL3DV2_Ctl3dRegister
#define	cnb_CTL3DV2_Ctl3dAutoSubclass cnb_CTL3DV2_Ctl3dRegister
#define	cnb_CTL3DV2_Ctl3dWinIniChange cnb_CTL3DV2_Ctl3dColorChange
#define	cnb_CTL3DV2_Ctl3dEnabled cnb_CTL3DV2_Ctl3dColorChange
#define	cnb_CTL3DV2_Ctl3dGetVer cnb_CTL3DV2_Ctl3dColorChange

HSMT_OEMENTRYTAB OEM_tab_CTL3D[] =
{	/* 000 */	{ "", 0 },
	/* 001 */	{ "CTL3DGETVER", cnb_CTL3DV2_Ctl3dGetVer },
	/* 002 */	{ "CTL3DSUBCLASSDLG", cnb_CTL3DV2_Ctl3dSubclassDlg },
	/* 003 */	{ "CTL3DSUBCLASSCTL", cnb_CTL3DV2_Ctl3dSubclassCtl },
	/* 004 */	{ "CTL3DCTLCOLOR", cnb_CTL3DV2_Ctl3dCtlColor },
	/* 005 */	{ "CTL3DENABLED", cnb_CTL3DV2_Ctl3dEnabled },
	/* 006 */	{ "CTL3DCOLORCHANGE", cnb_CTL3DV2_Ctl3dColorChange },
	/* 007 */	{ "BTNWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 008 */	{ "EDITWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 009 */	{ "LISTWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 00a */	{ "COMBOWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 00b */	{ "STATICWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 00c */	{ "CTL3DREGISTER", cnb_CTL3DV2_Ctl3dRegister },
	/* 00d */	{ "CTL3DUNREGISTER", cnb_CTL3DV2_Ctl3dUnregister },
	/* 00e */	{ "", cnb_CTL3DV2_Stub },
	/* 00f */	{ "", cnb_CTL3DV2_Stub },
	/* 010 */	{ "CTL3DAUTOSUBCLASS", cnb_CTL3DV2_Ctl3dAutoSubclass },
	/* 011 */	{ "CTL3DDLGPROC", cnb_CTL3DV2_Stub },
	/* 012 */	{ "CTL3DCTLCOLOREX", cnb_CTL3DV2_Ctl3dCtlColorEx },
	/* 013 */	{ "CTL3DSETSTYLE", cnb_CTL3DV2_Stub },
	/* 014 */	{ "CTL3DDLGFRAMEPAINT", cnb_CTL3DV2_Ctl3dDlgFramePaint },
	/* 015 */	{ "CTL3DSUBCLASSDLGEX", cnb_CTL3DV2_Ctl3dSubclassDlgEx },
	/* end */	{ 0, 0 }
};

HSMT_OEMENTRYTAB OEM_tab_CTL3DV2[] =
{	/* 000 */	{ "", 0 },
	/* 001 */	{ "CTL3DGETVER", cnb_CTL3DV2_Ctl3dGetVer },
	/* 002 */	{ "CTL3DSUBCLASSDLG", cnb_CTL3DV2_Ctl3dSubclassDlg },
	/* 003 */	{ "CTL3DSUBCLASSCTL", cnb_CTL3DV2_Ctl3dSubclassCtl },
	/* 004 */	{ "CTL3DCTLCOLOR", cnb_CTL3DV2_Ctl3dCtlColor },
	/* 005 */	{ "CTL3DENABLED", cnb_CTL3DV2_Ctl3dEnabled },
	/* 006 */	{ "CTL3DCOLORCHANGE", cnb_CTL3DV2_Ctl3dColorChange },
	/* 007 */	{ "BTNWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 008 */	{ "EDITWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 009 */	{ "LISTWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 00a */	{ "COMBOWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 00b */	{ "STATICWNDPROC3D", cnb_CTL3DV2_Stub },
	/* 00c */	{ "CTL3DREGISTER", cnb_CTL3DV2_Ctl3dRegister },
	/* 00d */	{ "CTL3DUNREGISTER", cnb_CTL3DV2_Ctl3dUnregister },
	/* 00e */	{ "", cnb_CTL3DV2_Stub },
	/* 00f */	{ "", cnb_CTL3DV2_Stub },
	/* 010 */	{ "CTL3DAUTOSUBCLASS", cnb_CTL3DV2_Ctl3dAutoSubclass },
	/* 011 */	{ "CTL3DDLGPROC", cnb_CTL3DV2_Stub },
	/* 012 */	{ "CTL3DCTLCOLOREX", cnb_CTL3DV2_Ctl3dCtlColorEx },
	/* 013 */	{ "CTL3DSETSTYLE", cnb_CTL3DV2_Stub },
	/* 014 */	{ "CTL3DDLGFRAMEPAINT", cnb_CTL3DV2_Ctl3dDlgFramePaint },
	/* 015 */	{ "CTL3DSUBCLASSDLGEX", cnb_CTL3DV2_Ctl3dSubclassDlgEx },
	/* 016 */	{ "CTL3DWININICHANGE", cnb_CTL3DV2_Ctl3dWinIniChange },
	/* end */	{ 0, 0 }
};

