/*
 * Win16 Unit test suite for window message handling.
 *
 * Copyright 2007 Jennfier Lai, Google
 * Modified from /wine-git/dlls/user32/tests/msg.c written by
 * Ove Kaaven, Copyright 1999
 * Dimitrie O. Paun, Copyright 2003
 * Dmitry Timoshkov, Copyright 2004 2005 
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
#include <dos.h>
#include <assert.h>
#include "test.h"

#define HEAP_CREATE_ENABLE_EXECUTE 0x00040000

typedef enum {
    sent = 0x1,
    posted = 0x2,
    parent = 0x4,
    wparam = 0x8,
    lparam = 0x10,
    defwinproc = 0x20,
    beginpaint = 0x40,
    optional = 0x80,
    hook = 0x100,
    winevent_hook = 0x200
} msg_flags_t;

struct message {
    UINT message;               /* the WM_* code */
    msg_flags_t flags;          /* message props */
    WPARAM wParam;              /* expected value of wParam */
    LPARAM lParam;              /* expected value of lParam */
};

static int sequence_cnt, sequence_size;
static struct message *sequence;

static void add_message(const struct message *msg)
{
    if (!sequence) {
        sequence_size = 10;
        sequence =
            (struct message *) GlobalAlloc(GMEM_DISCARDABLE,
                                           sequence_size *
                                           sizeof(struct message));
    }
    if (sequence_cnt == sequence_size) {
        sequence_size *= 2;
        sequence =
            (struct message *) GlobalReAlloc((char NEAR *) sequence,
                                             sequence_size *
                                             sizeof(struct message)
                                             , GMEM_DISCARDABLE);
    }
    assert(sequence);

    sequence[sequence_cnt].message = msg->message;
    sequence[sequence_cnt].flags = msg->flags;
    sequence[sequence_cnt].wParam = msg->wParam;
    sequence[sequence_cnt].lParam = msg->lParam;
    sequence_cnt++;
}

/* try to make sure pending X events have been processed before continuing */
static void flush_sequence(void)
{
    GlobalFree((char NEAR *) sequence);
    sequence = 0;
    sequence_cnt = sequence_size = 0;
}

#define ok_sequence( exp, contx, todo) \
		        ok_sequence_( (exp), (contx), (todo), __FILE__, __LINE__)


static void ok_sequence_(const struct message *expected, const char *context,
                         int todo, const char *file, int line)
{
    static const struct message end_of_sequence = { 0, 0, 0, 0 };
    const struct message *actual;
    int failcount = 0;

    add_message(&end_of_sequence);

    actual = sequence;

    while (expected->message && actual->message) {
        /* trace_(file, line) ("expected %04x - actual %04x\n", expected->message,
           actual->message); */

        if (expected->message == actual->message) {
            if (expected->flags & wparam) {
                if (expected->wParam != actual->wParam && todo) {
                    todo_wine {
                        failcount++;
                        ok_(file, line) (FALSE,
                                         "%s: in msg 0x%04x expecting wParam 0x%lx got 0x%lx\n",
                                         context, expected->message,
                                         expected->wParam, actual->wParam);
                    }
                } else
                    ok_(file, line) (expected->wParam == actual->wParam,
                                     "%s: in msg 0x%04x expecting wParam 0x%lx got 0x%lx\n",
                                     context, expected->message,
                                     expected->wParam, actual->wParam);
            }
            if (expected->flags & lparam) {
                if (expected->lParam != actual->lParam && todo) {
                    todo_wine {
                        failcount++;
                        ok_(file, line) (FALSE,
                                         "%s: in msg 0x%04x expecting lParam 0x%lx got 0x%lx\n",
                                         context, expected->message,
                                         expected->lParam, actual->lParam);
                    }
                } else
                    ok_(file, line) (expected->lParam == actual->lParam,
                                     "%s: in msg 0x%04x expecting lParam 0x%lx got 0x%lx\n",
                                     context, expected->message,
                                     expected->lParam, actual->lParam);
            }
            if ((expected->flags & defwinproc) != (actual->flags & defwinproc)
                && todo) {
                todo_wine {
                    failcount++;
                    ok_(file, line) (FALSE,
                                     "%s: the msg 0x%04x should %shave been sent by DefWindowProc\n",
                                     context, expected->message,
                                     (expected->
                                      flags & defwinproc) ? "" : "NOT ");
                }
            } else
                ok_(file,
                    line) ((expected->flags & defwinproc) ==
                           (actual->flags & defwinproc),
                           "%s: the msg 0x%04x should %shave been sent by DefWindowProc\n",
                           context, expected->message,
                           (expected->flags & defwinproc) ? "" : "NOT ");
            ok_(file,
                line) ((expected->flags & beginpaint) ==
                       (actual->flags & beginpaint),
                       "%s: the msg 0x%04x should %shave been sent by BeginPaint\n",
                       context, expected->message,
                       (expected->flags & beginpaint) ? "" : "NOT ");
            ok_(file,
                line) ((expected->flags & (sent | posted)) ==
                       (actual->flags & (sent | posted)),
                       "%s: the msg 0x%04x should have been %s\n", context,
                       expected->message,
                       (expected->flags & posted) ? "posted" : "sent");
            ok_(file,
                line) ((expected->flags & parent) == (actual->flags & parent),
                       "%s: the msg 0x%04x was expected in %s\n", context,
                       expected->message,
                       (expected->flags & parent) ? "parent" : "child");
            ok_(file, line) ((expected->flags & hook) == (actual->flags & hook),
                             "%s: the msg 0x%04x should have been sent by a hook\n",
                             context, expected->message);
            ok_(file,
                line) ((expected->flags & winevent_hook) ==
                       (actual->flags & winevent_hook),
                       "%s: the msg 0x%04x should have been sent by a winevent hook\n",
                       context, expected->message);
            expected++;
            actual++;
        }
        /* silently drop winevent messages if there is no support for them */
        else if ((expected->flags & optional)
                 || (expected->flags & winevent_hook))
            expected++;
        else if (todo) {
            failcount++;
            todo_wine {
                ok_(file, line) (FALSE,
                                 "%s: the msg 0x%04x was expected, but got msg 0x%04x instead\n",
                                 context, expected->message, actual->message);
            }
            flush_sequence();
            return;
        } else {
            ok_(file, line) (FALSE,
                             "%s: the msg 0x%04x was expected, but got msg 0x%04x instead\n",
                             context, expected->message, actual->message);
            expected++;
            actual++;
        }
    }

    /* skip all optional trailing messages */
    while (expected->message && ((expected->flags & optional) ||
                                 (expected->flags & winevent_hook)
           ))
        expected++;

    if (todo) {
        todo_wine {
            if (expected->message || actual->message) {
                failcount++;
                ok_(file, line) (FALSE,
                                 "%s: the msg sequence is not complete: expected %04x - actual %04x\n",
                                 context, expected->message, actual->message);
            }
        }
    }
    if (todo && !failcount)     /* succeeded yet marked todo */
        todo_wine {
        ok_(file, line) (TRUE, "%s: marked \"todo_wine\" but succeeds\n",
                         context);
        }

    flush_sequence();
}

/****************** static message test *************************/
#define WM_CTLCOLORSTATIC  0x0019
static const struct message WmSetFontStaticSeq[] = {
    {WM_SETFONT, sent},
    {WM_PAINT, sent | defwinproc},
    {WM_ERASEBKGND, sent | defwinproc | optional},
    {WM_CTLCOLORSTATIC, sent | defwinproc},
    {0}
};

static WNDPROC old_static_proc;

static LRESULT CALLBACK static_hook_proc(HWND hwnd, UINT message, WPARAM wParam,
                                         LPARAM lParam)
{
    static long defwndproc_counter = 0;
    LRESULT ret;
    struct message msg;

/*    trace("static: %p, %04x, %08lx, %08lx\n", hwnd, message, wParam, lParam); */

    msg.message = message;
    msg.flags = sent | wparam | lparam;
    if (defwndproc_counter)
        msg.flags |= defwinproc;
    msg.wParam = wParam;
    msg.lParam = lParam;
    add_message(&msg);

    defwndproc_counter++;
    ret = CallWindowProc(old_static_proc, hwnd, message, wParam, lParam);
    defwndproc_counter--;

    return ret;
}


static void subclass_static(void)
{
    WNDCLASS cls;

    if (!GetClassInfo(0, "static", &cls))
        assert(0);

    old_static_proc = cls.lpfnWndProc;

    cls.hInstance = GetModuleHandle(0);
    cls.lpfnWndProc = static_hook_proc;
    cls.lpszClassName = "BUTTON";
    UnregisterClass(cls.lpszClassName, cls.hInstance);
    if (!RegisterClass(&cls))
        assert(0);
}

static void test_static_messages(void)
{
    static const struct {
        DWORD style;
        DWORD dlg_code;
        const struct message *setfont;
    } static_ctrl[] = {
        {
        SS_LEFT, DLGC_STATIC, WmSetFontStaticSeq}
    };
    unsigned int i;
    HWND hwnd;
    DWORD dlg_code;

    subclass_static();

    for (i = 0; i < sizeof(static_ctrl) / sizeof(static_ctrl[0]); i++) {
        hwnd =
            CreateWindow("BUTTON", "test",
                         static_ctrl[i].style | WS_POPUP, 0, 0, 50, 14, 0, 0,
                         0, NULL);
        ok(hwnd != 0, "Failed to create static window\n");

        dlg_code = SendMessage(hwnd, WM_GETDLGCODE, 0, 0);
        ok(dlg_code == static_ctrl[i].dlg_code, "%u: wrong dlg_code %08x\n", i,
           dlg_code);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
        SetFocus(0);
        flush_sequence();

/*        trace("static style %08x\n", static_ctrl[i].style);	*/
        SendMessage(hwnd, WM_SETFONT, (WPARAM) GetStockObject(SYSTEM_FONT),
                    TRUE);
        ok_sequence(static_ctrl[i].setfont, "WM_SETFONT on a static", FALSE);

        DestroyWindow(hwnd);
    }
}

static void test_post_message(void)
{
    MSG msg;
    BOOL ret;

    /* test using PostMessage and PeekMessage */
    PostMessage(HWND_BROADCAST, WM_KEYUP, 0xde, 0xad);
    ret = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    ok(ret, "PeekMessage failed with error %d\n", GetLastError());
    ok(msg.message == WM_KEYUP, "Received message 0x%04x instead of WM_KEYUP\n",
       msg.message);
    ok(msg.wParam == 0xde, "wParam was 0x%x instead of 0xbf\n", msg.wParam);
    ok(msg.lParam == 0xad, "wParam was 0x%x instead of 0xbf\n", msg.lParam);

    /* test using WM_KEYDOWN */
    memset(&msg, 0, sizeof(msg));
    PostMessage(HWND_BROADCAST, WM_KEYDOWN, 0xbe, 0xef);
    ret = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    ok(ret, "PeekMessage failed with error %d\n", GetLastError());
    ok(msg.message == WM_KEYDOWN,
       "Received message 0x%04x instead of WM_KEYDOWN\n", msg.message);
    ok(msg.wParam == 0xbe, "wParam was 0x%x instead of 0xbf\n", msg.wParam);
    ok(msg.lParam == 0xef, "wParam was 0x%x instead of 0xbf\n", msg.lParam);

    /* test using WM_COMMAND */
    memset(&msg, 0, sizeof(msg));
    PostMessage(HWND_BROADCAST, WM_COMMAND, 0xab, 0xcd);
    ret = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    ok(ret, "PeekMessage failed with error %d\n", GetLastError());
    ok(msg.message == WM_COMMAND,
       "Received message 0x%04x instead of WM_COMMAND\n", msg.message);
    ok(msg.wParam == 0xab, "wParam was 0x%x instead of 0xbf\n", msg.wParam);
    ok(msg.lParam == 0xcd, "wParam was 0x%x instead of 0xbf\n", msg.lParam);

    /* test using WM_SYSCOMMAND */
    memset(&msg, 0, sizeof(msg));
    PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, 0xcb, 0xda);
    ret = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    ok(ret, "PeekMessage failed with error %d\n", GetLastError());
    ok(msg.message == WM_SYSCOMMAND,
       "Received message 0x%04x instead of WM_SYSCOMMAND\n", msg.message);
    ok(msg.wParam == 0xcb, "wParam was 0x%x instead of 0xbf\n", msg.wParam);
    ok(msg.lParam == 0xda, "wParam was 0x%x instead of 0xbf\n", msg.lParam);

    /* using GetMessage instead of PeekMessage to retrieve */
    memset(&msg, 0, sizeof(msg));
    PostMessage(HWND_BROADCAST, WM_KEYUP, 0xde, 0xad);
    ret = GetMessage(&msg, NULL, 0, 0);
    ok(ret, "GetMessage failed with error %d\n", GetLastError());
    ok(msg.message == WM_KEYUP, "Received message 0x%04x instead of WM_KEYUP\n",
       msg.message);
    ok(msg.wParam == 0xde, "wParam was 0x%x instead of 0xbf\n", msg.wParam);
    ok(msg.lParam == 0xad, "wParam was 0x%x instead of 0xbf\n", msg.lParam);

}

static int count = 0;
static VOID CALLBACK callback_count(HWND hwnd,
                                    UINT uMsg, UINT idEvent, DWORD dwTime)
{
    count++;
}

static void test_timers_no_wnd(void)
{
    UINT id, id2;
    MSG msg;

    count = 0;
    id = SetTimer(NULL, 0, 100, callback_count);
    ok(id != 0, "did not get id from SetTimer.\n");
    id2 = SetTimer(NULL, id, 200, callback_count);
    ok(id2 == id,
       "did not get same id from SetTimer when replacing (%li expected %li).\n",
       id2, id);
    delay(150);
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        DispatchMessage(&msg);
    ok(count == 0, "did not get zero count as expected (%i).\n", count);
    delay(150);
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        DispatchMessage(&msg);
    ok(count == 1, "did not get one count as expected (%i).\n", count);
    KillTimer(NULL, id);
    delay(250);
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        DispatchMessage(&msg);
    ok(count == 1, "killing replaced timer did not work (%i).\n", count);
}

static void test_wm_char(void)
{
    HWND hwnd, hwnd2;
    MSG msg;
    char buf[12];

    hwnd = CreateWindow("EDIT", NULL,
                        WS_OVERLAPPEDWINDOW, 100, 100, 200, 200, 0, 0, 0, NULL);
    hwnd2 = CreateWindow("STATIC", NULL,
                         WS_OVERLAPPEDWINDOW, 100, 100, 200, 200, 0, 0, 0,
                         NULL);
    ok(hwnd != 0, "Failed to create overlapped window\n");
    ok(hwnd2 != 0, "Failed to create overlapped window\n");

    /* posted message */
    ok(PostMessage(hwnd2, WM_CHAR, 0x16, 0), "PostMessage failed!\n");
    ok(PeekMessage(&msg, hwnd2, 0, 0, PM_REMOVE), "got message %x\n",
       msg.message);
    ok(msg.wParam == 0x16, "wrong msg");

    /* sent message - type in h, e, l, l, o to the edit window */
    memset(&msg, 0, sizeof(msg));
    SendMessage(hwnd, WM_CHAR, 'h', 1);
    SendMessage(hwnd, WM_CHAR, 'e', 1);
    SendMessage(hwnd, WM_CHAR, 'l', 1);
    SendMessage(hwnd, WM_CHAR, 'l', 1);
    SendMessage(hwnd, WM_CHAR, 'o', 1);
    ok(!PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE), "got message %x\n",
       msg.message);
    ok(GetWindowText(hwnd, buf, 12), "GetWindowText failed!\n");
    ok(GetWindowTextLength(hwnd) == 5, "GetWindowTextLength failed! \n");
    ok(lstrcmp(buf, "hello") == 0, "wrong text:%s!\n", buf);

    /* using SendMessage to retrive text written */
    memset(buf, 0, sizeof(buf));
    SendMessage(hwnd, WM_CHAR, 'w', 1);
    SendMessage(hwnd, WM_CHAR, 'o', 1);
    SendMessage(hwnd, WM_CHAR, 'r', 1);
    SendMessage(hwnd, WM_CHAR, 'l', 1);
    SendMessage(hwnd, WM_CHAR, 'd', 1);
    SendMessage(hwnd, WM_GETTEXT, 12, (long) buf);
    ok(lstrcmp(buf, "helloworld") == 0, "should get [hello] \n");
}


START_TEST(msg)
{
    test_post_message();
    test_timers_no_wnd();
    test_static_messages();
    test_wm_char();
}
