#define INCL_DOSPROCESS
#include <os2.h>
#include <time.h>
#include "signal.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <tftpd.h>

char path_prefix[256];
int  default_blksize = 512;
int  num_block = 0;
char verbose = FALSE;
int  Logging(char *fmt, ...);

int
decode(
    char type,
    char *raw,
    int len_raw,
    PACKET *packet)
{
    char           *pos[20];
    int            nbr;
    int            i;

    switch (type)
    {
            case RRQ:
                 nbr      = 0;
                 pos[nbr] = &raw[0];
                 for (i=0;i<len_raw;i++)
                 {
                      if (raw[i] == '\0') pos[++nbr] = &raw[i+1];
                 }
                 nbr--;
                 strcpy(packet->rrq.filename,pos[0]);

                 packet->rrq.type_trf = '?';
                 if (stricmp(pos[1],"octet") == 0)    packet->rrq.type_trf = 'o';
                 if (stricmp(pos[1],"netascii") == 0) packet->rrq.type_trf = 'n';
                 if (stricmp(pos[1],"mail") == 0)     packet->rrq.type_trf = 'm';
                 if (packet->rrq.type_trf == '?')
                 {
                     Logging("Fatal error unknown transfer type : %s\n",pos[1]);
                     exit(-1);
                 }
                 packet->rrq.opt      = FALSE;    /* tftp without option set by default */
                 packet->rrq.tsize   = -1;
                 packet->rrq.blksize = -1;
                 for (i=2; i < nbr; i += 2)
                 {
                      if (stricmp(pos[i],"tsize") == 0)
                      {
                          packet->rrq.tsize  = atol(pos[i+1]);
                          packet->rrq.opt    = TRUE;
                      }
                      if (stricmp(pos[i],"blksize") == 0)
                      {
                          packet->rrq.blksize = atol(pos[i+1]);
                          packet->rrq.opt     = TRUE;
                      }
                 }
                 break;
            case ACK:
                 break;
            case ERROR:
                 packet->err.rc = raw[0]*256+raw[1];
                 strcpy(packet->err.msger,&raw[2]);
                 break;
    }
    return 0;
}

void
combine(
    char path[],
    char *prefix,
    char *p)
{
    int i;

    for (i = 0; p[i] && i < 256; i++)
    {
         if (p[i] == '/') p[i] = '\\';
    }
    while (*p == '\\') p++;

    i = strlen(prefix);
    while (prefix[i - 1] == '\\')
    {
           prefix[i - 1] = '\0';
           i--;
    }
    sprintf(path, "%s\\%s", prefix, p);
}

void
break_handler(
    int sig_nummer )
{
    Logging("TFTPD stopped succesfully\n");
    exit(0);
}

int main(int argc,char **argv)
{
  int                sockint;
  int                s;
  struct sockaddr_in server;
  struct sockaddr_in client;
  struct servent     *tftpd_prot;
  int                client_address_size;
  char               buf[2048];
  int                nbr_lu;
  int                nbr_ecrit;
  PACKET             packet;
  struct stat        s_stat;
  char               path[256];
  FILE               *fichin;
  int                i;
  char               *p;

  signal(SIGINT,break_handler);
  printf("**********************************************\n");
  printf("*      IBM TCP/IP for OS/2                   *\n");
  printf("*      Advanced TFTP Server (TFTPD)          *\n");
  printf("*      Support blksize,tsize options         *\n");
  printf("*      Version: %s %s         *\n",__DATE__,__TIME__);
  printf("*      (C) Copyright Serge Sterck 2002       *\n");
  printf("**********************************************\n");
  printf("\n");
  verbose = FALSE;
  path_prefix[0] = '\0';
  if (argc > 1)
    {
      for(i=1;i<argc;i++)
       {
          if (stricmp(argv[i],"-v") == 0)
             verbose = TRUE;
          else strcpy(path_prefix,argv[1]);
       }
    }
  if ((sockint = sock_init()) != 0)
    {
       Logging(" TCP/IP stack is probably not running");
       exit(1);
    }
//loop_bind:
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        psock_errno("socket()");
        exit(1);
    }

  tftpd_prot = getservbyname("tftp", "udp");
  if (tftpd_prot == NULL)
    {
      Logging("The tftpd/udp protocol is not listed in the etc/services file\n");
      exit(1);
    }
  server.sin_family      = AF_INET;
  server.sin_port        = tftpd_prot->s_port;
  server.sin_addr.s_addr = INADDR_ANY;

  if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
      psock_errno("bind()");
      exit(2);
    }
  client_address_size = sizeof(client);
loop:
  nbr_lu = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &client,&client_address_size);
  if (nbr_lu < 0)
   {
       psock_errno("recvfrom()");
       exit(4);
   }
  decode(buf[1],&buf[2],nbr_lu,&packet);
loop_req:
  switch( buf[1] )
   {
     case RRQ:
          if (verbose)
            Logging("<< %15s :Read %s\n",inet_ntoa(client.sin_addr),packet.rrq.filename);
          Logging("Download file : %s\n",packet.rrq.filename);
          if (packet.rrq.opt == TRUE)     /* we receive a read with a option */
            {
              if (packet.rrq.tsize == 0)  /* client ask for the filesize     */
                {
                  combine(path, path_prefix, packet.rrq.filename);
                  //sprintf(path,"%s\\%s",path_prefix,packet.rrq.filename);
                  if (stat(path,&s_stat) != 0)
                    {
                      Logging("\ncannot stat %s\n",path);
                      buf[0] = 0;
                      buf[1] = ERROR;
                      buf[2] = 0;
                      buf[3] = 0;
                      sprintf(&buf[4],"Cannot stat %s",path);
                      nbr_ecrit = sendto(s, buf, 4+strlen(&buf[4])+1, 0, (struct sockaddr *) &client,client_address_size);
                      if (nbr_ecrit < 0)
                        {
                          psock_errno("sendto()");
                          exit(4);
                        }
                      goto loop;
                    }
                  if (verbose)
                    Logging(">> %15s :Oack size of %s = %d\n",inet_ntoa(server.sin_addr),packet.rrq.filename,s_stat.st_size);
                  buf[0] = 0;
                  buf[1] = OACK;
                  strcpy(&buf[2],"tsize");
                  buf[7] = '\0';
                  sprintf(&buf[8],"%d",s_stat.st_size);
                  nbr_ecrit = sendto(s, buf, 8+strlen(&buf[8])+1, 0, (struct sockaddr *) &client,client_address_size);
                  if (nbr_ecrit < 0)
                    {
                      psock_errno("sendto()");
                      exit(4);
                     }
                }
              if (packet.rrq.blksize != -1)  /* Initiate read with blkize       */
                   default_blksize = packet.rrq.blksize;
              else default_blksize = 512;
              if (verbose)
                Logging("                   :blksize set to :%d\n",default_blksize);
            }
          else
            {
              buf[0] = 0;      /* send ack block = 0 */
              buf[1] = ACK;
              buf[2] = 0;
              buf[3] = 0;
              nbr_ecrit = sendto(s, buf, 4, 0, (struct sockaddr *) &client,client_address_size);
              if (nbr_ecrit < 0)
                {
                  psock_errno("sendto()");
                  exit(4);
                }
            }
          combine(path, path_prefix, packet.rrq.filename);
          fichin    = fopen(path,"rb");
          if (fichin == NULL)
          {
            Logging("\ncannot open %s\n",path);
            goto loop;
          }
          num_block = 1;
          buf[1] = DATA;
          goto loop_req;
          break;
     case DATA:
          buf[0] = 0;
          buf[1] = DATA;
          buf[2] = num_block/256;
          buf[3] = num_block % 256;
          nbr_lu = fread(&buf[4],1,default_blksize,fichin);
          if (verbose)
            Logging(">> %15s :Data block :%d\n",inet_ntoa(server.sin_addr),num_block);
          if (nbr_lu == default_blksize)
            {
               nbr_ecrit = sendto(s, buf, 4+default_blksize, 0, (struct sockaddr *) &client,client_address_size);
               if (nbr_ecrit < 0)
                 {
                   psock_errno("sendto()");
                   exit(4);
                 }
            }
          else
              {
                nbr_ecrit = sendto(s, buf, 4+nbr_lu, 0, (struct sockaddr *) &client,client_address_size);
                if (nbr_ecrit < 0)
                  {
                    psock_errno("sendto()");
                    exit(4);
                  }
                fclose(fichin);
                //soclose(s);
                //goto loop_bind;
                goto loop;
              }
          if (nbr_ecrit < 0)
            {
              psock_errno("sendto()");
              exit(4);
            }
          break;
     case ACK:
          if (verbose)
            Logging("<< %15s :Ack block :%d \n",inet_ntoa(client.sin_addr),buf[2]*256+buf[3]);
          if (buf[2] == 0 && buf[3] == 0)
            goto loop;    /* we ack the block 0 */
          buf[1] = DATA;
          num_block++;
          goto loop_req;
     case ERROR:
          if (verbose)
            Logging("<< %15s :Error rc = %d msger = %s\n",inet_ntoa(client.sin_addr),packet.err.rc,packet.err.msger);
          else Logging("                Error --> %s\n",packet.err.msger);
          default_blksize = 512;
          fclose(fichin);
          break;
   }
  goto loop;
  soclose(s);
  return 0;
}
volatile int InStatusLine = 0;

/**
* Log file operation
* Multi thread, SMP ready
*
* @fmt    format string, see printf
* @...    parameters
* @return 0 - Ok, fails else
*/
char      tmp[1024];
FILE      *lg = NULL;
char      LogFile[] = "tfpd.log";
char *mtext[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int
Logging(
    char *fmt,
    ...)
{
    va_list   arg_ptr;
    time_t    ltime;
    struct tm *tp;

    DosSetPriority ( PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0);

#ifdef SMP_READY
    while (__lxchg(&InStatusLine, 1))
    {
           DosSleep(1);
    }
#endif

    if ((lg == NULL) && (lg=fopen(LogFile,"a+")) == NULL)
    {
        fprintf(stderr,"Can\'t open %s\n",LogFile);
        DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, 0, 0);
        InStatusLine--;
        return 1;
    }

    va_start (arg_ptr, fmt);
    if (fmt == NULL)
    {
        if (InStatusLine == 1)
        {
            fclose(lg);
            lg = NULL;
        }
        DosSetPriority ( PRTYS_THREAD, PRTYC_REGULAR, 0, 0);
        InStatusLine--;
        return 0;
    }

    (void) vsprintf (tmp, fmt, arg_ptr);
    (void) time (&ltime);
    tp = localtime (&ltime);
    if (lg != NULL)
    {
        (void) fprintf (lg, "%02i %s %02i:%02i:%02i tftpd %s",
                             tp->tm_mday, mtext[tp->tm_mon], tp->tm_hour, tp->tm_min, tp->tm_sec,
                             tmp);
    }

//    if (Verbose)
    {
        (void) fprintf (stdout, "%02i %s %02i:%02i:%02i tftpd %s",
                                 tp->tm_mday, mtext[tp->tm_mon], tp->tm_hour, tp->tm_min, tp->tm_sec,
                                 tmp);
    }

    va_end (arg_ptr);

    if (InStatusLine == 1)
    {
        fclose(lg);
        lg = NULL;
    }

    DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, 0, 0);
#ifdef SMP_READY
    __lxchg(&InStatusLine, 0);
#endif

    return 0;
}
