/*  OS/2 binary INI file
 *  structure
 *  author: valerius (_valerius (dog) mail (dot) ru)
 *  (definitions were found in Pascal unit interface
 *  section from NOPM by zuko)
 *  2012, Feb 4
 */

#ifndef __OS2INI_H__
#define __OS2INI_H__

typedef unsigned long  ULONG;
typedef unsigned short USHORT;

#define CCHMAXPATH 260
#define BYTESWAP(x) ((x >> 24) | ((x & 0xff0000) >> 8) | ((x & 0xff00) << 8) | ((x & 0xff) << 24))

#define INI_SIG 0xffffffff
#define INI_VER 0x14

/* Specified twice */
typedef USHORT INI_len[2];

/* INI header */
typedef struct
{
  ULONG signature;   /* always 0xffffffff */
  ULONG version;     /* always 0x14       */
  ULONG filesize;    /* file size         */
  ULONG reserved[2]; /* always 0          */
} INI_hdr;

/* INI application */
typedef struct
{
  ULONG nextapp;     /* next application ptr     */
  ULONG key;         /* INI key data pointer     */
  ULONG reserved;
  INI_len applen;   /* appname length           */
  ULONG appname;     /* appname pointer          */
} INI_app;

/* INI key */
typedef struct
{
  ULONG nextkey;     /* next key ptr             */
  ULONG reserved;
  INI_len keylen;   /* key length               */
  ULONG keyname;     /* key name (ASCIIZ)        */
  INI_len vallen;   /* value length             */
  ULONG val;         /* value (ASCIIZ)           */
} INI_key;

#endif
