/*    
	OleSvr.c	2.4
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */
#include "windows.h"
#include "windowsx.h"
#include "ole.h"
#include "Log.h"

static OLESERVER oleserver;
static OLESERVERVTBL vtbl;

OLESTATUS
OleBlockServer(LHSERVER lhsrvr)
{
    APISTR((LF_APISTUB,"OleBlockServer: STUB server %x\n",lhsrvr));
    return OLE_OK;
}

OLESTATUS
OleRegisterServer(LPCSTR lpszClass, LPOLESERVER lpsrvr,
		LHSERVER *lplhserver, HINSTANCE hInst,
		OLE_SERVER_USE srvuse)
{
    APISTR((LF_APISTUB,"OleRegisterServer: STUB class %s inst %x use %d\n",
	lpszClass,hInst,srvuse));

    vtbl = *lpsrvr->lpvtbl;
    oleserver.lpvtbl = &vtbl;

    *lplhserver = 0x100000;

    return OLE_OK;
}

OLESTATUS
OleRegisterServerDoc(LHSERVER lphsrvr, LPCSTR lpszDocName,
			LPOLESERVERDOC lpdoc, LHSERVERDOC *lplhdoc)
{
    APISTR((LF_APISTUB,"OleRegisterServerDoc: STUB server %x doc %s lpdoc %x\n",
	lphsrvr,lpszDocName,lpdoc));
    *lplhdoc = 0x100001;
    return OLE_OK;
}

OLESTATUS
OleRenameServerDoc(LHSERVERDOC lhDoc, LPCSTR lpszDocName)
{
    APISTR((LF_APISTUB,"OleRenameServerDoc: STUB lhdoc %x name %s\n",
	lhDoc,lpszDocName));
    return OLE_OK;
}

OLESTATUS
OleRevertServerDoc(LHSERVERDOC lhdoc)
{
    APISTR((LF_APISTUB,"OleRevertServerDoc: STUB lhdoc %x\n",lhdoc));
    return OLE_OK;
}

OLESTATUS
OleRevokeObject(LPOLECLIENT lpClient)
{
    APISTR((LF_APISTUB,"OleRevokeObject: STUB lpClient %x\n",lpClient));
    return OLE_OK;
}

OLESTATUS
OleRevokeServer(LHSERVER lhServer)
{
    OLESTATUS status = 0;

    APISTR((LF_APISTUB,"OleRevokeServer: STUB lhServer %x\n",lhServer));

    if (lhServer == 0x100000)
	status = oleserver.lpvtbl->Release(&oleserver);

    APISTR((LF_APISTUB,"OleRevokeServer: STUB Release returned %x\n",status));

    return status;
}

OLESTATUS
OleRevokeServerDoc(LHSERVERDOC lhdoc)
{
    APISTR((LF_APISTUB,"OleRevokeServerDoc: STUB lhdoc %x\n",lhdoc));
    return OLE_OK;
}

OLESTATUS
OleSavedServerDoc(LHSERVERDOC lhdoc)
{
    APISTR((LF_APISTUB,"OleSavedServerDoc: STUB lhdoc %x\n",lhdoc));
    return OLE_OK;
}

OLESTATUS
OleUnblockServer(LHSERVER lhsrvr,BOOL *lpfRequest)
{
    APISTR((LF_APISTUB,"OleUnblockServer: STUB server %x, flag addr %x\n",
			lhsrvr,lpfRequest));
	*lpfRequest = FALSE;
    return OLE_OK;
}
