/*  alias.h
*/

#ifndef ALIAS_H
#define ALIAS_H

#define MAX_ALIAS_LENGTH	40
#define N_ALIAS_HASH 		128

struct alias_hash
{
    struct alias_hash *next;
    char *name;
    char *replace;
};

extern char *alias_substitute(char *name);
extern void alias_change_alias(char *name, char *replace);

#endif /* ALIAS_H */
