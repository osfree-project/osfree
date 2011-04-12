/**************************** TRUE or FALSE ***********************************/
#ifndef TRUE
  #define TRUE  1
#endif
#ifndef FALSE
  #define FALSE 0
#endif
/**************************** REQUEST TYPE ************************************/
#define RRQ     01                      /* read request              */
#define WRQ     02                      /* write request             */
#define DATA    03                      /* data packet               */
#define ACK     04                      /* acknowledgement           */
#define ERROR   05                      /* error code                */
#define OACK    06                      /* option ack                */
/**************************** REQUEST PACKET STRUC ****************************/
typedef struct _RRQ_PACKET
 {
   char filename[80];                   /* filename to be read       */
   char type_trf;                       /* n(etascii),o(ctet),m(ail) */
   char opt;                            /* TRUE if tftp with opt     */
   long tsize;                          /* -1 not used               */
   long blksize;                        /* -1 not used               */
 } RRQ_PACKET;

typedef struct _ERR_PACKET
 {
   int  rc;                             /* return  code              */
   char msger[80];                      /* error msg                 */
 } ERR_PACKET;
typedef struct _PACKET
 {
   RRQ_PACKET rrq;
   ERR_PACKET err;
 } PACKET;
