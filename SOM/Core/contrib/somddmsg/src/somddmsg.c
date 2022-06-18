/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
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

/*
 * $Id$
 */

#ifdef _WIN32
	#include <windows.h>
#endif

#include <som.h>
#include <somderr.h>

#include <somddmsg.h>

#define SOMDDMSG_MAP(x)    {SOMDERROR_##x,SOMDDMSG_##x}

static struct 
{
	long err,msg;
} map[]=
{
SOMDDMSG_MAP(NoMemory),
SOMDDMSG_MAP(NotImplemented),
SOMDDMSG_MAP(UnexpectedNULL),
SOMDDMSG_MAP(IO),
SOMDDMSG_MAP(BadVersion),
SOMDDMSG_MAP(ParmSize),
SOMDDMSG_MAP(HostName),
SOMDDMSG_MAP(HostAddress),
SOMDDMSG_MAP(SocketCreate),
SOMDDMSG_MAP(SocketBind),
SOMDDMSG_MAP(SocketName),
SOMDDMSG_MAP(SocketReceive),
SOMDDMSG_MAP(SocketSend),
SOMDDMSG_MAP(SocketIoctl),
SOMDDMSG_MAP(SocketSelect),
SOMDDMSG_MAP(PacketSequence),
SOMDDMSG_MAP(PacketTooBig),
SOMDDMSG_MAP(AddressNotFound),
SOMDDMSG_MAP(NoMessages),
SOMDDMSG_MAP(UnknownAddress),
SOMDDMSG_MAP(RecvError),
SOMDDMSG_MAP(SendError),
SOMDDMSG_MAP(CommTimeOut),
SOMDDMSG_MAP(CannotConnect),
SOMDDMSG_MAP(BadConnection),
SOMDDMSG_MAP(NoHostName),
SOMDDMSG_MAP(BadBinding),
SOMDDMSG_MAP(BadMethodName),
SOMDDMSG_MAP(BadEnvironment),
SOMDDMSG_MAP(BadContext),
SOMDDMSG_MAP(BadNVList),
SOMDDMSG_MAP(BadFlag),
SOMDDMSG_MAP(BadLength),
SOMDDMSG_MAP(BadObjref),
SOMDDMSG_MAP(NullField),
SOMDDMSG_MAP(UnknownReposId),
SOMDDMSG_MAP(NVListAccess),
SOMDDMSG_MAP(NVIndexError),
SOMDDMSG_MAP(SysTime),
SOMDDMSG_MAP(SystemCallFailed),
SOMDDMSG_MAP(CouldNotStartProcess),
SOMDDMSG_MAP(NoServerClass),
SOMDDMSG_MAP(NoSOMDInit),
SOMDDMSG_MAP(SOMDDIRNotSet),
SOMDDMSG_MAP(NoImplDatabase),
SOMDDMSG_MAP(ImplNotFound),
SOMDDMSG_MAP(ClassNotFound),
SOMDDMSG_MAP(ServerNotFound),
SOMDDMSG_MAP(ServerAlreadyExists),
SOMDDMSG_MAP(ServerNotActive),
SOMDDMSG_MAP(CouldNotStartSOM),
SOMDDMSG_MAP(ObjectNotFound),
SOMDDMSG_MAP(NoParentClass),
SOMDDMSG_MAP(DispatchError),
SOMDDMSG_MAP(BadTypeCode),
SOMDDMSG_MAP(BadDescriptor),
SOMDDMSG_MAP(BadResultType),
SOMDDMSG_MAP(KeyInUse),
SOMDDMSG_MAP(KeyNotFound),
SOMDDMSG_MAP(CtxInvalidPropName),
SOMDDMSG_MAP(CtxNoPropFound),
SOMDDMSG_MAP(CtxStartScopeNotFound),
SOMDDMSG_MAP(CtxAccess),
SOMDDMSG_MAP(CouldNotStartThread),
SOMDDMSG_MAP(AccessDenied),
SOMDDMSG_MAP(BadParm),
SOMDDMSG_MAP(Interrupt),
SOMDDMSG_MAP(Locked),
SOMDDMSG_MAP(Pointer),
SOMDDMSG_MAP(Boundary),
SOMDDMSG_MAP(UnknownError),
SOMDDMSG_MAP(NoSpace),
SOMDDMSG_MAP(DuplicateQueue),
SOMDDMSG_MAP(BadQueueName),
SOMDDMSG_MAP(DuplicateSem),
SOMDDMSG_MAP(BadSemName),
SOMDDMSG_MAP(TooManyHandles),
SOMDDMSG_MAP(BadAddrFamily),
SOMDDMSG_MAP(BadFormat),
SOMDDMSG_MAP(BadDrive),
SOMDDMSG_MAP(SharingViolation),
SOMDDMSG_MAP(BadExeSignature),
SOMDDMSG_MAP(BadExe),
SOMDDMSG_MAP(Busy),
SOMDDMSG_MAP(BadThread),
SOMDDMSG_MAP(SOMDPORTNotDefined),
SOMDDMSG_MAP(ResourceExists),
SOMDDMSG_MAP(UserName),
SOMDDMSG_MAP(WrongRefType),
SOMDDMSG_MAP(MustOverride),
SOMDDMSG_MAP(NoSocketsClass),
SOMDDMSG_MAP(EManRegData),
SOMDDMSG_MAP(NoRemoteComm),
SOMDDMSG_MAP(RequestNotFound),
SOMDDMSG_MAP(SocketClose),
SOMDDMSG_MAP(GlobalAtomError),
SOMDDMSG_MAP(NamedMemoryTableError),
SOMDDMSG_MAP(WMQUIT),
SOMDDMSG_MAP(AnchorNotFound),
SOMDDMSG_MAP(NotQueueOwner),
SOMDDMSG_MAP(QueueNotFound),
SOMDDMSG_MAP(BadQueueHandle),
SOMDDMSG_MAP(LSBadCommand),
SOMDDMSG_MAP(BadRevision),
SOMDDMSG_MAP(DuplicateImplEntry),
SOMDDMSG_MAP(InvalidSOMSOCKETS),
SOMDDMSG_MAP(IRNotFound),
SOMDDMSG_MAP(ClassNotInIR),
SOMDDMSG_MAP(SOMDDNotRunning),
SOMDDMSG_MAP(SocketError),
SOMDDMSG_MAP(PacketError),
SOMDDMSG_MAP(Marshal),
SOMDDMSG_MAP(NotProcessOwner),
SOMDDMSG_MAP(ServerInactive),
SOMDDMSG_MAP(ServerDisabled),
SOMDDMSG_MAP(ServerInterrupt),
SOMDDMSG_MAP(SOMDDAlreadyRunning),
SOMDDMSG_MAP(ServerToBeDeleted),
SOMDDMSG_MAP(NoObjRefTable),
SOMDDMSG_MAP(UpdateImplDef),
SOMDDMSG_MAP(TranServiceUnavail),
SOMDDMSG_MAP(TranServiceError),
SOMDDMSG_MAP(AS400_Error01),
SOMDDMSG_MAP(AS400_Error02),
SOMDDMSG_MAP(AS400_Error03),
SOMDDMSG_MAP(AS400_Error04),
SOMDDMSG_MAP(AS400_Error05),
SOMDDMSG_MAP(AS400_Error06),
SOMDDMSG_MAP(AS400_Error07),
SOMDDMSG_MAP(AS400_Error08),
SOMDDMSG_MAP(AS400_Error09),
SOMDDMSG_MAP(AS400_Error10),
SOMDDMSG_MAP(AS400_Error11),
SOMDDMSG_MAP(AS400_Error12),
SOMDDMSG_MAP(AS400_Error13),
SOMDDMSG_MAP(AS400_Error14),
SOMDDMSG_MAP(AS400_Error15),
SOMDDMSG_MAP(NoImplDirectory),
SOMDDMSG_MAP(ORTError),
SOMDDMSG_MAP(AlreadyExists),
SOMDDMSG_MAP(ImplRepServerNotFound),
SOMDDMSG_MAP(WaitTimeError),
SOMDDMSG_MAP(TPoolError),
SOMDDMSG_MAP(PathConvertError),
SOMDDMSG_MAP(ServerENDJOBFailed),
SOMDDMSG_MAP(AllImplsNotReturned),
SOMDDMSG_MAP(ConvertFailed),
SOMDDMSG_MAP(SelidNotFound),
SOMDDMSG_MAP(EmptyEntry),
SOMDDMSG_MAP(FileNotFound),
SOMDDMSG_MAP(FileAlreadyExists),
SOMDDMSG_MAP(RemoteDispatchFailed),
SOMDDMSG_MAP(ClassNotInImplrep),
SOMDDMSG_MAP(ValueTooLong),
SOMDDMSG_MAP(ImplRepFileBad),
SOMDDMSG_MAP(ImplRepFileFFDC),
SOMDDMSG_MAP(AS400_Error34),
SOMDDMSG_MAP(AS400_Error35),
SOMDDMSG_MAP(AS400_Error36),
SOMDDMSG_MAP(AS400_Error37),
SOMDDMSG_MAP(AS400_Error38),
SOMDDMSG_MAP(AS400_Error39),
SOMDDMSG_MAP(AS400_Error40),
SOMDDMSG_MAP(AS400_Error41),
SOMDDMSG_MAP(AS400_Error42),
SOMDDMSG_MAP(AS400_Error43),
SOMDDMSG_MAP(AS400_Error44),
SOMDDMSG_MAP(AS400_Error45),
SOMDDMSG_MAP(ServerNotStoppable),
SOMDDMSG_MAP(AuthnFail),
SOMDDMSG_MAP(AuthzFail),
SOMDDMSG_MAP(SecurityFail),
SOMDDMSG_MAP(NoPersonaClass),
SOMDDMSG_MAP(NoAccess),
SOMDDMSG_MAP(ReservedForSec_2),
SOMDDMSG_MAP(ReservedForSec_3),
SOMDDMSG_MAP(ReservedForSec_4),
SOMDDMSG_MAP(ReservedForSec_5),
SOMDDMSG_MAP(ReservedForSec_6)
};

