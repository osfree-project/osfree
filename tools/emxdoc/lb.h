/* lb.h -- Line breaking
   Copyright (c) 1993-1995 Eberhard Mattes

This file is part of emxdoc.

emxdoc is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

emxdoc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with emxdoc; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#define LB_NOMEM        1
#define LB_INVAL        2
#define LB_INTERN       3

#define LBN_WORD        1
#define LBN_PRE         2
#define LBN_POST        3
#define LBN_GLUE        4
#define LBN_NEWLINE     5
#define LBN_END         6

#define LB_INFINITY             10000
#define LB_SQRT_INFINITY        100

#define LB_HYPHEN       '*'

struct lb;
struct lbh;

struct lb_node
{
  int type;
  int value;
  const char *word;
  const void *info;
};

int lb_init (struct lb **pp, int lmargin, int rmargin);
int lb_exit (struct lb **p);
int lb_first_lmargin (struct lb *p, int margin);
int lb_first_rmargin (struct lb *p, int margin);
int lb_penalty (struct lb *p, int penalty);
int lb_word (struct lb *p, int width, const char *word, const void *info);
int lb_discr (struct lb *p, int width_word, const char *word,
    int width_pre, const char *pre, int width_post, const char *post,
    const void *info);
int lb_hyphen (struct lb *p, int width, const void *info);
int lb_glue (struct lb *p, int width, const void *info);
int lb_format (struct lb *p);
int lb_next (struct lb *p, struct lb_node *dst);

int lb_use_hyphenation (struct lb *p, const struct lbh *h);

int lbh_init (struct lbh **pp);
int lbh_word (struct lbh *p, const char *s);
int lbh_exit (struct lbh **pp);
