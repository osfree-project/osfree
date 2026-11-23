#include <stdio.h>
#include <string.h>
#include <scentry.h>
#include "emitlib.h"

void emitClassExports(FILE *fp, const char *name)
{
    if (somtgetgatt("msvc")) {
        somtokfprintf(fp, "   %sCClassData\n", name);
        somtokfprintf(fp, "   %sClassData\n",  name);
        somtokfprintf(fp, "   %sNewClass\n",   name);
    } else {
        somtokfprintf(fp, "   _%sCClassData\n",  name);
        somtokfprintf(fp, "   _%sClassData\n",   name);
        somtokfprintf(fp, "   _%sNewClass@8\n",  name);
    }
}

#define EXT "def"

FILE * emit(char *file, Entry * cls, Stab * stab)
{
    FILE *fp;
    char *savedCopyright = NULL;
    AttList *a;
    int   c;
    int   len;
    char  buf[260];
    if (cls->type == SOMTClassE) {

        /* hide copyright */
        for (a = cls->atts; a; a = a->next)
            if (a->name && !strcmp(a->name, "somcopyright")) {
                savedCopyright = a->name;
                a->name = "";
                break;
            }

        fp = somtopenEmitFile(file, EXT);

        /* restore copyright */
        if (savedCopyright)
            a->name = savedCopyright;

        /* DEF header */
        if (!(somtg_f.append)) {
            if (somtgetgatt("msvc")) {
                somtokfprintf(fp, "LIBRARY %s INITINSTANCE\n", somtfileStem);
                somtokfprintf(fp, "DESCRIPTION '%s Class Library'\n", cls->name);
                somtokfprintf(fp, "PROTMODE\n");
                somtokfprintf(fp, "DATA MULTIPLE NONSHARED LOADONCALL\n");
            } else {
                somtokfprintf(fp, "LIBRARY %s\n", somtfileStem);
            }
            somtokfprintf(fp, "EXPORTS\n");
        }

        /* Class exports */
        emitClassExports(fp, cls->name);

        /* metaclass exports */
        if (somtimplicitMeta(cls)) {
            sprintf(buf, "M_%s", cls->name);
            emitClassExports(fp, buf);
        }


        return fp;
    }

    if (cls->type == SOMTModuleE) {
        fp = somtopenEmitFile(file, EXT);
        somtokfprintf(fp, "LIBRARY %s INITINSTANCE\n", somtfileStem);
        somtokfprintf(fp, "DESCRIPTION '%s Class Library'\n", cls->name);
        somtokfprintf(fp, "PROTMODE\n");
        somtokfprintf(fp, "DATA MULTIPLE NONSHARED LOADONCALL\n");
        somtokfprintf(fp, "EXPORTS\n");
        return fp;
    }

    return NULL;
}
