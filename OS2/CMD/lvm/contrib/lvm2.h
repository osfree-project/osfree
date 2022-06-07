/* lvm2.h */
#include "alvm.h"

int Quit( char *str);
void ConstructDiskPanels ( uint  top_row, uint  bottom_row );
void ConstructVolumePanels ( uint  top_row, uint  bottom_row );

uint DoVolumePanel ( void );
uint DoDiskPanel ( void );
int ParkCursor(void);
void DoEngineErrorPanel ( uint   error );
bool    NoStartablePartition ( void );

void *AllocateOrQuit ( uint n, uint size );
void *ReallocStringArray (void * item, uint size );

uint InputName (struct _panel *Partition_name_panel, char *Partition_name_input,int Partition_name_width, char *partition_name );
uint ChangePartitionName ( uint row, uint column, char *old_name, void   *handle );
uint ChangeDiskName ( uint row, uint column, char *old_name, void *handle );
uint ChangeVolumeName ( uint   row,  uint   column,  char   *old_name,  void   *handle );
uint ConstructBootmanPanels ( uint   row,  uint   column);




