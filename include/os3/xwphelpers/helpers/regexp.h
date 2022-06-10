#ifndef ERE_H
#define ERE_H

#ifndef BOOLEAN_DEFINED
#define BOOLEAN_DEFINED
typedef int BOOLEAN;
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

#define ERROR_REGEXP_FIRST      44000
// #define EREE_OK                 0
// #define EREE_MEM                (ERROR_REGEXP_FIRST + 0)
#ifndef ERROR_NOT_ENOUGH_MEMORY
#define ERROR_NOT_ENOUGH_MEMORY               8L
#endif
#define EREE_UNF_SUB            (ERROR_REGEXP_FIRST + 1)
#define EREE_UNEX_RANGE         (ERROR_REGEXP_FIRST + 2)
#define EREE_UNF_RANGE          (ERROR_REGEXP_FIRST + 3)
#define EREE_UNF_CCLASS         (ERROR_REGEXP_FIRST + 4)
#define EREE_UNEX_RSQR          (ERROR_REGEXP_FIRST + 5)
#define EREE_UNEX_RPAR          (ERROR_REGEXP_FIRST + 6)
#define EREE_UNEX_QUERY         (ERROR_REGEXP_FIRST + 7)
#define EREE_UNEX_PLUS          (ERROR_REGEXP_FIRST + 8)
#define EREE_UNEX_STAR          (ERROR_REGEXP_FIRST + 9)
#define EREE_UNEX_LCUR          (ERROR_REGEXP_FIRST + 10)
#define EREE_UNEX_RCUR          (ERROR_REGEXP_FIRST + 11)
#define EREE_BAD_CREP_M         (ERROR_REGEXP_FIRST + 12)
#define EREE_BAD_CREP_N         (ERROR_REGEXP_FIRST + 13)
#define EREE_UNF_CREP           (ERROR_REGEXP_FIRST + 14)
#define EREE_BAD_CREP           (ERROR_REGEXP_FIRST + 15)
#define EREE_TOO_MANY_SUB       (ERROR_REGEXP_FIRST + 16)
#define EREE_COMPILE_FSM        (ERROR_REGEXP_FIRST + 17)
#define EREE_POSIX_COLLATING    (ERROR_REGEXP_FIRST + 18)
#define EREE_POSIX_EQUIVALENCE  (ERROR_REGEXP_FIRST + 19)
#define EREE_POSIX_CCLASS_BAD   (ERROR_REGEXP_FIRST + 20)
#define EREE_BAD_BACKSLASH      (ERROR_REGEXP_FIRST + 21)
#define EREE_BAD_BACKREF        (ERROR_REGEXP_FIRST + 22)
#define EREE_SUBS_LEN           (ERROR_REGEXP_FIRST + 23)
#define ERROR_REGEXP_LAST       (ERROR_REGEXP_FIRST + 23)

#define ERECF_TOLOWER           0x01

#define EREMF_SHORTEST          0x01
#define EREMF_ANY           0x02

#define MAX_SPANS 9

typedef struct { int pos, len; } ERE_SPAN;

typedef struct
    {
        int      n_spans;
        ERE_SPAN spans[MAX_SPANS];
    } ERE_MATCHINFO;

#ifndef ERE_C

typedef void ERE;

extern ERE *rxpCompile(
    const char *str,
    int erecf,
    int *rc
    );

extern int rxpMinLen(const ERE *ere);

/* Returns the number of characters in the match, starting from pos characters
   into the string to be searched. Details of sub-matches can also be returend.
   Returns -1 if no match. */
extern int rxpMatch(
    const ERE *ere,
    int eremf,
    const char *str, int pos,
    ERE_MATCHINFO *mi       /* can be NULL */
    );

/* Returns TRUE if a match can be found starting pos characters into the string
   to be searched. Position and length of match are returned. Details of
   sub-matches can also be returned. */
extern BOOLEAN rxpMatch_fwd(
    const ERE *ere,
    int eremf,
    const char *str, int pos,
    int *pos_match, int *len_match,
    ERE_MATCHINFO *mi       /* can be NULL */
    );

/* Returns TRUE if a match can be found starting pos characters into the string
   to be searched and scanning backwards. Position and length of match are
   returned. Details of sub-matches can also be returned. */
extern BOOLEAN rxpMatch_bwd(
    const ERE *ere,
    int eremf,
    const char *str, int pos,
    int *pos_match, int *len_match,
    ERE_MATCHINFO *mi       /* can be NULL */
    );

extern void rxpFree(ERE *ere);

extern BOOLEAN rxpSubsWith(
    const char *str,        /* Original string searched          */
    int pos, int len,       /* Span of the entire match          */
    ERE_MATCHINFO *mi,      /* Details of sub-spans of match     */
    const char *with,       /* Specification of replacement      */
    char *out,          /* Substituted string buffer         */
    int len_out,            /* How much room in output buffer    */
    int *rc             /* Error, if FALSE returned          */
    );

#endif

#endif
