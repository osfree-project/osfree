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

#define   BOOT  ((struct hpfs_boot_block *)(FSYS_BUF))
#define   SUPER ((struct hpfs_super_block *)(FSYS_BUF + 512))
#define   FNODE ((struct fnode *)(FSYS_BUF + 1024))
#define   DNODE ((struct dnode *)(FSYS_BUF + 1536))
#define   ANODE ((struct anode *)(DNODE + 2048))

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
    return 0;

  return 1;
}

/* read len bytes from filepos and return number of bytes read */
int
hpfs_read (char *buf, int len)
{
  struct bplus_header *btree, *btree0;
  unsigned sec, length, l;
  char *pos;
  int i, n, boff;

  btree = (struct bplus_header *)(&(FNODE->btree));
  btree0 = btree;
  length = 0; pos = buf;

  for (;;)
  {
    n = btree->n_used_nodes;

    if (btree->internal) /* internal b+-tree nodes inside an fnode */
    {
      for (i = 0; i < n; i++)
        if ((btree->u.internal[i].file_secno << 9) > *pfilepos + length)
          break;
    }
    else /* leaves only */
    {
      for (i = 0; i < n; i++)
      {
        /* skip extents until filepos become inside the extent */
        if ((btree->u.external[i].file_secno + btree->u.external[i].length) << 9 <= *pfilepos + length)
          continue;

        /* start and size of an extent */
        /* starting disk sector                         */
        sec  = btree->u.external[i].disk_secno;
        /* the number of bytes in extent before filepos */
        boff = *pfilepos - (btree->u.external[i].file_secno << 9);
        /* amount to read inside an extent              */
        l    = (btree->u.external[i].length << 9) - boff;

        if (len < l)
          l = len;

        if (!(*pdevread)(sec, boff, l, pos))
          return length;

        length += l;
        pos    += l;

        if (length >= len)
          return length;
      }
    }

    /* navigate b+ tree up/down */
    if (i < n) /* one level down */
      sec = btree->u.internal[i].down;
    else       /* one level up */
      sec = ANODE->up;

    if (!(*pdevread)(sec, 0, sizeof(struct anode), (char *)ANODE)
        || ANODE->magic != ANODE_MAGIC)
      return 0;

    btree = (struct bplus_header *)(&ANODE->btree);
  }

  return length;
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

  while (*dirname == '/') /* loop through path components */
  {
    if (FNODE->btree.internal) /* internal nodes */
    {
      *perrnum = ERR_FILE_NOT_FOUND;
      return 0;
    }
    /* leaf node, read dnode */
    sec = FNODE->btree.u.external[0].disk_secno;
    if (!(*pdevread)(sec, 0, sizeof(struct dnode), (char *)DNODE)
        || (DNODE->magic != DNODE_MAGIC))
      return 0;
    /* root dnode is read */
    /* extract next level subdir name     */
    while (*dirname == '/') dirname++; /* skip leading '/' */
    p = dirname;
    /* skip to the next '/' */
    while (*p && *p != '/' && !((*pgrub_isspace)(*p))) p++;
    (*pgrub_memmove)(fn, dirname, p - dirname); fn[p - dirname] = '\0';
    dirname = p; /* now dirname points to the path rest */

    dirent = (struct hpfs_dirent *)(DNODE->dirent);

    if (dirent->first) /* skip '.' self drectory record */
      dirent = (struct hpfs_dirent *)((char *)dirent + dirent->length);

    for (;;) /* loop through directory entries */
    {
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
        if (*dirname == '/')
          /* goto next_path_component; */
          break;
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
  go_down:
          /* round up to the next dword boundary */
          down = *(unsigned *)((char *)dirent + dirent->length - 4);
          if (!(*pdevread)(down, 0, sizeof(struct dnode), (char *)DNODE)
              || (DNODE->magic != DNODE_MAGIC))
            return 0;
          dirent = (struct hpfs_dirent *)(DNODE->dirent);
          continue;
        }
      }
      if (n > 0)
      {
        if (!(dirent->last))
        {
          /* next dirent */
          dirent = (struct hpfs_dirent *)((char *)dirent + dirent->length);
          continue; /* go to the next dirent in the same dirblk */
        }
        else
          goto go_down;
      }
    }
  }

  return 1;
}

int
hpfs_embed (int *start_sector, int needed_sectors)
{
  if (needed_sectors > 15
      || !(*pdevread) (16, 0, sizeof(struct hpfs_super_block), (char *)SUPER)
      || (SUPER->magic != SB_MAGIC)
      || (SUPER->version != 2))
    return 0;

  *start_sector = 1;
  return 1;
}

#endif /* FSYS_HPFS */
