/* win 3.1-style ini-file-reading code for unix
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
 * Portions created by Patrick McPhee are Copyright © 2002
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/inifile.c,v 1.5 2022/08/21 23:16:42 mark Exp $
 */
#ifndef _WIN32
# include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "inifile.h"

#if defined(DOS)
# include <io.h>
#endif
typedef struct value_T {
   struct value_T * N;
   char * name;
   char * value;
   char * comment;
} * value_t;

typedef struct sec_T {
   struct sec_T * N;
   char * name;
   char * comment;
   long foff;
   value_t vals;
} * sec_t;

struct inif_T {
   struct inif_T * N;
   char * name;
   FILE * fp;
   int write_mode;
   off_t len;
   time_t mt;
   sec_t sct;
};

static inif_t top = NULL;

static void delete_val(value_t v)
{
   if (!v) return;
   delete_val(v->N);
   if (v->comment)
      free(v->comment);
   free(v);
}


static void delete_section(sec_t st)
{
   if (!st) return;

   delete_section(st->N);
   delete_val(st->vals);
   if (st->comment)
      free(st->comment);
   free(st);
}

#if defined(DOS)
static int ftruncate( int fno, long size )
{
  return _chsize( fno, size );
}

/* file locks ignored; it is DOS */
static void release_lock(inif_t fit)
{
   return;
}

/* take a read lock on the file. returns -1 on error, 0 if the file needs
 * to be re-read, and 1 if everything's fine */
static int take_read_lock(inif_t fit)
{
   struct stat st;
   int rc;

   rc = fstat(fileno(fit->fp), &st);
   if (rc == -1) return release_lock(fit), -1;

   if (fit->mt == st.st_mtime && fit->len == st.st_size) {
      rc = 1;
   }
   else {
      rc = 0;
   }

   return rc;
}

/* take a write lock on the file. returns -1 on error, 0 if the file needs
 * to be re-read, and 1 if everything's fine */
static int take_write_lock(inif_t fit)
{
   struct stat st;
   int rc;

   /* file must be opened in write mode */
   if (!fit->write_mode) {
      fit->fp = freopen(fit->name, "r+", fit->fp);
      if (fit->fp) {
         fit->write_mode = 1;
      }
      else
         return -1;
   }

   fstat(fileno(fit->fp), &st);

   if (fit->mt == st.st_mtime && fit->len == st.st_size) {
      rc = 0;
   }
   else {
      rc = 1;
   }
   return rc;
}
#else
/* get rid of all locks on the file. */
static void release_lock(inif_t fit)
{
   struct flock lk;

   lk.l_type = F_UNLCK;
   lk.l_whence = SEEK_SET;
   lk.l_start = 0;
   lk.l_len = 0;
   fcntl(fileno(fit->fp), F_SETLKW, &lk);
}

/* take a read lock on the file. returns -1 on error, 0 if the file needs
 * to be re-read, and 1 if everything's fine */
static int take_read_lock(inif_t fit)
{
   struct stat st;
   int rc;
   struct flock lk;

   lk.l_type = F_RDLCK;
   lk.l_whence = SEEK_SET;
   lk.l_start = 0;
   lk.l_len = 0;
   rc = fcntl(fileno(fit->fp), F_SETLKW, &lk);
   if (rc == -1) return -1;

   rc = fstat(fileno(fit->fp), &st);
   if (rc == -1) return release_lock(fit), -1;

   if (fit->mt == st.st_mtime && fit->len == st.st_size) {
      rc = 1;
   }
   else {
      rc = 0;
   }

   return rc;
}

/* take a write lock on the file. returns -1 on error, 0 if the file needs
 * to be re-read, and 1 if everything's fine */
static int take_write_lock(inif_t fit)
{
   struct stat st;
   int rc;
   struct flock lk;

   /* file must be opened in write mode */
   if (!fit->write_mode) {
      fit->fp = freopen(fit->name, "r+", fit->fp);
      if (fit->fp) {
         fit->write_mode = 1;
      }
      else
         return -1;
   }

   lk.l_type = F_WRLCK;
   lk.l_whence = SEEK_SET;
   lk.l_start = 0;
   lk.l_len = 0;
   fcntl(fileno(fit->fp), F_SETLKW, &lk);

   fstat(fileno(fit->fp), &st);

   if (fit->mt == st.st_mtime && fit->len == st.st_size) {
      rc = 0;
   }
   else {
      rc = 1;
   }
   return rc;
}
#endif


#define CV_WHITE 0
#define CV_NORM 1
#define CV_LBRACK 2
#define CV_RBRACK 3
#define CV_EQUALS 4
#define CV_COMMENT 5
#define CV_EOS 255


static int read_ini_raw(inif_t fit)
{
   static unsigned char pt[256];
   unsigned char buf[1024], *n, *v;
   sec_t lst = NULL, tst;
   value_t tvt, lvt = NULL;
   int nl, vl, cl;
   register int i;
   struct stat st;
   long oldoff=0;

   fstat(fileno(fit->fp), &st);

   fit->len = st.st_size;
   fit->mt = st.st_mtime;

   delete_section(fit->sct);

   /* set up parse table once only */
   if (!pt['a']) {
      memset(pt, CV_NORM, sizeof(pt));
      pt[' '] = pt['\t'] = pt['\f'] = pt['\v'] = CV_WHITE;
      pt['['] = CV_LBRACK;
      pt[']'] = CV_RBRACK;
      pt['='] = CV_EQUALS;
      pt[';'] = CV_COMMENT;
      pt['\n'] = pt['\r'] = pt[0] = CV_EOS;
   }

   fseek(fit->fp, 0, SEEK_SET);

   while ((oldoff = ftell(fit->fp)),
          fgets(buf, sizeof(buf), fit->fp) != NULL) {

      /* skip leading whitespace */
      for (i = 0; !pt[buf[i]]; i++)
         ;

      /* the first non-whitespace character determines what happens with this
       * line */
      switch (pt[buf[i]]) {
         /* errors */
         case CV_RBRACK: n = "closing bracket"; goto oops;
         case CV_EQUALS: n = "equals sign"; goto oops;

         oops:
            fprintf(stderr, "oops: line starts with %s: %s\n", n, buf);
            break;

         /* section start */
         case CV_LBRACK:
            /* skip whitespace after [ */
            for (i++; !pt[buf[i]]; i++)
               ;
            n = buf+i;
            /* now go until ] or end of string */
            for (; pt[buf[i]] != CV_RBRACK && pt[buf[i]] != CV_EOS; i++)
               ;
            nl = i - (n - buf);
            if (!nl) {
               fprintf(stderr, "oops: section name empty: %s\n", buf);
               break;
            }

            tst = malloc(nl+1+sizeof(*tst));
            tst->name = (char *)(tst+1);
            lvt = tst->vals = NULL;
            tst->N = NULL;
            tst->foff = oldoff;
            cl = 0;
            tst->comment = NULL;
            if (lst) {
               lst = lst->N = tst;
            }
            else {
               fit->sct = lst = tst;
            }
            memcpy(tst->name, n, nl);
            tst->name[nl] = 0;
            break;

         /* anything starting with an ordinary character is probably an
          * assignment */
         case CV_NORM:
            n = buf+i;
            /* find = */
            for (i++; pt[buf[i]] != CV_EQUALS && pt[buf[i]] != CV_EOS; i++)
               ;
            nl = i - (n - buf);

            /* value is the rest of the line */
            v = buf+i+1;
            for (i++; pt[buf[i]] != CV_EOS; i++)
               ;
            vl = i - (v - buf);

            tvt = malloc(sizeof(*tvt)+nl+vl+2);
            tvt->name = (char *)(tvt+1);
            tvt->value = tvt->name + nl + 1;
            tvt->N = NULL;
            cl = 0;
            tvt->comment = NULL;
            memcpy(tvt->name, n, nl);
            memcpy(tvt->value, v, vl);
            tvt->name[nl] = tvt->value[vl] = 0;
            if (lvt) {
               lvt = lvt->N = tvt;
            }
            else if (lst) {
               lvt = lst->vals = tvt;
            }
            else {
               free(lvt);
               fprintf(stderr, "oops: value out of context: %s\n", buf);
            }
            break;

         /* comment -- attach to preceding value or section. We don't care
          *            comments at the start of the file, since we never
          *            write anything before the first section */
         case CV_EOS:
         case CV_COMMENT:
            i = strlen(buf);
            if (lvt) {
               lvt->comment = realloc(lvt->comment, cl + i + 1);
               memcpy(lvt->comment+cl, buf, i+1);
            }
            else if (lst) {
               lst->comment = realloc(lst->comment, cl + i + 1);
               memcpy(lst->comment+cl, buf, i+1);
            }
            cl += i;

            break;

         default:
            fprintf(stderr, "this can't happen\n");
      }
   }

   return 0;
}

static int read_ini(inif_t fit)
{
   int rc, rcc;

   /* take an advisory lock on the file to avoid conflicts */
   rc = take_read_lock(fit);

   if (rc == 0) {
      rcc = read_ini_raw(fit);
   }
   else if (rc == 1)
      rcc = 0;
   else
      rcc = -1;


   if (rc != -1) {
      release_lock(fit);
   }

   return rcc;
}

static sec_t find_section(inif_t fit, const char * secname)
{
   register sec_t tst;

   for (tst = fit->sct; tst; tst=tst->N)
   {
      if (!strcasecmp(tst->name, secname))
         return tst;
   }
   return NULL;
}

static value_t find_val(inif_t fit, const char * secname, const char * valname)
{
   register sec_t tst;
   register value_t tvt;

   tst = find_section(fit, secname);

   if (tst)
      for (tvt = tst->vals; tvt; tvt=tvt->N)
         if (!strcasecmp(tvt->name, valname)) {
            return tvt;
         }

   return NULL;
}

/* write everything from this sec_t to the end, resetting the foff members
 * as we go -- no seeking! */
static void write_section(inif_t fit, sec_t st)
{
   register value_t tvt;

   if (!st)
      return;

   st->foff = ftell(fit->fp);

   fprintf(fit->fp, "[%s]\n", st->name);
   if (st->comment) {
      fprintf(fit->fp, "%s", st->comment);
   }
   for (tvt = st->vals; tvt; tvt = tvt->N) {
      fprintf(fit->fp, "%s=%s\n", tvt->name, tvt->value);
      if (tvt->comment)
         fprintf(fit->fp, "%s", tvt->comment);
   }

   write_section(fit, st->N);
}

static void write_sections(inif_t fit, sec_t mst)
{
   struct stat st;

   fseek(fit->fp, mst->foff, SEEK_SET);
   write_section(fit, mst);
   /* TODO - should really check for failure in ftruncate() */
   ftruncate(fileno(fit->fp), ftell(fit->fp));

   /* set the mod time and size */
   fstat(fileno(fit->fp), &st);
   fit->mt =st.st_mtime;
   fit->len = st.st_size;
   /* fix bug #433 */
   fflush(fit->fp);
}


char ** ini_enum_sec(inif_t fit, int * pcount)
{
   register sec_t tst;
   char ** sec = NULL;
   register int count;

   /* read the ini file if necessary */
   read_ini(fit);
   count = 0;

   for (tst = fit->sct; tst; tst=tst->N) {
      if (!(count % 10)) {
         sec = realloc(sec, (count + 10)*sizeof(*sec));
      }
      sec[count++] = tst->name;
   }

   *pcount = count;
   return sec;
}

char ** ini_enum_val(inif_t fit, const char * secname, int * pcount)
{
   register value_t tvt;
   sec_t sct;
   char ** val = NULL;
   register int count;

   /* read the ini file if necessary */
   read_ini(fit);
   sct = find_section(fit, secname);

   if (!sct) {
      *pcount = 0;
      return NULL;
   }

   count = 0;
   for (tvt = sct->vals; tvt; tvt=tvt->N) {
      if (!(count % 10)) {
         val = realloc(val, (count + 10)*sizeof(*val));
      }
      val[count++] = tvt->name;
   }

   *pcount = count;
   return val;
}

char * ini_get_val(inif_t fit, const char * secname, const char * valname)
{
   value_t val;

   read_ini(fit);
   val = find_val(fit, secname, valname);

   if (val) return val->value;
   else return NULL;
}


void ini_set_val(inif_t fit, const char * secname, const char * valname, const char * value)
{
   int rr = take_write_lock(fit);
   register sec_t lst;
   sec_t tst;
   register value_t tvt, lvt;
   value_t nvt;
   int sl, vl, vvl;

   if (rr == -1) return;

   /* got lock, but need to re-read file */
   if (rr == 0) {
      read_ini_raw(fit);
   }

   tst = find_section(fit, secname);

   /* if there is no such section, add it at the end */
   if (!tst) {
      sl = strlen(secname);
      tst = malloc(sizeof(*tst) + sl + 1);
      tst->name = (char *)(tst+1);
      memcpy(tst->name, secname, sl+1);
      tst->comment = NULL;
      tst->vals = NULL;
      tst->N = NULL;
      fseek(fit->fp, 0, SEEK_END);
      tst->foff = ftell(fit->fp);

      if (!fit->sct) {
         fit->sct = tst;
      }
      else {
         for (lst = fit->sct; lst->N; lst = lst->N)
            ;
         lst->N = tst;
      }
   }


   for (lvt = NULL, tvt = tst->vals; tvt; lvt = tvt, tvt = tvt->N) {
      if (!strcasecmp(tvt->name, valname))
         break;
   }

   /* build a new value structure */
   vl = strlen(valname);
   vvl = strlen(value);
   nvt = malloc(sizeof(*nvt) + vl + vvl + 2);
   nvt->name = (char *)(nvt+1);
   nvt->value = nvt->name+vl+1;
   memcpy(nvt->name, valname, vl+1);
   memcpy(nvt->value, value, vvl+1);

   /* if there was an old one, keep the comment & next pointers */
   if (tvt) {
      nvt->comment = tvt->comment;
      nvt->N = tvt->N;
      free(tvt);
   }
   else {
      nvt->comment = NULL;
      nvt->N  = NULL;
   }

   if (lvt) {
      lvt->N = nvt;
   }
   else {
      tst->vals = nvt;
   }

   /* now write everything from this section to the end */
   write_sections(fit, tst);

   release_lock(fit);
}


void ini_del_val(inif_t fit, const char * secname, const char * valname)
{
   int rr = take_write_lock(fit);
   sec_t tst;
   register value_t tvt, lvt;

   if (rr == -1) return;

   /* got lock, but need to re-read file */
   if (rr == 0) {
      read_ini_raw(fit);
   }

   tst = find_section(fit, secname);

   /* if there is no such section, we're done */
   if (tst) {
      for (lvt = NULL, tvt = tst->vals; tvt; lvt = tvt, tvt = tvt->N)
         if (!strcasecmp(tvt->name, valname))
            break;
   }
   else {
      tvt = NULL;
   }


   /* similarly, if ther is no such value, no need to delete it */
   if (tvt) {
      if (lvt) {
         lvt->N = tvt->N;
      }
      else {
         tst->vals = tvt->N;
      }

      if (tvt->comment)
         free(tvt->comment);
      free(tvt);

      write_sections(fit, tst);
   }


   release_lock(fit);
}


void ini_del_sec(inif_t fit, const char * secname)
{
   int rr = take_write_lock(fit);
   register sec_t lst;
   sec_t tst;

   if (rr == -1) return;

   /* got lock, but need to re-read file */
   if (rr == 0) {
      read_ini_raw(fit);
   }

   for (lst = NULL, tst = fit->sct; tst; lst = tst, tst = tst->N)
      if (!strcasecmp(tst->name, secname))
         break;

   /* if there is no such section, we're done */
   if (tst) {
      if (lst) {
         lst->N = tst->N;
      }
      else {
         lst = fit->sct = tst->N;
         lst->foff = tst->foff;
      }

      write_sections(fit, lst);

      tst->N = NULL;
      delete_section(tst);
   }

   release_lock(fit);
}


inif_t ini_open(const char * name)
{
   inif_t fit;
   int l;

   if (name == NULL)
      name = "win.ini";

   /* see if it's already open */
   for (fit = top; fit; fit = fit->N)
      if (!strcasecmp(fit->name, name))
         return fit;

   l = strlen(name);
   fit = malloc(sizeof(*fit)+l+1);

   fit->name = (char *)(fit + 1);
   memcpy(fit->name, name, l+1);

   fit->sct = NULL;

   fit->fp = fopen(name, "r");
   if (fit->fp) {
      fit->write_mode = 0;
   }
   else {
      fit->fp = fopen(name, "w+");
      if (fit->fp) {
         fit->write_mode = 1;
      }

      else {
         free(fit);
         return NULL;
      }
   }

   fit->len = 0;
   fit->mt = 0;
   fit->N = top;

   read_ini(fit);

   return fit;
}

void ini_close(inif_t fit)
{
   inif_t lit;

   if (!fit)
      return;

   /* get it out of the list of files */
   if (fit == top)
      top = top->N;

   else {
      for (lit = top; lit && lit->N != fit; lit = lit->N)
         ;
      if (lit) {
         lit->N = fit->N;
      }
   }

   if (fit->fp)
      fclose(fit->fp);

   delete_section(fit->sct);
   free(fit);
}
