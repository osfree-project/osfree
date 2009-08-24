/*
   Copyright (C) by Andrew Tridgell <tridge@samba.org> 1999, 2001
   Copyright (C) 2001 by Martin Pool <mbp@samba.org>

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

/* TODO: We could try allowing for different flavours of synchronous
   operation: data sync and so on.  Linux apparently doesn't make any
   distinction, however, and for practical purposes it probably
   doesn't matter.  On NFSv4 it might be interesting, since the client
   can choose what kind it wants for each OPEN operation. */

#include "dbench.h"
#include "getopt.h"

int sync_open = 0, sync_dirs = 0;
char *tcp_options = TCP_OPTIONS;
#ifdef OS2
int flat_namespace = 0;

void shm_attach_os2(void);
void sem_create_os2(void);
void sem_wait_os2(void);
void sem_signal_os2(void);
#endif

static struct child_struct *children;

#ifndef OS2
static void sigcont(void)
{
}
#endif

static void sig_alarm(void)
{
	double total = 0;
	int total_lines = 0;
	int running = 0;
	int i;
	int nprocs = children[0].nprocs;

        for (i=0;i<nprocs;i++) {
                total += children[i].bytes_in + children[i].bytes_out;
                total_lines += children[i].line;
                if (!children[i].done) running++;
        }
        /* yeah, I'm doing stdio in a signal handler. So sue me. */
        printf("%4d  %8d  %.2f MB/sec\r",
               running,
               total_lines / nprocs, 1.0e-6 * total / end_timer());
        fflush(stdout);
#ifndef OS2
        signal(SIGALRM, sig_alarm);
        alarm(PRINT_FREQ);
#endif
}

/* this creates the specified number of child processes and runs fn()
   in all of them */
static double create_procs(int nprocs, void (*fn)(struct child_struct * ))
{
	int i, status;
	int synccount;
#ifdef OS2
        int cRunning;
#endif

#ifndef OS2
        signal(SIGCONT, sigcont);
#endif
        start_timer();

	synccount = 0;

	if (nprocs < 1) {
		fprintf(stderr,
			"create %d procs?  you must be kidding.\n",
			nprocs);
		return 1;
	}

	children = shm_setup(sizeof(struct child_struct)*nprocs);
	if (!children) {
		printf("Failed to setup shared memory\n");
		return end_timer();
	}
        #ifdef OS2
        sem_create_os2();
        #endif

	memset(children, 0, sizeof(*children)*nprocs);

	for (i=0;i<nprocs;i++) {
		children[i].id = i;
		children[i].nprocs = nprocs;
	}

	for (i=0;i<nprocs;i++) {
		if (fork() == 0) {
                        #ifdef OS2
                        shm_attach_os2();
                        #endif
			setbuffer(stdout, NULL, 0);
			nb_setup(&children[i]);
			children[i].status = getpid();
                        #ifndef OS2
			pause();
                        #else
                        sem_wait_os2();
                        #endif
			fn(&children[i]);
			_exit(0);
		}
	}

	do {
		synccount = 0;
		for (i=0;i<nprocs;i++) {
			if (children[i].status) synccount++;
		}
		if (synccount == nprocs) break;
		sleep(1);
	} while (end_timer() < 30);

	if (synccount != nprocs) {
		printf("FAILED TO START %d CLIENTS (started %d)\n", nprocs, synccount);
		return end_timer();
	}

	start_timer();
#ifndef OS2
	kill(0, SIGCONT);

	signal(SIGALRM, sig_alarm);
	alarm(PRINT_FREQ);

	printf("%d clients started\n", nprocs);

	for (i=0;i<nprocs;) {
		if (waitpid(0, &status, 0) == -1) continue;
		if (WEXITSTATUS(status) != 0) {
			printf("Child failed with status %d\n",
			       WEXITSTATUS(status));
			exit(1);
		}
		i++;
	}

	alarm(0);
	sig_alarm();

#else

        sem_signal_os2();
        printf("%d clients started\n", nprocs);

        do
        {
            sleep(1);
            sig_alarm();

            for (cRunning = i = 0; i < nprocs; i++)
                    if (!children[i].done)
                        cRunning++;
        } while (cRunning > 0);
        status = status;
#endif

	printf("\n");
	return end_timer();
}


static void show_usage(void)
{
	printf("usage: dbench [OPTIONS] nprocs\n"
	       "options:\n"
	       "  -v               show version\n"
	       "  -c CLIENT.TXT    set location of client.txt\n"
	       "  -s               synchronous file IO\n"
	       "  -S               synchronous directories (mkdir, unlink...)\n"
	       "  -t options       set socket options for tbench\n"
        #ifdef OS2
               "  -f               Flat namespace everything in the clients subdir\n"
        #endif
                );

	exit(1);
}



static int process_opts(int argc, char **argv,
			int *nprocs)
{
	int c;
	extern char *client_filename;
	extern int sync_open;
	extern char *server;

#ifndef OS2
	while ((c = getopt(argc, argv, "vc:sSt:")) != -1)
#else
	while ((c = getopt(argc, argv, "fvc:sSt:")) != -1)
#endif
		switch (c) {
		case 'c':
			client_filename = optarg;
			break;
		case 's':
			sync_open = 1;
			break;
		case 'S':
			sync_dirs = 1;
			break;
		case 't':
			tcp_options = optarg;
			break;
		case 'v':
			printf("dbench version %s\nCopyright tridge@samba.org\n",
			       VERSION);
			exit(0);
			break;
                #ifdef OS2
                case 'f':
                        flat_namespace = 1;
                        break;
                #endif
		case '?':
			if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
					 "Unknown option character `\\x%x'.\n",
					 optopt);
			return 0;
		default:
			abort ();
		}
	
	if (!argv[optind])
		return 0;
	
	*nprocs = atoi(argv[optind++]);

	if (argv[optind])
		server = argv[optind++];

	return 1;
}



 int main(int argc, char *argv[])
{
	int nprocs;
	double t;
	double total_bytes = 0;
	int i;

	if (!process_opts(argc, argv, &nprocs))
		show_usage();

	t = create_procs(nprocs, child_run);

	for (i=0;i<nprocs;i++) {
		total_bytes += children[i].bytes_in + children[i].bytes_out;
	}

	t = end_timer();

	printf("Throughput %g MB/sec%s%s %d procs\n",
	       1.0e-6 * total_bytes / t,
	       sync_open ? " (sync open)" : "",
	       sync_dirs ? " (sync dirs)" : "", nprocs);
	return 0;
}
