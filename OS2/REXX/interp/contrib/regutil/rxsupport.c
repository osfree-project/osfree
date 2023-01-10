/* support functions which set rexx variables
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
 * $Header: /opt/cvs/Regina/regutil/rxsupport.c,v 1.4 2022/08/21 23:16:42 mark Exp $
 */
#include "rxproto.h"

#include <time.h>
#include <ctype.h>
#if !defined(_WIN32) && !defined(DOS)
# include <sys/param.h>
#endif

#ifndef MAXPATHLEN
# ifndef PATH_MAX
#  ifndef _POSIX_PATH_MAX
#   ifndef _MAX_PATH
#    define MAXPATHLEN 1024
#   else
#    define MAXPATHLEN _MAX_PATH
#   endif
#  else
#   define MAXPATHLEN _POSIX_PATH_MAX
#  endif
# else
#  define MAXPATHLEN PATH_MAX
# endif
#endif

/* it's preferable to set these things in one big blow, but sometimes
 * you have to do it one-off. */
void set_rc(const char * const value, const int len)
{
    SHVBLOCK avar;

    avar.shvnext = NULL;
    avar.shvname.strptr = "RC";
    avar.shvname.strlength = 2;
    avar.shvvalue.strptr = (char *)value;
    avar.shvvalue.strlength = len;
    avar.shvcode = RXSHV_SET;
    RexxVariablePool(&avar);
}


void rc_one()
{
    set_rc("1", 1);
}

void rc_zero()
{
    set_rc("0", 1);
}

void setavar(PRXSTRING varname, const char * const value, const int len)
{
    SHVBLOCK avar;

    avar.shvnext = NULL;
    avar.shvname.strptr = varname->strptr;
    avar.shvname.strlength = varname->strlength;
    avar.shvvalue.strptr = (char *)value;
    if (len == -1)
       avar.shvvalue.strlength = strlen(value);
    else
       avar.shvvalue.strlength = len;
    avar.shvcode = RXSHV_SYSET;
    RexxVariablePool(&avar);
}

/* given a stem name, find out the value of the .0 member */
int getstemsize(PRXSTRING varname, int * count)
{
   char * nameptr = alloca(varname->strlength+2), valbuf[11];
   SHVBLOCK stem;

   /* set up the .0 member */
   stem.shvname.strptr = nameptr;
   memcpy(nameptr, varname->strptr, varname->strlength);

   if (nameptr[varname->strlength-1] == '.') {
      nameptr[varname->strlength] = '0';
      stem.shvname.strlength = varname->strlength + 1;
   }
   else {
      memcpy(nameptr+varname->strlength, ".0", 2);
      stem.shvname.strlength = varname->strlength + 2;
   }

   stem.shvvalue.strptr = valbuf;
   stem.shvvalue.strlength = stem.shvvaluelen = sizeof(valbuf);
   stem.shvcode = RXSHV_SYFET;
   stem.shvnext = NULL;

   if (!RexxVariablePool(&stem)) {
      valbuf[stem.shvvalue.strlength] = 0;
      *count = atoi(valbuf);
      return 0;
   }
   else {
      *count = 0;
      return 1;
   }
}


/* given a stem name, set the value of the .0 member */
int setstemsize(PRXSTRING varname, const int count)
{
   char * nameptr = alloca(varname->strlength+2), valbuf[11];
   SHVBLOCK stem;

   /* set up the .0 member */
   stem.shvname.strptr = nameptr;
   memcpy(nameptr, varname->strptr, varname->strlength);

   if (nameptr[varname->strlength-1] == '.') {
      nameptr[varname->strlength] = '0';
      stem.shvname.strlength = varname->strlength + 1;
   }
   else {
      memcpy(nameptr+varname->strlength, ".0", 2);
      stem.shvname.strlength = varname->strlength + 2;
   }

   stem.shvvalue.strptr = valbuf;
   stem.shvvalue.strlength = stem.shvvaluelen = sprintf(valbuf, "%d", count);
   stem.shvcode = RXSHV_SYSET;
   stem.shvnext = NULL;

   if (!RexxVariablePool(&stem)) {
      return 0;
   }
   else {
      return 1;
   }
}


/* given an array of strings and the name of a stem variable, set the stem
 * variable to match the strings */
int setastem(PRXSTRING varname, const chararray * const values)
{
    SHVBLOCK *astem,
            stem0[2];
    register int i;
    int namelen = varname->strlength + 12, orignamelen, count;
    char * namebuf,
         * origname, valbuf[11], *nameptr;

    namebuf = malloc(namelen * (values->count+1));

    if (values && values->count) {
        count = values->count;
        astem = malloc(values->count*sizeof(*astem));
        memset(astem, 0, values->count * sizeof(*astem));
    }
    else {
       astem = NULL;
       count = 0;
    }

    memset(stem0, 0, sizeof(stem0));

    if (varname->strptr[varname->strlength-1] == '.') {
       rxstrdup(origname, *varname);
       orignamelen = varname->strlength;
    }
    else {
       origname = alloca(varname->strlength+2);
       memcpy(origname, varname->strptr, varname->strlength);
       origname[varname->strlength] = '.';
       origname[varname->strlength+1] = 0;
       orignamelen = varname->strlength+1;
    }
    strupr(origname);
    nameptr = namebuf;

    /* drop the stem */
    stem0[0].shvnext = stem0+1;
    stem0[0].shvname.strptr = origname;
    stem0[0].shvname.strlength = orignamelen;
    stem0[0].shvcode = RXSHV_DROPV;

    /* set up the .0 member */
    stem0[1].shvnext = astem;
    stem0[1].shvname.strptr = nameptr;
    stem0[1].shvname.strlength = sprintf(nameptr, "%s%d", origname, 0);
    stem0[1].shvvalue.strptr = valbuf;
    stem0[1].shvvalue.strlength = sprintf(valbuf, "%d", count);
    stem0[1].shvcode = RXSHV_SYSET;

    nameptr += namelen;

    for (i = 0; i < count; i++) {
        astem[i].shvnext = astem+i+1;
        astem[i].shvname.strptr = nameptr;
        astem[i].shvname.strlength = sprintf(nameptr, "%s%d", origname, i+1);
        astem[i].shvvalue = values->array[i];
        astem[i].shvcode = RXSHV_SYSET;
        nameptr += namelen;
    }

    if (i)
        astem[i-1].shvnext = NULL;

    RexxVariablePool(stem0);
    free(namebuf);
    if (astem)
        free(astem);

    return 0;
}

/* given an array of strings, a one-based index, and the name of a stem
 * variable, set the stem variable to match the strings, starting at the
 * index position. Leave any other values in the stem alone. Set the
 * length of the stem to ind+values->count. */
int setstemtail(PRXSTRING varname, const int ind, const chararray * const values)
{
   SHVBLOCK stem, * vars;
   register int i, j;
   int count, dcount;
   int namelen = varname->strlength + 12;
   char * origname, *nameptr;

   if (varname->strptr[varname->strlength-1] == '.') {
      rxstrdup(origname, *varname);
   }
   else {
      origname = alloca(varname->strlength+2);
      memcpy(origname, varname->strptr, varname->strlength);
      origname[varname->strlength] = '.';
      origname[varname->strlength+1] = 0;
   }
   strupr(origname);


   /* set the stem size if it's shrinking */
   getstemsize(varname, &count);

   /* and get rid of the ones we won't be needing any more */
   dcount =  count - (ind + values->count)+1;

   if (dcount > 0 && ind == 1) {
      /* can simply drop the stem if we're starting at 1 */
      stem.shvname.strptr = origname;
      stem.shvname.strlength = strlen(origname);
      stem.shvcode = RXSHV_DROPV;
      stem.shvnext = NULL;
      RexxVariablePool(&stem);

      setstemsize(varname, ind+values->count-1);
   }

   else if (dcount > 0) {

      setstemsize(varname, ind+values->count-1);

      /* get rid of the ones we won't need any more */
      vars = calloc(dcount, sizeof(*vars)+namelen);
      if (vars) {
         for (i = 0, j = ind + values->count; i < dcount; i++, j++) {
            nameptr = ((char *)(vars+dcount)) + i * namelen;
            vars[i].shvname.strptr = nameptr;
            vars[i].shvname.strlength = sprintf(nameptr, "%s%d", origname, j);
            vars[i].shvcode = RXSHV_DROPV;
            vars[i].shvnext = vars+i+1;
         }
         vars[i-1].shvnext = NULL;
         RexxVariablePool(vars);
         free(vars);
      }
      else {
         nameptr = alloca(namelen);

         stem.shvname.strptr = nameptr;
         stem.shvcode = RXSHV_DROPV;
         stem.shvnext = NULL;

         for (j = ind + values->count; j < count; j++) {
            stem.shvname.strlength = sprintf(nameptr, "%s%d", origname, j);
            RexxVariablePool(&stem);
         }
      }


   }

   vars = malloc(values->count * (sizeof(*vars)+namelen));


   /* if we couldn't allocate enough space, do it one-by-one using stem */
   if (vars == NULL) {
      nameptr = alloca(namelen);

      stem.shvname.strptr = nameptr;
      stem.shvcode = RXSHV_SYSET;
      stem.shvnext = NULL;

      for (i = 0; i < values->count; i++) {
         stem.shvname.strlength = sprintf(nameptr, "%s%d", origname, i+ind);
         stem.shvvalue = values->array[i];

         stem.shvret = 0;
         RexxVariablePool(&stem);
      }
   }
   else {
      for (i = 0; i < values->count; i++) {
         nameptr = (char *)(vars+values->count) + i * namelen;

         vars[i].shvname.strptr = nameptr;
         vars[i].shvname.strlength = sprintf(nameptr, "%s%d", origname, i+ind);

         vars[i].shvcode = RXSHV_SYSET;
         vars[i].shvvalue = values->array[i];
         vars[i].shvret = 0;

         vars[i].shvnext = vars+i+1;
      }

      if (i)
         vars[i-1].shvnext = NULL;

      RexxVariablePool(vars);
      free(vars);
   }
   return 0;
}


/* given a stem name, retrieve the values associated with that stem, starting
 * at index ind (1-based) and stick them in a chararray. I'm doing this by
 * retrieving one value at a time and adding it to the array using cha_addstr. */
int getstemtail(PRXSTRING varname, const int ind, chararray * const values)
{
    SHVBLOCK stem, * vars;
    register int i, count, vallen;
    int namelen = varname->strlength + 12;
    char * origname, valbuf[11], *nameptr, *valptr = NULL;

    if (!values)
        return -1;

    values->count = 0;

    if (varname->strptr[varname->strlength-1] == '.') {
       rxstrdup(origname, *varname);
    }
    else {
       origname = alloca(varname->strlength+2);
       memcpy(origname, varname->strptr, varname->strlength);
       origname[varname->strlength] = '.';
       origname[varname->strlength+1] = 0;
    }
    strupr(origname);

    /* first, find out how many there are */
    nameptr = alloca(namelen);
    memset(&stem, 0, sizeof(stem));

    /* set up the .0 member */
    stem.shvname.strptr = nameptr;
    stem.shvname.strlength = sprintf(nameptr, "%s%d", origname, 0);
    stem.shvvalue.strptr = valbuf;
    stem.shvvalue.strlength = stem.shvvaluelen = sizeof(valbuf);
    stem.shvcode = RXSHV_SYFET;

    RexxVariablePool(&stem);

    /* there is no stem, so there is nothing to return */
    if (stem.shvret & RXSHV_NEWV)
      return 0;

    valbuf[stem.shvvalue.strlength] = 0;
    count = atoi(valbuf) - ind + 1;

    /* likewise, if there is nothing to return, there is nothing
     * to return */
    if (count <= 0)
      return 0;

    vallen = 500;
    vars = malloc(count*(sizeof(*vars)+namelen+vallen));

    if (!vars) {
       stem.shvname.strptr = nameptr;
       stem.shvcode = RXSHV_SYFET;

       vallen = 10000;
       valptr = malloc(vallen);

       stem.shvvalue.strptr = valptr;

       for (i = 0; i < count; i++) {
           stem.shvname.strlength = sprintf(nameptr, "%s%d", origname, i+ind);
           stem.shvvaluelen = stem.shvvalue.strlength = vallen;

           RexxVariablePool(&stem);
           if (stem.shvret & RXSHV_TRUNC) {
              vallen = stem.shvvaluelen;
              valptr = realloc(valptr, vallen);
              stem.shvret = 0;
              stem.shvvalue.strptr = valptr;
              i--;
           }
           else {
              cha_addstr(values, stem.shvvalue.strptr, stem.shvvalue.strlength);
           }
       }
    }

    else {
       valptr = (char *)(vars+count) + namelen*count;
       for (i = 0; i < count; i++) {
          nameptr = (char *)(vars+count) + namelen*i;
          vars[i].shvname.strptr = nameptr;
          vars[i].shvcode = RXSHV_SYFET;
          vars[i].shvvalue.strptr = valptr+i*vallen;
          vars[i].shvvaluelen = vallen;
          vars[i].shvret = 0;

          vars[i].shvname.strlength = sprintf(nameptr, "%s%d", origname, i+ind);
          vars[i].shvvaluelen = vars[i].shvvalue.strlength = vallen;
          vars[i].shvnext = vars+i+1;
       }
       vars[i-1].shvnext = NULL;
       valptr = NULL;

       RexxVariablePool(vars);

       /* add strings to array. If any string is truncated, knock up the
        * buffer and try it again */
       for (i = 0; i < count; i++) {
           if (vars[i].shvret & RXSHV_TRUNC) {
              vars[i].shvnext = NULL;

              /* re-allocate only if we haven't already made the buffer big
               * enough */
              if (vallen < vars[i].shvvaluelen) {
                 vallen = vars[i].shvvaluelen;
                 valptr = realloc(valptr, vallen);
              }

              vars[i].shvvalue.strptr = valptr;
              vars[i].shvvaluelen = vallen;
              RexxVariablePool(vars+i);
           }

           cha_addstr(values, vars[i].shvvalue.strptr, vars[i].shvvalue.strlength);
       }

       free(vars);
    }


    if (valptr)
       free(valptr);

    return 0;
}


/* given a stem name, retrieve the values associated with that stem and stick
 * them in a chararray. This is equivalent to getstemtail with index 1. */
int getastem(PRXSTRING varname, chararray * const values)
{
   return getstemtail(varname, 1, values);
}


/* return the number of seconds in the string pointed to by ptime */
int rxint(PRXSTRING ptime)
{
   char * timestr;
   register int sec;

   rxstrdup(timestr, ptime[0]);
   sec = atoi(timestr);

   return sec;
}

/* return the number of microseconds in the string pointed to by ptime */
int rxuint(PRXSTRING ptime)
{
   char * timestr, digits[7];
   register int usec = 0;

   rxstrdup(timestr, ptime[0]);

   /* now look for a decimal place */
   timestr = strchr(timestr, '.');
   if (timestr++) {
      if (strlen(timestr) >= 6) timestr[6] = 0;
      else {
         memset(digits, '0', sizeof(digits));
         digits[6] = 0;
         memcpy(digits, timestr, strlen(timestr));
         timestr = digits;
      }
      usec = atoi(timestr);
   }

   return usec;
}


#ifndef _WIN32
/* convert a string to lower case */
char * strlwr(register char * s)
{
    register char * t = s;

    while (*t) {
        *t = tolower(*t);
        t++;
    }

   return s;
}

char * strupr(register char * s)
{
    register char * t = s;

    while (*t) {
        *t = toupper(*t);
        t++;
    }

   return s;
}

#endif

#define INITPTR 1000
int cha_addstr(chararray *ca, const char * str, int len)
{
    if (ca->count >= ca->ptr_alloc) {
        ca->ptr_alloc += INITPTR;
        ca->array = realloc(ca->array, ca->ptr_alloc*sizeof(*ca->array));
        if (ca->array == NULL) {
            ca->ptr_alloc = 0;
            ca->count = 0;
            return -1;
         }
    }

    /* there's not enough room in this buffer, so allocate a new one which
     * is big enough */
    if ((ca->chars->used+len+1) >= ca->chars->alloc) {
        struct caalloc * nd, *cd, *pd;

        /* keep the list of buffers ordered by free space -- this prevents some
         * needless memory allocation at the cost of a few cycles. This doesn't
         * seem to buy much, but reallocation is generally rare in any case. */
        for (cd = ca->chars, nd = cd->N, pd = NULL;
             nd && (nd->alloc - nd->used) > (cd->alloc - cd->used);
             pd = nd, nd = nd->N)
           ;

        if (pd) {
           ca->chars = cd->N;
           cd->N = pd->N;
           pd->N = cd;
        }

        /* maybe we now have enough space, so test again */
        if ((ca->chars->used+len+1) >= ca->chars->alloc) {
           nd = malloc(sizeof(*nd)+len+INITPTR*MAXPATHLEN);

           if (!nd) {
              return -1;
           }

           nd->alloc = len+INITPTR*MAXPATHLEN;
           nd->used = 0;
           nd->N = ca->chars;
           ca->chars = nd;
        }
     }


    ca->array[ca->count].strlength = len;
    ca->array[ca->count].strptr = ca->chars->data+ca->chars->used;
    memcpy(ca->array[ca->count++].strptr, str, len);
    ca->chars->used += len+1;
    ca->chars->data[ca->chars->used] = 0;

    return 0;
}

/* add a string without copying the data. This is more efficient,
 * but you have to be careful not to throw away the data before you're done
 * with the array. */
int cha_adddummy(chararray *ca, const char * str, int len)
{
    if (ca->count >= ca->ptr_alloc) {
        ca->ptr_alloc += INITPTR;
        ca->array = realloc(ca->array, ca->ptr_alloc*sizeof(*ca->array));
        if (ca->array == NULL) {
            ca->ptr_alloc = 0;
            ca->count = 0;
            return -1;
         }
    }

    ca->array[ca->count].strlength = len;
    ca->array[ca->count++].strptr = (char *)str;

    return 0;
}

chararray * new_chararray(void)
{
    chararray * ca;

    ca = malloc(sizeof(*ca));

    if (!ca)
        return NULL;

    /* allocate 1000 pointers by default. These point into one big buffer,
     * which we allocate separately from the points -- a chararray has four
     * mallocs, instead of one per pointer (on the other hand, we'll have
     * some big chunks of data to realloc ...) */
    ca->ptr_alloc = INITPTR;
    ca->array = malloc(INITPTR*sizeof(*ca->array));
    ca->chars = malloc(sizeof(*ca->chars) + INITPTR*MAXPATHLEN);

    if (!ca->chars || !ca->array) {
        if (ca->chars) free(ca->chars);
        if (ca->array) free(ca->array);
        free(ca);
        ca = NULL;
    }

    ca->chars->alloc = INITPTR*MAXPATHLEN;
    ca->chars->N = NULL;
    ca->count = 0;
    ca->chars->used = 0;

    return ca;
}

void free_caalloc(struct caalloc * nd)
{
   if (!nd) return;
   free_caalloc(nd->N);
   free(nd);
}


void delete_chararray(chararray * ca)
{
    if (!ca) return;

    if (ca->array) free(ca->array);
    free_caalloc(ca->chars);
    free(ca);
}
