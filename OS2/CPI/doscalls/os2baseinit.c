// Base subsystems initialization 32-bit version

APIRET APIENTRY OS2BaseInit(void)
{
  USHORT rc=-1;
  if (!VioInit())
  {
    DosGetInfoSeg()
    if (procTYPE<=LIS_PT_VIOWIN)
    {
      if (!AnsiInit()&&!BMSInit()) rc=0;
    }
  } else rc=0;
  return rc;
}