/*==============================================================================
 * FILE:       sparc.uqbt.h
 * OVERVIEW:   Provides macros and definitions for the UQBT binary translator's
 *             generated code for Sparc targets
 *
 * Copyright (C) 1999-2000, The University of Queensland, BT group
 *============================================================================*/

#define int32   int
#define int16   short
#define int8    char
#define int64   long long
#define float32 float
#define float64 double
#define floatmax long double

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
