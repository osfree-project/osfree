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

/* OS/2 character sort order (without national ones) */
static char order[] = "!#$%&'()+,-.0123456789;=@abcdefghijklmnopqrstuvwxyz[]^`{}~_";

static int
strocmp (char *s1, char *s2)
{
  char *p = s1;
  char *q = s2;
  int u, v;

  while (*p || *q)
  {
    u = grub_index(*p, order);
    v = grub_index(*q, order);
    if (u < v)
      return -1;
    if (u > v)
      return 1;
    p++;
    q++;
  }

  return 0;
}

int
hpfs_mount (void)
{
  char *c = (char *)BOOT->sig_hpfs;

  if  ((*pcurrent_slice != 0x07)
      || !(*pdevread) (0, 0, sizeof(struct hpfs_boot_block), (char *)BOOT)
      || c[0] != 'H' || c[1] != 'P' || c[2] != 'F' || c[3] !=  'S'
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
  unsigned sec;
  int l, length, read;
  char *pos;
  int i, n, boff, b, p, sl;

  btree = (struct bplus_header *)(&(FNODE->btree));
  btree0 = btree;
  length = len;
  read = 0;
  pos = buf;

  for (;;)
  {
    n = btree->n_used_nodes;

    if (btree->internal) /* internal b+-tree nodes inside an fnode */
    {
      for (i = 0; i < n; i++)
        if ((btree->u.internal[i].file_secno << 9) > *pfilepos)
          break;
    }
    else /* leaves only */
    {
      for (i = 0; i < n; i++)
      {
        /* skip extents until filepos become inside the extent */
        if ((btree->u.external[i].file_secno + btree->u.external[i].length) << 9 <= *pfilepos)
          continue;

        /* start and size of an extent */
        /* starting disk sector                         */
        sec  = btree->u.external[i].disk_secno;
        /* the number of bytes in extent before filepos */

        /* distance of filepos from extent start */
        l = *pfilepos - (btree->u.external[i].file_secno << 9);

        b = (l >> 9) << 9;
        boff = l - b;

        /* length until an extent end */
        l = (btree->u.external[i].length << 9) - l;
        sec += (b >> 9);

        if (length < l)
          l = length;

        if (!(*pdevread)(sec, boff, l, pos))
           return read;

        length -= l;
        read   += l;
        pos    += l;
        *pfilepos += l;

        if (!length)
          return read;
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

  return read;
}

int
hpfs_dir (char *dirname)
{
  unsigned sec;
  static struct hpfs_dirent *dirent;
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

  if (*dirname != '/')
    return 0;

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
      n = strocmp (fn, filename);
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
