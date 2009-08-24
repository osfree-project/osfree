//Call ldirNT.exe using a pipe
//
// Work in progress

#include <stdio.h>
#include <string.h>

void main(int argc, char* argv[])
{   FILE *fd;
    char temp[1024]="ldirNT.exe ";
    int i;
    
    for (i=1; i<argc; i++)					//Copy all command line arguments into temp
    {	strcat(temp, " ");
    	strcat(temp, argv[i]);
    }
   	
    if ((fd=_popen(temp,"rt"))==NULL)				//Open ldirNT using a pipe
    {	printf("Could not open ldirNT.exe\n");
    	return -1;
    }
    while(fgets(temp,sizeof(temp),fd))				//Print the results
    {	printf("%s",temp);
    }
    _pclose(fd);
}