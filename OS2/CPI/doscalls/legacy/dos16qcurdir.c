#include "kal.h"

USHORT APIENTRY16    DOS16QCURDIR(USHORT drivenum, char * _Seg16 pszPath, USHORT * _Seg16 cbPath)
{
	ULONG cb;
	USHORT rc;
	
	rc=DosQueryCurrentDir(drivenum, pszPath, &cb);
	cbPath=cb;
	return rc;
}
