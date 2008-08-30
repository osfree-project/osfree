/*
 * Copyright (C) 1999-2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       arm.uqbt.h
 * OVERVIEW:   Provides macros and definitions for the UQBT binary translator's
 *             generated code for ARM targets
 *
 * Copyright (C) 1999-2000, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

#define int32    int
#define int16    short
#define int8     char
#define int64    long long
#define float32  float
#define float64  double
#define floatmax long double

/*==============================================================================
 * The following declarations are used with the low-level C backend.
 * Each generated C file #includes this header file, so the _swap8* procedures
 * are declared static to avoid multiple definitions.
 *
 * Note: If you change one of these declarations, you must change the
 *       corresponding one below!
 *============================================================================*/

#define _swap2(src) ( \
        _arg2 = (unsigned short)(src), \
        _res = ((_arg2 >> 8) | (_arg2 << 8)) & 0xFFFF \
)

#define _swap4(src) ( \
        _arg = (unsigned) src, \
        _mask = 0xff00ff, \
        _wrk = _arg & _mask, \
        _arg = (unsigned)_arg >> 8, \
        _wrk = _wrk << 8, \
        _wrk = _wrk | (_arg & _mask), \
        _arg = (_wrk >> 16) | (_wrk << 16) \
)

/* Swap halves separately, and store (swapped) in the res array */
static float64 _swap8f(float64 src)
{
    unsigned res[2];           /* Result */
    register unsigned arg = *(unsigned*)&src;
    register unsigned wrk = arg & 0xff00ff;
    arg = arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[1] = (wrk >> 16) | (wrk << 16);
    arg = *(((unsigned*)&src)+1);
    wrk = arg & 0xff00ff;
    arg = (unsigned)arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[0] = (wrk >> 16) | (wrk << 16);
    return *(float64*)&res;
}

/* Swap halves separately, and store (swapped) in the res array */
static int64 _swap8i(int64 src)
{
    unsigned res[2];           /* Result */
    register unsigned arg = *(unsigned*)&src;
    register unsigned wrk = arg & 0xff00ff;
    arg = arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[1] = (wrk >> 16) | (wrk << 16);
    arg = *(((unsigned*)&src)+1);
    wrk = arg & 0xff00ff;
    arg = (unsigned)arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[0] = (wrk >> 16) | (wrk << 16);
    return *(int64*)&res;
}

extern unsigned _uqbt_nat;           /* Used by register call runtime logic */

/*==============================================================================
 * The following declarations are used with the VPO backends.
 * When a VPO backend is used, the C file _util.c is created. It #includes
 * this header file and so declares the various _vpo_swap*() procedures.
 *
 * Note: If you change one of these declarations, you must change the
 *       corresponding one above!
 *============================================================================*/

int32 _vpo_swap2(int32 src) {
    register unsigned _arg2, _res;
    _swap2(src);
    /* printf("_vpo_swap2: from 0x%x to 0x%x\n", src, _res); */
    return _res;
}

int32 _vpo_swap4(int32 src) {
    register unsigned _arg, _wrk, _mask;
    _swap4(src);
    /* printf("_vpo_swap4: from 0x%x to 0x%x\n", src, _arg); */
    return _arg;
}

/* Swap halves separately, and store (swapped) in the res array.
 * NB: same as _swap8f except not static and has a different name. */
float64 _vpo_swap8f(float64 src)
{
    unsigned res[2];           /* Result */
    register unsigned arg = *(unsigned*)&src;
    register unsigned wrk = arg & 0xff00ff;
    float64 ret;
    arg = arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[1] = (wrk >> 16) | (wrk << 16);
    arg = *(((unsigned*)&src)+1);
    wrk = arg & 0xff00ff;
    arg = (unsigned)arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[0] = (wrk >> 16) | (wrk << 16);
    ret = *(float64*)&res;
    /* printf("_vpo_swap8f: from 0x%x to 0x%x\n", src, ret); */
    return ret;
}

/* Swap halves separately, and store (swapped) in the res array.
 * NB: same as _swap8i except not static and has a different name. */
int64 _vpo_swap8i(int64 src)
{
    unsigned res[2];           /* Result */
    register unsigned arg = *(unsigned*)&src;
    register unsigned wrk = arg & 0xff00ff;
    int64 ret;
    arg = arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[1] = (wrk >> 16) | (wrk << 16);
    arg = *(((unsigned*)&src)+1);
    wrk = arg & 0xff00ff;
    arg = (unsigned)arg >> 8;
    wrk = wrk << 8;
    wrk = wrk | (arg & 0xff00ff);
    res[0] = (wrk >> 16) | (wrk << 16);
    ret = *(int64*)&res;
    /* printf("_vpo_swap8i: from 0x%x to 0x%x\n", src, ret); */
    return ret;
}

/* Swap argv to be opposite endianness. This is called from the main()
 * procedure in VPO-generated programs where the target platform has
 * different endianness than the source platform. This allows us to deal
 * with the endianness conversions by just swapping all memory reads. */
void _vpo_swapargv(int32 argc, char** argv) {
    /* Fix argv to be opposite endianness */
    register unsigned _arg, _wrk, _mask;
    int i;
    for (i=0;  i < argc;  i++) {
        _swap4(argv[i]);
        argv[i] = _arg;
    }
}
