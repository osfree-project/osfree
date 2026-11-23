#include <stdio.h>
#include <string.h>
#include <scentry.h>
#include "emitlib.h"

static char *g_dllname = NULL;

#define EXT "exp"

void emitClassExports(FILE *fp, const char *name)
{
    somtokfprintf(fp, "   %sCClassData\n", name);
    somtokfprintf(fp, "   %sClassData\n",  name);
    somtokfprintf(fp, "   %sNewClass\n",   name);
}

FILE * emit(char *file, Entry * cls, Stab * stab)
{
    FILE *fp;
    char *savedCopyright = NULL;
    AttList *a;
    FILE *tmp;
    int   c;
    int   len;
    char  buf[260];


    if (cls->type == SOMTClassE) {

        /* hide copyright attribute */
        for (a = cls->atts; a; a = a->next)
            if (a->name && !strcmp(a->name, "somcopyright")) {
                savedCopyright = a->name;
                a->name = "";
                break;
            }

        fp = somtopenEmitFile(file, EXT);

        /* restore copyright */
        if (savedCopyright) a->name = savedCopyright;

        if (g_dllname == NULL
            && somtgetatt(cls, "dllname") != NULL)
        {
            g_dllname = somtgetatt(cls, "dllname");
            len = g_dllname ? strlen(g_dllname) : 0;

            if (len != 0
                && g_dllname[0]       == '"'
                && g_dllname[len - 1] == '"') {
                g_dllname[len - 1] = '\0';
                g_dllname++;
            }

            if (g_dllname != NULL && *g_dllname != '\0') {
                tmp = tmpfile();
                somtokfprintf(tmp, "#! %s\n", g_dllname);

                somtfclose(fp);

                fp = fopen(somtoutPath, "r");
                while ((c = fgetc(fp)) != EOF)
                {
                    fputc(c, tmp);
                }
                somtfclose(fp);

                remove(somtoutPath);
                fp = fopen(somtoutPath, "a");

                rewind(tmp);
                while ((c = fgetc(tmp)) != EOF)
                    fputc(c, fp);
            }
        }


        emitClassExports(fp, cls->name);

        if (somtimplicitMeta(cls)) {
            sprintf(buf, "M_%s", cls->name);
            emitClassExports(fp, buf);
        }

        return fp;
    }

    if (cls->type == SOMTModuleE) {
        fp = somtopenEmitFile(file, EXT);
        return fp;
    }

    return NULL;
}
