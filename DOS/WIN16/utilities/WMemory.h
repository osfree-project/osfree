//==============================================================================
//
//	WMemory.h
//
//	Memory
//
//      @(#)WMemory.h	1.3 10:57:19 9/23/96 /users/sccs/src/win/utilities/s.WMemory.h 
//
//	Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//==============================================================================



#ifndef __WMEMORY_H__
#define __WMEMORY_H__



#include "windows.h"
#include "Willows.h"
#include "WLists.h"



typedef enum
{
    EWM_OPTIMIZEMEMORY,
    WWM_OPTIMIZESPEED
} EWM_OPTIMIZE;

typedef unsigned long   TWM_HANLDE;



const TWM_HANDLE cWM_INVALIDHANDLE = 0xFFFFFFFF;



class CWMHandle;
class CWMBlock;



class EXPORT CWMemory
{

public:

    CWMemory ( EWM_OPTIMIZE             Style );
    CWMemory ( CWMemory                 &that );
    ~CWMemory ( );

    int Alloc ( 
        TWM_HANDLE                      *pHandle,
        UINT                            Size );
    int ReAlloc ( 
        TWM_HANDLE                      *pHandle,
        UINT                            Size );
    int Free ( TWM_HANDLE               *pHandle );

    void Lock ( 
        TWM_HANDLE                      Handle,
        LPVOID*                         ppData );
    void Unlock ( 
        TWM_HANDLE                      Handle,
        LPVOID*                         ppData );

    UINT HandleLockCount ( TWM_HANDLE   Handle );
    UINT HandleSize ( TWM_HANDLE        Handle );

    void Defrag ( );


private:

    EWM_OPTIMIZE                        _Style;
    HGLOBAL                             _hMemory;
    LPBYTE                              _pMemory;
    UINT                                _Size;
    CWList                              *_pBlocks;
    CWList                              *_pHandles;
    TWM_HANDLE                          _Handle;
    UINT                                _ExpandSize;

    CWMHandle* FindFreeHandle ( );


};


#endif /* #ifndef __WMEMORY_H__ */

