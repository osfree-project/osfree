/*
   dbench version 1
   Copyright (C) Andrew Tridgell 1999

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* This file links against either fileio.c to do operations against a
   local filesystem (making dbench), or sockio.c to issue SMB-like
   command packets over a socket (making tbench).

   So, the pattern of operations and the control structure is the same
   for both benchmarks, but the operations performed are different.
*/

#include "dbench.h"

char *client_filename = DATADIR "client_oplocks.txt";
#ifdef OS2
extern int flat_namespace;
#endif


FILE * open_client_dump(void)
{
	FILE		*f;

	if ((f = fopen(client_filename, "rt")) != NULL)
		return f;

	fprintf(stderr,
		"dbench: error opening %s: %s\n", client_filename,
		strerror(errno));

	return NULL;
}

#define ival(s) strtol(s, NULL, 0)

void child_run(struct child_struct *child)
{
	int i;
	char line[1024];
	char cname[20];
	FILE *f;
	char *params[20];

	child->line = 0;

	sprintf(cname,"client%d", child->id);

	f = open_client_dump();

	if (!f) {
		exit(1);
	}

	while (fgets(line, sizeof(line)-1, f)) {
		child->line++;

		all_string_sub(line,"client1", cname);
                #ifndef OS2
		all_string_sub(line,"\\", "/");
                #else
                if (!flat_namespace)
                    all_string_sub(line,"/", " ");
                else
                {
                    all_string_sub(line,"\\", "_");
                    all_string_sub(line," _clients_", " /clients/");
                    all_string_sub(line," _clients",  " /clients");
                }
                #endif
		all_string_sub(line," /", " ");

		
		/* parse the command parameters */
		params[0] = strtok(line," \n");
		i = 0;
		while (params[i])
                    params[++i] = strtok(NULL," \n");
		params[i] = "";

		if (i < 2) continue;

		if (!strncmp(params[0],"SMB", 3)) {
			printf("ERROR: You are using a dbench 1 load file\n");
			exit(1);
		}

		if (!strcmp(params[0],"NTCreateX")) {
			nb_createx(child, params[1], ival(params[2]), ival(params[3]),
				   ival(params[4]));
		} else if (!strcmp(params[0],"Close")) {
			nb_close(child, ival(params[1]));
		} else if (!strcmp(params[0],"Rename")) {
			nb_rename(child, params[1], params[2]);
		} else if (!strcmp(params[0],"Unlink")) {
			nb_unlink(child, params[1]);
		} else if (!strcmp(params[0],"Deltree")) {
			nb_deltree(child, params[1]);
		} else if (!strcmp(params[0],"Rmdir")) {
			nb_rmdir(child, params[1]);
		} else if (!strcmp(params[0],"QUERY_PATH_INFORMATION")) {
			nb_qpathinfo(child, params[1]);
		} else if (!strcmp(params[0],"QUERY_FILE_INFORMATION")) {
			nb_qfileinfo(child, ival(params[1]));
		} else if (!strcmp(params[0],"QUERY_FS_INFORMATION")) {
			nb_qfsinfo(child, ival(params[1]));
		} else if (!strcmp(params[0],"FIND_FIRST")) {
			nb_findfirst(child, params[1]);
		} else if (!strcmp(params[0],"WriteX")) {
			nb_writex(child, ival(params[1]),
				  ival(params[2]), ival(params[3]), ival(params[4]));
		} else if (!strcmp(params[0],"ReadX")) {
			nb_readx(child, ival(params[1]),
				  ival(params[2]), ival(params[3]), ival(params[4]));
		} else if (!strcmp(params[0],"Flush")) {
			nb_flush(child, ival(params[1]));
		} else {
			printf("Unknown operation %s\n", params[0]);
			fflush(stdout);
			exit(1);
		}
	}
	fclose(f);

	nb_cleanup(child);

	child->done = 1;
}
