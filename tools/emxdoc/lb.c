/* lb.c -- Line breaking
   Copyright (c) 1993-1996 Eberhard Mattes

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lb.h"

#define FALSE   0
#define TRUE    1

#define NODE_WORD       1
#define NODE_GLUE       2
#define NODE_PENALTY    3
#define NODE_DISCR      4
#define NODE_PRE        5
#define NODE_POST       6
#define NODE_NEWLINE    7

#define HASH_SIZE       997

#define HYPHEN_PENALTY  18

struct node
{
  struct node *next;
  char *word, *pre, *post;
  const void *info;
  int value, value_pre, value_post;
  char type;
};

struct brkp
{
  struct node *node;
  int chain;
  int cost;
  char special;
};

struct hword
{
  struct hword *next;
  char *str;
};

struct lbh
{
  struct hword *hash_table[HASH_SIZE];
};

struct lb
{
  int lmargin;
  int rmargin;
  int lmargin_1;
  int rmargin_1;
  int brkp_count;
  const struct lbh *hyphenation;
  const struct node *cur_node;
  struct node *node_list;
  struct node **node_add;
  struct brkp *brkps;
  int *distance;
};


static const struct hword *lbh_find (const struct lbh *p, const char *s);


int lb_init (struct lb **pp, int lmargin, int rmargin)
{
  struct lb *p;

  *pp = NULL;
  if (lmargin < 0 || lmargin >= rmargin)
    return LB_INVAL;
  p = malloc (sizeof (struct lb));
  if (p == NULL) return LB_NOMEM;
  p->lmargin = p->lmargin_1 = lmargin;
  p->rmargin = p->rmargin_1 = rmargin;
  p->brkp_count = 0;
  p->cur_node = NULL;
  p->node_list = NULL;
  p->node_add = &p->node_list;
  p->brkps = NULL;
  p->distance = NULL;
  p->hyphenation = NULL;
  *pp = p;
  return 0;
}


static void lb_free_node (struct node *n1)
{
  if (n1->word != NULL) free (n1->word);
  if (n1->pre != NULL) free (n1->pre);
  if (n1->post != NULL) free (n1->post);
  free (n1);
}


int lb_exit (struct lb **pp)
{
  struct lb *p;
  struct node *n1, *n2;

  p = *pp; *pp = NULL;
  if (p != NULL)
    {
      for (n1 = p->node_list; n1 != NULL; n1 = n2)
        {
          n2 = n1->next;
          lb_free_node (n1);
        }
      if (p->brkps != NULL) free (p->brkps);
      if (p->distance != NULL) free (p->distance);
      free (p);
    }
  return 0;
}


static struct node * lb_new (struct lb *p, char type, int value,
                             const char *word, const void *info)
{
  struct node *n1;

  n1 = malloc (sizeof (struct node));
  if (n1 == NULL) return NULL;
  if (word == NULL)
    n1->word = NULL;
  else
    {
      n1->word = strdup (word);
      if (n1->word == NULL)
        {
          free (n1);
          return NULL;
        }
    }
  n1->next       = NULL;
  n1->type       = type;
  n1->value      = value;
  n1->value_pre  = 0;
  n1->value_post = 0;
  n1->pre        = NULL;
  n1->post       = NULL;
  n1->info       = info;
  return n1;
}


static int lb_add (struct lb *p, char type, int value, const char *word,
                   int value_pre, char *pre, int value_post, char *post,
                   const void *info)
{
  struct node *n1;

  n1 = lb_new (p, type, value, word, info);
  if (n1 == NULL) return LB_NOMEM;
  n1->pre        = pre;
  n1->post       = post;
  n1->value_pre  = value_pre;
  n1->value_post = value_post;
  *(p->node_add) = n1;
  p->node_add = &n1->next;
  return 0;
}


int lb_penalty (struct lb *p, int penalty)
{
  return lb_add (p, NODE_PENALTY, penalty, NULL, 0, NULL, 0, NULL, NULL);
}


int lb_word (struct lb *p, int width, const char *word, const void *info)
{
  return lb_add (p, NODE_WORD, width, word, 0, NULL, 0, NULL, info);
}


int lb_discr (struct lb *p, int width_word, const char *word,
              int width_pre, const char *pre, int width_post, const char *post,
              const void *info)
{
  char *p1, *p2;
  int rc;

  if (pre == NULL)
    p1 = NULL;
  else
    {
      p1 = strdup (pre);
      if (p1 == NULL)
        return LB_NOMEM;
    }
  if (post == NULL)
    p2 = NULL;
  else
    {
      p2 = strdup (post);
      if (p2 == NULL)
        {
          if (p1 != NULL) free (p1);
          return LB_NOMEM;
        }
    }
  rc = lb_add (p, NODE_DISCR, width_word, word, width_pre, p1, width_post, p2,
               info);
  if (rc != 0)
    {
      if (p1 != NULL) free (p1);
      if (p2 != NULL) free (p2);
    }
  return rc;
}


int lb_hyphen (struct lb *p, int width, const void *info)
{
  return lb_discr (p, 0, "", width, "-", 0, "", info);
}


int lb_glue (struct lb *p, int width, const void *info)
{
  return lb_add (p, NODE_GLUE, width, NULL, 0, NULL, 0, NULL, info);
}


int lb_first_rmargin (struct lb *p, int margin)
{
  if (margin < 1)
    return LB_INVAL;
  p->rmargin_1 = margin;
  return 0;
}


int lb_first_lmargin (struct lb *p, int margin)
{
  if (margin < 0)
    return LB_INVAL;
  p->lmargin_1 = margin;
  return 0;
}


int lb_use_hyphenation (struct lb *p, const struct lbh *h)
{
  p->hyphenation = h;
  return 0;
}


static int lb_hyphenation (struct lb *p)
{
  struct node *n1, *n2, *n3;
  const struct hword *hw;
  const char *start, *hyph;
  char *pre;
  int len;

  for (n1 = p->node_list; n1 != NULL; n1 = n1->next)
    if (n1->type == NODE_WORD && strchr (n1->word, LB_HYPHEN) == NULL
        && (hw = lbh_find (p->hyphenation, n1->word)) != NULL)
      {
        start = hw->str;
        free (n1->word); n1->word = NULL;
        while ((hyph = strchr (start, LB_HYPHEN)) != NULL)
          {
            if (hyph != start && hyph[-1] == '-')
              pre = NULL;
            else
              {
                pre = strdup ("-");
                if (pre == NULL) return LB_NOMEM;
              }

            n2 = lb_new (p, NODE_DISCR, 0, NULL, n1->info);
            if (n2 == NULL) return LB_NOMEM;
            n2->pre = pre;
            n2->value_pre = (pre != NULL ? (int)strlen (pre) : 0);
            n2->next = n1->next; n1->next = n2;

            n3 = lb_new (p, NODE_WORD, 0, NULL, n1->info);
            if (n3 == NULL) return LB_NOMEM;
            n3->next = n2->next; n2->next = n3;

            len = hyph - start;
            n1->value = len;
            n1->word = malloc ((size_t)(len + 1));
            if (n1->word == NULL) return LB_NOMEM;
            memcpy (n1->word, start, (size_t)len);
            n1->word[len] = 0;

            n1 = n3;
            start = hyph + 1;
          }
        n1->value = (int)strlen (start);
        n1->word = strdup (start);
        if (n1->word == NULL) return LB_NOMEM;
      }
  return 0;
}


static void lb_add_brkp (struct lb *p, struct node *n1, int store)
{
  if (store)
    p->brkps[p->brkp_count].node = n1;
  ++p->brkp_count;
}


static int lb_find_brkps (struct lb *p, int store)
{
  struct node *n1;

  p->brkp_count = 0;
  n1 = p->node_list;
  lb_add_brkp (p, n1, store);
  while (n1 != NULL)
    switch (n1->type)
      {
      case NODE_WORD:
        n1 = n1->next;
        break;
      case NODE_PENALTY:
        if (n1->value != LB_INFINITY)
          lb_add_brkp (p, n1, store);
        while (n1 != NULL && n1->type != NODE_WORD)
          n1 = n1->next;
        break;
      case NODE_GLUE:
        lb_add_brkp (p, n1, store);
        while (n1 != NULL && n1->type == NODE_GLUE)
          n1 = n1->next;
        break;
      case NODE_DISCR:
        lb_add_brkp (p, n1, store);
        n1 = n1->next;
        break;
      default:
        return LB_INTERN;
      }
  return 0;
}


static int cost_add (int c1, int c2)
{
  if (c1 >= LB_INFINITY || c2 >= LB_INFINITY || c1 + c2 >= LB_INFINITY)
    return LB_INFINITY;
  else
    return c1 + c2;
}


#define DISTANCE(P,B1,B2) ((P)->distance[(B1) * (P)->brkp_count + (B2)])

static int lb_compute_distance (struct lb *p)
{
  int i, j, c, w, w0, hc;
  struct node *n0, *n1, *n2;

  p->distance = malloc ((size_t)(p->brkp_count * p->brkp_count
                                 * sizeof (int)));
  if (p->distance == NULL) return LB_NOMEM;

  for (i = 0; i < p->brkp_count; ++i)
    {
      DISTANCE (p, i, i) = 0;
      for (j = i + 1; j < p->brkp_count; ++j)
        {
          n0 = p->brkps[i].node;
          n2 = p->brkps[j].node;
          w = 0; hc = 0;
          /* Discard glue at the beginning of a line unless preceded
             by a penalty. */
          n1 = n0;
          while (n1 != NULL && n1->type == NODE_GLUE)
            n1 = n1->next;
          for (; n1 != NULL; n1 = n1->next)
            {
              switch (n1->type)
                {
                case NODE_WORD:
                  w += n1->value;
                  break;
                case NODE_GLUE:
                  w += n1->value; /* Rigid glue */
                  break;
                case NODE_PENALTY:
                  break;
                case NODE_DISCR:
                  if (n1 == n0)
                    w += n1->value_post;
                  else if (n1 == n2)
                    {
                      w += n1->value_pre;
                      hc = HYPHEN_PENALTY;
                    }
                  else
                    w += n1->value;
                  break;
                default:
                  return LB_INTERN;
                }
              if (n1 == n2)
                break;
            }

          if (i == 0)
            w0 = p->rmargin_1 - p->lmargin_1;
          else
            w0 = p->rmargin - p->lmargin;

          if (w > w0)
            c = LB_INFINITY;
          else if (j == p->brkp_count - 1)
            {
              /* The length of the last line of a paragraph does not
                 matter.  Note that n2 is a penalty node with a value
                 of 0. */
              c = 0;
            }
          else if (w0 - w > LB_SQRT_INFINITY)
            c = LB_INFINITY;
          else
            c = (w0 - w) * (w0 - w);
          if (n2->type == NODE_PENALTY)
            c = cost_add (c, n2->value);
          c = cost_add (c, hc);

          DISTANCE (p, i, j) = c;
          DISTANCE (p, j, i) = c;
        }
    }

#if 0
  fprintf (stderr, "@@DISTANCE:\n");
  for (i = 0; i < p->brkp_count; ++i)
    {
      for (j = 0; j < p->brkp_count; ++j)
        fprintf (stderr, "%6d", DISTANCE (p, i, j));
      fprintf (stderr, "\n");
    }
  fprintf (stderr, "@@END\n");
#endif
  return 0;
}


static int lb_dijkstra (struct lb *p)
{
  int i, j, n, best, bcost, c;

  n = p->brkp_count;

  for (i = 0; i < n; ++i)
    {
      p->brkps[i].cost = DISTANCE (p, 0, i);
      p->brkps[i].chain = 0;
      p->brkps[i].special = FALSE;
    }
  p->brkps[0].special = TRUE;

  for (i = 1; i < n; ++i)
    {
      best = 0; bcost = LB_INFINITY + 1;
      for (j = 0; j < n; ++j)
        if (!p->brkps[j].special && p->brkps[j].cost < bcost)
          {
            best = j; bcost = p->brkps[j].cost;
          }
      if (best == n - 1)
        break;
      p->brkps[best].special = TRUE;
      for (j = best + 1; j < n; ++j)
        if (!p->brkps[j].special)
          {
            c = bcost + DISTANCE (p, best, j);
            if (c < p->brkps[j].cost)
              {
                p->brkps[j].cost = c;
                p->brkps[j].chain = best;
              }
          }
    }
  return 0;
}


static int lb_reverse (struct lb *p)
{
  int i, j, k;

  i = p->brkp_count - 1;
  j = -1;
  while (i != 0)
    {
      if (p->brkps[i].chain >= i)
        return LB_INTERN;
      k = p->brkps[i].chain; p->brkps[i].chain = j; j = i; i = k;
    }
  p->brkps[0].chain = j;

#if 0
  fprintf (stderr, "[");
  for (i = 0; i != -1; i = p->brkps[i].chain)
    fprintf (stderr, " %d", i);
  fprintf (stderr, "]\n");
#endif

  return 0;
}


static int lb_break (struct lb *p)
{
  int i;
  struct node **nn, *n2, *n3, *nb;

  i = p->brkps[0].chain;
  nb = (i >= p->brkp_count - 1 ? NULL : p->brkps[i].node);
  for (nn = &p->node_list; *nn != NULL; nn = &(*nn)->next)
    {
      if ((*nn) == nb)
        {
          switch (nb->type)
            {
            case NODE_DISCR:
              n2 = lb_new (p, NODE_NEWLINE, 0, NULL, NULL);
              n2->next = (*nn)->next; (*nn) = n2;

              if (nb->pre != NULL && nb->pre[0] != 0)
                {
                  n2 = lb_new (p, NODE_PRE, nb->value_pre, nb->pre, NULL);
                  n2->next = (*nn); (*nn) = n2;
                  nn = &n2->next;
                }

              if (nb->post != NULL && nb->post[0] != 0)
                {
                  n2 = lb_new (p, NODE_POST, nb->value_post, nb->post, NULL);
                  n2->next = (*nn)->next; (*nn)->next = n2;
                  nn = &n2->next;
                }

              lb_free_node (nb);
              break;

            case NODE_GLUE:
              (*nn)->type = NODE_NEWLINE;
              for (n2 = (*nn)->next; n2 != NULL && n2->type == NODE_GLUE;
                   n2 = n3)
                {
                  n3 = n2->next;
                  lb_free_node (n2);
                }
              (*nn)->next = n2;
              break;

            case NODE_PENALTY:
              n2 = lb_new (p, NODE_NEWLINE, 0, NULL, NULL);
              n2->next = (*nn); (*nn) = n2;
              break;

            default:
              return LB_INTERN;
            }
          i = p->brkps[i].chain;
          nb = (i >= p->brkp_count - 1 ? NULL : p->brkps[i].node);
        }
    }
  return 0;
}


int lb_format (struct lb *p)
{
  int rc;

  rc = lb_penalty (p, 0);
  if (rc != 0) return rc;

  if (p->hyphenation != NULL)
    {
      rc = lb_hyphenation (p);
      if (rc != 0) return rc;
    }

  rc = lb_find_brkps (p, FALSE);
  if (rc != 0) return rc;
  p->brkps = malloc ((size_t)(p->brkp_count * sizeof (struct brkp)));
  if (p->brkps == NULL) return LB_NOMEM;
  rc = lb_find_brkps (p, TRUE);
  if (rc != 0) return rc;

  rc = lb_compute_distance (p);
  if (rc != 0) return rc;

  rc = lb_dijkstra (p);
  if (rc != 0) return rc;

  rc = lb_reverse (p);
  if (rc != 0) return rc;

  rc = lb_break (p);
  if (rc != 0) return rc;

  p->cur_node = p->node_list;
  return 0;
}


int lb_next (struct lb *p, struct lb_node *dst)
{
  const struct node *n1;

redo:
  n1 = p->cur_node;
  if (n1 == NULL)
    {
      dst->type  = LBN_END;
      dst->value = 0;
      dst->word  = NULL;
      dst->info  = NULL;
      return 0;
    }
  p->cur_node = p->cur_node->next;

  switch (n1->type)
    {
    case NODE_WORD:
    case NODE_DISCR:
      if (n1->word == NULL || n1->word[0] == 0)
        goto redo;
      dst->type = LBN_WORD;
      break;
    case NODE_PRE:
      dst->type = LBN_PRE;
      break;
    case NODE_POST:
      dst->type = LBN_POST;
      break;
    case NODE_GLUE:
      dst->type = LBN_GLUE;
      break;
    case NODE_PENALTY:
      goto redo;
    case NODE_NEWLINE:
      dst->type = LBN_NEWLINE;
      break;
    default:
      return LB_INTERN;
    }
  dst->value = n1->value;
  dst->word  = n1->word;
  dst->info  = n1->info;
  return 0;
}


int lbh_init (struct lbh **pp)
{
  struct lbh *p;
  int i;

  *pp = NULL;
  p = malloc (sizeof (struct lbh));
  if (p == NULL) return LB_NOMEM;
  for (i = 0; i < HASH_SIZE; ++i)
    p->hash_table[i] = NULL;
  *pp = p;
  return 0;
}


int lbh_exit (struct lbh **pp)
{
  struct lbh *p;
  struct hword *w1, *w2;
  int i;

  p = *pp; *pp = NULL;
  if (p != NULL)
    {
      for (i = 0; i < HASH_SIZE; ++i)
        for (w1 = p->hash_table[i]; w1 != NULL; w1 = w2)
          {
            w2 = w1->next;
            free (w1->str);
            free (w1);
          }
      free (p);
    }
  return 0;
}


static unsigned lbh_hash (const char *s)
{
  const unsigned char *u;
  unsigned h;

  h = 0;
  for (u = s; *u != 0; ++u)
    if (*u != LB_HYPHEN)
      h = (h << 2) ^ *u;
  return h % HASH_SIZE;
}


static int lbh_equal (const char *s1, const char *s2)
{
  while (*s1 != 0 || *s2 != 0)
    {
      if (*s1 == LB_HYPHEN)
        ++s1;
      else if (*s2 == LB_HYPHEN)
        ++s2;
      else if (*s1 == *s2)
        ++s1, ++s2;
      else
        return FALSE;
    }
  return TRUE;
}


int lbh_word (struct lbh *p, const char *s)
{
  unsigned h;
  struct hword *w;

  h = lbh_hash (s);
  for (w = p->hash_table[h]; w != NULL; w = w->next)
    if (lbh_equal (w->str, s))
      return (strcmp (w->str, s) == 0 ? 0 : LB_INVAL);
  w = malloc (sizeof (struct hword));
  if (w == NULL) return LB_NOMEM;
  w->str = strdup (s);
  if (w->str == NULL)
    {
      free (w);
      return LB_NOMEM;
    }
  w->next = p->hash_table[h];
  p->hash_table[h] = w;
  return 0;
}


static const struct hword *lbh_find (const struct lbh *p, const char *s)
{
  unsigned h;
  struct hword *w;

  h = lbh_hash (s);
  for (w = p->hash_table[h]; w != NULL; w = w->next)
    if (lbh_equal (w->str, s))
      return w;
  return NULL;
}
