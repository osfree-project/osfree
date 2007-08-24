/*

gbmdlgrc.h - Resource ID's etc.

The HID_ help ID's must be decimal to keep IPFCPREP and IPFC happy.
A calling program using GBM dialog should not use Help ID's in the range
>= HID_GBM_MIN and < HID_GBM_MAX, as these are used below and are reserved for
use by GBM dialog. Only HID_GBM_FILEDLG and HID_GBM_SUPPORT will remain
unchanged. The others may change/disappear/be-added-to and should not be
referred to by :link. tags in application help files.

The DID_ control ID's are the additional controls on the GBM dialog.
If you provide your own subclass dialog procedure, you may wish to use these.
If you add your own additional controls, ensure they are >= DID_GBM_MAX.

If help is requested, then the help ID will always be for a mythical control
DID_GBM_FILEDLG. Thus the application need only supply the following help
tables.

	HELPTABLE RID_HELP_TABLE
		{
		HELPITEM RID_GBM_FILEDLG, RID_GBM_FILEDLG, HID_GBM_FILEDLG
		etc.
		}

	HELPSUBTABLE RID_GBM_FILEDLG
		{
		HELPSUBITEM DID_GBM_FILEDLG, HID_GBM_FILEDLG
		}

*/

#define	RID_GBM_FILEDLG		512

#define	DID_GBM_MIN		4096
#define	DID_GBM_FILEDLG		4096		/* Can safely refer to this */
#define	DID_GBM_OPTIONS_TXT	4097
#define	DID_GBM_OPTIONS_ED	4098
#define	DID_GBM_MAX		5120

#define	HID_GBM_MIN		2000
#define	HID_GBM_FILEDLG		2000		/* Can safely refer to this */
#define	HID_GBM_SUPPORT		2001		/* Can safely refer to this */
#define	HID_GBM_BITMAP		2002
#define	HID_GBM_GIF		2003
#define	HID_GBM_PCX		2004
#define	HID_GBM_TIFF		2005
#define	HID_GBM_TARGA		2006
#define	HID_GBM_ILBM		2007
#define	HID_GBM_YUV12C		2008
#define	HID_GBM_GREYMAP		2009
#define	HID_GBM_PIXMAP		2010
#define	HID_GBM_KIPS		2011
#define	HID_GBM_IAX		2012
#define	HID_GBM_XBITMAP		2013
#define	HID_GBM_SPRITE		2014
#define	HID_GBM_PSEG		2015
#define	HID_GBM_GEMRAS		2016
#define	HID_GBM_PORTRAIT	2017
#define	HID_GBM_JPEG		2018
#define	HID_GBM_PNG		2019
#define	HID_GBM_OTHERS		2020
#define	HID_GBM_MAX		3000
