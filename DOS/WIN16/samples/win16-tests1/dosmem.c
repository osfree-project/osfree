/*
 * win16 dos memory test programm
 *
 * Copyright 2007 Google, Jennifer Lai
 * Copyright 2004 Markus Amsler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Compiled with Open Watcom C16 Version 1.6 (http://www.openwatcom.org)
 *   > wcl -l=windows dosmem.c
 *
 * Tests following functions:
 *   GlobalDosAlloc
 *   GlobalDosFree
 *   GlobalFree
 *   GlobalReAlloc
 *   GlobelHandle
 *   GlobalSize
 *
 * FIXME: test more functions
 *
 * direct DOS memory manipulation with int 21 didn't work in win 3.11 nor in 
 * ntvdm (but in wine :-) ).
 *
 * FIXME: add dpmi memory tests
 * FIXME: fill allocated memory
 */

#include <stdio.h>
#include <windows.h>
#include <toolhelp.h>
#include <i86.h>
#include "test.h"

/* a few useful undocumented KERNEL functions */
extern DWORD FAR PASCAL GetSelectorBase(unsigned sel);
extern DWORD FAR PASCAL GetSelectorLimit(unsigned sel);
extern unsigned int FAR PASCAL SetSelectorBase(unsigned sel, DWORD base);
extern unsigned int FAR PASCAL SetSelectorLimit(unsigned sel, DWORD limit);


int dos_free_paragraphs(void)
{
    union REGS rin;
    union REGS rout;

    rin.x.ax = 0x4800;
    rin.x.bx = 0xffff;

    int86(0x21, &rin, &rout);

	ok(rout.x.cflag, "dos_free_paragraphs failed!\n");
    if (!rout.x.cflag) {
	trace(" context in: ");
	dump_context(&rin);
	trace(" context out: ");
	dump_context(&rout);
	return 0;
    }
    return rout.x.bx;
}

void dump_context(union REGS *r)
{
    trace("ax=%04x bx=%04xh cx=%04xh dx=%04xh cflag=%04xh\n", r->x.ax,
	   r->x.bx, r->x.cx, r->x.dx, r->x.cflag);

}

int dos_alloc(WORD paras)
{
    union REGS rin;
    union REGS rout;

    rin.x.ax = 0x4800;
    rin.x.bx = paras;

    int86(0x21, &rin, &rout);
#ifdef DEBUG
    dump_context(&rout);
#endif

	ok(!rout.x.cflag, "dos_alloc failed!\n");
	if (rout.x.cflag) {
	trace(" context in: ");
	dump_context(&rin);
	trace(" context out: ");
	dump_context(&rout);
	return 0;
    }
    return 1;
}


/*
 * checks wether the given amount of memory could be reserved
 */
BOOL got_memory(DWORD size)
{
    DWORD ret;

    ret = GlobalDosAlloc(size);
    if (!ret)
	return FALSE;
    GlobalDosFree(LOWORD(ret));
    return TRUE;
}

/*
 * returns the largest block size in bytes that could be reserved.
 * pretty time consuming, but good testing. It probes with alloc / free
 */
DWORD dos_largest(void)
{
    DWORD max, current, step;
    BOOL gotit;

    /* do some sort of binary search */
    max = 0x100000;
    current = max >> 1;
    step = max >> 2;

    while (step > 0) {
	gotit = got_memory(current);
#ifdef DEBUG
	trace
	    ("dos_largest max=%04x%04xh current=%04x%04xh step=%04x%04xh gotit=%d\n",
	     HIWORD(max), LOWORD(max), HIWORD(current), LOWORD(current),
	     HIWORD(step), LOWORD(step), gotit);
#endif
	if (gotit) {
	    current += step;
	} else {
	    current -= step;
	}
	step = step >> 1;
    }

    if (current == 1) {
	/* FIXME: 1 byte is not eactly no memory */
	return 0;
    }

    ok(got_memory(current - 1), "ERROR: didn't got_memory(current-1) should not happen\n");
    ok(!got_memory(current), "ERROR: got_memory(current) should not happen\n");
    ok(!got_memory(current + 1), "ERROR: got_memory(current+1) should not happen\n");
	if(got_memory(current) || got_memory(current + 1) || (!got_memory(current - 1)))
		return 0;
    return current - 1;
}


/*
 * The total amount of DOS memory in bytes.
 * pretty time consuming, but good testing. It sums up the largest blocks recursivly.
 */
DWORD dos_available(void)
{
    DWORD ret, largest, avail;

    largest = dos_largest();
    if (!largest) {
	/* recursion termination: no DOS memory */
#ifdef DEBUG
	trace("dos_available return 0\n");
#endif
	return 0;
    }

    ret = GlobalDosAlloc(largest);
    if (!ret)
	return 0;
    avail = dos_available();
    GlobalDosFree(LOWORD(ret));

#ifdef DEBUG
    trace("dos_available largest=%04x%04xh available=%04x%04xh\n",
	   HIWORD(largest), LOWORD(largest)
	   , HIWORD(avail), LOWORD(avail));
#endif
    return largest + avail;
}

/* returns allocation overhead for given block size or -1 on error */
int dos_alloc_overhead(DWORD size)
{
    DWORD avail, dos;
    int ret;

    avail = dos_available();
    dos = GlobalDosAlloc(size);
    if (!dos) {
	return -1;
    }
    ret = avail - dos_available() - size;
    GlobalDosFree(LOWORD(dos));
    return ret;
}


START_TEST(dosmem)
{
    DWORD largest_start, avail_start;
    DWORD test_size;
    DWORD avail, avail2, avail3;
    DWORD dos;
    DWORD handle, h2;

    trace("  Win16 DOS memory test programm\n");

    /* 
     * First check if dos_largest and dos_available works.
     * Very good test for GlobasDosAlloc and GlobalDosFree
     */
    trace("  Checking available memory\n");
    largest_start = dos_largest();
    trace("  Largest Block %04x%04xh bytes\n", HIWORD(largest_start),
	   LOWORD(largest_start));

    avail_start = dos_available();
    trace("  Available memory %04x%04xh bytes\n", HIWORD(avail_start),
	   LOWORD(avail_start));

    trace("  Testing GlobalReAlloc and GlobalHandle\n");
    test_size = (largest_start / 3) & 0xfffff000;
#ifdef DEBUG
    trace("  test_size %04x%04xh bytes\n", HIWORD(test_size),
	   LOWORD(test_size));
#endif
	dos = GlobalDosAlloc(test_size);
    ok(dos, "ERROR: GlobalDosAlloc (0x1000) failed\n");
    avail = dos_available();
#ifdef DEBUG
    trace("  avail %04x%04xh bytes\n", HIWORD(avail), LOWORD(avail));
#endif
    handle = GlobalHandle(LOWORD(dos));
	ok(handle, "ERROR: GlobalHandle failed\n");
    /* test shrinking */
    h2 = GlobalReAlloc(handle, test_size >> 1, 0);
#ifdef DEBUG
    trace("  handle %04x%04xh\n", HIWORD(handle), LOWORD(handle));
    trace("  h2 %04x%04xh\n", HIWORD(h2), LOWORD(h2));
#endif
    ok(h2, "ERROR: GlobalReAlloc 1 failed\n");
    avail2 = dos_available();
#ifdef DEBUG
    trace("  avail2 %04x%04xh bytes\n", HIWORD(avail2), LOWORD(avail2));
#endif
    ok((avail + (test_size >> 1)) == avail2, "ERROR: shrink arithmetic failed\n  avail %04x%04xh bytes\n  avail2 %04x%04xh bytes\n  test_size>>1 %04x%04xh bytes\n", HIWORD(avail), LOWORD(avail), HIWORD(avail2), LOWORD(avail2), HIWORD(test_size >> 1), LOWORD(test_size >> 1));
    /* test growing */
    h2 = GlobalReAlloc(h2, test_size << 1, 0);
#ifdef DEBUG
    trace("  h2 %04x%04xh\n", HIWORD(h2), LOWORD(h2));
#endif
    ok(h2, "ERROR: GlobalReAlloc 2 failed\n");
    avail2 = dos_available();
#ifdef DEBUG
    trace("  avail2 %04x%04xh bytes\n", HIWORD(avail2), LOWORD(avail2));
#endif
    ok((avail - test_size) == avail2, "ERROR: grow arithmetic failed\n  avail %04x%04xh bytes\n  avail2 %04x%04xh bytes\n  test_size %04x%04xh bytes\n",
	HIWORD(avail), LOWORD(avail), HIWORD(avail2), LOWORD(avail2), HIWORD(test_size), LOWORD(test_size));
    GlobalDosFree(h2);
    avail3 = dos_available();
#ifdef DEBUG
    trace("  avail3 %04x%04xh bytes\n", HIWORD(avail3), LOWORD(avail3));
#endif
    ok(avail3 != avail2, "ERROR: GlobalDosFree had no effect\n");

    trace("  Testing whether cleanup worked\n");
    ok(avail_start == avail3, "ERROR: not all memory was freed\n");
    ok(largest_start == dos_largest(), "ERROR: largest differ\n");
    trace("  DONE\n");

}
