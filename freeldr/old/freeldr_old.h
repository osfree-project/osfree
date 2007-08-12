void __cdecl DispNTS(char * CMsgBuff);
void __cdecl DispC(char * CMsgBuff);
void __cdecl DebugNTS(char * CMsgBuff);

//
//int pascal far muOpen(char far * fileName, unsigned long int far * fileSize);
//unsigned long int __cdecl muRead(unsigned long int far seekOffset,
//                         unsigned char far * dataBuffer,
//                         unsigned long int far bufferSize);
//void __cdecl far muClose();
//void __cdecl far muTerminate();

unsigned short muOpen(char far* fileName,
                      unsigned long far* fileSize);

unsigned long muRead(long seekOffset,
                     char far* dataBuffer,
                     unsigned long bufferSize);

void muClose(void);

void muTerminate(void);

