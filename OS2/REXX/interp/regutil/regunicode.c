/* Unicode conversion functions for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 2003
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/regunicode.c,v 1.3 2012/08/08 01:26:20 mark Exp $
 */
#include "rxproto.h"

#ifdef _WIN32
#include <windows.h>
#include <winnls.h>
#else

# ifdef HAVE_ICONV_H
#  include <iconv.h>

   /* this needs to be redefined if your system uses a different name for
    * the base unicode code page */
#  ifndef ICONV_UTF16
#   define ICONV_UTF16 "UTF-16LE"
#  endif

# endif

#define CP_ACP 0
#define CP_OEMCP 0
#define CP_MACCP 1
#define CP_UTF7 7
#define CP_UTF8 8

#endif

/* code pages can be a numeric value, one of the strings defined by IBM, or
 * MAC (which I include because everything else in this API follows the
 * windows function so closely, we might as well include it) */
static int cvtcp(const char * s)
{
   static const struct {
      const char * s;
      int cp;
   } cpgs[] = {
      {"ACP", CP_ACP},
#ifndef _WIN32
      {"THREAD_ACP", CP_ACP},
#endif
      {"OEMCP", CP_OEMCP},
      {"MAC", CP_MACCP },
      {"UTF7", CP_UTF7 },
      {"UTF8", CP_UTF8 }
   };
   int cp = -1;
   register int i;

   for (i = 0; i < DIM(cpgs); i++) {
      if (!strcasecmp(cpgs[i].s, s)) {
         cp = cpgs[i].cp;
         break;
      }
   }

#ifdef _WIN32
   /* query system to find the current thread's ACP (thread's ACP? Windows!) */
   if (cp == -1) {
      if (!strcasecmp(s, "THREAD_ACP")) {
         cp = GetACP();
      }
   }
#endif

   if (cp == -1) {
      cp = atoi(s);
      if (!cp)
         cp = -1;
   }

   return cp;
}

#ifdef _WIN32
struct mapping_flags_T {
   RXSTRING str;
   int flg;
};

/* find the flags in string s */
static int getflags(RXSTRING rxs, const struct mapping_flags_T * flgs,
                   int dim_flgs)
{
   int flags = 0;
   register int i;

   while (rxs.strlength > 0) {
      for (i = 0; i < dim_flgs; i++) {
         if (rxs.strlength >= flgs[i].str.strlength &&
             !memcmp(rxs.strptr, flgs[i].str.strptr, flgs[i].str.strlength)) {
            flags |= flgs[i].flg;
            rxs.strptr += flgs[i].str.strlength;
            rxs.strlength -= flgs[i].str.strlength;
         }

         /* skip non-spaces -- strictly, I'm supposed to return rc 1004 */
         i = strcspn(rxs.strptr, " ");
         rxs.strlength -= i;
         rxs.strptr += i;

         /* skip spaces */
         i = strspn(rxs.strptr, " ");
         rxs.strlength -= i;
         rxs.strptr += i;
      }
   }

   return flags;
}
#elif !defined(HAVE_MBSTOWCS)

/* trivial conversions between unicode and latin 1 */
static int utol1(unsigned char * l1s, const unsigned short * us, int ul)
{
   register int i;
   for (i = 0; i < ul; i++) {
      l1s[i] = (unsigned char)us[i];
   }

   return ul;
}

static int l1tou(unsigned short * us, const unsigned char * l1s, int ul)
{
   register int i;
   for (i = 0; i < ul; i++) {

      us[i] = l1s[i];
   }

   return ul;
}

#endif

/* similarly trivial conversions between unicode and utf8. utf8 is just
 * a different way of representing the same numbers as ucs-2. The first
 * bit determines how many bytes are used to represent a character. If it
 * is 0, 1 byte is used. Otherwise, the number of non-zero most significant
 * bits determines the number of bytes used to represent the character (if
 * the first two bits are set and the third is clear, two characters are
 * used, and so forth). For whatever reason, every byte in a multi-byte
 * sequence has as many most significant bits set as there are bytes
 * remaining in the sequence. */
static int utou8(unsigned char *u8s, const unsigned short * us, int ul)
{
   register int i, j;

   for (i = j = 0; i < ul; i++, j++) {
      /* the number of characters needed depends on character values in
       * the unicode string. Values up to 2^7-1 (0x80) fit in one byte.
       * Values up to 2^11-1 (2047) fit in two bytes. Values up to 2^15-1
       * (32767) fit in three bytes. Values up to 2^18-1 fit in four bytes.
       * Luckily, we never need more than 2^16-1.
       * Note that  0xf0 == (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5)
       *            0xe0 == (1 << 8) | (1 << 7) | (1 << 6)
       *            0xc0 == (1 << 8) | (1 << 7)
       *            0x80 == (1 << 8)
       *   which are the significant bits for a multi-byte character, and
       *            0x3f == (1 << 6) - 1
       *            0x1f == (1 << 5) - 1
       *            0x0f == (1 << 4) - 1
       *            0x07 == (1 << 3) - 1
       *   which are the corresponding masks giving the numeric values
       */
      if (us[i] < 128) {
         u8s[j] = us[i];
      }
      else if (us[i] < 2047) {
         u8s[j++] = (us[i] >> 6) | 0xc0;
         u8s[j] = (us[i] & 0x3f) | 0x80;
      }
      else if (us[i] < 32767) {
         u8s[j++] = (us[i] >> 11) | 0xe0;
         u8s[j++] = ((us[i] >> 6) & 0x1f) | 0xc0;
         u8s[j] = (us[i] & 0x3f) | 0x80;
      }
      else {
         u8s[j++] = (us[i] >> 15) | 0xf0;
         u8s[j++] = ((us[i] >> 11) & 0x0f) | 0xe0;
         u8s[j++] = ((us[i] >> 6) & 0x1f) | 0xc0;
         u8s[j] = (us[i] & 0x3f) | 0x80;
      }
   }
   return j;
}

static int u8tou(unsigned short *us, const unsigned char * u8s, int ul)
{
   register int i, j;

   for (i = j = 0; i < ul; i++, j++) {
      if (u8s[i] & 0x80) {
         us[j] = 0;
         /* 0xf8 == 11111000 */
         if ((u8s[i] & 0xf8) == 0xf0) {
            us[j] = u8s[i++] & 0x7;
         }
         /* fall through ... */
         if ((u8s[i] & 0xf0) == 0xe0) {
            us[j] <<= 4;
            us[j] |= u8s[i++] & 0xf;
         }
         /* the last two must be true */
         us[j] <<= 5;
         us[j] |= u8s[i++] & 0x1f;
         us[j] <<= 6;
         us[j] |= u8s[i] & 0x3f;
      }
      else {
         us[j] = u8s[i];
      }
   }
   return j;
}

/* similarly trivial conversions between unicode and utf7.
 * rfc 2152 describes a set of `directly encoded' characters and a set
 * of `optional direct characters'. This code directly encodes all of
 * them.
 * Everything else is converted to big-endian, and the resulting byte
 * stream is converted to base64. + is used to shift into base64,
 * and any non-base64 character can be used to shift out, however
 * - is handled specially as a shift-out character: - is absorbed.
 * I expect some converters think - is in fact the only shift-out
 * character, so this converter always uses it when going to utf-7.
 */

/* conversion to `modified base64' */
static int utomb64(unsigned char * mb64s, const unsigned short * us, int ul)
{
   register int i, j, nb, r;
   static const char coderange[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   unsigned char uc[2];

   nb = 6;      /* nb is the number of bits needed to complete the current character.
                 * since we're working with 16 bits at a time, it can be 6, 2, or 4 */
   r = 0;       /* r is the remainder from the previously encoded character */

   for (i = j = 0; i < ul; i++) {
      /* make sure it's big-endian */
      uc[0] = us[i] >> 8;
      uc[1] = us[i] & 0xff;

      if (nb == 6) {
         mb64s[j++] = coderange[uc[0] >> 2];
         mb64s[j++] = coderange[((uc[0]&3) << 4)|(uc[1]>>4)];
         r = uc[1] & 0xf;
         nb = 2;
      }
      else if (nb == 2) {
         mb64s[j++] = coderange[(r << 2) | (uc[0] >> 6)];
         mb64s[j++] = coderange[uc[0]&0x3f];
         mb64s[j++] = coderange[uc[1]>>2];
         r = uc[1] & 0x3;
         nb = 4;
      }
      else if (nb == 4) {
         mb64s[j++] = coderange[(r << 4) | (uc[0] >> 4)];
         mb64s[j++] = coderange[((uc[0]&0xf) << 2)|(uc[1]>>6)];
         mb64s[j++] = coderange[uc[1]&0x3f];
         r = 0;
         nb = 6;
      }
   }

   /* if we're have a partial character, need to pad it out appropriately
    * with 0s -- this is where base 64 is modified  */
   if (nb != 6) {
      mb64s[j++] = coderange[r << nb];
   }

   mb64s[j] = 0;

   return j;
}

static int utou7(unsigned char *u7s, const unsigned short * us, int ul)
{
   static unsigned char enc[127];
   register int i, j;

   if (!enc['A']) {
      static const unsigned char directs[] = "\t\n\r !\"#$%&'()*,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}";
      for (i = 0; i < sizeof(directs)-1; i++)
         enc[directs[i]] = 1;
   }

   for (i = j = 0; i < ul; i++, j++) {
      if (us[i] < 126 && enc[us[i]]) {
         u7s[j] = us[i];
      }
      else {
         register int l;

         u7s[j++] = '+';

         /* determine where the set of shift characters ends */
         for (l = i; l < ul && (us[l] >= 126 || !enc[us[l]]); l++)
            ;
         j += utomb64(u7s+j, us+i, l - i);
         u7s[j] = '-';
         i += l - 1;
      }
   }

   u7s[j] = 0;
   return j;
}

static int decode(unsigned char c)
{
    if (c == '+') return 62;
    else if (c == '/') return 63;
    else if ('0' <= c && c <= '9') return c - '0' + 52;
    else if ('A' <= c && c <= 'Z') return c - 'A';
    else if ('a' <= c && c <= 'z') return c - 'a' + 26;
    else return -1;
}

/* decodes a base-64 string until it encounters a character which isn't
 * allowed in base-64. The index of that character is returned in *pnul,
 * unless the character is -, in which case the index of the next character
 * is returned in *pnul. */
static int mb64tou(unsigned short * us, const unsigned char * mb64s, int ul, int * pnul)
{
    register int i, j, c, r, nb;
    unsigned char uc[2];

    nb = 16;    /* start needing all 16 bits */
    r = 0;
    c = 0;

    /* yes, this should be i++ */
    for (i = j = 0; i < ul; i++) {
       c = decode(mb64s[i]);

       if (c == -1) {
          if (mb64s[i] == '-')
             i++;
          break;
       }

       if (nb == 16) {
          uc[0] = c << 2;
          nb = 10;
        }
        else if (nb == 10) {
           uc[0] |= c >> 4;
           uc[1] = (c & 0xf) << 4;
           nb = 4;
        }
        else if (nb == 4) {
           uc[1] |= c >> 2;
           us[j++] = ((unsigned short)uc[0]) << 8 | uc[1];

           uc[0] = (c & 3) << 6;
           nb = 14;
        }
        else if (nb == 14) {
           uc[0] |= c;
           nb = 8;
        }
        else if (nb == 8) {
           uc[1] = c << 2;
           nb = 2;
        }
        else if (nb == 2) {
           uc[1] |= c >> 4;
           us[j++] = ((unsigned short)uc[0]) << 8 | uc[1];

           uc[0] = (c & 0xf) << 4;
           nb = 12;
        }
        else if (nb == 12) {
           uc[0] |= c >> 2;
           uc[1] = (c & 3) << 6;

           nb = 6;
        }
        else if (nb == 6) {
           uc[1] |= c;
           us[j++] = ((unsigned short)uc[0]) << 8 | uc[1];
           nb = 16;
        }
    }

    /* ignore any left-over bits. If they're not 0, the string is not
     * well-formed */

    *pnul = i;

    return j;
}

static int u7tou(unsigned short *us, const unsigned char * u7s, int ul)
{
   register int i, j;
   int nul;

   for (i = j = 0; i < ul; i++, j++) {
      if (u7s[i] == '+') {
         i++;
         j += mb64tou(us+j, u7s+i, ul - i, &nul) - 1;
         i += nul-1;
      }
      else {
         us[j] = u7s[i];
      }
   }

   us[j] = 0;
   return j;
}

/* SysToUnicode(string, [codepage], [mappingflags], outstem) */
rxfunc(systounicode)
{
   int cp;
   int flags = 0;
   RXSTRING outs;
   RXSTRING stemv;
   static const char text[] = "!TEXT";

   checkparam(4, 4);

   /* default code page is the `oem' code page. On most systems, this is
    * 437, while the `windows' code page is iso-latin 1. It's not a bad idea
    * to make the two be the same, though. */
   if (argv[1].strptr == NULL || argv[1].strlength == 0)
      cp = CP_OEMCP;
   else {
      char * s;
      rxstrdup(s, argv[1]);
      cp = cvtcp(s);
   }

#ifdef _WIN32
   if (argv[2].strptr && argv[2].strlength) {
      static const struct mapping_flags_T flgs[] = {
         {{11, "PRECOMPOSED"}, MB_PRECOMPOSED},
         {{ 9, "COMPOSITE"}, MB_COMPOSITE},
         {{17, "ERR_INVALID_CHARS"}, MB_ERR_INVALID_CHARS},
         {{13, "USEGLYPHCHARS"}, MB_USEGLYPHCHARS}
      };

      flags = getflags(argv[2], flgs, DIM(flgs));
   }
#endif

   outs.strptr = malloc(argv[0].strlength*2);

#ifdef _WIN32
   outs.strlength = MultiByteToWideChar(cp, flags, argv[0].strptr, argv[0].strlength, (LPWSTR)outs.strptr, argv[0].strlength);

   if (outs.strlength == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      outs.strlength = MultiByteToWideChar(cp, flags, argv[0].strptr, argv[0].strlength, NULL, 0);
      outs.strptr = realloc(outs.strptr, outs.strlength*2);
      MultiByteToWideChar(cp, flags, argv[0].strptr, argv[0].strlength, (LPWSTR)outs.strptr, outs.strlength);
   }

   if (outs.strlength == 0) {
      switch (GetLastError()) {
         /* these are my fault -- they shouldn't happen */
         case ERROR_INSUFFICIENT_BUFFER:
         case ERROR_INVALID_FLAGS:
         case ERROR_INVALID_PARAMETER:
            result->strlength = 2;
            result->strptr[0] = '4';
            result->strptr[1] = '0';
            break;
         case ERROR_NO_UNICODE_TRANSLATION:
            result->strlength = 2;
            result->strptr[0] = '8';
            result->strptr[1] = '7';
            break;
         /* this is their fault */
         default:
            result->strlength = 2;
            result->strptr[0] = '4';
            result->strptr[1] = '7';
            break;
      }
   }
   else {
      result_zero();
   }
#else

   /* perform default conversion using mbsrtowcs(), since it can be set up
    * portably, in contrast to iconv(). */
   if (cp == CP_ACP) {
# ifdef HAVE_MBSTOWCS
        wchar_t * output;

        if (sizeof(wchar_t) != sizeof(short)) {
           output = alloca(sizeof(wchar_t)*argv[0].strlength);
        }
        else
           output = (wchar_t *) outs.strptr;

        outs.strlength = mbstowcs(output, argv[0].strptr, argv[0].strlength);

        /* well, mostly portably -- we want 16 byte output, which might
         * not be the case for wchar_t */
        if (sizeof(wchar_t) != 2) {
           register int i;

           for (i = 0; i < outs.strlength; i++) {
           ((short *)outs.strptr)[i] = output[i];
           }
        }

# else
        /* unless, of course, the system doesn't support it -- in this case
         * assume the trivial conversion */
        outs.strlength = l1tou((unsigned short *)outs.strptr, argv[0].strptr, argv[0].strlength);
# endif
   }

   /* for the same reason, perform utf-7 and utf-8 conversions here, rather
    * than using iconv() */
   else if (cp == CP_UTF7) {
      outs.strlength = u7tou((unsigned short *)outs.strptr,  (const unsigned char *)argv[0].strptr, argv[0].strlength);
   }
   else if (cp == CP_UTF8) {
      outs.strlength = u8tou((unsigned short *) outs.strptr, (const unsigned char *)argv[0].strptr, argv[0].strlength);
   }

   /* if requesting a specific code page, we need iconv, or we return an
    * error */
   else {
# ifdef HAVE_ICONV
      iconv_t ic;
      char * cps;

      rxstrdup(cps, argv[1]);

      ic = iconv_open(ICONV_UTF16, cps);

      if (ic == (iconv_t)-1) {
         result->strlength = 2;
         result->strptr[0] = '8';
         result->strptr[1] = '7';
         outs.strlength = 0;
      }
      else {
         char * inbuf = argv[0].strptr, * outbuf = outs.strptr;
         size_t inlen = argv[0].strlength, outlen = argv[0].strlength*2;

         iconv(ic, &inbuf, &inlen, &outbuf, &outlen);

         if (inlen) {
            outs.strlength = 0;
            result->strlength = 4;
            memcpy(result->strptr, "1113", 4);
         }
         else {
            outs.strlength = (argv[0].strlength * 2 - outlen)/2;
            result_zero();
         }

         iconv_close(ic);
      }
# else
      result->strlength = 2;
      result->strptr[0] = '8';
      result->strptr[1] = '7';
      outs.strlength = 0;
# endif
   }
#endif

   /* outstem.!TEXT is the return value */

   stemv.strptr = alloca(argv[3].strlength + sizeof(text) + 1);
   memcpy(stemv.strptr, argv[3].strptr, argv[3].strlength);
   if (stemv.strptr[argv[3].strlength-1] == '.') {
      stemv.strlength = argv[3].strlength;
   }
   else {
      stemv.strptr[argv[3].strlength] = '.';
      stemv.strlength = argv[3].strlength + 1;
   }

   memcpy(stemv.strptr+stemv.strlength, text, sizeof(text)-1);
   stemv.strlength += sizeof(text) - 1;
   setavar(&stemv, outs.strptr, outs.strlength*2);

   free(outs.strptr);

   return 0;
}

rxfunc(sysfromunicode)
{
   int cp;
   int flags = 0;
   RXSTRING outs;
   RXSTRING stemv;
   static const char text[] = "!TEXT", usedd[] = "!USEDDEFAULTCHAR";
   int usedDefaultChar = 0;
   char * defchar = NULL;
   int * pusedDefaultChar = NULL;

   checkparam(5, 5);

   if (argv[1].strptr == NULL || argv[1].strlength == 0)
      cp = CP_OEMCP;
   else {
      char * s;
      rxstrdup(s, argv[1]);
      cp = cvtcp(s);
   }

#ifdef _WIN32
   if (argv[2].strptr && argv[2].strlength) {
      static const struct mapping_flags_T flgs[] = {
         {{14, "COMPOSITECHECK"}, WC_COMPOSITECHECK},
         {{ 8, "SEPCHARS"}, WC_SEPCHARS},
         {{ 9, "DISCARDNS"}, WC_DISCARDNS},
         {{11, "DEFAULTCHAR"}, WC_DEFAULTCHAR}
      };

      flags = getflags(argv[2], flgs, DIM(flgs));
      if (flags & WC_DEFAULTCHAR) {
         pusedDefaultChar = &usedDefaultChar;
      }
   }

   if (argv[3].strlength && argv[3].strptr) {
      defchar = (char *)argv[3].strptr;
   }
#endif

   outs.strptr = malloc(argv[0].strlength*2);

#ifdef _WIN32
   outs.strlength = WideCharToMultiByte(cp, flags, (LPWSTR)argv[0].strptr, argv[0].strlength/2, outs.strptr, argv[0].strlength, defchar, pusedDefaultChar);

   if (outs.strlength == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      outs.strlength = WideCharToMultiByte(cp, flags, (LPWSTR)argv[0].strptr, argv[0].strlength/2, NULL, 0, NULL, NULL);
      outs.strptr = realloc(outs.strptr, outs.strlength);
      WideCharToMultiByte(cp, flags, (LPWSTR)argv[0].strptr, argv[0].strlength/2, outs.strptr, outs.strlength, defchar, &usedDefaultChar);
   }

   if (outs.strlength == 0) {
      switch (GetLastError()) {
         /* these are my fault -- they shouldn't happen */
         case ERROR_INSUFFICIENT_BUFFER:
         case ERROR_INVALID_FLAGS:
         case ERROR_INVALID_PARAMETER:
            result->strlength = 2;
            result->strptr[0] = '4';
            result->strptr[1] = '0';
            break;
         case ERROR_NO_UNICODE_TRANSLATION:
            result->strlength = 2;
            result->strptr[0] = '8';
            result->strptr[1] = '7';
            break;
         /* this is their fault */
         default:
            result->strlength = 2;
            result->strptr[0] = '4';
            result->strptr[1] = '7';
            break;
      }
   }
   else {
      result_zero();
   }

#else
   /* perform default conversion using wcsrtombs(), since it can be set up
    * portably, in contrast to iconv(). */
   if (cp == CP_ACP) {
# ifdef HAVE_MBSTOWCS
        wchar_t * s;

        if (sizeof(wchar_t) != 2) {
           register int i;
           s = alloca(argv[0].strlength*sizeof(wchar_t));
           for (i = 0; i < argv[0].strlength/2; i++)
              s[i] = ((short *)argv[0].strptr)[i];
        }
        else
           s = (wchar_t *)argv[0].strptr;

        outs.strlength = wcstombs(outs.strptr, s, argv[0].strlength/2);
# else
        /* unless, of course, the system doesn't support it -- in this case
         * assume the trivial conversion */
        outs.strlength = utol1(outs.strptr, (unsigned short *)argv[0].strptr, argv[0].strlength/2);
# endif
   }

   /* for the same reason, perform utf-7 and utf-8 conversions here, rather
    * than using iconv() */
   else if (cp == CP_UTF7) {
      outs.strlength = utou7((unsigned char *)outs.strptr,  (unsigned short *)argv[0].strptr, argv[0].strlength/2);
   }
   else if (cp == CP_UTF8) {
      outs.strlength = utou8((unsigned char *)outs.strptr, (unsigned short *)argv[0].strptr, argv[0].strlength/2);
   }

   /* if requesting a specific code page, we need iconv, or we return an
    * error */
   else {
# ifdef HAVE_ICONV
      iconv_t ic;
      char * cps;

      rxstrdup(cps, argv[1]);

      ic = iconv_open(cps, ICONV_UTF16);

      if (ic == (iconv_t)-1) {
         result->strlength = 2;
         result->strptr[0] = '8';
         result->strptr[1] = '7';
         outs.strlength = 0;
      }
      else {
         char * inbuf = argv[0].strptr, * outbuf = outs.strptr;
         size_t inlen = argv[0].strlength, outlen = argv[0].strlength*2;

         iconv(ic, &inbuf, &inlen, &outbuf, &outlen);

         if (inlen) {
            outs.strlength = 0;
            result->strlength = 4;
            memcpy(result->strptr, "1113", 4);
         }
         else {
            outs.strlength = argv[0].strlength * 2 - outlen;
            result_zero();
         }

         iconv_close(ic);
         result_zero();
      }
# else
      result->strlength = 2;
      result->strptr[0] = '8';
      result->strptr[1] = '7';
      outs.strlength = 0;
# endif
   }
#endif

   /* outstem.!TEXT is the return value
    * outstem.!USEDDEFAULTCHAR is the value of the default character if
    *  applicable */

   stemv.strptr = alloca(argv[4].strlength + sizeof(usedd) + 1);
   memcpy(stemv.strptr, argv[4].strptr, argv[4].strlength);
   if (stemv.strptr[argv[4].strlength-1] == '.') {
      stemv.strlength = argv[4].strlength;
   }
   else {
      stemv.strptr[argv[4].strlength] = '.';
      stemv.strlength = argv[4].strlength + 1;
   }

   memcpy(stemv.strptr+stemv.strlength, text, sizeof(text)-1);
   stemv.strlength += sizeof(text) - 1;
   setavar(&stemv, outs.strptr, outs.strlength);

   memcpy(stemv.strptr+stemv.strlength - (sizeof(text) - 1), usedd, sizeof(usedd)-1);
   stemv.strlength += sizeof(usedd) - sizeof(text);
   if (usedDefaultChar) {
      setavar(&stemv, "-", 1);
   }
   else {
      setavar(&stemv, "", 0);
   }

   return 0;
}

