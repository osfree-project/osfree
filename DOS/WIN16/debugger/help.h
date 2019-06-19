/* help.h
 *
 * Copyright 1995 - Willows Software, Inc.
 */
#ifndef DEBUGGER_HELP_H
#define DEBUGGER_HELP_H

typedef struct help_menu_entry help_menu_entry_t;
typedef struct help_arglist help_arglist_t;

struct help_menu_entry
{
    char *keyword;
    char *description;
    char *help_text;
    help_menu_entry_t *submenu;
};

struct help_arglist
{
    help_arglist_t *next;
    char *arg;
};

#endif /* DEBUGGER_HELP_H */
