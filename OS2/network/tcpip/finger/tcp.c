/* tcp.c -- Functions for talking to other machines via TCP services. */

/* Copyright (C) 1988, 1990, 1992  Free Software Foundation, Inc.

   This file is part of GNU Finger.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <config.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#if !defined (hpux)
#include <arpa/inet.h>
#endif

#include <signal.h>

#include <tcp.h>
#include <general.h>

/* Because Unix is too stupid to make this a define.  This is
   worse than the ptrace stuff.  Value signifies Internet Protocol. */
#define IP 0

/* **************************************************************** */
/*                                                                  */
/*                      TCP Stream Functions                        */
/*                                                                  */
/* **************************************************************** */

/* How come these don't come in a library somewhere?  Everyone could
   use one.  Like to open a SMTP connection, or talk, or anything. */

/* Default number of seconds before timing out on connect call. */
#define TIME_OUT 4

/* Number of seconds before timing out on connect call. */
int connection_timeout_counter = TIME_OUT;

/* Non-zero means only allow TIME_OUT seconds for a connect () to
   succeed, instead of whatever the infernal network code allows. */
int allow_time_outs = 0;

/* Open a filedes to SERVICE at ADDRESS.  If SERVICE is the name of a
   service, then it must exist on the local machine.  SERVICE can also
   be the ASCII representation of a decimal number, in which case it is
   interpreted as the port number to connect to.  Returns a valid file
   descriptor if successful, or -1 if not. */
int
tcp_to_service (service, address)
  char *service;
  char *address;
{
  struct servent *server;
  struct sockaddr_in name;
  int connection;

  /* Prepare the socket name for binding. */
  bzero (&name, sizeof (name));

  name.sin_family = AF_INET;
  bcopy (address, &name.sin_addr.s_addr, 4);

  /* Find the port to use for the requested service. */
  if (strcmp (service, "cfinger") == 0)
    name.sin_port = htons (2003);
  else if (digit (*service))
    name.sin_port = htons (atoi (service));
  else
    {
      server = getservbyname (service, "tcp");
      if (!server)
        return (-1);
      name.sin_port = server->s_port;
    }

  /* Make a new socket. */
  connection = socket (PF_INET, SOCK_STREAM, IP);

  if (connection < 0)
    return (-1);

  /* Connect to the desired port.  We have a shorter timeout than
     the connect call uses by default. */
  {
    int error;
    void connect_timed_out ();

    if (allow_time_outs)
      {
        signal (SIGALRM, connect_timed_out);
        alarm (TIME_OUT);
        error = connect (connection, &name, sizeof (name));
        alarm (0);
        signal (SIGALRM, SIG_DFL);
      }
    else
      error = connect (connection, &name, sizeof (name));

    if (error < 0)
      {
        close (connection);
        return (-1);
      }
  }


  return (connection);
}

void
connect_timed_out ()
{
  alarm (0);
}


/* Compare hosts for equality: returns non-zero if HOST1 is the same
   as HOST2.  They are considered the same either if they have
   matching names, or translate to the same IP address. The latter
   requires that the two addresses are comparable, i.e. are of the
   same type and size. The IP address comparison has been added to
   avoid problems where hosts are sometimes specified with a FQDN and
   sometimes by host name alone.  In the case where one host name is a
   FQDN and the other a local network hostname, comparing the host
   name to the host name portion of the FQDN would be boobtytrapped,
   since hosts in different networks often have the same name. This
   would, in fact, be a considerable security hole, since any host
   could pretend to be any other, possibly trusted, host. */

#ifndef __EMX__
int
host_cmp (host1, host2)
  char *host1, *host2;
{
  struct hostent *ht1, *ht2;
  int type1, len1, retval;
  char *addr1, ulongbuf[sizeof (unsigned long)];


  /* Guard */
  if (!host1 || !host2)
    return 0;

  /* First just compare names */
  if (!xstricmp (host1, host2))
    return 1;

  /* Names didn't match; check if they translate
     to the same IP address. */

  if (!(ht1 = gethostbyname (host1)))
    return 0;

  if (ht1->h_length <= sizeof (unsigned long))
    addr1 = ulongbuf;
  else
    addr1 = xmalloc (ht1->h_length);

  len1 = ht1->h_length;
  type1 = ht1->h_addrtype;
  bcopy (*ht1->h_addr_list, addr1, len1);

  retval = ((ht2 = gethostbyname (host2))
            && type1 == ht2->h_addrtype
            && len1 == ht2->h_length
            && !bcmp (addr1, *ht2->h_addr_list, ht1->h_length));

  if (addr1 != ulongbuf)
    free (addr1);

  return retval;
}
#endif (__EMX__