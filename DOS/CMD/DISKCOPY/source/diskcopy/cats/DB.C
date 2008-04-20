/* A library of functions that implements a key-value database using a
   hash as the data storage/retrieval system. */

/* Copyright (C) 1999-2000 Jim Hall <jhall1@isd.net> */

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

/* The following functions are deprecated: db_init, db_free, db_compare */

/* Hash concept borrowed from _The C Programming Language_, Second Ed.,
   BWK/DMR. */


#include <stdio.h>
#include <stdlib.h>			/* malloc */
#include <string.h>			/* strdup */

#include "db.h"


/* Function prototypes */

unsigned hash (char *s);


/* Global variables */

static db_t *hashtab[HASHSIZE];


/* Functions */

/* hash() - My hash function.  This is a little simple, but it will
   suffice. */

unsigned
hash (char *s)
{
  unsigned hashval;

  for (hashval = 0; *s != '\0'; s++)
    {
      /* assign a hash value (iterative) */

      hashval = *s + 31 * hashval;
    }

  /* return the hash value */

  return (hashval % HASHSIZE);
}

/* db_fetch() - Query the hash and return a struct that contains the
   key and the pointer.  The calling function should not look beyond
   that. */

db_t *
db_fetch (char *s)
{
  db_t *db_ptr;

  for (db_ptr = hashtab[hash(s)]; db_ptr != NULL; db_ptr = db_ptr->next)
    {
      if (strcmp (s, db_ptr->key) == 0)
	{
	  /* found it */

	  return (db_ptr);
	}
    }

  /* else, not found */

  return (NULL);
}

/* db_insert() - Inserts a key,value pair into the hash.  If the key
   already exists in the hash, the new value is NOT inserted. */

db_t *
db_insert (char *key, char *value)
{
  db_t *db_ptr;
  unsigned hashval;

  if ((db_ptr = db_fetch (key)) == NULL)
    {
      /* not found */

      db_ptr = (db_t *) malloc (sizeof (*db_ptr));

      if (db_ptr == NULL || (db_ptr->key = strdup (key)) == NULL)
	{
	  return (NULL);
	}

      /* insert the key,value into the hash. */

      hashval = hash(key);
      db_ptr->next = hashtab[hashval];
      hashtab[hashval] = db_ptr;
    }

  else
    {
      /* already there */

      free ((void *) db_ptr->value);
    }

  if ((db_ptr ->value = strdup (value)) == NULL)
    {
      return (NULL);
    }

  /* else */

  return (db_ptr);
}
