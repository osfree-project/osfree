/*! @file F_pipe.hpp
    @brief OS/2 named-pipe transport for FreePM: PipeLink class.
*/
#ifndef F_PIPE_HPP
#define F_PIPE_HPP

#if defined(__OS2__) || defined(__OS2V2__)

  #define INCL_DOSNMPIPES
  #include "F_platform.h"
  #include "F_link.hpp"

  #define FREEPM_PIPE_NAME        "\\PIPE\\FREEPM"
  #define FREEPM_PIPE_MAX_INST    32
  #define FREEPM_PIPE_OUTB_SIZE   0x1000
  #define FREEPM_PIPE_INPB_SIZE   0x1000
  #define FREEPM_PIPE_TIMEOUT     20000L
  #define FREEPM_PIPE_THREAD_STACK 65536

  #define PIPE_SERVER_MODE   1
  #define PIPE_CLIENT_MODE   2

  class PipeLink : public F_Link {
  public:
      HPIPE   Hpipe;
      char    name[256];
      int     mode;
      int     connected;

      PipeLink();
      PipeLink(const char *pszName, int iMode);
      virtual ~PipeLink();

      virtual int Open(void);
      virtual int Accept(F_Link **ppNew);
      virtual int Handshake(void);
      virtual int Send(const void *pvData, int cbData);
      virtual int Recv(void *pvData, int *pcbReceived, int cbBuffer);
      virtual int Close(void);
  };

#else
  #error PIPE transport is OS/2 only
#endif

#endif /* F_PIPE_HPP */
