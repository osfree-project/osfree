/* alias.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alias.h"

static struct alias_hash *alias_table[N_ALIAS_HASH];

static unsigned int alias_hash(const char * name)
{
    unsigned int hash = 0;
    const char * p;

    p = name;

    while (*p) 
	hash = (hash << 15) + (hash << 3) + (hash >> 3) + *p++;

    return hash % N_ALIAS_HASH;
}

char *alias_substitute(char *name)
{
    struct alias_hash *ah;
    
    for (ah = alias_table[alias_hash(name)]; ah; ah = ah->next)
	if (strcasecmp(ah->name, name) == 0)
	    return ah->replace;
    
    return NULL;
}

void alias_change_alias(char *name, char *replace)
{
    struct alias_hash **head;
    struct alias_hash *ah;
    
    head = &alias_table[alias_hash(name)];
    for (ah = *head; ah; ah = ah->next)
	if (strcasecmp(ah->name, name) == 0)
	    break;
    
    if (!ah)
    {
	ah = (struct alias_hash *) calloc(1, sizeof(*ah));
	ah->name = (char *) strdup(name);
    }
    else
    {
	free(ah->replace);
    }
    
    ah->replace = (char *) strdup(replace);
    ah->next = *head;
    *head = ah;
}
