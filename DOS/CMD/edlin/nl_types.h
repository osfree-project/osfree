/* nl_types.h - Substitute nl_types.h that is kitten-compatible

   AUTHOR: Gregory Pietsch

   DESCRIPTION:

   The <nl_types.h> header shall contain definitions of at least the following 
   types:

   nl_catd
     Used by the message catalog functions catopen(), catgets(), and 
         catclose() to identify a catalog descriptor.
   nl_item
     Used by nl_langinfo() to identify items of langinfo data. Values of 
         objects of type nl_item are defined in <langinfo.h> .
*/
#ifndef _NL_TYPES
#define _NL_TYPES

#if defined(__STDC__) || defined(STDC_HEADERS) || defined(HAVE_STDDEF_H)
#include <stddef.h>
#endif
#ifndef _NL_CATD_DEFINED
#define _NL_CATD_DEFINED
typedef size_t nl_catd;
#endif
#ifndef _NL_ITEM_DEFINED
#define _NL_ITEM_DEFINED
typedef int nl_item;
#endif
/*
   The <nl_types.h> header shall contain definitions of at least the following 
   constants:

   NL_SETD
     Used by gencat when no $set directive is specified in a message text 
         source file; see the Internationalization Guide. This constant can be passed 
         as the value of set_id on subsequent calls to catgets() (that is, to 
         retrieve messages from the default message set). The value of NL_SETD is 
         implementation-defined.
   NL_CAT_LOCALE
     Value that must be passed as the oflag argument to catopen() to ensure that 
         message catalog selection depends on the LC_MESSAGES locale category, rather 
         than directly on the LANG environment variable.
*/
#define NL_SETD                 0
#define NL_CAT_LOCALE   1
/*
   The following shall be declared as functions and may also be defined as macros. 
   Function prototypes shall be provided.
*/
#ifdef __cplusplus
extern "C"
{
#endif
  int catclose (nl_catd);
  char *catgets (nl_catd, int, int, const char *);
  nl_catd catopen (const char *, int);
#ifdef __cplusplus
};
#endif

#endif /* _NL_TYPES */

/* END OF FILE */
