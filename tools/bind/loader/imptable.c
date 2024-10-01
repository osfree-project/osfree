#include <os2.h>
typedef
struct _entry {
  char mod[9];     // module name
  char func[21];   // function name
  void far *ptr;   // pointer to function
} entry;

entry imptable[2] =
{
  {"DOSCALLS", "DOSWRITE", DosWrite},
  {"DOSCALLS", "DOSREAD", DosRead},
};

