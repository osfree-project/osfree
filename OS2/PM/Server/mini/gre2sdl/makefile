#
# A Makefile for gre2sdl.dll
# (c) osFree project
#

PROJECT      = gre2sdl
DESCRIPTION  = FreePM gre2sdl presentation driver
TARGET_CLASS = LIBRARY

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
OPTIONS = manyautodata,
IMPORTS = F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16, &
          SDL_Init SDL2, &
          SDL_Quit SDL2, &
          SDL_WasInit SDL2, &
          SDL_CreateWindow SDL2, &
          SDL_GetError SDL2, &
          SDL_CreateRenderer SDL2, &
          SDL_DestroyRenderer SDL2, &
          SDL_DestroyWindow SDL2, &
          SDL_SetRenderDrawColor SDL2, &
          SDL_RenderDrawLine SDL2, &
          SDL_RenderPresent SDL2, &
          SDL_RenderDrawPoint SDL2, &
          SDL_RenderFillRect SDL2, &
          SDL_PumpEvents SDL2, &
          SDL_MapRGB SDL2, &
          SDL_FillRect SDL2, &
          SDL_GetWindowSurface SDL2, &
          SDL_PollEvent SDL2, &
          SDL_UpperBlit SDL2, &
          SDL_UpdateWindowSurface SDL2, &
          SDL_CreateRGBSurfaceWithFormat SDL2, &
          SDL_FreeSurface SDL2, &
          SDL_RenderDrawRect SDL2
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT += -bt=nt
IMPORTS = F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16, &
          _SDL_Init SDL2.SDL_Init, &
          _SDL_Quit SDL2.SDL_Quit, &
          _SDL_WasInit SDL2.SDL_WasInit, &
          _SDL_CreateWindow SDL2.SDL_CreateWindow, &
          _SDL_GetError SDL2.SDL_GetError, &
          _SDL_CreateRenderer SDL2.SDL_CreateRenderer, &
          _SDL_DestroyRenderer SDL2.SDL_DestroyRenderer, &
          _SDL_DestroyWindow SDL2.SDL_DestroyWindow, &
          _SDL_SetRenderDrawColor SDL2.SDL_SetRenderDrawColor, &
          _SDL_RenderDrawLine SDL2.SDL_RenderDrawLine, &
          _SDL_RenderPresent SDL2.SDL_RenderPresent, &
          _SDL_RenderDrawPoint SDL2.SDL_RenderDrawPoint, &
          _SDL_RenderFillRect SDL2.SDL_RenderFillRect, &
          _SDL_PumpEvents SDL2.SDL_PumpEvents, &
          _SDL_MapRGB SDL2.SDL_MapRGB, &
          _SDL_FillRect SDL2.SDL_FillRect, &
          _SDL_GetWindowSurface SDL2.SDL_GetWindowSurface, &
          _SDL_PollEvent SDL2.SDL_PollEvent, &
          _SDL_UpperBlit SDL2.SDL_UpperBlit, &
          _SDL_UpdateWindowSurface SDL2.SDL_UpdateWindowSurface, &
          _SDL_CreateRGBSurfaceWithFormat SDL2.SDL_CreateRGBSurfaceWithFormat, &
          _SDL_FreeSurface SDL2.SDL_FreeSurface, &
          _SDL_RenderDrawRect SDL2.SDL_RenderDrawRect
!endif

EXPORTS = OS2_PM_DRV_ENABLE.200

ADD_COPT     +=  -fp5 -ei -i=$(SRCDIR) -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)include  -sg -of+ -bd -5s -bm

ADD_LINKOPT  = segment   type DATA nonshared
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS     += caseexact



!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)
