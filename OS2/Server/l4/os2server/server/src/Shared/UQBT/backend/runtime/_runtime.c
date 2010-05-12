/*==============================================================================
 * FILE:       _runtime.c
 * OVERVIEW:   This file contains the runtime support code for the UQBT
 *             binary translator.
 *
 * Copyright (C) 2000, The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision: 1.4 $
 *
 * 03 Jul 00 - Mike: created
 * 10 Jul 00 - Mike: Replaced call to library function bsearch with a simple
 *              binary search implemented in C. Speeds up very register call
 *              intensive code by about 9 times.
 * 11 Apr 01 - Mike: map renamed to _map to avoid name collisions
 */


/* All the functions are prototyped as void returning, no parameters. This is
   often not true, but we need to give a type to keep the C++ compiler happy */
typedef void (*PVOID)();

/* The map consists of an array of these simple structs */
typedef struct {
    unsigned int nat;           /* Native (source machine) address */
    PVOID host;                 /* Host (target machine) address */
} MAP_EL;
typedef MAP_EL* PEL;


/* Include the generated file. Prototypes all the callees as PVOIDs,
   declares the map (called _map), and defines NUM_MAP as the #entries */
#include "_runtime.h"

extern _uqbt_nat;

PVOID __translate_addr() {
    /* Lookup the map */
    MAP_EL* pEnt;
    /* Binary search algorithm. bsearch() turns out to be too slow! */
    int bot = 0;
    int top = NUM_MAP-1;
    int curr;
    do {
        curr = (bot + top) >> 1;
        pEnt = _map + curr;      /* Point to the current entry */
        if (pEnt->nat > _uqbt_nat)
            top = curr-1;
        else if (pEnt->nat < _uqbt_nat)
            bot = curr+1;
        else
            /* Found the address */
            return pEnt->host;
    } while (bot <= top);
        
    /* Error! */
    printf("\nUQBT translation error: could not find address %X in the function"
        " map\n", _uqbt_nat);
    exit(1);
    return 0;
}

