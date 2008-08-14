 /*********************************************************************
 * Here everything starts. This is the main function of the           *
 * OS/2 Server.                                                       *
 **********************************************************************/
int main(int argc, const char **argv)
{
    int err, ret = 1;
    void *addr;
    int  off = 0;
    const char *servername;
    const char *filename;
    char line[MAXLENGTH]; // here I store the lines I read
    int len=0;          // length of returned line
    char c;
    l4_size_t size;    // size of config.sys

    printf("OS/2 Server started\n");

    init_options();

    len=0;
    while (off<size)
    {
      c=((char *)addr+off)[0];
      line[len]=c;
      if (c=='\n')
      {
        line[len]='\0';
        if(!parse(line,len)) error("parse: an error occured\n");
        len=0;
      } else {
        len++;
      }
      off++;
    }


    printf("PROTSHELL=%s\n", options.protshell);

    cleanup();

    return ret;
}
