
/* Functions that emulate UNIX catgets */

/* Copyright (C) 1999,2000,2001 Jim Hall <jhall@freedos.org> */

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

#include <stdio.h>			/* sprintf */
#include <stdlib.h>			/* getenv  */
#include <string.h>			/* strchr */
#include <fcntl.h>

#include "kitten.h"


/* DB stuff */

struct db_list{
  struct db_list *next;
  char *key;
  char *value;
};

typedef struct db_list db_t;

db_t *db_insert (char *key, char *value);
db_t *db_fetch (char *key);


/* External functions */

int get_line (int file, char *buffer, int size);


/* Local prototypes */

int catread (char *catfile);		/* Reads a catfile into the hash */
char *processEscChars(char *line);  /* Converts c escape sequences to chars */

/* Globals */

nl_catd _kitten_catalog = 0;			/* _kitten_catalog descriptor, either 0 or 1 */



#if 1							/* it's not very portable ;) */

#include <dos.h>
/* assert we are running in small model */
/* else pointer below has to be done correctly */
char verify_small_pointers[sizeof(void*) == 2 ? 1 : -1];


int dos_open(char *filename, int mode);
#define open(filename,mode) dos_open(filename,mode)

int dos_read(int file, void *ptr, unsigned count);
#define read(file, ptr, count) dos_read(file,ptr,count)

void dos_close(int file);
#define close(file) dos_close(file)

#ifdef __WATCOMC__

#pragma aux dos_open = \
"mov ax, 0x3d00" \
"int 0x21" \
"jnc noerror" \
"mov ax, 0xffff" \
"noerror:" \
parm [dx] [ax] value [ax];

#pragma aux dos_read = \
"mov ah, 0x3f" \
"int 0x21" \
"jnc noerror" \
"xor ax, ax" \
"noerror:" \
parm [bx] [dx] [cx] value [ax];

#pragma aux dos_close = \
"mov ah, 0x3e" \
"int 0x21" \
parm [bx];

#else

int dos_open(char *filename, int mode)
{   
  union REGS r;
	
  if (mode);					/* mode ignored - readonly supported */
	
  r.h.ah = 0x3d;
  r.h.al = 0;					/* read mode only supoported now !! */
  r.x.dx = (unsigned)filename;
  intdos(&r,&r);
	
  if (r.x.cflag)
    return -1;
  return r.x.ax;
}			

int dos_read(int file, void *ptr, unsigned count)
{   
  union REGS r;
	
  r.h.ah = 0x3f;
  r.x.bx = file;
  r.x.cx = count;
  r.x.dx = (unsigned)ptr;
  intdos(&r,&r);
	
  if (r.x.cflag)
    return 0;
  return r.x.ax;
}			

void dos_close(int file)
{   
  union REGS r;
	
  r.h.ah = 0x3e;
  r.x.bx = file;
  intdos(&r,&r);
}			
	
#endif
#endif

#ifndef NOCATS

/* Functions */

char *
pascal kittengets(int set_number, int message_number, char *message)
{
  /* get message from a message _kitten_catalog */

  /* 'message' should really be const, but not when it is returned */

  /* On success, catgets() returns a pointer to an internal buffer
     area containing the null-terminated message string.  On failure,
     catgets() returns the value 'message'.  */

  char key[10];
  db_t *ptr;


  /* fetch the message that goes with the set/message number */

  sprintf (key, "%d.%d", set_number, message_number);
  ptr = db_fetch (key);

  /* printf("\ncatgets %s\n",message); */

  if (ptr)
    {
      /*     printf("------> %s\n",ptr->value); */

      return (ptr->value);
    }

  /* else */

  return (message);
}

nl_catd
kittenopen(char *name)
{
  /* catopen() returns a message _kitten_catalog descriptor of type nl_catd on
     success.  On failure, it returns -1. */

  /* 'flag' is completely ignored here. */

  char catfile[256];		        /* full path to the msg _kitten_catalog */
  char *nlsptr;				/* ptr to NLSPATH */
  char *lang;                   /* ptr to LANG */
  
  
  	
  /* Open the _kitten_catalog file */

  /* The value of `_kitten_catalog' will be set based on catread */

  if (_kitten_catalog)
    {
      /* Already one open */

      printf("cat already open\n");	
      return (-1);
    }

  /* If the message _kitten_catalog file name contains a directory separator,
     assume that this is a real path to the _kitten_catalog file.  Note that
     catread will return a true or false value based on its ability
     to read the catfile. */

  if (strchr (name, '\\'))
    {
      /* first approximation: 'name' is a filename */
      
      printf("found \\\n");

      _kitten_catalog = catread (name);
      return (_kitten_catalog);
    }

  /* If the message _kitten_catalog file name does not contain a directory
     separator, then we need to try to locate the message _kitten_catalog on
     our own.  We will use several methods to find it. */

  /* We will need the value of LANG, and may need a 2-letter abbrev of
     LANG later on, so get it now. */

  lang = getenv ("LANG");

  if (lang == NULL)
    {  
      /* printf("no lang= found\n"); */
    
      /* Return failure - we won't be able to locate the cat file */
      return (-1);
    }


  /* step through NLSPATH */

  nlsptr = getenv ("NLSPATH");
  

  if (nlsptr == NULL)
    {
      /* printf("no NLSPATH= found\n"); */

      /* Return failure - we won't be able to locate the cat file */
      return (-1);
    }

      /* printf("nlsptr:%s\n",nlsptr); */

  while (*nlsptr)
    {
      char *tok = strchr(nlsptr, ';');
      int toklen;    
      

      if (tok == NULL) tok = nlsptr + strlen(nlsptr);
      toklen=tok-nlsptr;
      /* Try to find the _kitten_catalog file in each path from NLSPATH */

      /* Rule #1: %NLSPATH%\%LANG%\cat */

      memcpy(catfile,nlsptr,toklen);
      sprintf(catfile+toklen,"\\%s\\%s",lang,name);

      _kitten_catalog = catread (catfile);
      if (_kitten_catalog)
	{
	  return (_kitten_catalog);
	}

      /* Rule #2: %NLSPATH%\cat.%LANG% */

      sprintf(catfile+toklen,"\\%s.%s",name, lang);

      _kitten_catalog = catread (catfile);

      if (_kitten_catalog)
	{
	  return (_kitten_catalog);
	}

      /* Rule #3: if LANG looks to be in format "en-UK" then
         %NLSPATH%\cat.EN */

      if (lang[2] == '-')
        {
	  lang[2] = 0;
	  sprintf(catfile+toklen,"\\%s.%s",name,lang);
	  lang[2] = '-';

	  _kitten_catalog = catread (catfile);
	  if (_kitten_catalog)
	    {
	      return (_kitten_catalog);
	    }
        }

      /* Grab next tok for the next while iteration */
      nlsptr = tok;
      if (*nlsptr) nlsptr++;
      
    } /* while tok */

  /* We could not find it.  Return failure. */

  return (0);
}

int
catread (char *catfile)
{
  int   file;				/* pointer to the catfile */
  char *key;				/* part of key-value for hash */
  char *value;				/* part of key-value for hash */
  char inBuffer[256];                   /* the string read from the file */

  /* Open the catfile for reading */
  
  /*printf("catread %s\n",catfile); */

  file = open (catfile, O_RDONLY | O_TEXT);
  if (file < 0)
    {
      /* Cannot open the file.  Return failure */
	  /* printf("catread: cant read %s\n",catfile); */
      return (0);
    }

  /*printf("catread %s success\n",catfile);*/

  /* Read the file into memory */

  while (get_line (file, inBuffer, sizeof(inBuffer)))
    {
      /* Break into parts.  Entries should be of the form:
	 "1.2:This is a message" */

      /* A line that starts with '#' is considered a comment, and will
         be thrown away without reading it. */

      if (inBuffer[0] == '#')		/* comment */
	continue;

      if ((key = strchr (inBuffer, ':')) != NULL)
	{
	  *key = 0;
	  	
	  value = processEscChars(key+1);
	  	
	  db_insert (inBuffer, value);
	}

    } /* while */

  close (file);

  /* Return success */

  return (1);
}

void
kittenclose (void)
{
  /* close a message _kitten_catalog */

  _kitten_catalog = 0;
}



/**
 * Process strings, converting \n, \t, \v, \b, \r, \f, \\, \ddd, \xdd and \x0dd 
 * to actual chars. (Note: \x is an extension to support hexadecimal)
 * This method is used to allow the message _kitten_catalog to use c escape sequences.
 * Modifies the line in-place (always same size or shorter).
 * Returns a pointer to input string.
 */

int mystrtoul(char *src, int base, int size, int *error)
{
  int ret = 0;

  *error = 1;
	
  for (; size > 0; size--)
    {
      int digit;
      int ch = *src++;
	  
      if (ch >= '0' && ch <= '9') digit = ch - '0';
      else if (ch >= 'A' && ch <= 'Z') digit = ch - 'A' + 10;
      else if (ch >= 'a' && ch <= 'z') digit = ch - 'a' + 10;
      else
	{
	  return 0;
	}

      if (digit >= base)
	{
	  return 0;
	}
	  
      ret = ret * base + digit;
    }

  *error = 0;
	
  return ret;
}		  	  	
	  	

char *processEscChars(char *line)
{
  register char *src = line, *dst = line;

  /* used when converting \xdd and \ddd (hex or octal) characters */
  char ch;

  if (line == NULL) return NULL;

  /* cycle through copying characters, except when a \ is encountered. */
  for ( ; *src != '\0'; src++, dst++)
    {
      ch = *src;
    
      if (ch == '\\')
	{           
	  src++; /* point to char following slash */
	  switch (ch = *src)
	    {
	    case '\\': /* a single slash */
	      ch = '\\';
	      break;
	    case 'n': /* a newline (linefeed) */
	      ch = '\n';
	      break;
	    case 'r': /* a carriage return */
	      ch = '\r';
	      break;
	    case 't': /* a horizontal tab */
	      ch = '\t';
	      break;
	    case 'v': /* a vertical tab */
	      ch = '\v';
	      break;
	    case 'b': /* a backspace */
	      ch = '\b';
	      break;
	    case 'a': /* alert */
	      ch = '\a';
	      break;
	    case 'f': /* formfeed */
	      ch = '\f';
	      break;
	    case 'x': /* extension supporting hex numbers \xdd or \x0dd */
	      {
		int error;
		ch  = mystrtoul(src+1,16,2, &error); /* get value */
		if (!error) /* store character */
		  {
		    src += 2;
		  }
		else /* error so just store x (loose slash) */
		  {
		    ch = *src;
		  }
	      }

	      break;
	    default: /* just store letter (loose slash) or handle octal */
	  
	      {
		int error;
		ch  = mystrtoul(src,8,3, &error); /* get value */
		if (!error) /* store character */
		  {
		    src += 3;
		  }
		else
		  ch = *src;
	      }  
	  
	      break;
	    }
	}

      *dst = ch;
    }

  /* ensure '\0' terminated */
  *dst = '\0';

  return line;
}



int 
get_line (int file, char *str, int size)
{
  int success = 0;

  /* now, read the string */

  for ( ; size > 0; )
    {
      if (read(file,str,1) <= 0)
    	break;
    	
      success = 1;
	
      if (*str == '\r')
    	continue;
    
      if (*str == '\n')
    	break;
    	
      str++;
      size--;	

    } /* while */

  *str = 0;

  return success;
}

/* Function prototypes */


/* Global variables */

static db_t *hashtab[1];


/* Functions */


/* db_fetch() - Query the hash and return a struct that contains the
   key and the pointer.  The calling function should not look beyond
   that. */

db_t *
db_fetch (char *s)
{
  db_t *db_ptr;

  for (db_ptr = hashtab[0]; db_ptr != NULL; db_ptr = db_ptr->next)
    {
      if (strcmp (s, db_ptr->key) == 0)
	{
	  break;

	}
    }


  return (db_ptr);
}

/* db_insert() - Inserts a key,value pair into the hash.  If the key
   already exists in the hash, the new value is NOT inserted. */

db_t *
db_insert (char *key, char *value)
{
  db_t *db_ptr;

  if ((db_ptr = db_fetch (key)) == NULL)
    {
      /* not found */

      db_ptr = (db_t *) malloc (sizeof (*db_ptr));

      if (db_ptr == NULL || (db_ptr->key = strdup (key)) == NULL)
	{
	  return (NULL);
	}

      /* insert the key,value into the hash. */

      db_ptr->next = hashtab[0];
      hashtab[0] = db_ptr;
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



#endif /* NOCATS */
