/* $Id: db.h,v 1.3 2000/06/09 03:12:01 jhall Exp $ */

/* include file for a simple key-value db */

/* Copyright (C) 1999,2000 Jim Hall <jhall1@isd.net> */

/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef _DB_H
#define _DB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Symbolic constants */

#define HASHSIZE 101

/* Typedefs and structs */

struct db_list{
  struct db_list *next;
  char *key;
  char *value;
};

typedef struct db_list db_t;


/* Functions */

/* db.c */

int db_init (size_t init_size);
int db_free (void);
db_t *db_insert (char *key, char *value);
int db_compare (db_t *p1, db_t *p2);
db_t *db_fetch (char *key);

#ifdef __cplusplus
}
#endif

#endif /* _DB_H */
