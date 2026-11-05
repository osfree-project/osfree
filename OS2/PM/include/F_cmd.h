/* F_cmd.hpp */
#ifndef FREEPM_CMD
#define FREEPM_CMD

#include "gpi_wire.h"   /* NGpi* function codes for GPI class */

/* ------------------------------------------------------------------------
 * Wire command layout:
 *   bits 31..16 — class   (F_CMD_CLASS_*)
 *   bits 15..0  — opcode  within class
 *
 *   CORE   : opcode = historical value
 *   GPI    : opcode = NGpi*        (gpi_wire.h)
 *   GORDER : opcode = OCODE_*      (pmord.h, included by client/server
 *                                   that use orders; not pulled in here)
 *   GL*    : opcode = local 16-bit index inside GL/WGL/GLUT/GLU/GLPM.
 *
 *   Convenience wrappers:
 *     F_CMD_CORE(op)
 *     F_CMD_GPI(NGpiSetColor)
 *     F_CMD_ORDER(OCODE_GLINE)
 * ------------------------------------------------------------------------ */

#define F_CMD_CLASS_MASK      0xFFFF0000UL
#define F_CMD_OPCODE_MASK     0x0000FFFFUL
#define F_CMD_MAKE(cls, op)   (((ULONG)(cls) << 16) | ((ULONG)(op) & 0xFFFFUL))
#define F_CMD_GET_CLASS(cmd)  ((ULONG)(((cmd) & F_CMD_CLASS_MASK) >> 16))
#define F_CMD_GET_OPCODE(cmd) ((ULONG)((cmd) & F_CMD_OPCODE_MASK))

#define F_CMD_CORE(op)        F_CMD_MAKE(F_CMD_CLASS_CORE,   (op))
#define F_CMD_GPI(fn)         F_CMD_MAKE(F_CMD_CLASS_GPI,    (fn))
#define F_CMD_ORDER(oc)       F_CMD_MAKE(F_CMD_CLASS_GORDER, (oc))

/* Classes */
#define F_CMD_CLASS_CORE   0x0000  /* Session / window / HPS / debug */
#define F_CMD_CLASS_GPI    0x0001  /* Direct GPI calls (opcode = NGpi*) */
#define F_CMD_CLASS_GORDER 0x0002  /* GOCA orders   (opcode = OCODE_*) */
#define F_CMD_CLASS_GL     0x0010
#define F_CMD_CLASS_WGL    0x0011
#define F_CMD_CLASS_GLUT   0x0012
#define F_CMD_CLASS_GLU    0x0013
#define F_CMD_CLASS_GLPM   0x0018

/* ---- CORE class ------------------------------------------------------- */

/* ---- CORE class: client lifecycle ------------------------------------- */

#define F_CMD_CLIENT_EXIT         F_CMD_CORE(0x0000)
#define F_CMD_CLIENT_CONNECT      F_CMD_CORE(0x0004)

/* ======================================================================== */
/*  Client lifecycle structures (shared client/server)                       */
/* ======================================================================== */

/*!
 * @struct F_CLIENTCONFIG
 * @brief Client configuration, sent by server after CONNECT/RECONNECT.
 */
typedef struct _F_CLIENTCONFIG {
    ULONG idClient;     /*!< Client identifier, stable across reconnect. */
    ULONG ulInactive;   /*!< Inactivity timeout, seconds. */
    ULONG ulGrace;      /*!< Grace timeout (DETACHED->FREE), seconds. */
    ULONG flConfig;     /*!< F_CFG_* flags. */
} F_CLIENTCONFIG, *PF_CLIENTCONFIG;

/*! @brief Body of F_CMD_CLIENT_RECONNECT (sent by client on link loss). */
typedef struct _F_CLIENT_RECONNECT {
    ULONG idClient;     /*!< idClient from previous CONNECT. */
} F_CLIENT_RECONNECT, *PF_CLIENT_RECONNECT;

/* ---- flConfig bits ---------------------------------------------------- */
#define F_CFG_WANT_LOGS        0x00000001UL  /*!< Server accepts logs from client. */
#define F_CFG_PING_REQUIRED    0x00000002UL  /*!< Client must PING within ulInactive. */

#define F_CMD_CLIENT_RECONNECT    F_CMD_CORE(0x0005)
#define F_CMD_CLIENT_PING         F_CMD_CORE(0x0006)


#define F_CMD_GET_IHAB            F_CMD_CORE(0x0001)
#define F_CMD_WINPOSTMSG          F_CMD_CORE(0x0002)
#define F_CMD_WINSENDMSG          F_CMD_CORE(0x0003)

#define F_CMD_WINCREATE_HWND      F_CMD_CORE(0x0010)
#define F_CMD_WINSET_PARENT_HWND  F_CMD_CORE(0x0011)

#define F_CMD_WINQUERY_MSG        F_CMD_CORE(0x0020)
#define F_CMD_WINGET_MSG          F_CMD_CORE(0x0021)

#define F_CMD_GET_HPS             F_CMD_CORE(0x0040)
#define F_CMD_RELEASE_HPS         F_CMD_CORE(0x0041)

/* send hwnd + 7*sizeof(in) data - hwndInsertBehind,x,y,cx,cy,fl,[z] */
#define F_CMD_WIN_SET_WND_SIZE    F_CMD_CORE(0x0060)
#define F_CMD_WIN_SET_WND_POS     F_CMD_CORE(0x0061)
#define F_CMD_WIN_GET_WND_SWP     F_CMD_CORE(0x0062)
#define F_CMD_WIN_SHOW            F_CMD_CORE(0x0063)

/* Debug / fatal — kept in CORE (no separate DB class for now) */
#define F_CMD_DB_PRINT            F_CMD_CORE(0x0064)
#define F_CMD_FATAL_COMMON        F_CMD_CORE(0x0065)

/* ---- GL family (opcode = local 16-bit index) ------------------------- */

#define F_GL_INIT_DRV                F_CMD_MAKE(F_CMD_CLASS_GL, 0x0000)
#define F_GL_INIT_HAB                F_CMD_MAKE(F_CMD_CLASS_GL, 0x0001)
#define F_GL_SET_HAB                 F_CMD_MAKE(F_CMD_CLASS_GL, 0x0002)
#define F_GL_GET_HAB                 F_CMD_MAKE(F_CMD_CLASS_GL, 0x0003)
#define F_GL_DISABLE                 F_CMD_MAKE(F_CMD_CLASS_GL, 0x0004)
#define F_GL_ENABLE                  F_CMD_MAKE(F_CMD_CLASS_GL, 0x0005)
#define F_GL_CULLFACE                F_CMD_MAKE(F_CMD_CLASS_GL, 0x0006)
#define F_GL_CLEARDEPTH              F_CMD_MAKE(F_CMD_CLASS_GL, 0x0007)
#define F_GL_CLEARCOLOR              F_CMD_MAKE(F_CMD_CLASS_GL, 0x0008)
#define F_GL_LIGHTFV                 F_CMD_MAKE(F_CMD_CLASS_GL, 0x0009)
#define F_GL_LIGHTMODELFV            F_CMD_MAKE(F_CMD_CLASS_GL, 0x000A)
#define F_GL_MATERIALFV              F_CMD_MAKE(F_CMD_CLASS_GL, 0x000B)
#define F_GL_COLORMATERIAL           F_CMD_MAKE(F_CMD_CLASS_GL, 0x000C)
#define F_GL_BEGIN                   F_CMD_MAKE(F_CMD_CLASS_GL, 0x000D)
#define F_GL_END                     F_CMD_MAKE(F_CMD_CLASS_GL, 0x000E)
#define F_GL_SHADEMODEL              F_CMD_MAKE(F_CMD_CLASS_GL, 0x000F)
#define F_GL_MATRIXMODE              F_CMD_MAKE(F_CMD_CLASS_GL, 0x0010)
#define F_GL_PUSHMATRIX              F_CMD_MAKE(F_CMD_CLASS_GL, 0x0011)
#define F_GL_POPMATRIX               F_CMD_MAKE(F_CMD_CLASS_GL, 0x0012)
#define F_GL_CLEAR                   F_CMD_MAKE(F_CMD_CLASS_GL, 0x0013)
#define F_GL_ENABLECLIENTSTATE       F_CMD_MAKE(F_CMD_CLASS_GL, 0x0014)
#define F_GL_DISABLECLIENTSTATE      F_CMD_MAKE(F_CMD_CLASS_GL, 0x0015)
#define F_GL_TRANSLATEF              F_CMD_MAKE(F_CMD_CLASS_GL, 0x0016)
#define F_GL_ROTATEF                 F_CMD_MAKE(F_CMD_CLASS_GL, 0x0017)
#define F_GL_FLUSH                   F_CMD_MAKE(F_CMD_CLASS_GL, 0x0018)
#define F_GL_VIEWPORT                F_CMD_MAKE(F_CMD_CLASS_GL, 0x0019)

#define F_GL_W_DESCRIBE_PIXEL_FORMAT F_CMD_MAKE(F_CMD_CLASS_WGL, 0x0000)

#define F_GLUT_INIT                  F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0000)
#define F_GLUT_INIT_DISPLAYMODE      F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0001)
#define F_GLUT_INIT_WINDOWSIZE       F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0002)
#define F_GLUT_INIT_WINDOPOSITION    F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0003)
#define F_GLUT_CREATEWINDOW          F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0004)
#define F_GLUT_DISPLAYFUNC           F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0005)
#define F_GLUT_RESHAPEFUNC           F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0006)
#define F_GLUT_MOUSEFUNC             F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0007)
#define F_GLUT_MOTIONFUNC            F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0008)
#define F_GLUT_PASSIVEMOTIONFUNC     F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0009)
#define F_GLUT_KEYBOARDFUNC          F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x000A)
#define F_GLUT_MAINLOOP              F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0010)
#define F_GLUT_MAINLOOPEND           F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0110)
#define F_GLUT_SWAPBUFFERS           F_CMD_MAKE(F_CMD_CLASS_GLUT, 0x0200)

#define F_GLU_PERSPECTIVE            F_CMD_MAKE(F_CMD_CLASS_GLU, 0x0000)

#define F_GLPM_WININITIALIZE         F_CMD_MAKE(F_CMD_CLASS_GLPM, 0x0000)
#define F_GLPM_WINCREATEMSGQUEUE     F_CMD_MAKE(F_CMD_CLASS_GLPM, 0x0001)

#endif /* FREEPM_CMD */
