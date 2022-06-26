
// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __SecHlp_h
#define __SecHlp_h

// #include <secure.h>

/* struct SecurityHelpers {
    ULONG (*SecHlpQuerySubjectHandle)( PID, TID, ULONG, PHSUBJECT);
    ULONG (*SecHlpQuerySubjectInfo)( PID, TID, ULONG, PSUBJECTINFO);
    ULONG (*SecHlpQueryContextStatus)( PID, TID, PULONG);
    ULONG (*SecHlpQuerySecurityContext)( PID, TID, PSECURITYCONTEXT );
    ULONG (*SecHlpQuerySubjectHandleInfo)( HSUBJECT, PSUBJECTINFO);
    ULONG (*SecHlpSetSubjectHandle)( PID, TID, ULONG, HSUBJECT);
    ULONG (*SecHlpSetSecurityContext)(PID, TID, PSECURITYCONTEXT);
    ULONG (*SecHlpSetContextStatus)(PID, TID, ULONG);
    ULONG (*SecHlpResetThreadContext)(PID, TID, ULONG, ULONG);
    ULONG (*SecHlpQueryAuthorityID)(PID, TID, PULONG);
    ULONG (*SecHlpSetAuthorityID)(PID, TID, ULONG);
    ULONG (*SecHlpReserveHandle)(USHORT, USHORT, HSUBJECT);
    ULONG (*SecHlpReleaseHandle)(USHORT, USHORT, HSUBJECT);
    ULONG (*SecHlpSetChildSecurityContext)(PSECURITYCONTEXT);
};

extern struct SecurityHelpers SecHlp;

INLINE ULONG SecHlpQuerySubjectHandle( PID pid, TID tid, ULONG ulong, PHSUBJECT phsubject) {
    return SecHlp.SecHlpQuerySubjectHandle(pid, tid, ulong, phsubject);
}
INLINE ULONG SecHlpQuerySubjectInfo( PID pid, TID tid, ULONG ulong, PSUBJECTINFO phsubjectinfo) {
    return SecHlp.SecHlpQuerySubjectInfo(pid, tid, ulong, phsubjectinfo);
}
INLINE ULONG SecHlpQuerySubjectHandleInfo( HSUBJECT hsubject, PSUBJECTINFO phsubjectinfo) {
    return SecHlp.SecHlpQuerySubjectHandleInfo(hsubject, phsubjectinfo);
}
INLINE ULONG SecHlpQueryContextStatus( PID pid, TID tid, PULONG pulong) {
    return SecHlp.SecHlpQueryContextStatus(pid, tid, pulong);
}
INLINE ULONG SecHlpQuerySecurityContext( PID pid, TID tid, PSECURITYCONTEXT psecuritycontext) {
    return SecHlp.SecHlpQuerySecurityContext(pid, tid, psecuritycontext);
}
INLINE ULONG SecHlpSetSubjectHandle( PID pid, TID tid, ULONG ulong, HSUBJECT hsubject) {
    return SecHlp.SecHlpSetSubjectHandle(pid, tid, ulong, hsubject);
}
INLINE ULONG SecHlpSetContextStatus(PID pid, TID tid, ULONG ulong) {
    return SecHlp.SecHlpSetContextStatus(pid, tid, ulong);
}
INLINE ULONG SecHlpSetSecurityContext(PID pid, TID tid, PSECURITYCONTEXT psecuritycontext) {
    return SecHlp.SecHlpSetSecurityContext(pid, tid, psecuritycontext);
}
INLINE ULONG SecHlpResetThreadContext(PID pid, TID tid, ULONG ulong1, ULONG ulong2) {
    return SecHlpResetThreadContext(pid, tid, ulong1, ulong2);
}
INLINE ULONG SecHlpQueryAuthorityID(PID pid, TID tid, PULONG pulong) {
    return SecHlp.SecHlpQueryAuthorityID(pid, tid, pulong);
}
INLINE ULONG SecHlpSetAuthorityID(PID pid, TID tid, ULONG ulong) {
    return SecHlp.SecHlpSetAuthorityID(pid, tid, ulong);
}
INLINE ULONG SecHlpReserveHandle(USHORT ushort1, USHORT ushort2, HSUBJECT hsubject) {
    return SecHlp.SecHlpReserveHandle(ushort1, ushort2, hsubject);
}
INLINE ULONG SecHlpReleaseHandle(USHORT ushort1, USHORT ushort2, HSUBJECT hsubject) {
    return SecHlp.SecHlpReleaseHandle(ushort1, ushort2, hsubject);
}
INLINE ULONG SecHlpSetChildSecurityContext(PSECURITYCONTEXT psecuritycontext) {
    return SecHlp.SecHlpSetChildSecurityContext(psecuritycontext);
}

extern struct SecExp_s SecurityExports;

INLINE ULONG CallType SecHlpRead(ULONG SFN, PULONG pcbBytes, PUCHAR pBuffer,ULONG p16Addr,  ULONG Offset) {
    return SecurityExports.SecHlpRead(SFN, pcbBytes, pBuffer, p16Addr, Offset);
}
INLINE ULONG CallType SecHlpWrite(ULONG SFN, PULONG pcbBytes, PUCHAR pBuffer,ULONG p16Addr,  ULONG Offset) {
    return SecurityExports.SecHlpWrite(SFN, pcbBytes, pBuffer, p16Addr, Offset);
}
INLINE ULONG CallType SecHlpOpen(PSZ pszFileName,PULONG pSFN,ULONG ulOpenFlag,ULONG ulOpenMode) {
    return SecurityExports.SecHlpOpen(pszFileName, pSFN, ulOpenFlag, ulOpenMode);
}
INLINE ULONG CallType SecHlpClose(ULONG SFN) {
    return SecurityExports.SecHlpClose(SFN);
}
INLINE ULONG CallType SecHlpQFileSize(ULONG SFN, PULONG pSize) {
    return SecurityExports.SecHlpQFileSize(SFN, pSize);
}
INLINE ULONG CallType SecHlpChgFilePtr(ULONG SFN, LONG Offset,ULONG TYPE, PULONG pAbs) {
    return SecurityExports.SecHlpChgFilePtr(SFN, Offset, TYPE, pAbs);
}
INLINE ULONG CallType SecHlpSFFromSFN(ULONG SFN) {
    return SecurityExports.SecHlpSFFromSFN(SFN);
}
INLINE ULONG CallType SecHlpFindNext(PFINDPARMS pParms) {
    return SecurityExports.SecHlpFindNext(pParms);
}
INLINE ULONG CallType SecHlpPathFromSFN(ULONG SFN) {
    return SecurityExports.SecHlpPathFromSFN(SFN);
} */

#endif
