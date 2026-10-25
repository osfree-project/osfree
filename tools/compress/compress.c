/*
 *  mscompress: Microsoft "compress.exe/expand.exe" compatible compressor
 *
 *  Copyright (c) 2000 Martin Hinner <mhi@penguin.cz>
 *  Algorithm & data structures by M. Winterhoff <100326.2776@compuserve.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

extern char *version_string;

#define BSWAP32(x)             ((((x)&0xFF)<<24)+(((x)<<8)&0xFF0000)+\
			        (((x)>>8)&0xFF00)+(((x)>>24)&0xFF))

#define N 4096
#define F 16
#define THRESHOLD 3

#define dad (node+1)
#define lson (node+1+N)
#define rson (node+1+N+N)
#define root (node+1+N+N+N)
#define NIL -1

char *buffer;
int *node;
int pos;

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef min
#define min(x,y) ((y)>(x)?(x):(y))
#endif

int
insert (int i, int run)
{
  int c, j, k, l, n, match;
  int *p;

  k = l = 1;
  match = THRESHOLD - 1;
  p = &root[(unsigned char) buffer[i]];
  lson[i] = rson[i] = NIL;

  while ((j = *p) != NIL)
    {
      n = min (k, l);
      while (n < run && (c = (buffer[j + n] - buffer[i + n])) == 0)
	n++;

      if (n > match)
	{
	  match = n;
	  pos = j;
	}
      if (c < 0)
	{
	  p = &lson[j];
	  k = n;
	}
      else if (c > 0)
	{
	  p = &rson[j];
	  l = n;
	}
      else
	{
	  dad[j] = NIL;
	  dad[lson[j]] = lson + i - node;
	  dad[rson[j]] = rson + i - node;
	  lson[i] = lson[j];
	  rson[i] = rson[j];
	  break;
	}
    }
  dad[i] = p - node;
  *p = i;
  return match;
}

void
delete (int z)
{
  int j;

  if (dad[z] != NIL)
    {
      if (rson[z] == NIL)
	{
	  j = lson[z];
	}
      else if (lson[z] == NIL)
	{
	  j = rson[z];
	}
      else
	{
	  j = lson[z];
	  if (rson[j] != NIL)
	    {
	      do
		{
		  j = rson[j];
		}
	      while (rson[j] != NIL);
	      node[dad[j]] = lson[j];
	      dad[lson[j]] = dad[j];
	      lson[j] = lson[z];
	      dad[lson[z]] = lson + j - node;
	    }
	  rson[j] = rson[z];
	  dad[rson[z]] = rson + j - node;
	}
      dad[j] = dad[z];
      node[dad[z]] = j;
      dad[z] = NIL;
    }
}

int
getbyte (int f)
{
  unsigned char b;

  /* Is there a better way?  */
  if (read (f, &b, sizeof (b)) != 1)
    return -1;
  return b;
}

int
compress (int in, char *inname, int out, char *outname)
{
  int ch, i, run, len, match, size, mask;
  char buf[17];
  struct stat st;
  uint32_t magic1;
  uint32_t magic2;
  uint16_t magic3;
  uint32_t filesize;

  /* 28.5 kB */
  buffer = malloc (N + F + (N + 1 + N + N + 256) * sizeof (int));
  if (!buffer)
    {
      fprintf (stderr, "%s: Not enough memory!\n", inname);
      return -1;
    }


  if (fstat (in, &st) < 0)
    {
      perror (inname);
      return -1;
    }

  /* Fill in header */
#ifdef WORDS_BIGENDIAN		/* Sparc, MC68000, PPC, ... */
  magic1 = 0x535A4444;		/* SZDD */
  magic2 = 0x88f02733;
  magic3 = 0x4100;
  filesize = BSWAP32 (st.st_size);
#else				/* Intel, VAX, ... */
  magic1 = 0x44445A53L;		/* SZDD */
  magic2 = 0x3327F088L;
  magic3 = 0x0041;
  filesize = st.st_size;
#endif

  /* Write header to the output file */
  if (write (out, &magic1, sizeof (magic1)) != sizeof (magic1))
    {
      perror (outname);
      free (buffer);
      return -1;
    }
  if (write (out, &magic2, sizeof (magic2)) != sizeof (magic2))
    {
      perror (outname);
      free (buffer);
      return -1;
    }

  if (write (out, &magic3, sizeof (magic3)) != sizeof (magic3))
    {
      perror (outname);
      free (buffer);
      return -1;
    }

  if (write (out, &filesize, sizeof (filesize)) != sizeof (filesize))
    {
      perror (outname);
      free (buffer);
      return -1;
    }

  node = (int *) (buffer + N + F);
  for (i = 0; i < 256; i++)
    root[i] = NIL;
  for (i = NIL; i < N; i++)
    dad[i] = NIL;
  size = mask = 1;
  buf[0] = 0;
  i = N - F - F;
  for (len = 0; len < F && (ch = getbyte (in)) != -1; len++)
    {
      buffer[i + F] = ch;
      i = (i + 1) % N;
    }
  run = len;
  do
    {
      ch = getbyte (in);
      if (i >= N - F)
	{
	  delete (i + F - N);
	  buffer[i + F] = buffer[i + F - N] = ch;
	}
      else
	{
	  delete (i + F);
	  buffer[i + F] = ch;
	}
      match = insert (i, run);
      if (ch == -1)
	{
	  run--;
	  len--;
	}
      if (len++ >= run)
	{
	  if (match >= THRESHOLD)
	    {
//printf("%u{%u,%u}",size,pos, match-3);
	      buf[size++] = pos;
	      buf[size++] = ((pos >> 4) & 0xF0) + (match - 3);
	      len -= match;
	    }
	  else
	    {
	      buf[0] |= mask;
//printf("%u[%c]",size,buffer[i]);
	      buf[size++] = buffer[i];
	      len--;
	    }
	  if (!((mask += mask) & 0xFF))
	    {
	      if (write (out, buf, size) != size)
		{
		  perror (outname);
		  free (buffer);
		  return -1;
		}
	      size = mask = 1;
	      buf[0] = 0;
	    }
	}
      i = (i + 1) & (N - 1);
    }
  while (len > 0);

  if (size > 1)
    if (write (out, buf, size) != size)
      {
	perror (outname);
	free (buffer);
	return -1;
      }

  free (buffer);
  return 0;
}

void
usage (char *progname)
{
  printf ("Usage: %s [-h] [-V] [file ...]\n"
	  " -h --help        give this help\n"
	  " -V --version     display version number\n"
	  " file...          files to compress."
	  "\n"
	  "Report bugs to <mhi@penguin.cz>\n", progname);
  exit (0);
}

int
main (int argc, char **argv)
{
  int in, out;
  char *argv0;
  int c;
  char name[0x100];

  argv0 = argv[0];

  while ((c = getopt (argc, argv, "hV")) != -1)
    {
      switch (c)
	{
	case 'h':
	  usage (argv0);
	case 'V':
	  printf ("mscompress version %s "__DATE__ " \n",
			version_string);
	  return 0;
	default:
	  usage (argv0);
	}
    }

  argc -= optind;
  argv += optind;

  if (argc == 0)
    {
      fprintf (stderr, "%s: No files specified\n", argv0);
      usage (argv0);
    }

  while (argc)
    {
      if (argv[0][strlen (argv[0]) - 1] == '_')
	{
	  fprintf (stderr, "%s: Already ends with underscore -- ignored\n", argv[0]);
	  argc--;
	  argv++;
	  continue;
	}

      in = open (argv[0], O_RDONLY);
      if (in < 0)
	{
	  perror (argv[0]);
	  return 1;
	}

      strcpy (name, argv[0]);
      strcat (name, "_");

      out = open (name, O_WRONLY | O_CREAT | O_EXCL, 0644);
      if (out < 0)
	{
	  perror (name);
	  return 1;
	}

      compress (in, argv[0], out, name);
      close (in);
      close (out);

      argc--;
      argv++;
    }

  return 0;
}
