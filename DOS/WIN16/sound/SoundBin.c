
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define NOAPIPROTO
#include "windows.h"
#include "Willows.h"

#include "WinDefs.h"
#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "Resources.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "ModTable.h"

extern long int OpenSound();
extern long int CloseSound();
extern long int SetVoiceQueueSize();
extern long int SetVoiceNote();
extern long int SetVoiceAccent();
extern long int StartSound();
extern long int StopSound();

extern long int Trap();
extern long int IT_V();
extern long int IT_V();
extern long int IT_2I();
extern long int IT_4I();
extern long int IT_5I();
extern long int IT_V();
extern long int IT_V();


 /* Interface Segment Image SOUND: */

static long int (*seg_image_SOUND_0[])() =
#if !defined(TWIN_EMPTY_MODTABLE)
{	/* nil */	0, 0,
	/* 001 */	IT_V,  OpenSound,
	/* 002 */	IT_V,  CloseSound,
	/* 003 */	IT_2I,  SetVoiceQueueSize,
	/* 004 */	IT_4I,  SetVoiceNote,
	/* 005 */	IT_5I, SetVoiceAccent,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	IT_V,  StartSound,
	/* 00a */	IT_V,  StopSound,
	/* 00b */	Trap, 0,
	/* 00c */	Trap, 0,
	/* 00d */	Trap, 0,
	/* 00e */	Trap, 0,
	/* 00f */	Trap, 0,
	/* 010 */	Trap, 0,
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
	/* 00c */	Trap, 0,
	/* 00d */	Trap, 0,
	/* 00e */	Trap, 0,
	/* 00f */	Trap, 0,
	/* 010 */	Trap, 0,
	0, 0
};
#endif

 /* Segment Table SOUND: */

SEGTAB SegmentTableSOUND[] =
{	{ (char *) seg_image_SOUND_0, 128, TRANSFER_CALLBACK, 128, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

