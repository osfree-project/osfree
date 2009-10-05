/*
 *         File:    sm.h.
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
} Entry;

#endif /* __SOMIDL__ */
#endif /* sm_h */
