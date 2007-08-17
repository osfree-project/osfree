/*
 *    FAT filesystem-
 *    specific code.
 */

/* Sector read function prototype */
void __cdecl read_run(unsigned long start,
                      unsigned short len,
                      void far *buf);


void read_super (void)
{

};


unsigned short __cdecl mu_Open (char far *pName, unsigned long far *pulFileSize)
{

};


unsigned long  __cdecl mu_Close (void)
{

};


unsigned long  __cdecl mu_Read (long loffseek, char far *pBuf, unsigned long cbBuf)
{

};



unsigned long  __cdecl mu_Mount (void)
{

}
