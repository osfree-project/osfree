/*
	test IO95 lib

	prerequisites:
	1) file "longfilename1" must exist
	1) file "longfilename2" must not exist
	3) subdir "longpathname" must not exist

	What is done:
	1) "longfilename1" is opened
	2) "longfilename2" is created
	3) "longpathname\longerpath" is created
	4) SFN("longpathname\longerpath") is displayed
	5) chdir("longpathname\longerpath")
	6) Display fullpath(.)
	7) move longfilename2 into "."
	8) display ".", "..", "longfilenameX"
*/

#include <stdio.h>
#include <string.h>

#define USE_IO95
#include "io95.h"
#include "find95.h"
#include "dir95.h"

main(void)
{	int fd, i, chdOK;
	char buf[2048];
	struct ffblk ff;
	DIR *dirp;
	struct dirent95 *dep;
	long cnt;

	fd = open("longfilename1", O_SNOOP | O_TEXT);
	if(fd == -1)
		puts("Failed to open filename1");
	else close(fd);

	fd = open("longfilename2", O_CREAT | O_TRUNC | O_TEXT | O_RDWR, S_IREAD | S_IWRITE);
	if(fd == -1)
		puts("Failed to create filename2");
	else {
		for(i = 0; ++i < 1000;) {
			sprintf(buf, "line %d\n", i);
			write(fd, buf, strlen(buf));
		}
		close(fd);
	}

	if(mkdir("longpathname"))
		puts("Cannot create longpathname");
	else if(mkdir("longpathname\\longerpath"))
		puts("Cannot create longerpath");
	if(lfn2sfn("longpathname\\longerpath", buf))
		puts("Failed to get SFN(longpathname\\longerpath)");
	else printf("SFN(longpathname\\longerpath) = \"%s\"\n", buf);
	if(chdir("longpathname\\longerpath")) {
		puts("chdir(longpathname\\longerpath) failed");
		chdOK = 0;
	}
	else chdOK = 1;
	if(fullpath(buf, ".", sizeof(buf)) == NULL)
		puts("fullpath(.) failed");
	else printf("Current fullpath = \"%s\"\n", buf);

	if(rename("..\\..\\longfilename2", "longfilenameX"))
		puts("Failed to move longfilename here");

	puts("Entries found with findfirst/next loop:");
	printf("%14s|%14s|%8s|%8s|%8s|%16s|%s\n", "DOS SFN", "LFN SFN", "DOS ATTR", "LFN ATTR", "DOS SIZE", "LFN SIZE", "LFN NAME");
	if(findfirst("*.*", &ff, -1) == 0) do {
		printf("%14s|%14s|%08lx|%08lx|%08lx|%08lx%08lx|%s\n"
		 , ff.ff_name, ff.ff_95.ff_shortname
		 , (long)ff.ff_attr, (long)ff.ff_95.ff_attr95
		 , ff.ff_size, ff.ff_95.ff_hisize, ff.ff_95.ff_losize
		 , ff.ff_95.ff_longname);
	} while(findnext(&ff) == 0);

	puts("Entries retrieved with readdir():");
	if((dirp = opendir(".")) == NULL)
		puts("Failed to opendir(.)");
	else {
		while((dep = readdir(dirp)) != NULL) {
			printf("%ld. = ", telldir(dirp));
			puts(dep->d_name);
		}
		closedir(dirp);
	}

	if(chdOK)
		chdir("..\\..");

	return 0;
}
