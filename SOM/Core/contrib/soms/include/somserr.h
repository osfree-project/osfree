/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

static long Sockets_GetError(long err)
{
	switch (err)
	{
#ifdef _WIN32
#define CASE_ERR(x)		case WSA##x: return SOMS_##x;
#else
#define CASE_ERR(x)		case x: return SOMS_##x;
#endif
	CASE_ERR(EWOULDBLOCK)
	CASE_ERR(EINPROGRESS)
	CASE_ERR(EALREADY)
	CASE_ERR(ENOTSOCK)
	CASE_ERR(EDESTADDRREQ)
	CASE_ERR(EMSGSIZE)
	CASE_ERR(EPROTOTYPE)
	CASE_ERR(ENOPROTOOPT)
	CASE_ERR(EADDRINUSE)
	CASE_ERR(EADDRNOTAVAIL)
	CASE_ERR(ENETDOWN)
	CASE_ERR(ENETUNREACH)
	CASE_ERR(ENETRESET)
	CASE_ERR(ECONNABORTED)
	CASE_ERR(ECONNRESET)
	CASE_ERR(ENOTCONN)
	CASE_ERR(EISCONN)
	CASE_ERR(ESHUTDOWN)
	CASE_ERR(EINTR)
	CASE_ERR(ECONNREFUSED)
	CASE_ERR(EHOSTDOWN)
	CASE_ERR(EHOSTUNREACH)
	}

	return SOMS_UNKNOWNERROR;
}
