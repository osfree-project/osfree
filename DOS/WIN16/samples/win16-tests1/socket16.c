/*
 * Win16 Unit test suite for networking functions.
 *
 * Copyright 2007 Jennfier Lai, Google
 * Modified from /wine-git/dlls/ws2_32/tests/sock.c written by
 * Thomas Kho, Copyright 2005
 * Martin Wilck, Copyright 2002 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#define STRICT
#include <stdio.h>
#include <windows.h>
#include <winsock.h>
#include <dos.h>                /* for delay function */
#include "test.h"

#define BIND_SLEEP 10           /* seconds to wait between attempts to bind() */
#define BIND_TRIES 6            /* Number of bind() attempts */
#define NUM_UDP_PEERS 3
#define NUM_QUERIES 250    /* Number of getservbyname queries per thread */
#define SERVERIP "127.0.0.1"    /* IP to bind to */
#define SERVERPORT 9374         /* Port number to bind to */

/* Information in the server about open client connections */
typedef struct sock_info {
    SOCKET s;
    struct sockaddr_in addr;
    struct sockaddr_in peer;
    char *buf;
    int n_recvd;
    int n_sent;
} sock_info;


/********* some tests for getsockopt(setsockopt(X)) == X ***********/
/* optname = SO_LINGER */
LINGER linger_testvals[] = {
    {0, 0}
    ,
    {0, 73}
    ,
    {1, 0}
    ,
    {5, 189}
};


/* optname = SO_RCVTIMEO, SOSNDTIMEO */
#define SOCKTIMEOUT1 6000       /* 6 seconds. Do not test fractional part because of a
                                   bug in the linux kernel (fixed in 2.6.8) */
#define SOCKTIMEOUT2 18000      /* 18 seconds */

/**************** General utility functions ***************/
static int tcp_socketpair16(SOCKET * src, SOCKET * dst)
{
    SOCKET server = INVALID_SOCKET;
    struct sockaddr_in addr;
    int len;
    int ret;

    *src = INVALID_SOCKET;
    *dst = INVALID_SOCKET;

    *src = socket(AF_INET, SOCK_STREAM, 0);
    if (*src == INVALID_SOCKET)
        goto end;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
        goto end;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ret = bind(server, (struct sockaddr *) &addr, sizeof(addr));
    if (ret != 0)
        goto end;

    len = sizeof(addr);
    ret = getsockname(server, (struct sockaddr *) &addr, &len);
    if (ret != 0)
        goto end;

    ret = listen(server, 1);
    if (ret != 0)
        goto end;

    ret = connect(*src, (struct sockaddr *) &addr, sizeof(addr));
    if (ret != 0)
        goto end;

    len = sizeof(addr);
    *dst = accept(server, (struct sockaddr *) &addr, &len);

  end:
    if (server != INVALID_SOCKET)
        closesocket(server);
    if (*src != INVALID_SOCKET && *dst != INVALID_SOCKET)
        return 0;
    closesocket(*src);
    closesocket(*dst);
    return -1;
}

static int strcmp32(char FAR *s1, char FAR *s2)
{
	while(*s1 == *s2++)
		if(*s1++ == 0)
			return 0;
	return 1;
}
/**************** Server utility functions ***************/
/*
 *  Even if we have closed our server socket cleanly,
 *  the OS may mark the address "in use" for some time -
 *  this happens with native Linux apps, too.
 */
static void do_bind(SOCKET s, struct sockaddr *addr, int addrlen)
{
    int err, wsaerr = 0, n_try = BIND_TRIES;

    while ((err = bind(s, addr, addrlen)) != 0 &&
           (wsaerr = WSAGetLastError()) == WSAEADDRINUSE && n_try-- >= 0) {
        trace("address in use, waiting ...\n");
        delay(1000 * BIND_SLEEP);
    }
    ok(err == 0, "failed to bind: %d\n", wsaerr);
}

static void test_set_getsockopt(void)
{
    SOCKET s;
    int i, err;
    int timeout;
    LINGER lingval;
    int size;
    int bNodelay = 1;

    size = sizeof(bNodelay);
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ok(s != INVALID_SOCKET, "socket() failed error: %d\n", WSAGetLastError());
    if (s == INVALID_SOCKET)
        return;
    err = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &bNodelay, size);
    ok(!err, "failed! \n");
    if (!err)
        err =
            getsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &bNodelay, &size);
    ok(!err, "get/setsockopt failed!\n");

    /* In order to pass these tests, we need to fix some bugs in setsockopt/
     * setsockopt */
    /* SO_RCVTIMEO */
    timeout = SOCKTIMEOUT1;
    size = sizeof(timeout);
    err =
        setsockopt(s, (int) SOL_SOCKET, (int) SO_SNDTIMEO, (char *) &timeout,
                   size);
    ok(!err, "setsockopt failed!\n");
    if (!err)
        err =
            getsockopt(s, (int) SOL_SOCKET, (int) SO_SNDTIMEO,
                       (char *) &timeout, &size);
    ok(!err, "getsockopt(SO_RCVTIMEO) failed!\n");

    ok(timeout == SOCKTIMEOUT1,
       "getsockopt(SO_RCVTIMEO) returned wrong value %d\n", timeout);

    /* SO_SNDTIMEO */
    timeout = SOCKTIMEOUT2;
    size = sizeof(timeout);
    err =
        setsockopt(s, (int) SOL_SOCKET, (int) SO_SNDTIMEO,
                   (char *) &timeout, size);
    if (!err)
        err =
            getsockopt(s, (int) SOL_SOCKET, (int) SO_SNDTIMEO,
                       (char *) &timeout, &size);
    ok(!err, "get/setsockopt(SO_SNDTIMEO) failed error: %d\n",
       WSAGetLastError());
    ok(timeout == SOCKTIMEOUT2,
       "getsockopt(SO_SNDTIMEO) returned wrong value %d\n", timeout);
    /* SO_LINGER */
    for (i = 0; i < sizeof(linger_testvals) / sizeof(LINGER); i++) {
        size = sizeof(lingval);
        lingval = linger_testvals[i];
        err =
            setsockopt(s, (int) SOL_SOCKET, (int) SO_LINGER,
                       (char *) &lingval, size);
        if (!err)
            err =
                getsockopt(s, (int) SOL_SOCKET, (int) SO_LINGER,
                           (char *) &lingval, &size);
        ok(!err, "get/setsockopt(SO_LINGER) failed error: %d\n",
           WSAGetLastError());
        ok(!lingval.l_onoff == !linger_testvals[i].l_onoff
           && (lingval.l_linger == linger_testvals[i].l_linger
               || (!lingval.l_linger && !linger_testvals[i].l_onoff))
           , "getsockopt(SO_LINGER #%d) returned wrong value %d,%d not %d,%d\n",
           i, lingval.l_onoff, lingval.l_linger, linger_testvals[i].l_onoff,
           linger_testvals[i].l_linger);
    }
    ok(!closesocket(s), "closesocket failed! \n");;
}

static void test_so_reuseaddr(void)
{
    struct sockaddr_in saddr;
    SOCKET s1, s2;
    unsigned int rc, reuse;
    int size;

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9375);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    s1 = socket(AF_INET, SOCK_STREAM, 0);
    ok(s1 != INVALID_SOCKET, "socket() failed error: %d\n", WSAGetLastError());
    rc = bind(s1, (struct sockaddr *) &saddr, sizeof(saddr));
    ok(rc != SOCKET_ERROR, "bind(s1) failed error: %d\n", WSAGetLastError());

    s2 = socket(AF_INET, SOCK_STREAM, 0);
    ok(s2 != INVALID_SOCKET, "socket() failed error: %d\n", WSAGetLastError());

    reuse = 0x1234;
    size = sizeof(reuse);
    rc = getsockopt(s2, (int) SOL_SOCKET, (int) SO_REUSEADDR, (char *) &reuse,
                    &size);
    ok(rc == 0
       && reuse == 0,
       "wrong result in getsockopt(SO_REUSEADDR): rc=%d reuse=%d\n", rc, reuse);

    rc = bind(s2, (struct sockaddr *) &saddr, sizeof(saddr));
    ok(rc == SOCKET_ERROR, "bind() succeeded\n");

    reuse = 1;
    rc = setsockopt(s2, (int) SOL_SOCKET, (int) SO_REUSEADDR, (char *) &reuse,
                    sizeof(reuse));
    ok(rc == 0, "setsockopt() failed error: %d\n", WSAGetLastError());

    todo_wine {
        rc = bind(s2, (struct sockaddr *) &saddr, sizeof(saddr));
        ok(rc == 0, "bind() failed error: %d\n", WSAGetLastError());
    }

    ok(!closesocket(s2), "closesocket failed! \n");;
}

static void test_UDP(void)
{
    /* This function tests UDP sendto() and recvfrom(). UDP is unreliable, so it is
     *        possible that this test fails due to dropped packets. */

    /* peer 0 receives data from all other peers */
    struct sock_info peer[NUM_UDP_PEERS];
    char buf[16];
    int ss, i, n_recv, n_sent;

    for (i = NUM_UDP_PEERS - 1; i >= 0; i--) {
        ok((peer[i].s =
            socket(AF_INET, SOCK_DGRAM, 0)) != INVALID_SOCKET,
           "UDP: socket failed\n");

        peer[i].addr.sin_family = AF_INET;
        peer[i].addr.sin_addr.s_addr = inet_addr(SERVERIP);
        if (i == 0) {
            peer[i].addr.sin_port = htons(SERVERPORT);
        } else {
            peer[i].addr.sin_port = htons(0);
        }

        do_bind(peer[i].s, (struct sockaddr *) &peer[i].addr,
                sizeof(peer[i].addr));

        /* test getsockname() to get peer's port */
        ss = sizeof(peer[i].addr);
        ok(getsockname(peer[i].s, (struct sockaddr *) &peer[i].addr, &ss) !=
           SOCKET_ERROR, "UDP: could not getsockname()\n");
        ok(peer[i].addr.sin_port != htons(0),
           "UDP: bind() did not associate port\n");
    }

    /* test getsockname() */
    ok(peer[0].addr.sin_port == htons(SERVERPORT),
       "UDP: getsockname returned incorrect peer port\n");

    for (i = 1; i < NUM_UDP_PEERS; i++) {
        /* send client's ip */
        memcpy(buf, &peer[i].addr.sin_port, sizeof(peer[i].addr.sin_port));
        n_sent =
            sendto(peer[i].s, buf, sizeof(buf), 0,
                   (struct sockaddr *) &peer[0].addr, sizeof(peer[0].addr));
        ok(n_sent == sizeof(buf),
           "UDP: sendto() sent wrong amount of data or socket error: %d\n",
           n_sent);
    }

    for (i = 1; i < NUM_UDP_PEERS; i++) {
        n_recv =
            recvfrom(peer[0].s, buf, sizeof(buf), 0,
                     (struct sockaddr *) &peer[0].peer, &ss);
        ok(n_recv == sizeof(buf),
           "UDP: recvfrom() received wrong amount of data or socket error: %d\n",
           n_recv);
        ok(memcmp(&peer[0].peer.sin_port, buf, sizeof(peer[0].addr.sin_port)) ==
           0, "UDP: port numbers do not match\n");
    }
}

/* ws_servent_32_to_16 doesn't seem to perform correctly*/
static void WINAPI test_getservbyname()
{
    struct {
        char FAR *name;
		char FAR *aliases;
        int port;
		char FAR *proto;
    } FAR serv[2] = { {
    "domain", "", 53, "udp"}, {
    "telnet", "", 23, "tcp"}};

    int i, j;
    struct servent FAR *pserv[2];

    /* ensure that necessary buffer resizes are completed */
    for (j = 0; j < 2; j++) {
        pserv[j] = getservbyname(serv[j].name, serv[j].proto);
    }

    for (i = 0; i < NUM_QUERIES / 2; i++) {
        for (j = 0; j < 2; j++) {
            pserv[j] = getservbyname(serv[j].name, serv[j].proto);

            ok(pserv[j] != NULL,
               "getservbyname could not retrieve information for %s: %d\n",
               serv[j].name, WSAGetLastError());
            ok(pserv[j]->s_port == htons(serv[j].port),
               "getservbyname returned the wrong port for %s: %d\n",
               serv[j].name, ntohs(pserv[j]->s_port));
            ok(!strcmp32(pserv[j]->s_proto, serv[j].proto),
               "getservbyname returned the wrong protocol for %s: %s\n",
               serv[j].name, pserv[j]->s_proto);
            ok(!strcmp32(pserv[j]->s_name, serv[j].name),
               "getservbyname returned the wrong name for %s: %s\n",
               serv[j].name, pserv[j]->s_name);
        }

        ok(pserv[0] == pserv[1],
           "getservbyname: winsock resized servent buffer when not necessary\n");
    }
}

static void test_listen(void)
{
    int ret;
    SOCKET server_socket = INVALID_SOCKET;
    struct sockaddr_in address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        trace("error creating server socket: %d\n", WSAGetLastError());
        goto done;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    ret = bind(server_socket, (struct sockaddr *) &address, sizeof(address));
    ok(!ret, "bind failed! \n");
    if (ret) {
        trace("error binding server socket: %d\n", WSAGetLastError());
        goto done;
    }

    ret = listen(server_socket, 1);
    ok(!ret, "listen failed! \n");
    if (ret) {
        trace("error making server socket listen: %d\n", WSAGetLastError());
        goto done;
    }
  done:
    if (server_socket != INVALID_SOCKET)
        ok(!closesocket(server_socket), "closesocket failed! \n");
}

static void test_getsockname(void)
{
    WSADATA wsa;
    int sock;
    struct sockaddr_in sa_set, sa_get;
    int sa_set_len = sizeof(struct sockaddr_in);
    int sa_get_len = sa_set_len;
    static const unsigned char null_padding[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    if (WSAStartup(0x0101, &wsa)) {
        trace("Winsock failed: %d. Aborting test\n", WSAGetLastError());
        return;
    }

    memset(&sa_set, 0, sa_set_len);

    sa_set.sin_family = AF_INET;
    sa_set.sin_port = htons(0);
    sa_set.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP)) < 0) {
        trace("Creating the socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    memcpy(&sa_get, &sa_set, sizeof(sa_set));
    if (getsockname(sock, (struct sockaddr *) &sa_get, &sa_get_len) == 0)
        ok(0, "getsockname on unbound socket should fail\n");
    else {
        ok(WSAGetLastError() == WSAEINVAL, "getsockname on unbound socket "
           "failed with %d, expected %d\n", WSAGetLastError(), WSAEINVAL);
        ok(memcmp(&sa_get, &sa_set, sizeof(sa_get)) == 0,
           "failed getsockname modified sockaddr when it shouldn't\n");
    }

    if (bind(sock, (struct sockaddr *) &sa_set, sa_set_len) < 0) {
        trace("Failed to bind socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return;
    }

    if (getsockname(sock, (struct sockaddr *) &sa_get, &sa_get_len) != 0) {
        trace("Failed to call getsockname: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return;
    }

    ok(memcmp(sa_get.sin_zero, null_padding, 8) == 0,
       "getsockname did not zero the sockaddr_in structure\n");

    closesocket(sock);
    WSACleanup();
}

static void test_dns(void)
{
    struct hostent FAR *h;

    h = gethostbyname("");
    ok(h != NULL, "gethostbyname(\"\") failed with %d\n", h_errno);
}

static void test_inet_addr(void)
{
    u_long addr;

    addr = inet_addr(NULL);
    ok(addr == INADDR_NONE, "inet_addr succeeded unexpectedly\n");
}

static void test_send(void)
{
    int sock;
    SOCKET src = INVALID_SOCKET;
    SOCKET dst = INVALID_SOCKET;
    const int buflen = 1024 * 1024;
    char *buffer = NULL;
    int ret;

    if (tcp_socketpair16(&src, &dst) != 0) {
        ok(0, "creating socket pair failed, skipping test\n");
        return;
    }

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP)) < 0) {
        trace("Creating the socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    buffer = (char *) GlobalAlloc(0, buflen);
    if (buffer == NULL) {
        ok(0, "HeapAlloc failed, error %d\n", GetLastError());
        goto end;
    }

    ret = send(src, buffer, buflen, 0);
    if (ret >= 0)
        ok(ret == buflen,
           "send should have sent %d bytes, but it only sent %d\n", buflen,
           ret);
    else
        ok(0, "send failed, error %d\n", WSAGetLastError());

  end:
    if (src != INVALID_SOCKET)
        closesocket(src);
    if (dst != INVALID_SOCKET)
        closesocket(dst);
    GlobalFree(buffer);
}

START_TEST(socket16)
{
    test_set_getsockopt();
    test_so_reuseaddr();

    test_UDP();
	test_getservbyname();

    test_listen();
    test_getsockname();
    test_inet_addr();
    test_dns();

    test_send();
}
