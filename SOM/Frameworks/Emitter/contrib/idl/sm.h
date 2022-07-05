/**************************************************************************
 *
 *  Copyright 2022, Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
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
