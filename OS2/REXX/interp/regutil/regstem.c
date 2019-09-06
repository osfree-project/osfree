/* Stem and variable manipulation functions for regutil
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
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/regstem.c,v 1.10 2013/11/07 23:02:50 mark Exp $
 */

/* ******************************************************************** */
/* ************************** Stem Routines *************************** */
/* ******************************************************************** */

#include "regutil.h"
#include <ctype.h>
#include <limits.h>

/* compare two strings case-sensitively */
static int rxstrcmp(const PRXSTRING l, const PRXSTRING r)
{
   register int len = min(l->strlength, r->strlength),
                d = l->strlength - r->strlength,
                c = memcmp(l->strptr, r->strptr, len);

   return c ? c : d;
}

/* case-insensitive comparison of two regions of memory */
static int casecmp(const unsigned char * l, const unsigned char * r, const int len)
{
   register int i, c;
   for (i = c = 0; !c && i < len; i++)
      c = toupper(l[i]) - toupper(r[i]);

   return c;
}

/* compare two strings case-insensitively */
static int rxstrcasecmp(const PRXSTRING l, const PRXSTRING r)
{
   register int len = min(l->strlength, r->strlength),
                d = l->strlength - r->strlength,
                c = casecmp((const unsigned char *)l->strptr, (const unsigned char *)r->strptr, len);

   return (len && c) ? c : d;
}

static int rxstrcmpbackwards(const PRXSTRING l, const PRXSTRING r)
{
   return rxstrcmp(r, l);
}

static int rxstrcasecmpbackwards(const PRXSTRING l, const PRXSTRING r)
{
   return rxstrcasecmp(r, l);
}

/* either map a file into memory or allocate a buffer */
#ifdef HAVE_MMAP
# include <unistd.h>
# include <fcntl.h>
# ifdef HAVE_SYS_MMAN_H
#  include <sys/mman.h>
# endif
# include <sys/stat.h>

# ifndef MAP_FAILED
#  define MAP_FAILED ((void *)-1)
# endif

char * mapfile(const char * name, int * len)
{
   int fd;
   struct stat st;
   char * s;

   if ((fd = open(name, O_RDONLY)) == -1 || fstat(fd, &st)) {
      if (fd != -1)
         close(fd);
      return NULL;
   }

   s = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
   close(fd);

   if (s == MAP_FAILED)
      return NULL;

   *len = st.st_size;
   return s;
}

int unmapfile(char *buf, int size)
{
   if (buf == NULL)
      return -1;
   else
      return munmap(buf, size);
}

#elif defined(MAPVIEWOFFILE)

#include <windows.h>

/* open a file and create a memory mapping to it */
char * mapfile(const char * s, int *len)
{
   HFILE fh;
   HANDLE hShm;
   char * buf;
   OFSTRUCT ofs;
   BY_HANDLE_FILE_INFORMATION bhfi;

   memset(&ofs, 0, sizeof(ofs));
   fh = OpenFile(s, &ofs, OF_READ);

   if (fh == INVALID_HANDLE_VALUE) {
      return NULL;
   }

   GetFileInformationByHandle(fh, &bhfi);
   *len = bhfi.nFileSizeLow;

   hShm = CreateFileMapping((HANDLE)fh, NULL, PAGE_READONLY, 0, 0, NULL);

   if (hShm == NULL) {
      CloseHandle((HANDLE)fh);
      return NULL;
   }

   buf = (char *)MapViewOfFileEx(hShm, FILE_MAP_READ, 0, 0, 0, NULL);
   CloseHandle(hShm);
   CloseHandle((HANDLE)fh);

   return buf;
}

int unmapfile(char *buf, int size)
{
   if (buf == NULL)
      return -1;
   else
      return UnmapViewOfFile(buf) - 1;
}


#else

#include <sys/stat.h>

char * mapfile(const char * name, int * len)
{
   FILE * fp;
   struct stat st;
   char * s;

   if ((fp = fopen(name, "rb")) == NULL || fstat(fileno(fp), &st)) {
      if (fp != NULL)
         fclose(fp);
      return NULL;
   }

   s = malloc(st.st_size);

   if (s == NULL) {
      fclose(fp);
      return NULL;
   }

   fread(s, 1, st.st_size, fp);
   fclose(fp);

   *len = st.st_size;
   return s;
}

int unmapfile(char *buf, int size)
{
   if (buf == NULL)
      return -1;
   else {
      free(buf);
      return 0;
   }
}

#endif




/* sysstemsort(stemname[, order] [,sensitivity] [,startpos,endpos] [,firstcol,lastcol]) */
rxfunc(sysstemsort)
{
   chararray * ca;
   int insensitive = 0, backwards = 0, start = 0, count = 0, first = 0, width = 0;
   register int i;
   char * s;
   int (*cmpfn)(const PRXSTRING l, const PRXSTRING r);

   checkparam(1, 7);

   /* default to ascending sort, but go descending if the second arg is given
    * and the first character is `d' */
   if (argc > 1 && argv[1].strptr && toupper(argv[1].strptr[0]) == 'D')
      backwards = 1;

   /* default to case-sensitive sort, but go insensitive if the third arg is
    * given and the first character is `i' */
   if (argc > 2 && argv[2].strptr && toupper(argv[2].strptr[0]) == 'I')
      insensitive = 1;

   /* one can choose to sort a sub-set of the array, from index start to index
    * end. We must convert start from 1-based to 0-based, and we really want
    * end to be the count of elements to sort. */
   if (argc > 3 && argv[3].strptr) {
      rxstrdup(s, argv[3]);
      start = atoi(s) - 1;

      if (start < 0)
         start = 0;

   }

   if (argc > 4 && argv[4].strptr) {
      rxstrdup(s, argv[4]);
      count = atoi(s) - start;

      if (count < 0)
         count = 0;
   }

   /* finally, one can choose to compare the full data or
    * substr(first,last-first+1) of the data. As with start and end,
    * we want first to be 0-based, and se want a width, not a last */
   if (argc > 5 && argv[5].strptr) {
      rxstrdup(s, argv[5]);
      first = atoi(s) - 1;

      if (first < 0)
         first = 0;
   }

   if (argc > 6 && argv[6].strptr) {
      rxstrdup(s, argv[6]);
      width = atoi(s) - first;

      if (width < 0)
         width = 0;
   }

   ca = new_chararray();

   if (!ca) {
      return NOMEMORY;
   }

   getastem(argv, ca);

   if (!count) {
      count = ca->count - start;
      if (count < 0)
         count = 0;
   }

   if (insensitive && backwards)
      cmpfn = rxstrcasecmpbackwards;
   else if (insensitive)
      cmpfn = rxstrcasecmp;
   else if (backwards)
      cmpfn = rxstrcmpbackwards;
   else
      cmpfn = rxstrcmp;

   if (!first && !width)
      qsort(ca->array+start, count, sizeof(*ca->array), (int(*)(const void *, const void *))cmpfn);
   else {
      struct {
         RXSTRING se;
         RXSTRING orig;
      } * aa;

      aa = malloc(sizeof(*aa) * count);
      if (!aa) {
         delete_chararray(ca);
         return NOMEMORY;
      }

      for (i = 0; i < count; i++) {
         aa[i].orig = ca->array[i+start];

         aa[i].se.strptr = aa[i].orig.strptr + first;

         aa[i].se.strlength = aa[i].orig.strlength - first;

         if (width && (int)aa[i].se.strlength > width)
            aa[i].se.strlength = width;

         else if ((int)aa[i].se.strlength < 0)
            aa[i].se.strlength = 0;
      }

      qsort(aa, count, sizeof(*aa), (int(*)(const void *, const void *))cmpfn);

      for (i = 0; i < count; i++) {
         ca->array[i+start] = aa[i].orig;
      }

      free(aa);
   }

   setastem(argv, ca);
   delete_chararray(ca);

   result_zero();
   return 0;
}

/* regmultistemsort(order,sensitivity,firstcol,lastcol,stemname1,stemname2[,stemname3,...]) */
rxfunc(regmultistemsort)
{
   struct {
      RXSTRING se;
      RXSTRING orig;
      int idx;
   } * aa;
   chararray * ca, *tmpca;
   int insensitive = 0, backwards = 0, count = 0, first = 0, width = 0, size;
   register int i, j;
   char * s;
   int (*cmpfn)(const PRXSTRING l, const PRXSTRING r);

   checkparam(6, -1);

   /* validate that all stems are stems and they all have the same length */
   if ( getstemsize(&argv[4], &count ) ) {
      result_minus_one();
      return 0;
   }
   for ( i = 5; i < argc; i++ ) {
      if ( getstemsize(&argv[i], &size ) ) {
         result_minus_one();
         return 0;
      }
      /* check that this stem is the same size as the first */
      if ( size != count ) {
         result_minus_one();
         return 0;
      }
   }

   /* default to ascending sort, but go descending if the first arg is given
    * and the first character is `d' */
   if (argv[0].strptr && toupper(argv[0].strptr[0]) == 'D')
      backwards = 1;

   /* default to case-sensitive sort, but go insensitive if the second arg is
    * given and the first character is `i' */
   if (argv[1].strptr && toupper(argv[1].strptr[0]) == 'I')
      insensitive = 1;

   /* finally, one can choose to compare the full data or
    * substr(first,last-first+1) of the data.
    * We want first to be 0-based, and we want a width, not a last */
   if (argv[2].strptr) {
      rxstrdup(s, argv[2]);
      first = atoi(s) - 1;

      if (first < 0)
         first = 0;
   }

   if (argv[3].strptr) {
      rxstrdup(s, argv[3]);
      width = atoi(s) - first;

      if (width < 0)
         width = 0;
   }

   /*
    * Allocate a new chararray for the first stem */
   ca = new_chararray();

   if (!ca) {
      return NOMEMORY;
   }

   /*
    * Copy the first stem into the chararray */
   getastem(&argv[4], ca);

   if (insensitive && backwards)
      cmpfn = rxstrcasecmpbackwards;
   else if (insensitive)
      cmpfn = rxstrcasecmp;
   else if (backwards)
      cmpfn = rxstrcmpbackwards;
   else
      cmpfn = rxstrcmp;

   /* sort the first stem */
   aa = malloc(sizeof(*aa) * count);
   if (!aa) {
      delete_chararray(ca);
      return NOMEMORY;
   }

   for (i = 0; i < count; i++) {
      aa[i].idx = i;
      aa[i].orig = ca->array[i];
      aa[i].se.strptr = aa[i].orig.strptr + first;
      aa[i].se.strlength = aa[i].orig.strlength - first;
      if (width && (int)aa[i].se.strlength > width)
         aa[i].se.strlength = width;
      else if ((int)aa[i].se.strlength < 0)
         aa[i].se.strlength = 0;
   }

   qsort(aa, count, sizeof(*aa), (int(*)(const void *, const void *))cmpfn);

   for (i = 0; i < count; i++) {
      ca->array[i] = aa[i].orig;
   }
   setastem(&argv[4], ca);

   /*
    * Allocate a new chararray for the first stem */
   tmpca = new_chararray();

   if (!tmpca) {
      return NOMEMORY;
   }
   /*
    * We have sorted the first stem, now rearrange the order of the other stems to match the sorted
    * order of the first */
   for ( i = 5; i < argc; i++ ) {
      /* copy the stem to a chararray */
      getastem(&argv[i], ca);
      /* and again to a temp chararray */
      getastem(&argv[i], tmpca);
      for (j = 0; j < count; j++) {
         ca->array[j] = tmpca->array[aa[j].idx];
      }
      setastem(&argv[i], ca);
   }

   delete_chararray(ca);
   delete_chararray(tmpca);
   free(aa);
   result_zero();
   return 0;
}

/* sysstemcopy(from, to [, fromindex, toindex, count, insertoverlay]) */
rxfunc(sysstemcopy)
{
   chararray * ca, *tca;
   char * s;
   register int i;
   int find = 0, tind = 0, count = 0, insert = 0;

   checkparam(2, 6);

   /* as with stemsort, we want the indices to be offsets (ie, 0-based) */
   if (argc > 2 && argv[2].strptr) {
      rxstrdup(s, argv[2]);
      find = atoi(s) - 1;
   }

   if (argc > 3 && argv[3].strptr) {
      rxstrdup(s, argv[3]);
      tind = atoi(s) - 1;
   }

   /* but this time, there's a count which should be left alone */
   if (argc > 4 && argv[4].strptr) {
      rxstrdup(s, argv[4]);
      count = atoi(s);
   }

   if (argc > 5 && argv[5].strptr && toupper(argv[5].strptr[0]) == 'I')
      insert = 1;

   ca = new_chararray();

   getastem(argv, ca);

   if (!find && !tind && !count && !insert) {
      setastem(argv+1, ca);
   }
   /* I don't know why we needed to screw up this simple interface, but here
    * goes: */
   else {
      if (!count || (ca->count < (find+count))) {
         count = ca->count - find;
      }

      if (count <= 0) {
         delete_chararray(ca);

         /* should this be -1? */
         result_zero();
         return 0;
      }

      tca = new_chararray();
      getastem(argv+1, tca);
      if (insert) {

         /* make sure there's room */
         if (tca->ptr_alloc < (tca->count + count + tind)) {
            tca->ptr_alloc = tca->count+count+tind;
            tca->array = realloc(tca->array, tca->ptr_alloc*sizeof(tca->array));
         }

         /* if we're extending past the end of the current array, set the
          * intervening elements to null */

         if (tind > tca->count) {
            for (i = tca->count; i < tind; i++) {
               tca->array[i].strptr = NULL;
               tca->array[i].strlength = 0;
            }
            tca->count = tind + count;
         }
         else {
            memmove(tca->array+tind+count, tca->array+tind, (tca->count-tind)*sizeof(*tca->array));
            tca->count += count;
         }
      }
      else {
         /* still make sure there's room */
         if (tca->ptr_alloc < (count + tind)) {
            tca->ptr_alloc = count+tind;
            tca->array = realloc(tca->array, tca->ptr_alloc*sizeof(tca->array));
         }

         /* if we're extending past the end of the current array, set the
          * intervening elements to null */

         if (tind > tca->count) {
            for (i = tca->count; i < tind; i++) {
               tca->array[i].strptr = NULL;
               tca->array[i].strlength = 0;
            }
            tca->count = tind + count;
         }
         else if (tca->count < (tind+count)) {
            tca->count = tind + count;
         }
      }

      memcpy(tca->array+tind, ca->array+find, count*sizeof(*ca->array));

      setastem(argv+1, tca);
      delete_chararray(tca);
   }

   delete_chararray(ca);

   result_zero();
   return 0;
}

/* sysstemdelete(stem, index [, count]) */
rxfunc(sysstemdelete)
{
   chararray * ca;
   char * counts, *inds;
   int count = 1, ind, size;

   checkparam(2, 3);

   rxstrdup(inds, argv[1]);
   ind = atoi(inds);

   if (argc > 2) {
      rxstrdup(counts, argv[2]);
      count = atoi(counts);
   }

   /* find out how many there are */
   getstemsize(argv, &size);

   if (ind <= 0 || ind > size || count < 1 || count > (size - ind + 1)) {
      memcpy(result->strptr, "-1", 2);
      result->strlength = 2;
      return 0;
   }

   ca = new_chararray();

   /* retrieve the parts we want to keep, unless we're deleting the full
    * array */
   if ( ind > 1 && ind+count > size ) {
      /* set it with new size */
      setstemsize(argv, ind-1);
   } else if (ind > 1 || count < size) {
      /* retrieve from ind+count to the end */
      getstemtail(argv, ind+count, ca);

      /* and set it with new indices */
      setstemtail(argv, ind, ca);
   }
   else {
      setastem(argv, ca);
   }

   delete_chararray(ca);

   result_zero();
   return 0;
}

/* syssteminsert(stem, index, value) */
rxfunc(syssteminsert)
{
   chararray * ca;
   char *inds;
   int ind;

   checkparam(3, 3);

   ca = new_chararray();

   getastem(argv, ca);

   rxstrdup(inds, argv[1]);
   ind = atoi(inds) - 1;   /* translate rexx index to C index */

   if (ind < 0 || ind > ca->count) {
      memcpy(result->strptr, "-1", 2);
      result->strlength = 2;
      return 0;
   }

   /* add the value to the end of the array -- this is just to ensure there's space */
   cha_adddummy(ca, argv[2].strptr, argv[2].strlength);

   /* if inserting at the end of the array, there's no need to screw
    * around */
   if (ind < (ca->count - 1)) {
      /* move the array over */
      memmove(ca->array+ind+1, ca->array+ind, sizeof(*ca->array)*(ca->count-ind-1));
      /* and set the pointers again */
      ca->array[ind] = argv[2];
   }

   setastem(argv, ca);
   delete_chararray(ca);

   result_zero();
   return 0;
}

/* stemread(filename, stemname[,minlen[,[maxlen]) */
rxfunc(regstemread)
{
   static char *smaxlen = NULL;
   static char *sminlen = NULL;
   char * filname, *fdata, *cp;
   int flen, linelen;
   register int offs = 0, oldoffs = 0;
   chararray * ca;
   register int count = 1, maxlen = 0, minlen = INT_MAX;

   checkparam(2, 4);

   rxstrdup(filname, argv[0]);
   fdata = mapfile(filname, &flen);

   if (!fdata || !flen) {
      result_one();
      return 0;
   }

   ca = new_chararray();

   /* deal with a leading newline */
   if (fdata[0] == '\n') {
      oldoffs = 1;
      cha_adddummy(ca, fdata, 0);
   }
   else {
      oldoffs = 0;
   }

   for (cp = memchr(fdata+oldoffs, '\n', flen - oldoffs); cp; cp = memchr(fdata+oldoffs, '\n', flen - oldoffs)) {
      offs = cp - fdata;

      /* some systems print cr-lf, not just lf */
      if (cp[-1] == '\r')
         offs--;

      linelen = offs - oldoffs;
      cha_adddummy(ca, fdata+oldoffs, linelen);
      if ( argc > 2 && RXVALIDSTRING(argv[2]) )
      {
         if ( linelen < minlen )
            minlen = linelen;
      }
      if ( argc > 3 && RXVALIDSTRING(argv[3]) )
      {
         if ( linelen > maxlen )
            maxlen = linelen;
      }
      oldoffs = cp - fdata + 1;

      /* set 1000 elements at a time to cut down on memory allocation */
      if (ca->count >= 1000) {
         setstemtail(argv+1, count, ca);
         count += ca->count;
         ca->count = 0;
      }
   }

   /* handle incomplete last lines */
   if (flen > 0 && fdata[flen-1] != '\n')
   {
      cha_adddummy(ca, fdata+oldoffs, flen-oldoffs);
   }

   if (ca->count)
      setstemtail(argv+1, count, ca);
   /* subtract 1 because count is one greater than the number of
    * elements set before calling setstemtail */
   setstemsize(argv+1, count+ca->count - 1);
   delete_chararray(ca);
   unmapfile(fdata, flen);

   result_zero();
   /* set the min line length parameter */
   if ( argc > 2 && RXVALIDSTRING(argv[2]) )
   {
      if ( sminlen )
         free( sminlen );
      sminlen = malloc( 50 ); /* enough for an integer */
      if ( count )
      {
         int mylen;
         mylen = sprintf( sminlen, "%d", minlen );
         setavar( &argv[2], sminlen, mylen );
      }
   }
   /* set the max line length parameter */
   if ( argc > 3 && RXVALIDSTRING(argv[3]) )
   {
      if ( smaxlen )
         free( smaxlen );
      smaxlen = malloc( 50 ); /* enough for an integer */
      if ( count )
      {
         int mylen;
         mylen = sprintf( smaxlen, "%d", maxlen );
         setavar( &argv[3], smaxlen, mylen );
      }
   }
   return 0;
}

/* stemwrite(filename, stemname) */
rxfunc(regstemwrite)
{
   FILE * fp;
   char * filname;
   register int i;
   chararray * ca;

   checkparam(2, 2);

   rxstrdup(filname, argv[0]);

   fp = fopen(filname, "w");
   if (!fp) {
      result_one();
      return 0;
   }

   ca = new_chararray();
   getastem(argv+1, ca);

   for (i = 0; i < ca->count; i++) {
      fwrite(ca->array[i].strptr, 1, ca->array[i].strlength, fp);
      fputc('\n', fp);
   }

   fclose(fp);
   delete_chararray(ca);

   result_zero();

   return 0;
}

/* regstemdover(stem, variable[, outstem])
 * returns 0 when it's finished
 */
rxfunc(regstemdoover)
{
   static chararray * ca = NULL;
   static char * name = NULL;
   static int len = 0, cur = 0;
   char * s;
   SHVBLOCK sb;

   checkparam(2,3);

   rxstrdup(s, argv[0]);
   strupr(s);

   /* we can only do one of these at a time */
   if (!name || len != argv[0].strlength || memcmp(name, s, len)) {
      if (name)
         free(name);

      if (ca)
         delete_chararray(ca);
      ca = NULL;

      cur = 0;
      len = argv[0].strlength;
      name = malloc(len+1);
      memcpy(name, s, len+1);
   }

   if (!ca) {
      ca = new_chararray();

      sb.shvcode = RXSHV_NEXTV;
      sb.shvnext = NULL;
      sb.shvret = 0;
      sb.shvvalue.strptr = (void *)-1;

      while (! (sb.shvret & RXSHV_LVAR)) {
         sb.shvvaluelen = sb.shvvalue.strlength = 0;
         sb.shvname.strptr = NULL;
         RexxVariablePool(&sb);
         if (sb.shvname.strptr && sb.shvname.strlength > len &&
             !memcmp(sb.shvname.strptr, name, len)) {
            cha_addstr(ca, sb.shvname.strptr+len, sb.shvname.strlength - len);
         }
         if (sb.shvname.strptr)
            REXXFREEMEMORY(sb.shvname.strptr);
      }
   }

   if (!ca) {
      result_zero();
   }

   else if (cur >= ca->count) {
      delete_chararray(ca);
      ca = NULL;
      free(name);
      name = NULL;
      cur = len = 0;
      result_zero();
   }
   else {
      sb.shvcode = RXSHV_SET;
      sb.shvnext = NULL;
      sb.shvret = 0;
      sb.shvvalue = ca->array[cur++];
      sb.shvname = argv[1];
      RexxVariablePool(&sb);

      result_one();
   }

   return 0;
}

/* compare the offsetth element of stem haystack against the match table
 * needle. If exact is true, it must match exactly. Otherwise, it needs to
 * be contained somewhere in there. */
static int stemcompare(PRXSTRING needle, PRXSTRING haystack,
                       int offset, rxbool exact, rxbool casesensitive)
{
   register int i, offs = 0, rc = 0;
   SHVBLOCK shv;

   /* get the value of offset */
   memset(&shv, 0, sizeof(shv));
   shv.shvcode = RXSHV_FETCH;
   shv.shvvalue.strlength = shv.shvvaluelen = 1024;
   shv.shvvalue.strptr = alloca(1024);
   shv.shvname.strptr = alloca(haystack->strlength + 10);
   shv.shvnamelen = shv.shvname.strlength =
               sprintf(shv.shvname.strptr, "%.*s%d", (int)haystack->strlength,
                                                     haystack->strptr, offset);
   RexxVariablePool(&shv);
   if (shv.shvret & RXSHV_TRUNC) {
      shv.shvret = 0;
      shv.shvvalue.strptr = alloca(shv.shvvaluelen);
      shv.shvvalue.strlength = shv.shvvaluelen;
      RexxVariablePool(&shv);
   }

   /* can't match if it's not long enough */
   if (shv.shvvalue.strlength < needle->strlength) {
      /* if performing an exact match, we may be doing a binary search and
       * so need to know which side we're on */
      if (exact) {
         if (casesensitive) {
            rc = memcmp(needle->strptr, shv.shvvalue.strptr, shv.shvvalue.strlength);
         }
         else {
            for (i = 0; i < shv.shvvalue.strlength; i++) {
               rc = ((int)(unsigned char)needle->strptr[i]) -
                    toupper((unsigned char)shv.shvvalue.strptr[i]);

               if (rc) {
                  break;
               }
            }
         }

         /* match up to the end of the haystack string, so return 1,
          * since substrings go first */
         if (!rc)
           rc = 1;
      }
      else
         rc = 1;
   }

   else do {
      /* loop until something in haystack doesn't match */
      if (casesensitive) {
         rc = memcmp(needle->strptr, shv.shvvalue.strptr+offs, needle->strlength);

         /* skip ahead quickly to the next possible match */
         if (rc && !exact) {
            char * cp = memchr(shv.shvvalue.strptr+offs+1, needle->strptr[0], shv.shvvalue.strlength - offs - 1);
            if (cp) {
               offs = (long)cp - (long)shv.shvvalue.strptr - 1;
            }
            else {
               offs = shv.shvvalue.strlength;
            }
         }
      }
      else {
         for (i = 0; i < needle->strlength; i++) {
            rc = ((int)(unsigned char)needle->strptr[i]) -
                 toupper((unsigned char)shv.shvvalue.strptr[i+offs]);

            if (rc)
               break;
         }
      }
   } while (rc && !exact && (++offs <= (shv.shvvalue.strlength - needle->strlength)));

   /* if this is an exact match, make sure the lengths match. We do this at
    * the end because it only applies when needle is a substring of
    * haystack */
   if (exact && !rc && shv.shvvalue.strlength != needle->strlength) {
      rc = -1;
   }

   return rc;
}


/* regstemsearch(needle, haystack, [start], [flags]) */
rxfunc(regstemsearch)
{
   RXSTRING needle, haystack;
   char *sstart;
   register int offs = 1;
   rxbool casesensitive = false, exact = false, sorted = false;
   register int i;
   int size;
   int rc = 0;

   checkparam(2, 4);

   haystack = argv[1];

   /* make sure the stem name is null terminated */
   if (haystack.strptr[haystack.strlength-1] != '.') {
      rxstrdup(haystack.strptr, argv[1]);
      haystack.strptr[haystack.strlength++] = '.';
   }

   getstemsize(&haystack, &size);

   if (!size) {
      result_zero();
      return 0;
   }

   if (argc > 2 && argv[2].strptr) {
      rxstrdup(sstart, argv[2]);
      offs = atoi(sstart);
   }

   if (argc > 3) {
      if (memchr(argv[3].strptr, 'c', argv[3].strlength) ||
          memchr(argv[3].strptr, 'C', argv[3].strlength))
         casesensitive = true;

      if (memchr(argv[3].strptr, 'e', argv[3].strlength) ||
          memchr(argv[3].strptr, 'E', argv[3].strlength))
         exact = true;

      if (memchr(argv[3].strptr, 's', argv[3].strlength) ||
          memchr(argv[3].strptr, 'S', argv[3].strlength))
         sorted = true;
   }

   if (casesensitive)
      needle = argv[0];
   else {
      needle.strlength = argv[0].strlength;
      needle.strptr = alloca(needle.strlength);
      for (i = 0; i < needle.strlength; i++) {
         needle.strptr[i] = toupper(argv[0].strptr[i]);
      }
   }

   /* bsearch and lsearch are not appropriate if we want to avoid the
    * overhead of retrieving the values of all members of the stem */
   if (!sorted)
   {
      for (i = offs; i <= size; i++) {
         if (!(rc = stemcompare(&needle, &haystack, i, exact, casesensitive)))
            break;
      }
   }
   else
   {
      register int l = offs, r = size;
      for (i = (r - l) / 2 + l; i <= r  && i >= l; i = (r - l) / 2 + l) {
         if ((rc = stemcompare(&needle, &haystack, i, exact, casesensitive)) < 0) {
            r = i - 1;
         }
         else if (rc > 0) {
            l = i + 1;
         }
         else
            break;
      }
   }

   if (!rc) {
      result->strlength = sprintf(result->strptr, "%d", i);
   }
   else
      result_zero();
   return 0;
}

/* sysdumpvariables([filename])
 * write all variables either to file filename or to stdout
 */
rxfunc(sysdumpvariables)
{
   static const char dumpformat[] = "Name=%.*s, Value=\"%.*s\"\n";
   SHVBLOCK sb;
   FILE * fp;
   char * filename;

   checkparam(0, 1);
   if (argc == 1) {
      rxstrdup(filename, argv[0]);
      fp = fopen(filename, "a");
   }
   else {
      fp = stdout;
   }

   if (fp == NULL) {
      memcpy(result->strptr, "-1", 2);
      result->strlength = 2;
   }
   else {
      /* let's guess it will work from here on in */
      result_zero();
   }

   sb.shvcode = RXSHV_NEXTV;
   sb.shvnext = 0;
   sb.shvret = 0;

   while (! (sb.shvret & RXSHV_LVAR)) {
      /* set ptrs to NULL so RexxVariablePool will allocate memory
       * for us -- this is slightly more expensive, but prevents
       * the possibility of truncation (we can't recover from
       * truncation when stepping through the variable pool). */
      sb.shvvalue.strptr = NULL;
      sb.shvname.strptr = NULL;

      RexxVariablePool(&sb);

      if (sb.shvname.strptr && sb.shvvalue.strptr) {
         fprintf(fp, dumpformat, (int)sb.shvname.strlength, sb.shvname.strptr,
                 (int)sb.shvvalue.strlength, sb.shvvalue.strptr);
         REXXFREEMEMORY(sb.shvname.strptr);
         REXXFREEMEMORY(sb.shvvalue.strptr);
      }
   }

   if (argc == 1)
      fclose(fp);

   return 0;
}
