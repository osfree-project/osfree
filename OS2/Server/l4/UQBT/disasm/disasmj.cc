/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       disasmj.cc
 * OVERVIEW:   BinaryFile API stub for java disassembler.
 *
 * (C) 2001 Sun Microsystems, Inc.
 *============================================================================*/

/* $Revision: 1.3 $
 *
 * 28 May 01 - Nathan: Created
 */

#include "BinaryFile.h"
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
    JNIEXPORT jlongArray JNICALL Java_disasm_loadBinaryFile( JNIEnv *env, jclass clz, jstring arg );
    JNIEXPORT void JNICALL Java_disasm_unloadBinaryFile( JNIEnv *env, jclass clz, jlong arg );
#ifdef __cplusplus
}
#endif

JNIEXPORT jlongArray JNICALL Java_disasm_loadBinaryFile( JNIEnv *env, jclass clz, jstring arg ) 
{
    const char *fname = env->GetStringUTFChars( arg, 0 );
    BinaryFile *pbf = BinaryFile::Load( fname );
    if( pbf == NULL )
        return env->NewLongArray(0);
    env->ReleaseStringUTFChars( arg, fname );

    // Disassemble the CODE section, or at least the same section as main
    ADDRESS uAddr = pbf->GetMainEntryPoint();
    if( !uAddr ) { /* don't know the code segment. yay */
        pbf->UnLoad();
        return env->NewLongArray(0);
    }

    SectionInfo* pSect = pbf->GetSectionInfoByAddr(uAddr);
    assert (pSect);
    ADDRESS hiAddress = pSect->uNativeAddr + pSect->uSectionSize;
    int delta = pSect->uHostAddr - pSect->uNativeAddr;

    // Start at the start of the code section
    uAddr = pSect->uNativeAddr;
    jlongArray result = env->NewLongArray(4);
    jlong *arr = env->GetLongArrayElements(result,0);
    arr[0] = (jlong)pbf;// so we can free it later
    arr[1] = uAddr;     // start
    arr[2] = hiAddress; // end
    arr[3] = delta;     // value of mem
    env->ReleaseLongArrayElements(result, arr, 0);
    return result;
}

JNIEXPORT void JNICALL Java_disasm_unloadBinaryFile( JNIEnv *env, jclass clz, jlong arg )
{
    BinaryFile *pbf = (BinaryFile *)arg;
    pbf->UnLoad();
    delete pbf;
}
