void ntfiletime2os2(const FILETIME &ft, FDATE *fdate, FTIME *ftime);
void ntfileattr2os2(DWORD dwAttrs, ULONG *ulAttr);
void os22ntfiletime(FDATE fdate, FTIME ftime, FILETIME *ft);

