#include <rhbopt.h>
#include <somossvr.h>

#if defined(_WIN32) && !defined(_CONSOLE)
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev,LPSTR lpCmdLine,INT nCmdShow)
{
	char buf[256];
	if (GetModuleFileName(hInstance,buf,sizeof(buf)))
	{
		char *p=lpCmdLine;
		char *argv[3]={buf,NULL,NULL};
		int argc=1;
		if (p) while (*p) p++;
		if (p) if (*p) argv[argc++]=p;
		return SOMOSSVR_main(argc,argv);
	}

	return 1;
}
#else
int main(int argc,char **argv)
{
	return SOMOSSVR_main(argc,argv);
}
#endif
