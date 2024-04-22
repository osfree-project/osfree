#include <kal.h>

USHORT APIENTRY16     DOS16PUTMESSAGE(HFILE hf, USHORT msglen, char * _Seg16 pMsgBuf)
{
	return DosPutMessage(hf, msglen, pMsgBuf);
}
