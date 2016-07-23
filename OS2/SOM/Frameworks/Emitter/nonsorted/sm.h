/*
 *   COMPONENT_NAME: somc
 *
 *   ORIGINS: 27
 *
 *
 *   10H9767, 10H9769  (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* @(#) somc/sm.pub 2.8 7/27/95 16:12:28 [12/22/96 21:11:32] */


/*
 *         File:    emitlib.h.
 *     Contents:    Basic types used by Emitter Framework.
 */

#ifndef sm_h
#define sm_h

#ifdef __SOMIDL__
typedef void *Entry;

#else
typedef void *Stab;

typedef enum SOMTTypes {
    SOMTAnyBE,
    SOMTArgumentE,
    SOMTAttE,
    SOMTBadEntryE,
    SOMTBaseE,
    SOMTBooleanBE,
    SOMTCaseEntryE,
    SOMTCaseListE,
    SOMTCaseSTME,
    SOMTCharBE,
    SOMTClassE,
    SOMTConstE,
    SOMTCopyrightE,
    SOMTDataE,
    SOMTDclListE,
    SOMTDefaultE,
    SOMTDoubleBE,
    SOMTEBaseE,
    SOMTEEnumE,
    SOMTEnumBE,
    SOMTEnumE,
    SOMTEnumPE,
    SOMTFloatBE,
    SOMTGroupE,
    SOMTLongBE,
    SOMTMetaE,
    SOMTModuleE,
    SOMTNegativeBE,
    SOMTNewMethodE,
    SOMTOctetBE,
    SOMTOverriddenMethodE,
    SOMTOverrideMethodE,
    SOMTPassthruE,
    SOMTSequenceE,
    SOMTSequenceTDE,
    SOMTShortBE,
    SOMTStringBE,
    SOMTStringE,
    SOMTStructE,
    SOMTStructPE,
    SOMTStructSE,
    SOMTTyDclE,
    SOMTTypeCodeBE,
    SOMTTypedefBE,
    SOMTTypedefE,
    SOMTUnionE,
    SOMTUnionPE,
    SOMTUnionSE,
    SOMTUnsignedLongBE,
    SOMTUnsignedShortBE,
    SOMTVoidBE,
    SOMTVoidPtrBE,
    SOMTEmitterBeginE,
    SOMTEmitterEndE,
    SOMTGlobalOnlyE
} SOMTTypes;

typedef struct Entry {
    char *name;
    SOMTTypes type;
#ifdef __PRIVATE__
    RHBelement *data;
    char *filestem;
#endif    
} Entry;

#endif /* __SOMIDL__ */
#endif /* sm_h */
