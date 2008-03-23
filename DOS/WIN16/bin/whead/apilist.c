/*----------------------------------------------------------------------------- 
 *
 *----------------------------------------------------------------------------*/

#define	DEBUG

#ifdef	DEBUG
#include	<stdio.h>
#endif

#include	<string.h>
#include	"apilist.h"

char *
cnvrt_ordinal(module, ordinal)
char *module;
int ordinal;
{
	register int j;
	register int i;
	static char buffer[128];

	for (i = 0; api_list[i] != 0; i++) {
		if (strcmp(api_list[i][0].name, module))
			continue;
		for (j = 0; api_list[i][j].ordinal != -1; j++)
			if (api_list[i][j].ordinal == ordinal) {
				sprintf(buffer, "%s.%s", module, api_list[i][j].name);
				return (buffer);
			}
	}
	sprintf(buffer, "%s.%d", module, ordinal);
	return (buffer);
}

#ifdef	DEBUG
main()
{
	char mod[32];
	int ord;

	while(1) {
		printf("\nModule name: ");
		scanf("%s", mod);
		printf("Ordinal number:");
		scanf("%d", &ord);
		printf("%s\n", cnvrt_ordinal(mod, ord));
	}
}
#endif

