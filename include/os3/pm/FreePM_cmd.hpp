/* 
 $Id: FreePM_cmd.hpp,v 1.8 2003/08/14 20:48:03 evgen2 Exp $ 
*/
/* FreePM_cmd.hpp */
#ifndef FREEPM_CMD
	#define FREEPM_CMD

#define F_CMD_CLIENT_EXIT	0
#define F_CMD_GET_IHAB		1
#define F_CMD_WINPOSTMSG	2
#define F_CMD_WINSENDMSG	3

#define F_CMD_WINCREATE_HWND	0x10
#define F_CMD_WINSET_PARENT_HWND	0x11

#define F_CMD_WINQUERY_MSG	0x20
#define F_CMD_WINGET_MSG	0x21

#define F_CMD_GET_HPS		0x40
#define F_CMD_RELEASE_HPS	0x41

/* send hwnd + 7*sizeof(in) data - hwndInsertBehind,x,y,cx,cy,fl,[z] */
#define F_CMD_WIN_SET_WND_SIZE	0x60  
#define F_CMD_WIN_SET_WND_POS	0x61  
#define F_CMD_WIN_GET_WND_SWP	0x62  
#define F_CMD_WIN_SHOW		0x63

#define F_CMD_GPI_SET_COLOR	0x100
#define F_CMD_GPI_MOVE		0x101
#define F_CMD_GPI_LINE		0x102
#define F_CMD_GPI_DRAW_LINE	0x103
#define F_CMD_GPI_DRAW_RECT	0x104

//OpenGL cmds, all & 0x10000
//wgl  subset  all & 0x11000
//glut subset  all & 0x12000
//glu  subset  all & 0x13000
//WinPM OpenGL all F_GLPM_WININITIALIZE		0x18000

#define F_GL_MASK			0x10000

#define F_GL_INIT_DRV			0x10000
#define F_GL_INIT_HAB			0x10001
#define F_GL_SET_HAB			0x10002
#define F_GL_GET_HAB			0x10003
#define F_GL_DISABLE			0x10004
#define F_GL_ENABLE			0x10005
#define F_GL_CULLFACE			0x10006
#define F_GL_CLEARDEPTH			0x10007
#define F_GL_CLEARCOLOR			0x10008
#define F_GL_LIGHTFV			0x10009
#define F_GL_LIGHTMODELFV		0x1000a
#define F_GL_MATERIALFV			0x1000b
#define F_GL_COLORMATERIAL		0x1000c
#define F_GL_BEGIN			0x1000d
#define F_GL_END			0x1000e
#define F_GL_SHADEMODEL			0x1000f
#define F_GL_MATRIXMODE			0x10010
#define F_GL_PUSHMATRIX			0x10011
#define F_GL_POPMATRIX			0x10012
#define F_GL_CLEAR			0x10013
#define F_GL_ENABLECLIENTSTATE		0x10014
#define F_GL_DISABLECLIENTSTATE		0x10015
#define F_GL_TRANSLATEF			0x10016
#define F_GL_ROTATEF			0x10017
#define F_GL_FLUSH			0x10018
#define F_GL_VIEWPORT			0x10019

#define F_GL_W_DESCRIBE_PIXEL_FORMAT	0x11000

#define F_GLUT_INIT			0x12000
#define F_GLUT_INIT_DISPLAYMODE		0x12001
#define F_GLUT_INIT_WINDOWSIZE		0x12002
#define F_GLUT_INIT_WINDOPOSITION	0x12003
#define F_GLUT_CREATEWINDOW		0x12004

#define F_GLUT_DISPLAYFUNC		0x12005
#define F_GLUT_RESHAPEFUNC		0x12006
#define F_GLUT_MOUSEFUNC		0x12007
#define F_GLUT_MOTIONFUNC		0x12008
#define F_GLUT_PASSIVEMOTIONFUNC	0x12009
#define F_GLUT_KEYBOARDFUNC		0x1200a

#define F_GLUT_MAINLOOP			0x12010
#define F_GLUT_MAINLOOPEND		0x12110

#define F_GLUT_SWAPBUFFERS              0x12200

#define F_GLU_PERSPECTIVE		0x13000

#define F_GLPM_WININITIALIZE		0x18000
#define F_GLPM_WINCREATEMSGQUEUE	0x18001

#endif