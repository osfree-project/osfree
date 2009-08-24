/* Win16 Unit test suite for BuildCommDCB function
 *
 * Copyright 2007 Andreas Mohr
 * Copyright 2007 Google, Jennifer Lai
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
 
/* Note:
 * WinXP is used to find out/implement checks for use with this function.
 */

#include <stdio.h>
#include <windows.h>
#include "test.h"

/* helper function for debugging
static void dump_dcb(DCB *dcb, DCB *dcb_expect)
{
    char buffer[1024], temp[256];
    char *p;

	memset(buffer, 0, sizeof(buffer));
	memset(temp, 0, sizeof(temp));
    strcpy(buffer, "is:\n");
    for (p = (char *)dcb; p < ((char *)dcb)+sizeof(DCB); p++)
    {
        sprintf(temp, "%02x ", *p);
        strcat(buffer, temp);
    }
    strcat(buffer, "\nexpected:\n");
    for (p = (char *)dcb_expect; p < ((char *)dcb_expect)+sizeof(DCB); p++)
    {
        sprintf(temp, "%02x ", *p);
        strcat(buffer, temp);
    }
    printf("%s\n", buffer);
}
*/

/* differences between Win16 and Win32:
 * Win16: COMx: prefix mandatory! odd baud rates illegal, baud rates > 19200 illegal
 */
static void test_BuildCommDCB(void)
{
    DCB dcb, dcb_expect;
    BOOL res;
	
	/* NULL parameters */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1:2400,n,8,1", NULL);
    ok(res == -1, "DCB lpdcb NULL failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */

    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB(NULL, &dcb);
    ok(res == -1, "DCB device NULL failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* invalid settings string */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("", &dcb);
    ok(res == -1, "DCB empty settings failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* incomplete string setting */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM", &dcb);
    ok(res == -1, "DCB incomplete string failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */
   
    /* only device */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1", &dcb);
    ok(res == -1, "DCB only device failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* only device:  */ 
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1:", &dcb);
    ok(res == -1, "DCB only device: failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* double colon */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1::2400,n,8,1", &dcb);
    ok(res == -1, "DCB double colon failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* only device:baudrate */
    memset(&dcb, 0xaa, sizeof(DCB));
    memset(&dcb_expect, 0x0, sizeof(DCB));
    res = BuildCommDCB("COM1:2400", &dcb);
    dcb_expect.Id = 0;
    dcb_expect.BaudRate = 2400;
    dcb_expect.fBinary = TRUE;
    dcb_expect.XonChar  = 0x11;
    dcb_expect.XoffChar = 0x13;
    dcb_expect.XonLim   = 0x0a;
    dcb_expect.XoffLim  = 0x0a;
    ok((res == 0) && (memcmp(&dcb, &dcb_expect, sizeof(dcb)) == 0), "DCB only device:baudrate failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */
    
    /* only device baudrate */
    memset(&dcb, 0xaa, sizeof(DCB));
    memset(&dcb_expect, 0x0, sizeof(DCB));
    res = BuildCommDCB("COM1 2400", &dcb);
    dcb_expect.Id = 0;
    dcb_expect.BaudRate = 2400;
    dcb_expect.fBinary = TRUE;
    dcb_expect.XonChar  = 0x11;
    dcb_expect.XoffChar = 0x13;
    dcb_expect.XonLim   = 0x0a;
    dcb_expect.XoffLim  = 0x0a;
    ok((res == 0) && (memcmp(&dcb, &dcb_expect, sizeof(dcb)) == 0), "DCB only device baudrate failed\n");
	/* dump_dcb(&dcb, &dcb_expect); */
   
    /* only  device baudrate */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB(" COM1 2400", &dcb);
    ok(res == -1, "DCB only  device baudrate failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* only device: baudrate  */
    memset(&dcb, 0xaa, sizeof(DCB));
    memset(&dcb_expect, 0x0, sizeof(DCB));
    res = BuildCommDCB("COM1: 2400", &dcb);
    dcb_expect.Id = 0;
    dcb_expect.BaudRate = 2400;
    dcb_expect.fBinary = TRUE;
    dcb_expect.XonChar  = 0x11;
    dcb_expect.XoffChar = 0x13;
    dcb_expect.XonLim   = 0x0a;
    dcb_expect.XoffLim  = 0x0a;
    ok((res == 0) && (memcmp(&dcb, &dcb_expect, sizeof(dcb)) == 0), "DCB only device: baudrate failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* only devicebaudrate, COM has to be followed by ' ' or ':'*/ 
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM12400", &dcb);
    ok(res == -1, "DCB only devicebaudrate failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* use all space fields */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1: , , , ,", &dcb);
    ok(res == -1, "DCB all space fields failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* invalid device name setting */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("CO:2400,n,8,1", &dcb);
    ok(res == -1, "DCB invalid device 2400 baud failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* underlimit device name setting */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM0:2400,n,8,1", &dcb);
    ok(res == -1, "DCB underlimit device 2400 baud failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* no device name setting */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("2400,n,8,1", &dcb);
    ok(res == -1, "DCB no device 2400 baud failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* wrong stopbit */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1:2400,n,8,0", &dcb);
    ok(res == -1, "DCB wrong stopbit failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* space #1 in parameter */
    memset(&dcb, 0xaa, sizeof(DCB));
    memset(&dcb_expect, 0x0, sizeof(DCB));
    res = BuildCommDCB("COM1:4800,n, 8,1", &dcb);
    dcb_expect.BaudRate = 4800;
    dcb_expect.Parity = 0;
    dcb_expect.ByteSize = 8;
    dcb_expect.StopBits = ONESTOPBIT;
    dcb_expect.fBinary = TRUE;
    dcb_expect.XonChar  = 0x11;
    dcb_expect.XoffChar = 0x13;
    dcb_expect.XonLim   = 0x0a;
    dcb_expect.XoffLim  = 0x0a;
    ok((res == 0) && (memcmp(&dcb, &dcb_expect, sizeof(dcb)) == 0), "DCB space #1 in parameter failed");
	/* dump_dcb(&dcb, &dcb_expect); */


    /* space #2 in parameter */
    memset(&dcb, 0xaa, sizeof(DCB));
    memset(&dcb_expect, 0x0, sizeof(DCB));
    res = BuildCommDCB("COM1:4800,n,8 ,1", &dcb);
    dcb_expect.BaudRate = 4800;
    dcb_expect.Parity = 0;
    dcb_expect.ByteSize = 8;
    dcb_expect.StopBits = ONESTOPBIT;
    dcb_expect.fBinary = TRUE;
    dcb_expect.XonChar  = 0x11;
    dcb_expect.XoffChar = 0x13;
    dcb_expect.XonLim   = 0x0a;
    dcb_expect.XoffLim  = 0x0a;
    ok((res == 0) && (memcmp(&dcb, &dcb_expect, sizeof(dcb)) == 0), "DCB space #2 in parameter failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* tab in parameter */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1:4800,n,\t8,1", &dcb);
    ok(res == -1, "DCB tab in parameter failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* invalid char in parameter */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1:4800,n,r8,1", &dcb);
    ok(res == -1, "DCB invalid char in parameter failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* trailing spaces */
    memset(&dcb, 0xaa, sizeof(DCB));
    memset(&dcb_expect, 0x0, sizeof(DCB));
    res = BuildCommDCB("COM1:4800,n,8,1    ", &dcb);
    dcb_expect.BaudRate = 4800;
    dcb_expect.Parity = 0;
    dcb_expect.ByteSize = 8;
    dcb_expect.StopBits = ONESTOPBIT;
    dcb_expect.fBinary = TRUE;
    dcb_expect.XonChar  = 0x11;
    dcb_expect.XoffChar = 0x13;
    dcb_expect.XonLim   = 0x0a;
    dcb_expect.XoffLim  = 0x0a;
    ok((res == 0) && (memcmp(&dcb, &dcb_expect, sizeof(dcb)) == 0), "DCB trailing spaces failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* trailing garbage #2 */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("COM1:4800,n,8,1 asdfadf", &dcb);
    ok(res == -1, "DCB trailing garbage #2 failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* trailing garbage #3 */
    memset(&dcb, 0xaa, sizeof(DCB));
    memset(&dcb_expect, 0x0, sizeof(DCB));
    res = BuildCommDCB("COM1:4800,n,8,1,", &dcb);
    dcb_expect.BaudRate = 4800;
    dcb_expect.Parity = 0;
    dcb_expect.ByteSize = 8;
    dcb_expect.StopBits = ONESTOPBIT;
    dcb_expect.fBinary = TRUE;
    dcb_expect.XonChar  = 0x11;
    dcb_expect.XoffChar = 0x13;
    dcb_expect.XonLim   = 0x0a;
    dcb_expect.XoffLim  = 0x0a;
    ok((res == 0) && (memcmp(&dcb, &dcb_expect, sizeof(dcb)) == 0), "DCB trailing garbage #3 failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* wrong order #1 */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("4800,8,n,1", &dcb);
    ok(res == -1, "DCB wrong order #1 failed");
	/* dump_dcb(&dcb, &dcb_expect); */

    /* wrong order #2 */
    memset(&dcb, 0xaa, sizeof(DCB));
    res = BuildCommDCB("4800,n,8,1:COM1", &dcb);
    ok(res == -1, "DCB wrong order #2 failed");
	/* dump_dcb(&dcb, &dcb_expect); */
}

START_TEST(comm16)
{
    test_BuildCommDCB();
}
