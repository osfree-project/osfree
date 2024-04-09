#include <kal.h>

APIRET16 APIENTRY16 DOS16CWAIT(USHORT ActionCode, USHORT WaitOption, PRESULTCODES ReturnCodes, PPID ProcessIDWord, PID ProcessID)
{
	return DosWaitChild(ActionCode, WaitOption, ReturnCodes, ProcessIDWord, ProcessID);
}
