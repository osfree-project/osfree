#include "kal.h"

USHORT APIENTRY16  DOS16QNMPHANDSTATE(HPIPE Handle, USHORT * _Seg16 PipeHandleState)
{
	USHORT rc;
	ULONG hs;
	rc=DosQueryNPHState(Handle, &hs);
	*PipeHandleState=hs;
	return rc;
}
