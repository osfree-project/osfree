/* catgets.c - Kitten-like message catalog functions

   AUTHOR: Gregory Pietsch

   DESCRIPTION:
   
   This file, combined with nl_catd.h, provides a kitten-like version of
   the catgets() functions.  It's different from kitten in that it uses
   the dynamic string functions in the background and you could have more
   than one catalog open at a time.
*/

#include "config.h"
/*#ifdef HAVE_NL_CATD_H*/
#include "nl_types.h"
/*#endif*/

#include <ctype.h>
#include <errno.h>
#if defined(__STDC__) || defined(STDC_HEADERS) || defined(HAVE_LIMITS_H)
#include <limits.h>
#endif
#include <stdio.h>
#if defined(__STDC__) || defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#include <string.h>
#endif
#include "defines.h"
#include "dynstr.h"

typedef struct
{
  int set_id, msg_id;
  STRING_T *msg;
} _CAT_MESSAGE_T;

static void
_CAT_MESSAGE_ctor (_CAT_MESSAGE_T * x)
{
  x->set_id = x->msg_id = 0;
  x->msg = DScreate ();
}

static void
_CAT_MESSAGE_dtor (_CAT_MESSAGE_T * x)
{
  x->set_id = x->msg_id = 0;
  DSdestroy (x->msg);
  x->msg = 0;
}

static _CAT_MESSAGE_T *
_CAT_MESSAGE_assign (_CAT_MESSAGE_T * x, _CAT_MESSAGE_T * y)
{
  if (x != y)
    {
      x->set_id = y->set_id;
      x->msg_id = y->msg_id;
      DSassign (x->msg, y->msg, 0, NPOS);
    }
  return x;
}

#define T       _CAT_MESSAGE_T
#define TS      _CATMSG
#define Tassign _CAT_MESSAGE_assign
#define Tctor   _CAT_MESSAGE_ctor
#define Tdtor   _CAT_MESSAGE_dtor
#define PROTOS_ONLY
#include "dynarray.h"
#undef  PROTOS_ONLY
#include "dynarray.h"
#undef  Tdtor
#undef  Tctor
#undef  Tassign
#undef  TS
#undef  T

typedef struct
{
  int is_opened;
  _CATMSG_ARRAY_T *msgs;
} _CAT_CATALOG_T;

static void
_CAT_catalog_ctor (_CAT_CATALOG_T * x)
{
  x->is_opened = 0;
  x->msgs = _CATMSG_create ();
}

static void
_CAT_catalog_dtor (_CAT_CATALOG_T * x)
{
  x->is_opened = 0;
  _CATMSG_destroy (x->msgs);
  x->msgs = 0;
}

static _CAT_CATALOG_T *
_CAT_catalog_assign (_CAT_CATALOG_T * x, _CAT_CATALOG_T * y)
{
  if (x != y)
    {
      x->is_opened = y->is_opened;
      _CATMSG_assign (x->msgs, _CATMSG_base (y->msgs),
                      _CATMSG_length (y->msgs), 1);
    }
  return x;
}

#define T       _CAT_CATALOG_T
#define TS      _CATCAT
#define Tassign _CAT_catalog_assign
#define Tctor   _CAT_catalog_ctor
#define Tdtor   _CAT_catalog_dtor
#define PROTOS_ONLY
#include "dynarray.h"
#undef  PROTOS_ONLY
#include "dynarray.h"
#undef  Tdtor
#undef  Tctor
#undef  Tassign
#undef  TS
#undef  T

static _CATCAT_ARRAY_T *theCatalogues = 0;

/* static functions used by catopen */
enum
{
  FSM_ANY = UCHAR_MAX + 1,
  FSM_DIGIT,
  FSM_ODIGIT,
  FSM_XDIGIT,
  FSM_OUTPUT,
  FSM_SUPPRESS,
  FSM_BASE8,
  FSM_BASE8_OUTPUT,
  FSM_BASE10,
  FSM_BASE10_OUTPUT,
  FSM_BASE16,
  FSM_BASE16_OUTPUT,
  FSM_RETAIN
};

static void
transform_string (STRING_T * t, STRING_T * s)
{
  int c, state = 0, accum = 0;
  size_t ip = 0, i;
  static int fsm[] = {
    0, '\\', FSM_SUPPRESS, 1,
    0, FSM_ANY, FSM_OUTPUT, 0,
    1, 'b', '\b', 0,
    1, 'e', '\033', 0,
    1, 'f', '\f', 0,
    1, 'n', '\n', 0,
    1, 'r', '\r', 0,
    1, 't', '\t', 0,
    1, 'v', '\v', 0,
    1, '\\', '\\', 0,
    1, 'd', FSM_SUPPRESS, 2,
    1, FSM_ODIGIT, FSM_BASE8, 3,
    1, 'x', FSM_SUPPRESS, 4,
    1, FSM_ANY, FSM_OUTPUT, 0,
    2, FSM_DIGIT, FSM_BASE10, 21,
    2, FSM_ANY, FSM_RETAIN, 0,
    3, FSM_ODIGIT, FSM_BASE8, 31,
    3, FSM_ANY, FSM_RETAIN, 0,
    4, FSM_XDIGIT, FSM_BASE16, 4,
    4, FSM_ANY, FSM_RETAIN, 0,
    21, FSM_DIGIT, FSM_BASE10, 22,
    21, FSM_ANY, FSM_RETAIN, 0,
    22, FSM_DIGIT, FSM_BASE10_OUTPUT, 0,
    22, FSM_ANY, FSM_RETAIN, 0,
    31, FSM_ODIGIT, FSM_BASE8_OUTPUT, 0,
    31, FSM_ANY, FSM_RETAIN, 0,
    -1, -1, -1, -1
  };
  static char hexits[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
    'e', 'f', '\0'
  };

  DSresize (t, 0, 0);
  while (ip < DSlength (s) && (c = DSget_at (s, ip)) != '\0')
    {
      for (i = 0;
           !(fsm[i] == state
             && (fsm[i + 1] == c
                 || fsm[i + 1] == FSM_ANY
                 || (fsm[i + 1] == FSM_DIGIT && isdigit (c))
                 || (fsm[i + 1] == FSM_ODIGIT && strchr ("01234567", c) != 0)
                 || (fsm[i + 1] == FSM_XDIGIT && isxdigit (c)))); i += 4);
      switch (fsm[i + 2])
        {
        case FSM_OUTPUT:
          DSappendchar (t, c, 1);
          ip++;
          break;
        case FSM_SUPPRESS:
          accum = 0;
          ip++;
          break;
        case FSM_BASE10:
          accum = (accum * 10) + (c - '0');
          ip++;
          break;
        case FSM_BASE8:
          accum = (accum << 3) + (c - '0');
          ip++;
          break;
        case FSM_BASE16:
          accum = (accum << 4) + (strchr (hexits, tolower (c)) - hexits);
          ip++;
          break;
        case FSM_BASE10_OUTPUT:
          accum = (accum * 10) + (c - '0');
          DSappendchar (t, accum, 1);
          accum = 0;
          ip++;
          break;
        case FSM_BASE8_OUTPUT:
          accum = (accum << 3) + (c - '0');
          DSappendchar (t, accum, 1);
          accum = 0;
          ip++;
          break;
        case FSM_RETAIN:
          DSappendchar (t, accum, 1);
          accum = 0;
          break;
        default:
          DSappendchar (t, fsm[i + 2], 1);
          ip++;
          break;
        }
      state = fsm[i + 3];
    }
}

static int
catmsg_cmp (const void *a, const void *b)
{
  const _CAT_MESSAGE_T pa = *(_CAT_MESSAGE_T *) a;
  const _CAT_MESSAGE_T pb = *(_CAT_MESSAGE_T *) b;

  if (pa.set_id != pb.set_id)
    return pa.set_id < pb.set_id ? -1 : +1;
  if (pa.msg_id != pb.msg_id)
    return pa.msg_id < pb.msg_id ? -1 : +1;
  return 0;
}

/* catread - read a catalogue file */
static _CAT_CATALOG_T *
catread (const char *name)
{
  FILE *f;
  STRING_T *s = DScreate (), *t = DScreate ();
  int c;
  _CAT_CATALOG_T *cat = 0;
  size_t z;
  _CAT_MESSAGE_T catmsg = { 0, 0, 0 };
  /* Open the catfile */
  f = fopen (name, "r");
  if (f == 0)
    return 0;                   /* could not open file */
  setvbuf (f, 0, _IOFBF, 16384);
  while ((c = fgetc (f)) != EOF)
    {
      if (c == '\n')
        {
          /* We have a full line */
          if (DSlength (s) > 0)
            {
              z = DSfind_first_not_of (s, " \t\f\v\r", 0, NPOS);
              DSremove (s, 0, z);
              z = DSfind_last_not_of (s, " \t\f\v\r", NPOS, NPOS);
              DSresize (s, z + 1, 0);
            }
          if (DSlength (s) > 0 && DSget_at (s, DSlength (s) - 1) == '\\')
            {
              /* continuation */
              DSresize (s, DSlength (s) - 1, 0);
            }
          else
            {
              if (DSlength (s) > 0 && isdigit (DSget_at (s, 0)))
                {
                  /* if it starts with a digit, assume it's a catalog line */
                  for (z = 0, catmsg.set_id = 0;
                       isdigit (c = DSget_at (s, z));
                       catmsg.set_id = catmsg.set_id * 10 + (c - '0'), z++);
                  z++;
                  for (catmsg.msg_id = 0;
                       isdigit (c = DSget_at (s, z));
                       catmsg.msg_id = catmsg.msg_id * 10 + (c - '0'), z++);
                  z++;
                  DSremove (s, 0, z);
                  transform_string (t, s);
                  if (catmsg.msg == 0)
                    catmsg.msg = DScreate ();
                  DSassign (catmsg.msg, t, 0, NPOS);
                  if (cat == 0)
                    {
                      cat = malloc (sizeof (_CAT_CATALOG_T));
                      if (cat == 0)
                        Nomemory ();
                      cat->is_opened = 0;
                      cat->msgs = _CATMSG_create ();
                    }
                  _CATMSG_append (cat->msgs, &catmsg, 1, 0);
                }
              DSresize (s, 0, 0);
            }
        }
      else
        DSappendchar (s, c, 1);
    }
  fclose (f);
  qsort (_CATMSG_base (cat->msgs), _CATMSG_length (cat->msgs),
         sizeof (_CAT_MESSAGE_T), catmsg_cmp);
  return cat;
}

/* install_catalog - install a _CAT_CATALOG_T, return where */
static nl_catd
install_catalog (_CAT_CATALOG_T * cat)
{
  size_t i;
  cat->is_opened = 1;
  if (theCatalogues == 0)
    {
      theCatalogues = _CATCAT_create ();
      _CATCAT_append (theCatalogues, cat, 1, 0);
      return (nl_catd) 0;
    }
  else
    {
      for (i = 0;
           i < _CATCAT_length (theCatalogues)
           && _CATCAT_get_at (theCatalogues, i)->is_opened; i++);
      _CATCAT_put_at (theCatalogues, i, cat);
      return (nl_catd) i;
    }
}


/*
  NAME

    catopen - open a message catalog

  SYNOPSIS

    #include <nl_types.h>

    nl_catd catopen(const char *name, int oflag);

  DESCRIPTION

    The catopen() function shall open a message catalog and return a message 
        catalog descriptor. The name argument specifies the name of the message 
        catalog to be opened. If name contains a '/' , then name specifies a 
        complete name for the message catalog. Otherwise, the environment variable 
        NLSPATH is used with name substituted for the %N conversion specification 
        (see the Base Definitions volume of IEEE Std 1003.1-2001, Chapter 8, 
        Environment Variables). If NLSPATH exists in the environment when the 
        process starts, then if the process has appropriate privileges, the 
        behavior of catopen() is undefined. If NLSPATH does not exist in the 
        environment, or if a message catalog cannot be found in any of the 
        components specified by NLSPATH, then an implementation-defined default 
        path shall be used. This default may be affected by the setting of 
        LC_MESSAGES if the value of oflag is NL_CAT_LOCALE, or the LANG 
        environment variable if oflag is 0.

    A message catalog descriptor shall remain valid in a process until that 
        process closes it, or a successful call to one of the exec functions. A 
        change in the setting of the LC_MESSAGES category may invalidate 
        existing open catalogs.

    If a file descriptor is used to implement message catalog descriptors, the 
        FD_CLOEXEC flag shall be set; see <fcntl.h>.

    If the value of the oflag argument is 0, the LANG environment variable is 
        used to locate the catalog without regard to the LC_MESSAGES category. If 
        the oflag argument is NL_CAT_LOCALE, the LC_MESSAGES category is used to 
        locate the message catalog (see the Base Definitions volume of IEEE Std 
        1003.1-2001, Section 8.2, Internationalization Variables).

  RETURN VALUE

    Upon successful completion, catopen() shall return a message catalog 
        descriptor for use on subsequent calls to catgets() and catclose(). 
        Otherwise, catopen() shall return (nl_catd)(-1) and set errno to indicate 
        the error.

  ERRORS

    The catopen() function may fail if:

    [EACCES]
        Search permission is denied for the component of the path prefix of 
                the message catalog or read permission is denied for the message 
                catalog.
    [EMFILE]
        {OPEN_MAX} file descriptors are currently open in the calling process.
    [ENAMETOOLONG]
        The length of a pathname of the message catalog exceeds {PATH_MAX} or 
                a pathname component is longer than {NAME_MAX}.
    [ENAMETOOLONG]
        Pathname resolution of a symbolic link produced an intermediate result 
                whose length exceeds {PATH_MAX}.
    [ENFILE]
        Too many files are currently open in the system.
    [ENOENT]
        The message catalog does not exist or the name argument points to an 
                empty string.
    [ENOMEM]
        Insufficient storage space is available.
    [ENOTDIR]
        A component of the path prefix of the message catalog is not a directory.

*/

nl_catd (catopen) (const char *name, int oflag)
{
  _CAT_CATALOG_T *_kitten_catalog = 0;
  char *lang = 0;
  char *nlsptr = 0;
  static STRING_T *catfile = 0;

  /* Open the catalog file. */
  /* If the message _kitten_catalog file name contains a directory separator,
     assume that this is a real path to the _kitten_catalog file.  Note that
     catread will return a true or false value based on its ability
     to read the catfile. */
  if (strpbrk (name, "\\/:") != 0)
    {
      /* first approximation: 'name' is a filename */

      _kitten_catalog = catread (name);
      return _kitten_catalog ?
        install_catalog (_kitten_catalog) : (nl_catd) (-1);
    }

  /* If the message _kitten_catalog file name does not contain a directory
     separator, then we need to try to locate the message _kitten_catalog on
     our own.  We will use several methods to find it. */

  /* We will need the value of LANG, and may need a 2-letter abbrev of
     LANG later on, so get it now. */

  lang = getenv ("LANG");
  if (lang == 0)
    {
      /* printf("no lang= found\n"); */

      /* Return failure - we won't be able to locate the cat file */
      return (-1);
    }


  /* step through NLSPATH */

  nlsptr = getenv ("NLSPATH");
  if (nlsptr == 0)
    {
      /* printf("no NLSPATH= found\n"); */

      /* Return failure - we won't be able to locate the cat file */
      return (-1);
    }

  while (nlsptr != 0)
    {
      char *tok = strchr (nlsptr, ';');
      size_t toklen;
      if (tok == 0)
        tok = nlsptr + strlen (nlsptr);
      toklen = tok - nlsptr;
      if (catfile == 0)
        catfile = DScreate ();
      /* Try to find the _kitten_catalog file in each path from NLSPATH */
      /* Rule #1: %NLSPATH%\%LANG%\cat */
      DSassigncstr (catfile, nlsptr, toklen);
      DSappendchar (catfile, '/', 1);
      DSappendcstr (catfile, lang, NPOS);
      DSappendcstr (catfile, "/cat", NPOS);
      _kitten_catalog = catread (DScstr (catfile));
      if (_kitten_catalog != 0)
        break;
      /* Rule #2: %NLSPATH%\cat.%LANG% */
      DSassigncstr (catfile, nlsptr, toklen);
      DSappendcstr (catfile, "/cat.", NPOS);
      DSappendcstr (catfile, lang, NPOS);
      _kitten_catalog = catread (DScstr (catfile));
      if (_kitten_catalog != 0)
        break;
      /* Rule #3: if LANG looks to be in format "en-UK" then
         %NLSPATH%\cat.EN */
      if (lang[2] == '-')
        {
          DSassigncstr (catfile, nlsptr, toklen);
          DSappendcstr (catfile, "/cat.", NPOS);
          DSappendcstr (catfile, lang, 2);
          _kitten_catalog = catread (DScstr (catfile));
          if (_kitten_catalog != 0)
            break;
        }

      /* Grab next tok for the next while iteration */
      nlsptr = tok;
      if (nlsptr)
        nlsptr++;
      DSdestroy (catfile);
      catfile = 0;
    }                           /* while tok */

  /* If we could not find it, return failure. Otherwise, install the
     catalog and return a cookie.  */
  return _kitten_catalog ? install_catalog (_kitten_catalog) : (nl_catd) (-1);
}

/*
  NAME

    catgets - read a program message

  SYNOPSIS

    #include <nl_types.h>

    char *catgets(nl_catd catd, int set_id, int msg_id, const char *s); 

  DESCRIPTION

    The catgets() function shall attempt to read message msg_id, in set 
        set_id, from the message catalog identified by catd. The catd argument is 
        a message catalog descriptor returned from an earlier call to catopen(). 
        The s argument points to a default message string which shall be returned 
        by catgets() if it cannot retrieve the identified message.

    The catgets() function need not be reentrant. A function that is not 
        required to be reentrant is not required to be thread-safe.

  RETURN VALUE

    If the identified message is retrieved successfully, catgets() shall 
        return a pointer to an internal buffer area containing the null-terminated 
        message string. If the call is unsuccessful for any reason, s shall be 
        returned and errno may be set to indicate the error.

  ERRORS

    The catgets() function may fail if:

    [EBADF]
        The catd argument is not a valid message catalog descriptor open for 
                reading.
    [EBADMSG]
        The message identified by set_id and msg_id in the specified message 
                catalog did not satisfy implementation-defined security criteria.
    [EINTR]
        The read operation was terminated due to the receipt of a signal, and 
                no data was transferred.
    [EINVAL]
        The message catalog identified by catd is corrupted.
    [ENOMSG]
        The message identified by set_id and msg_id is not in the message catalog.

*/
char *(catgets) (nl_catd catd, int set_id, int msg_id, const char *s)
{
  _CAT_CATALOG_T *cat;
  static _CAT_MESSAGE_T keymsg = { 0, 0, 0 };
  _CAT_MESSAGE_T *msgptr = 0;

  if (theCatalogues == 0 || _CATCAT_length (theCatalogues) <= catd
      || (cat = _CATCAT_get_at (theCatalogues, catd))->is_opened == 0)
    {
      errno = EBADF;
      return (char *) s;
    }
  keymsg.set_id = set_id;
  keymsg.msg_id = msg_id;
  msgptr =
    bsearch (&keymsg, _CATMSG_base (cat->msgs), _CATMSG_length (cat->msgs),
             sizeof (_CAT_MESSAGE_T), catmsg_cmp);
  if (msgptr == 0)
    {
#ifdef ENOMSG
      errno = ENOMSG;
#endif
      return (char *) s;
    }
  return DScstr (msgptr->msg);
}

/*
  NAME

    catclose - close a message catalog descriptor

  SYNOPSIS

    #include <nl_types.h>

    int catclose(nl_catd catd);

  DESCRIPTION

    The catclose() function shall close the message catalog identified by 
        catd. If a file descriptor is used to implement the type nl_catd, that 
        file descriptor shall be closed.

  RETURN VALUE

    Upon successful completion, catclose() shall return 0; otherwise, -1 
        shall be returned, and errno set to indicate the error.

  ERRORS

    The catclose() function may fail if:

    [EBADF]
        The catalog descriptor is not valid.
    [EINTR]
        The catclose() function was interrupted by a signal.
*/
int (catclose) (nl_catd catd)
{
  _CAT_CATALOG_T *cat;

  if (theCatalogues == 0 || _CATCAT_length (theCatalogues) <= catd
      || (cat = _CATCAT_get_at (theCatalogues, catd))->is_opened == 0)
    {
      errno = EBADF;
      return -1;
    }
  cat->is_opened = 0;
  _CATMSG_resize (cat->msgs, 0, 0);
  return 0;
}

/* END OF FILE */