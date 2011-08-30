/**
 * \file   l4vfs/term_server/lib/vt100/tabs.c
 * \brief  tabulator management
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/log/l4log.h>

#include "lib.h"

extern int _DEBUG;

void set_tab( termstate_t *term, int index )
{
    int i=0;
    int j=0;

    if (term->tab_last == MAX_TABS-1)
    {
        LOG("cannot set any more tabulators.");
        return;
    }

    LOGd(_DEBUG, "setting tab at %d", index );

    // to keep the array sorted, skip all entries for
    // tabs before current pos
    while ( (term->tabs[i]<index) && (i<=term->tab_last) )
        i++;

    // now shift all remaining entries one step to the right
    for (j=term->tab_last+1 ; j > i; j-- )
    {
        term->tabs[j] = term->tabs[j-1];
    }

    // now we can store index in tabs[i]
    term->tabs[i] = index;

    // increment tab_last
    term->tab_last++;

    return;
}

void remove_tab( termstate_t *term, int index )
{
    int i=0;

    LOGd(_DEBUG, "removing tab at %d", index);

    // find position where index tab should be
    while ( term->tabs[i] < index )
        i++;

    // if the tab found is index, erase it
    if (term->tabs[i] == index)
    {
        int j;
        for (j=i; j < term->tab_last; j++)
        {
            term->tabs[j] = term->tabs[j+1];
        }
        term->tab_last--;
    }

    return;
}

void clear_tabs(termstate_t *term)
{
    // simply invalidate tab_last
    term->tab_last = -1;

    return;
}

int next_tab( termstate_t *term, int pos )
{
    int i=0;

    while (term->tabs[i] < pos)
        i++;

    // if we are behind the last tab, cursor
    // shall move to the end of the line
    if ( i>term->tab_last )
        return term->w-1;
    else
        return term->tabs[i];
}
