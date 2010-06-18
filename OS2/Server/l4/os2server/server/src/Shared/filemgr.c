// This is initialization of file manager
void FilInit()
{
  unsigned short i;

  // Initialization of current directories
  for(i=0;i<25;i++)
  {
    FilCurrentDirs[i]="\";
  }

  // Initialization of current disk
  FilCurrentDisk=options.bootdrive[0];
}
