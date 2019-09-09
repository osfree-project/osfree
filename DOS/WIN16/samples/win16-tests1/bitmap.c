/*
 * Win16 Unit test suite for bitmap functions.
 *
 * Copyright 2007 Jennfier Lai, Google
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
#include <windows.h>
#include "test.h"

/*
static void test_CreateBitmapIndirect(void)
{
    BITMAP bm, bm2;
    HBITMAP hbmNew;

    bm.bmType = 0;
    bm.bmWidth = 15;
    bm.bmHeight = 25;
    bm.bmPlanes = 1;
    bm.bmBitsPixel = 24;

    hbmNew = CreateBitmapIndirect(&bm); //This is failing in Windows(not in wine) 
    ok(hbmNew != NULL, "CreateBitMapIndirect() failed!\n");
    memset(&bm2, 0, sizeof(bm2));
    ok(GetObject(hbmNew, sizeof(bm2), &bm2) == sizeof(bm2),
       "GetObject(hbmNew) failed!\n");
    ok(bm2.bmWidth == 15, "(CreateBitmapdirect) Width:%d is wrong!\n",
       bm2.bmWidth);
    ok(bm2.bmHeight == 25, "(CreateBitmapdirect) Height:%d is wrong!\n",
       bm2.bmHeight);
    ok(bm2.bmPlanes == 1, "(CreateBitmapdirect) Planes:%d is wrong!\n",
       bm2.bmPlanes);
    ok(bm2.bmBitsPixel == 24, "(CreateBitmapdirect) BitsPixel:%d is wrong!\n",
       bm2.bmBitsPixel);
    ok(bm2.bmBits == NULL, "(CreateBitmapdirect) Bits is wrong!\n");
    ok(DeleteObject(hbmNew), "DeleteObject(hbmNew) failed\n");
}
*/

static void test_CreateBitmap(void)
{
    BITMAP bm2;
    HBITMAP hbmOld;

    hbmOld = CreateBitmap(20, 30, 1, 1, NULL);
    ok(hbmOld != NULL, "CreateBitMap() failed!\n");
    memset(&bm2, 0, sizeof(bm2));
    ok(GetObject(hbmOld, sizeof(bm2), &bm2) == sizeof(bm2),
       "GetObject(hbmOld) failed!\n");
    ok(bm2.bmWidth == 20, "Width is wrong!\n");
    ok(bm2.bmHeight == 30, "Height is wrong!\n");
    ok(bm2.bmType == 0, "Type is wrong!\n");
    ok(bm2.bmPlanes == 1, "Type is wrong!\n");
    ok(bm2.bmBitsPixel == 1, "BitsPixel is wrong!\n");
    ok(bm2.bmBits == NULL, "Bits is wrong!\n");
    ok(DeleteObject(hbmOld), "DeleteObject(hbmOld) failed\n");


}

static void test_CreateCompatibleBitmap(void)
{
    HBITMAP hbmOld;
    BITMAP bm2;
    HDC hDC;

    hDC = GetDC(0);
    ok(hDC != 0, "GetDC() failed!\n");

    hbmOld = CreateCompatibleBitmap(hDC, 100, 300);
    ok(hbmOld != NULL, "CreateCompatibleBitmap() failed!\n");
    memset(&bm2, 0, sizeof(bm2));
    ok(GetObject(hbmOld, sizeof(bm2), &bm2) == sizeof(bm2),
       "GetObject(hbmOld) failed!\n");
    ok(bm2.bmWidth == 100, "(CreateCompatibleBitmap) Width:%d is wrong!\n",
       bm2.bmWidth);
    ok(bm2.bmHeight == 300, "(CreateCompatibleBitmap) Height:%d is wrong!\n",
       bm2.bmHeight);
    ok(bm2.bmType == 0, "Type is wrong!\n");
    ok(bm2.bmBits == NULL, "Bits is wrong!\n");

    ok(DeleteObject(hbmOld), "DeleteObject(hbmdOld) failed\n");
}

static void test_CreateDiscardableBitmap(void)
{
    HBITMAP hbmNew;
    BITMAP bm2;
    HDC hDC;

    hDC = GetDC(0);
    ok(hDC != 0, "GetDC() failed!\n");

    hbmNew = CreateDiscardableBitmap(hDC, 200, 400);
    ok(hbmNew != NULL, "CreateDiscardableBitmap() failed!\n");
    memset(&bm2, 0, sizeof(bm2));
    ok(GetObject(hbmNew, sizeof(bm2), &bm2) == sizeof(bm2),
       "GetObject(hbmNew) failed!\n");
    ok(bm2.bmWidth == 200, "(CreateDiscardableBitmap) Width:%d is wrong!\n",
       bm2.bmWidth);
    ok(bm2.bmHeight == 400, "(CreateDiscardableBitmap) Height:%d is wrong!\n",
       bm2.bmHeight);
    ok(bm2.bmType == 0, "Type is wrong!\n");
    ok(bm2.bmBits == NULL, "Bits is wrong!\n");

    ok(DeleteObject(hbmNew), "DeleteObject(hbmNew) failed\n");
}

static void test_CreateDIBitmap(void)
{
    HBITMAP hbmNew, hbmOld;
    BITMAP bm2;
    HDC hDC;
    BITMAPINFOHEADER bmInfo;
    int depth, bytes, ret;
    BYTE *buf;

    bmInfo.biSize = sizeof(bmInfo);
    bmInfo.biWidth = 25;
    bmInfo.biHeight = 50;
    bmInfo.biPlanes = 1;
    bmInfo.biBitCount = 16;
    bmInfo.biCompression = BI_RGB;
    bmInfo.biSizeImage = 0;
    bmInfo.biClrImportant = 0;

    hDC = GetDC(0);
    ok(hDC != 0, "GetDC() failed!\n");
    depth = GetDeviceCaps(hDC, BITSPIXEL);
    ok(depth > 0, "GetDeviceCaps failed!\n");

    hbmNew = CreateDIBitmap(hDC, &bmInfo, 0, NULL, NULL, 0);
    ok(hbmNew != NULL, "CreateDIBitmap() failed!\n");
    memset(&bm2, 0, sizeof(bm2));
    ok(GetObject(hbmNew, sizeof(bm2), &bm2) == sizeof(bm2),
       "GetObject() failed!\n");
    ok(bm2.bmWidth == 25, "(CreateDIBitmap) Width:%d is wrong!\n", bm2.bmWidth);
    ok(bm2.bmHeight == 50, "(CreateDIBitmap) Height:%d is wrong!\n",
       bm2.bmHeight);
    ok(bm2.bmPlanes == 1, "(CreateDIBitmap) Planes:%d is wrong!\n",
       bm2.bmPlanes);
    ok(bm2.bmBitsPixel == depth, "BitsPixel: %d != %d.\n", bm2.bmBitsPixel,
       depth);
    ok(bm2.bmBits == 0, "bmBits should be 0 for DI bitmaps.\n");

    /*      GetBitmapBits, SetBitmapBits */
    bytes = GetBitmapBits(hbmNew, 0, NULL);
    ok(bytes == bm2.bmWidthBytes * bm2.bmHeight,
       "GetBitmapBits returns wrong number of bytes!\n");

    buf = malloc(bytes + 4096);
    memset(buf, 0xAA, bytes + 4096);
    bytes = GetBitmapBits(hbmNew, bytes, buf);
    ok(bytes == bm2.bmWidthBytes * bm2.bmHeight,
       "GetBitmapBits returns wrong number of bytes!\n");

    bytes = SetBitmapBits(hbmNew, bytes, buf);
    ok(bytes == bm2.bmWidthBytes * bm2.bmHeight,
       "GetBitmapBits returns wrong number of bytes!\n");


    /*  GetObject */
    memset(&bm2, 0, sizeof(bm2));
    ret = GetObject(hbmNew, sizeof(bm2) * 2, &bm2);
    ok(ret == sizeof(bm2), "wrong size %d\n", ret);

    /* SelectObject */
    hDC = CreateCompatibleDC(0);
    ok(hDC != NULL, "CreateCompatibleDC failed!\n");
    hbmOld = SelectObject(hDC, hbmNew);
    ok(hbmOld != NULL, "SelectObject() failed!\n");
    ok(DeleteDC(hDC), "DeleteDC failed!\n");

    ok(DeleteObject(hbmOld), "DeleteObject(hbmNew) failed\n");
    ok(DeleteObject(hbmNew), "DeleteObject(hbmNew) failed\n");
}


START_TEST(bitmap)
{
    /* CreateBitmapIndirect fails for some reasons */
    //test_CreateBitmapIndirect();  
    test_CreateBitmap();
    test_CreateCompatibleBitmap();
    test_CreateDiscardableBitmap();
    test_CreateDIBitmap();
}
