/*
 *  binfmt_mz.c
 *
 *  Copyright (C) 1995 Wayne Meissner
 *  Copyright (C) 1996  Martin von Löwis
 */

/*
 * Usage: insmod binfmt_mz.o [loader=<path to mzloader>]
 */

/* Check for 1.2.13 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < (1*65536 + 3*256)
#define OLD_LINUX
#endif

#include <linux/config.h>
#ifdef CONFIG_MODVERSIONS
#define MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/malloc.h>
#include <linux/binfmts.h>

#ifdef MZLOADER
	static char *loader = MZLOADER;
#else
	static char *loader = "/usr/sbin/mzloader";
#endif

static char * extensions[] = {
	"bat", "com", "exe", NULL
};

/* Return true if the binary is supported by dosemu or Wine */

static int is_dos_file(struct linux_binprm *bprm)
{
	int i;
	char *ext;
	if ((bprm->buf[0] == 'M') && (bprm->buf[1] == 'Z'))
		return 1;
	/* check for the extension (.bat, .com) */
	ext = strchr (bprm->filename, '.');
	/* if no extension, then who knows what it could be */
	if (!ext)
		return 0;
	ext++;

	/* lastly, check if its a .bat, .com, etc */
	for (i = 0; extensions[i] != NULL; i++) 
		if (strcmp(ext, extensions[i]) == 0) 
			return 1;
	return 0;
}

#ifdef OLD_LINUX
char kernel_version[] = UTS_RELEASE;

static int do_load_mz_binary(struct linux_binprm *bprm,struct pt_regs *regs)
{
	if(!is_dos_file(brprm))
		return -ENOEXEC;

	/* I believe there is a leak of argument pages hidden in that call.
	   Since I do not have 1.2.13 anymore, and since this is soon obsolete
	   code, I'm not going to deal with it. MvL */
	return do_execve (loader,
		(char **)regs->ecx,
		(char **)regs->edx, regs);
}

#else /* New kernel (1.3.71 and above */

static int do_load_mz_binary(struct linux_binprm *bprm,struct pt_regs *regs)
{
	int retval;
	char *mzloader;
	mzloader=loader;

	if(!is_dos_file(bprm))
		return -ENOEXEC;
					 
	iput(bprm->inode);
	bprm->dont_iput=1;

#if 0
	/* This prepends the emulator name before the command line.
	   mzloader does not need it */
	bprm->p = copy_strings(1, &loader, bprm->page, bprm->p, 2);
	bprm->argc++;
	if (!bprm->p) 
		return -E2BIG;
#endif
	retval = open_namei(mzloader, 0, 0, &bprm->inode, NULL);
	if (retval)
		return retval;
	bprm->dont_iput=0;

	/* This avoids recursion, as is_dos_file checks for the filename, too.
	   Just changing the pointer is OK, since the binfmt system will not
	   mess with it. The only effect is that the command in the process table
       is now the emulator, not the program image */

	bprm->filename = mzloader;

	retval=prepare_binprm(bprm);
	if(retval<0)
		return retval;
	return search_binary_handler(bprm,regs);
}
#endif

static int load_mz_binary(struct linux_binprm *bprm,struct pt_regs *regs)
{
	int retval;
	MOD_INC_USE_COUNT;
	retval = do_load_mz_binary(bprm,regs);
	MOD_DEC_USE_COUNT;
	return retval;
}

static struct linux_binfmt mz_format = {
#ifndef MODULE
	NULL, 0, load_mz_binary, NULL, NULL
#else
	NULL, &mod_use_count_, load_mz_binary, NULL, NULL
#endif
};

#ifdef MODULE
int init_module(void)
{
	register_binfmt(&mz_format);
	return 0;
}

void cleanup_module( void) {
	unregister_binfmt(&mz_format);
}
#endif
