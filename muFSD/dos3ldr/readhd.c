/*
 *    ReadRun function wrapper
 *    callable from C.
 *   
 *    (c) osFree project
 *    valerius 2007, Jan 13 
 */

/*; 
 *; ReadRun:
 *; Reads a contiguous run of sectors.
 *;
 *;          Input:
 *;          dx:ax  -- logical sector number to read from
 *;          cl     -- sector count to read 1 <= cl <= 128
 *;          es:di  -- address to read to
 *;          bl     -- drive number
 *;          ds     == 0x7c0 -- a bootsector segment
 */

// int readhd(u8 device)
