/*    
	OleCli.c	2.4
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

OLESTATUS WINAPI
OleActivate(LPOLEOBJECT lpObject, UINT iVerb, BOOL fShow, BOOL fTakeFocus,
		HWND hWnd, const RECT *lpBounds)
{
    APISTR((LF_APISTUB, "OleActivate: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleClone(LPOLEOBJECT lpObject, LPOLECLIENT lpClient, LHCLIENTDOC lhClientDoc,
	LPCSTR lpszObjname, LPOLEOBJECT *lplpObject)
{
    APISTR((LF_APISTUB, "OleClone: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleClose(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleClose: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleCopyFromLink(LPOLEOBJECT lpObject, LPCSTR lpszProtocol,
		LPOLECLIENT lpClient, LHCLIENTDOC lhDoc,
		LPCSTR lpszObjname, LPOLEOBJECT *lplpObject)
{
    APISTR((LF_APISTUB, "OleCopyFromLink: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleCopyToClipboard(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleCopyToClipboard: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleCreate(LPCSTR lpszProtocol, LPOLECLIENT lpClient, LPCSTR lpszClass,
	LHCLIENTDOC lhDoc, LPCSTR lpszObjname, LPOLEOBJECT *lplpObject,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB, "OleCreate: \n"));
    return OLE_ERROR_PROTOCOL;
}

OLESTATUS WINAPI
OleCreateFromClip(LPCSTR lpszProtocol, LPOLECLIENT lpClient,
	LHCLIENTDOC lhDoc, LPCSTR lpszObjname, LPOLEOBJECT *lplpObject,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB, "OleCreateFromClip: \n"));
    return OLE_ERROR_PROTOCOL;
}

OLESTATUS WINAPI
OleCreateFromFile(LPCSTR lpszProtocol, LPOLECLIENT lpClient, LPCSTR lpszClass,
	LPCSTR lpszFile, LHCLIENTDOC lhDoc, LPCSTR lpszObjname, LPOLEOBJECT *lplpObject,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB, "OleCreateFromFile: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleCreateInvisible(LPCSTR lpszProtocol, LPOLECLIENT lpClient, LPCSTR lpszClass,
	LHCLIENTDOC lhDoc, LPCSTR lpszObjname, LPOLEOBJECT *lplpObject,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat, BOOL fActivate)
{
    APISTR((LF_APISTUB, "OleCreateInvisible: \n"));
    return OLE_ERROR_PROTOCOL;
}

OLESTATUS WINAPI
OleCreateLinkFromClip(LPCSTR lpszProtocol, LPOLECLIENT lpClient,
	LHCLIENTDOC lhDoc, LPCSTR lpszObjname, LPOLEOBJECT *lplpObject,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB, "OleCreateLinkFromClip: \n"));
    return OLE_ERROR_PROTOCOL;
}

OLESTATUS WINAPI
OleCreateLinkFromFile(LPCSTR lpszProtocol, LPOLECLIENT lpClient, LPCSTR lpszClass,
	LPCSTR lpszFile, LPCSTR lpszItem,
	LHCLIENTDOC lhDoc, LPCSTR lpszObjname, LPOLEOBJECT *lplpObject,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB, "OleCreateLinkFromFile: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleDelete(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleDelete: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleDraw(LPOLEOBJECT lpObject, HDC hDC, const RECT *lprcBounds,
	const RECT *lprcWBounds, HDC hDCFormat)
{
    APISTR((LF_APISTUB, "OleDraw: \n"));
    return OLE_ERROR_MEMORY;
}

OLECLIPFORMAT WINAPI
OleEnumFormats(LPOLEOBJECT lpObject1, OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB, "OleEnumFormats: \n"));
    return 0;
}

OLESTATUS WINAPI
OleEnumObjects(LHCLIENTDOC lhDoc, LPOLEOBJECT *lplpObject)
{
    APISTR((LF_APISTUB, "OleEnumObjects: doc %x\n",lhDoc));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleEqual(LPOLEOBJECT lpObject1, LPOLEOBJECT lpObject2)
{
    APISTR((LF_APISTUB, "OleEqual: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleGetData(LPOLEOBJECT lpObject1, OLECLIPFORMAT cfFormat, HANDLE *phData)
{
    APISTR((LF_APISTUB, "OleGetData: \n"));
    return OLE_ERROR_FORMAT;
}

OLESTATUS WINAPI
OleGetLinkUpdateOptions(LPOLEOBJECT lpObject, OLEOPT_UPDATE *lpUpdateOpt)
{
    APISTR((LF_APISTUB, "OleGetLinkUpdateOptions: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleLoadFromStream(LPOLESTREAM lpStream, LPCSTR lpszProtocol,
		LPOLECLIENT lpClient, LHCLIENTDOC lhClientDoc,
		LPCSTR lpszObjname, LPOLEOBJECT *lplpObject)
{
    APISTR((LF_APISTUB,
	"OleLoadFromStream: stream %x proto %s cli %x doc %x obj %s\n",
	lpStream,lpszProtocol,lpClient,lhClientDoc,lpszObjname));
    return OLE_ERROR_PROTOCOL;
}

OLESTATUS WINAPI
OleObjectConvert(LPOLEOBJECT lpObject, LPCSTR lpszProtocol,
		LPOLECLIENT lpClient, LHCLIENTDOC lhClientDoc,
		LPCSTR lpszObjname, LPOLEOBJECT *lplpObject)
{
    APISTR((LF_APISTUB,"OleObjectConvert: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleRelease(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleRelease: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleQueryBounds(LPOLEOBJECT lpObject, RECT *lpBounds)
{
    APISTR((LF_APISTUB, "OleQueryBounds: \n"));
    return OLE_ERROR_OBJECT;
}

DWORD
OleQueryClientVersion(void)
{
    APISTR((LF_APISTUB,"OleQueryClientVersion: \n"));
    return MAKELONG(0x1001,0);
}

OLESTATUS
OleQueryCreateFromClip(LPCSTR lpszProtocol, OLEOPT_RENDER renderopt,
			OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB,
	"OleQueryCreateFromClip: proto %s renderopts %x cf %x\n",
	lpszProtocol,renderopt,cfFormat));
    return OLE_ERROR_PROTOCOL;
}

OLESTATUS
OleQueryLinkFromClip(LPCSTR lpszProtocol, OLEOPT_RENDER renderopt,
			OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB,
	"OleQueryLinkFromClip: proto %s renderopts %x cf %x\n",
	lpszProtocol,renderopt,cfFormat));
    return OLE_ERROR_PROTOCOL;
}

OLESTATUS WINAPI
OleQueryName(LPOLEOBJECT lpObject, LPSTR lpszObject, UINT *lpwBuffSize)
{
    APISTR((LF_APISTUB, "OleQueryName: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleQueryOpen(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleQueryOpen: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleQueryReleaseError(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleQueryReleaseError: \n"));
    return OLE_ERROR_OBJECT;
}

OLE_RELEASE_METHOD WINAPI
OleQueryReleaseMethod(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleQueryReleaseMethod: \n"));
    return OLE_NONE;
}

OLESTATUS WINAPI
OleQueryReleaseStatus(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleQueryReleaseStatus: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleQuerySize(LPOLEOBJECT lpObject, LPDWORD lpdwSize)
{
    APISTR((LF_APISTUB, "OleQuerySize: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleQueryType(LPOLEOBJECT lpObject, LPLONG lpType)
{
    APISTR((LF_APISTUB, "OleQuerySize: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleReconnect(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleReconnect: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS
OleRegisterClientDoc(LPCSTR lpszClass, LPCSTR lpszDoc, LONG reserved,
			LHCLIENTDOC *lplhDoc)
{
    APISTR((LF_APISTUB,"OleRegisterClientDoc: %s %s\n",
	lpszClass,lpszDoc));
    *lplhDoc = 0x1234;
    return OLE_OK;
}

OLESTATUS
OleRename(LPOLEOBJECT lpObject, LPCSTR lpszNewname)
{
    APISTR((LF_APISTUB,"OleRename: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS
OleRenameClientDoc(LHCLIENTDOC lhClientDoc, LPCSTR lpszNewDocname)
{
    APISTR((LF_APISTUB,"OleRenameClientDoc: %x %s\n",
	lhClientDoc,lpszNewDocname));
    return OLE_OK;
}

OLESTATUS WINAPI
OleRequestData(LPOLEOBJECT lpObject, OLECLIPFORMAT cfFormat)
{
    APISTR((LF_APISTUB,"OleRequestData: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS
OleRevertClientDoc(LHCLIENTDOC lhClientDoc)
{
    APISTR((LF_APISTUB,"OleRevertClientDoc: %x\n",lhClientDoc));
    return OLE_OK;
}

OLESTATUS
OleRevokeClientDoc(LHCLIENTDOC lhClientDoc)
{
    APISTR((LF_APISTUB,"OleRevokeClientDoc: %x\n",lhClientDoc));
    return OLE_OK;
}

OLESTATUS
OleSavedClientDoc(LHCLIENTDOC lhClientDoc)
{
    APISTR((LF_APISTUB,"OleSavedClientDoc: %x\n",lhClientDoc));
    return OLE_OK;
}

OLESTATUS
OleSaveToStream(LPOLEOBJECT lpObject, LPOLESTREAM lpStream)
{
    APISTR((LF_APISTUB,"OleSaveToStream: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleSetBounds(LPOLEOBJECT lpObject, const RECT *lpBounds)
{
    APISTR((LF_APISTUB,"OleSetBounds: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleSetColorScheme(LPOLEOBJECT lpObject, const LOGPALETTE *lpPalette)
{
    APISTR((LF_APISTUB,"OleSetColorScheme: \n"));
    return OLE_ERROR_OBJECT;
}

OLESTATUS WINAPI
OleSetData(LPOLEOBJECT lpObject, OLECLIPFORMAT cfFormat, HANDLE hData)
{
    APISTR((LF_APISTUB, "OleSetData: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleSetHostNames(LPOLEOBJECT lpObject, LPCSTR lpszClient, LPCSTR lpszClientObj)
{
    APISTR((LF_APISTUB, "OleSetHostNames: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleSetLinkUpdateOptions(LPOLEOBJECT lpObject, OLEOPT_UPDATE UpdateOpt)
{
    APISTR((LF_APISTUB, "OleSetLinkUpdateOptions: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleSetTargetDevice(LPOLEOBJECT lpObject, HGLOBAL hTargetDevice)
{
    APISTR((LF_APISTUB, "OleSetTargetDevice: \n"));
    return OLE_ERROR_MEMORY;
}

OLESTATUS WINAPI
OleUpdate(LPOLEOBJECT lpObject)
{
    APISTR((LF_APISTUB, "OleUpdate: \n"));
    return OLE_ERROR_OBJECT;
}

