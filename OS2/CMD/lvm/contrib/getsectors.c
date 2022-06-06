/* getsectors.c */

#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#include <os2.h>

int GetBytesPerSector(char DriveLetter)
{
    int rc, cbSector=0;
    FSALLOCATE fsinfo = {0};   /* fs info buffer */

    rc = DosQueryFSInfo(DriveLetter, /* FS Info for drive */
                    1,                               /* info level 1 */
                    (PBYTE)&fsinfo,                   /* info buffer */
                    sizeof(fsinfo));             /* info buffer size */
    if(rc == 0) cbSector = fsinfo.cbSector;
    return cbSector;
}
