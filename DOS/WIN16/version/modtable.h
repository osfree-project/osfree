
#ifndef NO_VERSION

 /* Entry Table VER: */

extern long int GetFileResourceSize();
extern long int GetFileResource();
extern long int GetFileVersionInfoSize();
extern long int GetFileVersionInfo();
extern long int VerFindFile();
extern long int VerInstallFile();
extern long int VerLanguageName();
extern long int VerQueryValue();

static ENTRYTAB entry_tab_VER[] =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "GETFILERESOURCESIZE", 0x0008, 0x0010, GetFileResourceSize },
	/* 003 */	{ "GETFILERESOURCE", 0x0008, 0x0018, GetFileResource },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "GETFILEVERSIONINFOSIZE", 0x0008, 0x0030, GetFileVersionInfoSize },
	/* 007 */	{ "GETFILEVERSIONINFO", 0x0008, 0x0038, GetFileVersionInfo },
	/* 008 */	{ "VERFINDFILE", 0x0008, 0x0040, VerFindFile },
	/* 009 */	{ "VERINSTALLFILE", 0x0008, 0x0048, VerInstallFile },
	/* 00a */	{ "VERLANGUAGENAME", 0x0008, 0x0050, VerLanguageName },
	/* 00b */	{ "VERQUERYVALUE", 0x0008, 0x0058, VerQueryValue },
	/* end */	{ 0, 0, 0, 0 }
};
#else
{
	/* 000 */	{ "", 0, 0, 0 },
	/* 001 */	{ "", 0, 0, 0 },
	/* 002 */	{ "", 0, 0, 0 },
	/* 003 */	{ "", 0, 0, 0 },
	/* 004 */	{ "", 0, 0, 0 },
	/* 005 */	{ "", 0, 0, 0 },
	/* 006 */	{ "", 0, 0, 0 },
	/* 007 */	{ "", 0, 0, 0 },
	/* 008 */	{ "", 0, 0, 0 },
	/* 009 */	{ "", 0, 0, 0 },
	/* 00a */	{ "", 0, 0, 0 },
	/* 00b */	{ "", 0, 0, 0 },
	/* end */	{ 0, 0, 0, 0 }
};
#endif


 /* Interface Segment Image VER: */

extern long int IT_GETFILERESOURCESIZE();
extern long int IT_GETFILERESOURCE();
extern long int IT_GETFILEVERSIONINFOSIZE();
extern long int IT_GETFILEVERSIONINFO();
extern long int IT_1UI7LP();
extern long int IT_VERLANGUAGENAME();
extern long int IT_VERQUERYVALUE();

static long int (*seg_image_VER_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	IT_GETFILERESOURCESIZE,		GetFileResourceSize,
	/* 003 */	IT_GETFILERESOURCE,		GetFileResource,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	IT_GETFILEVERSIONINFOSIZE,	GetFileVersionInfoSize,
	/* 007 */	IT_GETFILEVERSIONINFO,		GetFileVersionInfo,
	/* 008 */	IT_1UI7LP,			VerFindFile,
	/* 009 */	IT_1UI7LP,			VerInstallFile,
	/* 00a */	IT_VERLANGUAGENAME,		VerLanguageName,
	/* 00b */	IT_VERQUERYVALUE,		VerQueryValue,
	0
};
#else
{
	/* nil */	0, 0,
	/* 001 */	Trap, 0,
	/* 002 */	Trap, 0,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	Trap, 0,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
	/* 00a */	Trap, 0,
	/* 00b */	Trap, 0,
	0, 0
};
#endif

 /* Segment Table VER: */

static SEGTAB seg_tab_VER[] =
{	{ (char *) seg_image_VER_0, 88, TRANSFER_CALLBACK, 88, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

 /* Module Descriptor for VER: */

static MODULEDSCR hsmt_mod_dscr_VER =
{	"VER",
	entry_tab_VER,
	seg_tab_VER,
	0
};

#endif	/* NO_VERSION */
