/**************************************************************************
Etherboot -  BOOTP/TFTP Bootstrap Program
UNDI NIC driver for Etherboot

This file Copyright (C) 2003 Michael Brown <mbrown@fensystems.co.uk>
of Fen Systems Ltd. (http://www.fensystems.co.uk/).  All rights
reserved.

$Id: undi.c,v 1.8 2003/10/25 13:54:53 mcb30 Exp $
***************************************************************************/

/*
 * Support for most UNDI code removed since not needed and support for PXE
 * TFTP added.
 * Reformatted.
 *
 * Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

//#include "etherboot.h"
//#include "nic.h"
//#include "pci.h"
#include "dev.h"
#include <shared.h>
#include "undi.h"
//#include "pxe_tftp.h"

#pragma aux gateA20 "*"
void gateA20(int);

#define PCI_CLASS_NETWORK_ETHERNET      0x0200

/* NIC specific static variables go here */
static undi_t undi = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, 0, NULL, 0, NULL,
                       0, 0, 0, 0,
                       { 0, 0, 0, NULL, 0, 0, 0, 0, 0, NULL },
                       IRQ_NONE };
static undi_base_mem_data_t undi_base_mem_data;

static int pxe_tftp_driver_active_value;
static int pxe_tftp_opened;

/* Function prototypes */
static int allocate_base_mem_data ( void );
static int free_base_mem_data ( void );

/**************************************************************************
 * Utility functions
 **************************************************************************/

static inline uint32_t get_free_base_memory(void)
{
  return *((uint16_t *)phys_to_virt(0x413)) << 10;
}


/* Checksum a block.
 */

static uint8_t checksum (void *block, size_t size)
{
  uint8_t sum = 0;
  uint16_t i;

  for (i = 0; i < size; i++)
    sum += ((uint8_t *)block)[i];

  return sum;
}

/* Print the status of a !PXE structure
 */
static void pxe_dump (void)
{
  printf("API %hx:%hx St %hx:%hx UD %hx:%hx UC %hx:%hx "
         "BD %hx:%hx BC %hx:%hx\n",
         undi.pxe->EntryPointSP.segment, undi.pxe->EntryPointSP.offset,
         undi.pxe->Stack.Seg_Addr, undi.pxe->Stack.Seg_Size,
         undi.pxe->UNDIData.Seg_Addr, undi.pxe->UNDIData.Seg_Size,
         undi.pxe->UNDICode.Seg_Addr, undi.pxe->UNDICode.Seg_Size,
         undi.pxe->BC_Data.Seg_Addr, undi.pxe->BC_Data.Seg_Size,
         undi.pxe->BC_Code.Seg_Addr, undi.pxe->BC_Code.Seg_Size );
}

/* Allocate/free space for structures that must reside in base memory
 */
int allocate_base_mem_data(void)
{
  /* In GRUB, anything is in base address, so we do not need
   * allocate anything */
  undi.base_mem_data  = &undi_base_mem_data;
  memset(undi.base_mem_data, 0, sizeof(undi_base_mem_data_t));
  undi.undi_call_info = &undi.base_mem_data->undi_call_info;
  undi.pxs            = &undi.base_mem_data->pxs;
  undi.xmit_data      = &undi.base_mem_data->xmit_data;
  undi.xmit_buffer    = undi.base_mem_data->xmit_buffer;

  return 1;
}

int free_base_mem_data(void)
{
  /* Just pretend to free something :-) */
  undi.base_mem_data  = NULL;
  undi.undi_call_info = NULL;
  undi.pxs            = NULL;
  undi.xmit_data      = NULL;
  undi.xmit_buffer    = NULL;

  return 1;
}

/* Debug macros
 */

#define TRACE_UNDI
#ifdef TRACE_UNDI
#define DBG(...) printf ( __VA_ARGS__ )
#else
#define DBG(...)
#endif

#define UNDI_STATUS(pxs) ( (pxs)->Status == PXENV_EXIT_SUCCESS ? \
                              "SUCCESS" : \
                              ( (pxs)->Status == PXENV_EXIT_FAILURE ? \
                                "FAILURE" : "UNKNOWN" ) )

/**************************************************************************
 * Base memory scanning functions
 **************************************************************************/

/* Locate the $PnP structure indicating a PnP BIOS.
 */

static int hunt_pnp_bios(void)
{
  uint32_t off = 0x10000;

  printf("Hunting for PnP BIOS...");
  while (off > 0)
    {
      off -= 16;
      undi.pnp_bios = (pnp_bios_t *)phys_to_virt(0xf0000 + off);
      if (undi.pnp_bios->signature == PNP_BIOS_SIGNATURE)
        {
          printf("found $PnP at f000:%x...", off);
          if (checksum(undi.pnp_bios,sizeof(pnp_bios_t)))
            {
              printf("invalid checksum\r\n...");
              continue;
            }
          printf("ok\r\n");
          return 1;
        }
    }
  printf("none found\r\n");
  undi.pnp_bios = NULL;
  return 0;
}

/* Locate the !PXE structure indicating a loaded UNDI driver.
 */

static int hunt_pixie (void)
{
  static uint32_t ptr = 0;
  pxe_t *pxe = NULL;

  printf("Hunting for pixies...");
  if (ptr == 0)
    ptr = 0xa0000;
  while (ptr > 0x10000 )
    {
      ptr -= 16;
      pxe = (pxe_t *)phys_to_virt(ptr);
      if (memcmp(pxe->Signature, "!PXE", 4) == 0)
        {
          printf("found !PXE at %x...", ptr);
          if (checksum(pxe, sizeof(pxe_t)) != 0)
            {
              printf("invalid checksum\r\n...");
              continue;
            }
          if (ptr < get_free_base_memory())
            {
              printf("in free base memory!\r\n\r\n"
                     "WARNING: a valid !PXE structure was "
                     "found in an area of memory marked "
                     "as free!\r\n\r\n" );
              undi.pxe = pxe;
              pxe_dump();
              undi.pxe = NULL;
              printf("\r\nIgnoring and continuing, but this "
                     "may cause problems later!\r\n\r\n" );
              continue;
            }
          printf("ok\r\n");
          undi.pxe = pxe;
          return 1;
      }
  }
  printf("none found\r\n");
  ptr = 0;
  return 0;
}

/**************************************************************************
 * Low-level UNDI API call wrappers
 **************************************************************************/

/* Make a real-mode UNDI API call to the UNDI routine at
 * routine_seg:routine_off, passing in three uint16 parameters on the
 * real-mode stack.
 * Calls the assembler wrapper routine __undi_call.
 */

static inline PXENV_EXIT_t _undi_call(uint16_t routine_seg,
                                      uint16_t routine_off, uint16_t st0,
                                      uint16_t st1, uint16_t st2)
{
  PXENV_EXIT_t ret = PXENV_EXIT_FAILURE;

  undi.undi_call_info->routine.segment = routine_seg;
  undi.undi_call_info->routine.offset = routine_off;
  undi.undi_call_info->stack[0] = st0;
  undi.undi_call_info->stack[1] = st1;
  undi.undi_call_info->stack[2] = st2;
  printf("undi.undi_call_info=0x%x\r\n", undi.undi_call_info);
  ret = __undi_call(SEGMENT(undi.undi_call_info), OFFSET(undi.undi_call_info));

  /* UNDI API calls may rudely change the status of A20 and not
   * bother to restore it afterwards.  Intel is known to be
   * guilty of this.
   *
   * Note that we will return to this point even if A20 gets
   * screwed up by the UNDI driver, because Etherboot always
   * resides in an even megabyte of RAM.
   */

  gateA20(1);

  return ret;
}

/* Make a real-mode UNDI API call, passing in the opcode and the
 * seg:off address of a pxenv_structure on the real-mode stack.
 *
 * Two versions: undi_call() will automatically report any failure
 * codes, undi_call_silent() will not.
 */

static int undi_call_silent(uint16_t opcode)
{
  PXENV_EXIT_t pxenv_exit = PXENV_EXIT_FAILURE;
  printf("undi.pxs=0x%x, undi.pxe->EntryPointSP=0x%x:0x%x\r\n",
         undi.pxs,
         undi.pxe->EntryPointSP.segment,
         undi.pxe->EntryPointSP.offset);
  pxenv_exit = _undi_call(undi.pxe->EntryPointSP.segment,
                          undi.pxe->EntryPointSP.offset,
                          opcode, OFFSET(undi.pxs), SEGMENT(undi.pxs));

  /* Return 1 for success, to be consistent with other routines */
  return pxenv_exit == PXENV_EXIT_SUCCESS ? 1 : 0;
}

static int undi_call(uint16_t opcode)
{
  if (undi_call_silent(opcode))
    return 1;
  printf("UNDI API call 0x%x failed with status 0x%x\r\n",
         opcode, undi.pxs->Status);
  return 0;
}

/**************************************************************************
 * High-level PXE API call wrappers
 **************************************************************************/

static int pxe_get_cache_info(void)
{
  int ret;
  BOOTPLAYER bp;

  memset(&bp, 0, sizeof(bp));

  undi.pxs->cached_info.PacketType = PXENV_PACKET_TYPE_BINL_REPLY;
  undi.pxs->cached_info.BufferSize = sizeof(BOOTPLAYER);
  undi.pxs->cached_info.Buffer.segment = SEGMENT(&bp);
  undi.pxs->cached_info.Buffer.offset  = OFFSET(&bp);

  ret = undi_call(PXENV_GET_CACHED_INFO);

  if (undi.pxs->cached_info.BufferSize != sizeof(BOOTPLAYER))
    printf("strange copied: %d vs %d\n",
           undi.pxs->cached_info.BufferSize, sizeof(BOOTPLAYER));

  //arptable[ARP_CLIENT].ipaddr.s_addr  = bp.yip;
  //arptable[ARP_GATEWAY].ipaddr.s_addr = bp.gip;
  //arptable[ARP_SERVER].ipaddr.s_addr  = bp.sip;

  return ret;
}

static void __stop(void)
{
  printf("--HALTED--\n");
  while (1) { __asm {hlt} }
}

#if 0

#endif

/**
 * return 1 if active, 0 if not active
 */
int pxe_tftp_driver_active(void)
{
  return pxe_tftp_driver_active_value;
}

enum { PXE_TFTP_BUFFER_SIZE = 1432 }; // ATT: minimum 512 Bytes!
// some TFTP servers ignore the block size and always use 512 bytes and do
// not care about the negotiation, thus we do not really detect this
// we will just fail then. If you have such a broken TFTP server, change the
// above PXE_TFTP_BUFFER_SIZE to 512 or install a better TFTP server.
static char pxe_tftp_read_buf[PXE_TFTP_BUFFER_SIZE];
static int  pxe_tftp_negotiated_buffer_size;
static char pxe_tftp_read_filename[128]; // filename, needed for seeking
static int  pxe_tftp_read_buf_size; // size still available to read
static int  pxe_tftp_packet_size;   // size of current buffer
static int  pxe_tftp_read_buf_pos;  // current position in buf
   // i.e.: pxe_tftp_read_buf_pos + pxe_tftp_read_buf_size
   //       == pxe_tftp_packet_size
static int  pxe_tftp_read_last_packet; // bool, indicate last packet
static int  pxe_tftp_saved_filepos;    // position in file as a whole, in bytes
static int  pxe_tftp_buf_pos_in_file; // position in file where buf is

#if 0

/**
 * \param filename file to open
 * \return 1 success, 0 failed
 */
int pxe_tftp_open(const char *filename)
{
  int ret;

  undi.pxs->tftp_open.ServerIPAddress  = arptable[ARP_SERVER].ipaddr.s_addr;
  undi.pxs->tftp_open.GatewayIPAddress = arptable[ARP_GATEWAY].ipaddr.s_addr;

  if (strlen(filename) + 1 > sizeof(undi.pxs->tftp_open.FileName))
    {
      printf("%s: filename too large.\n", __func__);
      return 0;
    }

  memcpy(undi.pxs->tftp_open.FileName, filename, strlen(filename) + 1);
  memcpy(pxe_tftp_read_filename, filename, strlen(filename) + 1);
  undi.pxs->tftp_open.TFTPPort = htons(TFTP_PORT);
  undi.pxs->tftp_open.PacketSize = PXE_TFTP_BUFFER_SIZE;

  ret = undi_call(PXENV_TFTP_OPEN);

  if (ret)
    {
      pxe_tftp_negotiated_buffer_size = undi.pxs->tftp_open.PacketSize;

      pxe_tftp_read_buf_pos = 0;
      pxe_tftp_read_buf_size = 0;
      pxe_tftp_read_last_packet = 0;
      pxe_tftp_saved_filepos = 0;
      pxe_tftp_buf_pos_in_file = 0;
      pxe_tftp_packet_size = 0;
      pxe_tftp_opened = 1;
    }

  return ret;
}


/**
 * Read a packet of size PXE_TFTP_BUFFER_SIZE bytes to internal
 * buffer. Reset state variables.
 *
 * \return 1 success, 0 failure
 */
static int pxe_tftp_read_packet(void)
{
  int ret;

  pxe_tftp_buf_pos_in_file += pxe_tftp_packet_size;

  undi.pxs->tftp_read.Buffer.segment = SEGMENT(pxe_tftp_read_buf);
  undi.pxs->tftp_read.Buffer.offset  = OFFSET(pxe_tftp_read_buf);

  ret = undi_call(PXENV_TFTP_READ);

  pxe_tftp_read_buf_size    = undi.pxs->tftp_read.BufferSize;
  pxe_tftp_packet_size      = pxe_tftp_read_buf_size;
  pxe_tftp_read_buf_pos     = 0;
  pxe_tftp_read_last_packet = pxe_tftp_read_buf_size <
    pxe_tftp_negotiated_buffer_size;

  return ret;
}

/**
 * \param seekto    Position to seek file to, in bytes
 *                    This operation is expensive with backwards == 1
 * \param backwards Seek backwards, need to close and reopen the file
 *
 * \return 1 success, 0 failure
 */
static int pxe_tftp_read_seek(unsigned seekto, int backwards)
{
  if (backwards)
    {
       pxe_tftp_close();

       if (!pxe_tftp_open(pxe_tftp_read_filename))
         return 0;
    }

  if (!pxe_tftp_packet_size)
    pxe_tftp_read_packet();

  while (seekto > pxe_tftp_buf_pos_in_file + pxe_tftp_packet_size)
    {
      pxe_tftp_read_packet();
    }

  if (seekto > pxe_tftp_buf_pos_in_file + pxe_tftp_packet_size)
    return 0;

  pxe_tftp_saved_filepos = filepos = seekto;

  pxe_tftp_read_buf_pos = seekto - pxe_tftp_buf_pos_in_file;
  pxe_tftp_read_buf_size = pxe_tftp_packet_size - pxe_tftp_read_buf_pos;

  return 1;
}

/**
 * The API forces us to read packets, minimum size 512 Bytes,
 * but Grub requests smaller chunks, i.e. we need to do some buffering...
 *
 * \param buf       buffer to fill, must be min of 512 Bytes
 * \param len       size in bytes to read in
 *
 * \return bytes read, or -1 in failure
 */
int pxe_tftp_read(char *buf, int len)
{
  int bytes_read = 0;

  if (!pxe_tftp_opened)
    return -1;

  if (filepos < pxe_tftp_saved_filepos)
    {
      // someone did a seek... grmbl
      // see, if we have filepos still in the current buf
      if (filepos >= pxe_tftp_buf_pos_in_file)
        {
          // Yes, adjust variables
          int diff = pxe_tftp_saved_filepos - filepos;
          pxe_tftp_read_buf_pos -= diff;
          pxe_tftp_read_buf_size += diff;
          pxe_tftp_saved_filepos = filepos;
        }
      else
        {
          if (!pxe_tftp_read_seek(filepos, 1))
            return -1;
        }
    }
  else if (pxe_tftp_saved_filepos < filepos)
    {
      if (!pxe_tftp_read_seek(filepos, 0))
        return -1;
    }

  while (len)
    {
      if (pxe_tftp_read_buf_size == 0)
        {
          if (pxe_tftp_read_last_packet)
            return 0;

          if (!pxe_tftp_read_packet())
            return -1;
        }

      if (pxe_tftp_read_buf_size >= len)
        {
          memcpy(buf, &pxe_tftp_read_buf[pxe_tftp_read_buf_pos], len);
          bytes_read             += len;
          pxe_tftp_read_buf_size -= len;
          pxe_tftp_read_buf_pos  += len;
          pxe_tftp_saved_filepos += len;
          filepos                += len;


          if (pxe_tftp_read_buf_size < 0) {
            printf("ASSERT: pxe_tftp_read_buf_size(%d) < 0\n",
                pxe_tftp_read_buf_size);
            __stop();
          }
          if (pxe_tftp_read_buf_pos > pxe_tftp_negotiated_buffer_size) {
            printf("ASSERT: pxe_tftp_read_buf_pos(%d) >= pxe_tftp_negotiated_buffer_size\n", pxe_tftp_read_buf_pos);
            __stop();
          }
          return bytes_read;
        }

      memcpy(buf, &pxe_tftp_read_buf[pxe_tftp_read_buf_pos], pxe_tftp_read_buf_size);
      bytes_read             += pxe_tftp_read_buf_size;
      buf                    += pxe_tftp_read_buf_size;
      len                    -= pxe_tftp_read_buf_size;
      pxe_tftp_saved_filepos += pxe_tftp_read_buf_size;
      filepos                += pxe_tftp_read_buf_size;

      pxe_tftp_read_buf_pos += pxe_tftp_read_buf_size;
      if (pxe_tftp_read_buf_pos != pxe_tftp_negotiated_buffer_size) {
          printf("ASSERT: pxe_tftp_read_buf_pos != pxe_tftp_negotiated_buffer_size\n");
          __stop();
      }

      pxe_tftp_read_buf_size = 0;
    }

  return bytes_read;
}

/**
 * \return 1 on success, 0 on failure
 */
int pxe_tftp_close(void)
{
  int ret;

  if (!pxe_tftp_opened)
    return 1;

  ret = undi_call(PXENV_TFTP_CLOSE);
  pxe_tftp_opened = 0;

  return ret;
}

/**
 * \param filename   path to get size of
 * \return -1 on error, size in bytes otherwise
 */
int pxe_tftp_get_file_size(const char *filename)
{
  int ret, size;

  undi.pxs->tftp_get_fsize.ServerIPAddress = arptable[ARP_SERVER].ipaddr.s_addr;
  undi.pxs->tftp_get_fsize.GatewayIPAddress = 0;
  memcpy(undi.pxs->tftp_get_fsize.FileName, filename, strlen(filename) + 1);
  ret = undi_call(PXENV_TFTP_GET_FSIZE);

  if (ret) // file size extension supported by TFTP server
    return undi.pxs->tftp_get_fsize.FileSize;

  if (!pxe_tftp_open(filename))
    return -1;

  printf("Requesting %s:\n"
         "TFTP server at %@ does not support TSIZE extension, upgrade!\n",
         filename, arptable[ARP_SERVER].ipaddr);

  while (!pxe_tftp_read_last_packet)
    if (!pxe_tftp_read_packet())
      return -1;

  size = pxe_tftp_buf_pos_in_file + pxe_tftp_read_buf_size;

  __stop();

  if (!pxe_tftp_close())
    return -1;

  return size;
}

static int pxe_tftp_poll(struct nic *nic_struct)
{
  printf("%s called\n", __func__);
  __stop();
  return 0;
}

static void pxe_tftp_transmit(struct nic *nic_struct,
                              const char *d,    /* Destination */
                              unsigned int t,   /* Type */
                              unsigned int s,   /* size */
                              const char *p)    /* Packet */
{
  printf("%s called\n", __func__);
  __stop();
}

#endif

static int eb_pxenv_stop_base(void)
{
  int success = 0;

  DBG("PXENV_STOP_BASE => (void)\r\n");
  success = undi_call ( PXENV_STOP_BASE );
  DBG("PXENV_STOP_BASE <= Status=%s\r\n", UNDI_STATUS(undi.pxs));
  return success;
}

static void pxe_tftp_disable(void) // (struct dev *dev)
{
  eb_pxenv_stop_base();
  free_base_mem_data();
}


/* ======================= */

static void pxe_tftp_disable(void);

//static
int pxe_probe(void) // struct dev *dev, struct pci_device *pci)
{
  //struct nic *nic_struct = (struct nic *)dev;

  /* Zero out global undi structure */
  memset(&undi, 0, sizeof(undi));

  /* Find the BIOS' $PnP structure */
  if (!hunt_pnp_bios())
    {
      printf ( "No PnP BIOS found; aborting\r\n" );
      return 0;
    }

  /* Allocate base memory data structures */
  if (!allocate_base_mem_data())
    return 0;

  if (!hunt_pixie())
    {
      printf("hunting for PXE image failed\r\n");
      return 0;
    }

#if 0
  if (!pxe_get_cache_info())
    {
      printf("PXE: Could not get boot info\r\n");
      return 0;
    }
#endif

  pxe_tftp_driver_active_value = 1;

  undi_call(PXENV_UNDI_CLEANUP);

  //dev->disable         = pxe_tftp_disable;
  //nic_struct->poll     = pxe_tftp_poll;
  //nic_struct->transmit = pxe_tftp_transmit;

  return 1;
}

#if 0

/* UNDI driver states that it is suitable for any PCI NIC (i.e. any
 * PCI device of class PCI_CLASS_NETWORK_ETHERNET).  If there are any
 * obscure UNDI NICs that have the incorrect PCI class, add them to
 * this list.
 */
static struct pci_id pxe_nics[] = {
        PCI_ROM(0x0000, 0x0000, "pxe", "PXE adaptor")
};

struct pci_driver pxe_driver = {
        .type     = NIC_DRIVER,
        .name     = "PXE",
        .probe    = pxe_probe,
        .ids      = pxe_nics,
        .id_count = sizeof(pxe_nics)/sizeof(pxe_nics[1]),
        .class    = PCI_CLASS_NETWORK_ETHERNET,
};

#endif
