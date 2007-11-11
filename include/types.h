/*
 *   Common types
 *
 *   (c) osFree project
 *   valerius 2007, Jan 13
 */

#ifndef __TYPES_H__
#define __TYPES_H__


 typedef unsigned char        u8;
 typedef unsigned short       u16;
 typedef unsigned long        u32;
 typedef unsigned long long   u64;

 typedef char            i8;
 typedef short           i16;
 typedef long            i32;
 typedef long long       i64;

 /* Far pointer as a structure */
 typedef _Packed struct fp {
   unsigned short off;
   unsigned short seg;
 } fp_t;


#endif
