/*  HPFS uFSD implementation
 *  (c) osFree project
 *  author Valery Sedletski,
 *  _valerius (at) mail (dot) ru
 *  2009, Oct 26
 */

#if defined(fsys_hpfs) || defined(FSYS_HPFS)

int print_possibilities = 0;

#include "shared.h"
#include "filesys.h"
#include "hpfs.h"

#include "fsys.h"
#include "misc.h"
#include "fsd.h"

#define   BOOT  ((struct hpfs_boot_block *)FSYS_BUF)
#define   SUPER ((struct hpfs_super_block *)(FSYS_BUF + 512))
#define   FNODE ((struct fnode *)(FSYS_BUF + 1024))
#define   DNODE ((struct dnode *)(FSYS_BUF + 1536))

int
strncmp (char *s1, char *s2, int n)
{
  char *p = s1;
  char *q = s2;
  int  i = n;

  while (i)
  {
    if (*p < *q)
      return -1;
    else
    if (*p > *q)
      return 1;
    else
    {
      p++; q++; i--;
    }
  }

  return 0;
}

int
hpfs_mount (void)
{
  if  ((*pcurrent_slice != 0x07)
      || !(*pdevread) (0, 0, sizeof(struct hpfs_boot_block), (char *)BOOT)
      || (strncmp ((char *)BOOT->sig_hpfs, "HPFS", 4))
      || (BOOT->sig_28h != 0x28)
      || !(*pdevread) (16, 0, sizeof(struct hpfs_super_block), (char *)SUPER)
      || (SUPER->magic != SB_MAGIC)
      || (SUPER->version != 2))
  {
    return 0;
  }

  return 1;
}

int
hpfs_read (char *buf, int len)
{

  return 1;
}

int
hpfs_dir (char *dirname)
{
  unsigned sec;
  struct hpfs_dirent *dirent;
  dnode_secno down;
  char fn[0x100];
  char filename[0x100];
  char *p;
  int  n;

  /* read the root fnode */
  if (!(*pdevread)(SUPER->root, 0, sizeof(struct fnode), (char *)FNODE)
      || (FNODE->magic != FNODE_MAGIC))
  {
    *perrnum = ERR_FILE_NOT_FOUND;
    return 0;
  }
  while (*dirname) /* loop through path components */
  {
  next_path_component:
    if (FNODE->btree.internal) /* internal nodes */
    {
      *perrnum = ERR_FILE_NOT_FOUND;
      return 0;
    }
    /* leaf node, read dnode */
    sec = FNODE->u.external[0].file_secno; /* 1st dword after [0] */
    if (!(*pdevread)(sec, 0, sizeof(struct dnode), (char *)DNODE)
        || (DNODE->magic != DNODE_MAGIC))
      return 0;
    /* root dnode is read */
  next_dnode:
    /* extract next level subdir name     */
    if (*dirname == '/') dirname++; /* skip '/' */
    p = dirname;
    while (*p && *p != '/') p++; /* skip to the next '/' */
    (*pgrub_memmove)(fn, dirname, p - dirname); fn[p - dirname] = '\0';
    dirname = p; /* now dirname points to the path rest */
    dirent = (struct hpfs_dirent *)(DNODE->dirent);
    for (;;) /* loop through directory entries */
    {
      dirent = (struct hpfs_dirent *)((char *)dirent + dirent->length);

      (*pgrub_memmove) (filename, dirent->name, dirent->namelen);
      filename[dirent->namelen] = '\0';
      n = (*pgrub_strcmp) (fn, filename);
      if (!n)
      {
        /* name found */
        if (!(*pdevread) (dirent->fnode, 0, sizeof(struct fnode), (char *)FNODE)
            || (FNODE->magic != FNODE_MAGIC))
        {
           *perrnum = ERR_FILE_NOT_FOUND;
           return 0;
        }
        if (*dirname)
          goto next_path_component;
        else
        {
          *pfilepos = 0;
          *pfilemax = dirent->file_size;
          break;
        }
      }
      if (n < 0)
      {
        if (!(dirent->down))
        {
          *perrnum = ERR_FILE_NOT_FOUND;
          return 0;
        }
        else
        {
          /* round up to the next dword boundary */
          down = (((unsigned)dirent->name + dirent->namelen + 3) & ~3);
          if (!(*pdevread)(down, 0, sizeof(struct dnode), (char *)DNODE)
              || (DNODE->magic != DNODE_MAGIC))
            return 0;
          goto next_dnode;
        }
      }
      if (n > 0 && !(dirent->last))
        continue; /* go to the next dirent in the same dirblk */
    }
  }

  return 1;
}

int
hpfs_embed (int *start_sector, int needed_sectors)
{
  if (needed_sectors > 16
      || !(*pdevread) (16, 0, sizeof(struct hpfs_super_block), (char *)SUPER)
      || (SUPER->magic != SB_MAGIC)
      || (SUPER->version != 2))
    return 0;

  *start_sector = 1;
  return 1;
}

#endif /* FSYS_HPFS */
