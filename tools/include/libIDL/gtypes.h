#ifndef _GTYPESH_
#define _GTYPESH_
typedef int            (*GCompareFunc)         (const void *  a,
                                                 const void *  b);
typedef int            (*GCompareDataFunc)     (const void *  a,
                                                 const void *  b,
                                                 void *       user_data);
typedef int        (*GEqualFunc)           (const void *  a,
                                                 const void *  b);
typedef void            (*GDestroyNotify)       (void *       data);
typedef void            (*GFunc)                (void *       data,
                                                 void *       user_data);
typedef unsigned int           (*GHashFunc)            (const void *  key);
typedef void            (*GHFunc)               (void *       key,
                                                 void *       value,
                                                 void *       user_data);
typedef void            (*GFreeFunc)            (void *       data);
typedef const char *   (*GTranslateFunc)       (const char   *str,
                                                 void *       data);

#define GPOINTER_TO_SIZE(p)     ((unsigned long) (p))
#define GSIZE_TO_POINTER(s)     ((void *) (unsigned long) (s))
#define GPOINTER_TO_INT(p)      ((int)   (p))
#define GPOINTER_TO_UINT(p)     ((unsigned int)  (p))

#define GINT_TO_POINTER(i)      ((void *)  (i))
#define GUINT_TO_POINTER(u)     ((void *)  (u))

#endif
