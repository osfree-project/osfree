/*! bind.c - FamilyAPI Binder
 *
 *  (c) osFree Project 2024, <https://www.osfree.org>
 *  for licence see licence.txt in root directory, or project website
 *
 *  @author Yuri Prokushev <yuri.prokushev@gmail.com>
 *
 *  Uses the following osFree libraries:
 *    - ccl.h       Common Collection Library (HVECTOR, HSTRSET)
 *    - omf.h       OMF record parsing and generation
 *    - lib.h       OMF library (.LIB) reader
 *    - newexe.h    New Executable (NE) reader and binder
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef __UNIX__
#include <sys/stat.h>
#else
#include <direct.h>
#endif

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "newexe.h"
#include "omf.h"
#include "lib.h"

/*! @file bind.c
 *  @brief FamilyAPI Binder implementation.
 *
 *  The binder performs three major steps:
 *    1. Reads the dynamic-link entry points from an OS/2 NE
 *       executable and builds a vector of imported functions.
 *    2. Generates a temporary OMF object file (tmp.obj) that
 *       declares those functions as EXTDEF records, then invokes
 *       WLINK to link the object with API.LIB and any additional
 *       libraries.
 *    3. Merges the DOS real-mode stub produced by WLINK with the
 *       original OS/2 protected-mode NE image into a single file
 *       that runs in either mode.
 *
 *  The imported-function list and the module-name list are kept in
 *  CCL vectors (HVECTOR). No hand-rolled containers are used.
 */

/*! @brief One imported function entry.
 *
 *  Stored by value in a CCL vector; the vector is not a linked
 *  list, so no @c next field is present.
 */
typedef struct _apientry {
    char mod[9];    /*!< Module name (NUL padded, max 8 chars). */
    WORD ord;       /*!< Function ordinal (informational). */
    char func[21];  /*!< Function name (NUL padded, max 20 chars). */
} apientry;

/*! @brief One module-name entry.
 *
 *  The NE module reference table gives at most 255-char names, but
 *  in practice they are short. A fixed buffer avoids per-element
 *  dynamic allocation; the vector stores these structs by value.
 */
typedef struct _modname {
    char name[256]; /*!< NUL-terminated module name. */
} modname;

/*! @brief Binder options.
 *
 *  Populated from the command line before any work begins.
 */
typedef struct _opts {
    int quiet;                  /*!< Quiet mode (no banner). */
    int logo;                   /*!< Show logo banner. */
    char outfile[_MAX_PATH];    /*!< Output filename. */
    char infile[_MAX_PATH];     /*!< Input filename. */
    char mapfile[_MAX_PATH];    /*!< MAP filename. */
    char libpath[_MAX_PATH];    /*!< Additional library search path. */
    int map;                    /*!< Generate MAP file for DOS stub. */
    int dosformat;              /*!< Options in DOS format. */
    int DoscallsLIB;            /*!< Use DOSCALLS.LIB instead of OS2.LIB. */
    int MouAPI;                 /*!< Mouse API used. */
    int KbdAPI;                 /*!< Keyboard API used. */
    int VioAPI;                 /*!< Video API used. */
    int DLLAPI;                 /*!< DLL API used. */
} opts;

/*! @brief LNAMES index assignments for tmp.obj.
 *
 *  The order matches the sequence of OmfWriteLnames calls in
 *  generate_imptable. Adding or reordering names requires updating
 *  this enum accordingly.
 */
enum {
    LNAME_EMPTY  = 1,   /*!< "" (empty, required at index 1). */
    LNAME_CODE   = 2,   /*!< "CODE". */
    LNAME_DATA   = 3,   /*!< "DATA". */
    LNAME_BSS    = 4,   /*!< "BSS". */
    LNAME_TLS    = 5,   /*!< "TLS". */
    LNAME_DGROUP = 6,   /*!< "DGROUP". */
    LNAME_TEXT   = 7,   /*!< "_TEXT". */
    LNAME_CONST  = 8,   /*!< "CONST". */
    LNAME_CONST2 = 9,   /*!< "CONST2". */
    LNAME_DATA_  = 10   /*!< "_DATA". */
};

/*! @brief SEGDEF indices for tmp.obj. */
enum {
    SEGDEF_TEXT   = 1,  /*!< _TEXT segment. */
    SEGDEF_CONST  = 2,  /*!< CONST segment. */
    SEGDEF_CONST2 = 3,  /*!< CONST2 segment. */
    SEGDEF_DATA   = 4   /*!< _DATA segment (import table). */
};

HVECTOR hvApi = NULLHANDLE;     /*!< Vector of apientry. */
opts options;                   /*!< Global binder options. */

/* Forward declarations of file-local helpers. */
void printhlp(void);
int check_environment(void);
int addtolist(char * mod, char * func);
int bind(char * fname);
int searchlib(char * libname, char * fullpath);
char * addpathsep(char * buf);
char * mktmpdir(char *tmpdir);
void generate_imptable(void);
void generate_lnk(void);

/* ------------------------------------------------------------------ */
/* Path utilities                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Append a path separator to a buffer.
 *
 *  @param[in,out] buf  Buffer to append to. Not NULL.
 *
 *  @return @p buf, for convenient chaining.
 */
char * addpathsep(char * buf)
{
#if defined(_WIN32) || defined(__OS2__) || defined(__DOS__)
    return strncat(buf, "\\", 1);
#else
    return strncat(buf, "/", 1);
#endif
}

/*! @brief Create and return a unique temporary directory path.
 *
 *  @param[out] tmpdir  Buffer of at least _MAX_PATH bytes. Not NULL.
 *
 *  @return @p tmpdir on success, or NULL on failure.
 */
char * mktmpdir(char *tmpdir)
{
    const char *base;
    size_t cbBase;

#if defined(__UNIX__)
    base = getenv("TMPDIR");
    if (!base) base = "/tmp";
#else
    base = getenv("TMP");
    if (!base) base = getenv("TEMP");
    if (!base) base = "C:\\TEMP";
#endif

    cbBase = strlen(base);
    if (cbBase == 0 || cbBase + 16 >= _MAX_PATH) return NULL;

    strncpy(tmpdir, base, _MAX_PATH - 1);
    tmpdir[_MAX_PATH - 1] = '\0';

    if (tmpdir[cbBase - 1] != '/' && tmpdir[cbBase - 1] != '\\')
        addpathsep(tmpdir);

#if defined(_WIN32) || defined(__OS2__) || defined(__DOS__)
    strncat(tmpdir, "bndXXXXXX", _MAX_PATH - strlen(tmpdir) - 1);
    if (_mktemp(tmpdir) == NULL) return NULL;
    if (_mkdir(tmpdir) != 0) return NULL;
#else
    strncat(tmpdir, "bndXXXXXX", _MAX_PATH - strlen(tmpdir) - 1);
    if (mkdtemp(tmpdir) == NULL) return NULL;
#endif

    return addpathsep(tmpdir);
}

/* ------------------------------------------------------------------ */
/* LNK file generation                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Generate the WLINK response file (bind.lnk). */
void generate_lnk(void)
{
    FILE * f;

    f = fopen("bind.lnk", "w");
    if (!f) return;
    fputs("system dos\n", f);
    fputs("name fstub.exe\n", f);
    fputs("file tmp.obj\n", f);
    if (options.map) {
        fputs("op m=", f);
        fputs(options.mapfile, f);
        fputs("\n", f);
    }
    fputs("lib api.lib\n", f);
    if (options.DLLAPI) fputs("lib dll.lib\n", f);
    if (options.VioAPI == 1) fputs("lib vios.lib\n", f);
    if (options.VioAPI == 2) fputs("lib viof.lib\n", f);
    if (options.MouAPI == 1) fputs("lib mous.lib\n", f);
    if (options.MouAPI == 2) fputs("lib mouf.lib\n", f);
    if (options.KbdAPI == 1) fputs("lib kbds.lib\n", f);
    if (options.KbdAPI == 2) fputs("lib kbdf.lib\n", f);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* tmp.obj generation                                                  */
/* ------------------------------------------------------------------ */

/*! @brief Generate the temporary import object file (tmp.obj).
 *
 *  Iterates the CCL vector of imported functions and emits EXTDEF,
 *  LEDATA, FIXUPP and footer records through the OmfWrite* API. All
 *  record bytes are produced by the OMF library, not by hard-coded
 *  byte arrays.
 */
void generate_imptable(void)
{
    HOMFFILE hFile;
    APIRET rc;
    ULONG cEntries;
    ULONG i;
    UCHAR * puchLedata;
    USHORT usDataLen;
    OMF_FIXUPP_FIXUP * paFixups;
    const char ** ppszNames;

    if (VectorGetCount(hvApi, &cEntries) != NO_ERROR) return;
    if (cEntries == 0) return;

    /* LEDATA payload is 34 bytes per entry. */
    usDataLen = (USHORT)(cEntries * 34);
    puchLedata = (UCHAR *)malloc(usDataLen);
    if (!puchLedata) return;
    memset(puchLedata, 0, usDataLen);

    /* Fill LEDATA buffer: 9 mod + 21 func + 4 far pointer placeholder. */
    for (i = 0; i < cEntries; i++) {
        apientry entry;
        UCHAR * q = &puchLedata[i * 34];

        if (VectorGetItem(hvApi, i, &entry, sizeof(entry), NULL)
            != NO_ERROR) {
            free(puchLedata);
            return;
        }
        memcpy(q, entry.mod, 9);
        memcpy(q + 9, entry.func, 21);
        /* 4-byte far pointer left zero. */
    }

    /* FIXUPP: one subrecord per entry. */
    paFixups = (OMF_FIXUPP_FIXUP *)
        malloc(cEntries * sizeof(OMF_FIXUPP_FIXUP));
    if (!paFixups) {
        free(puchLedata);
        return;
    }
    for (i = 0; i < cEntries; i++) {
        USHORT usOffset = (USHORT)(34 * i + 30);

        paFixups[i].usLocat =
            (USHORT)((((0xCC + ((usOffset >> 8) & 0xFF)) << 8)
                      | (usOffset & 0xFF)));
        paFixups[i].uchFixDat =
            OMF_FIXDAT_FRAME_TARGET |
            OMF_FIXDAT_P_8BIT |
            OMF_FIXDAT_TARGET_EXTDEF;
        paFixups[i].uchFrameDatum = 0;
        paFixups[i].usTargetDatum = (USHORT)(i + 1);
        paFixups[i].usTargetDisp = 0;
    }

    /* EXTDEF name array, built on the fly from the vector. */
    ppszNames = (const char **)malloc(cEntries * sizeof(char *));
    if (!ppszNames) {
        free(paFixups);
        free(puchLedata);
        return;
    }
    for (i = 0; i < cEntries; i++) {
        apientry entry;
        if (VectorGetItem(hvApi, i, &entry, sizeof(entry), NULL)
            != NO_ERROR) {
            free((void *)ppszNames);
            free(paFixups);
            free(puchLedata);
            return;
        }
        ppszNames[i] = entry.func;
    }

    /* Open the output file. */
    rc = OmfOpen("tmp.obj", &hFile,
                 OMF_OPEN_WRITE | OMF_OPEN_TRUNCATE);
    if (rc != NO_ERROR) {
        free((void *)ppszNames);
        free(paFixups);
        free(puchLedata);
        return;
    }

    /* ---- Header records ---- */

    OmfWriteTheadr(hFile,
        "D:\\osfree\\dual\\fap\\i\\loader\\imptable.c");
    OmfWriteComentCodeView(hFile);

    {
        static const UCHAR auchWopt[5] = {
            0x30, 0x73, 0x4F, 0x65, 0x64
        };
        OmfWriteComentWatcomOptions(hFile, auchWopt, 5);
    }

    OmfWriteComentBorlandDependency(hFile,
        "D:\\osfree\\dual\\fap\\i\\loader\\imptable.c");
    OmfWriteComentBorlandDependency(hFile, NULL);

    /* ---- LNAMES + SEGDEF ---- */

    {
        static const char * const apszNames1[] = {
            "", "CODE", "DATA", "BSS", "TLS", "DGROUP", "_TEXT"
        };
        OmfWriteLnames(hFile, apszNames1, 7);
    }
    OmfWriteSegdef(hFile,
                   OMF_ACBP_ALIGN_BYTE | OMF_ACBP_BIG_BIT,
                   0x0000, LNAME_TEXT, LNAME_CODE, LNAME_EMPTY);

    OmfWriteComentOptimizeFarCall(hFile, 0x00);

    {
        static const char * const apszNames2[] = { "CONST" };
        OmfWriteLnames(hFile, apszNames2, 1);
    }
    OmfWriteSegdef(hFile,
                   OMF_ACBP_ALIGN_WORD | OMF_ACBP_BIG_BIT,
                   0x0000, LNAME_CONST, LNAME_DATA, LNAME_EMPTY);

    {
        static const char * const apszNames3[] = { "CONST2" };
        OmfWriteLnames(hFile, apszNames3, 1);
    }
    OmfWriteSegdef(hFile,
                   OMF_ACBP_ALIGN_WORD | OMF_ACBP_BIG_BIT,
                   0x0000, LNAME_CONST2, LNAME_DATA, LNAME_EMPTY);

    {
        static const char * const apszNames4[] = { "_DATA" };
        OmfWriteLnames(hFile, apszNames4, 1);
    }
    OmfWriteSegdef(hFile,
                   OMF_ACBP_ALIGN_WORD | OMF_ACBP_BIG_BIT,
                   usDataLen, LNAME_DATA_, LNAME_DATA, LNAME_EMPTY);

    {
        static const UCHAR auchMembers[6] = {
            OMF_GRPDEF_MEMBER_SEGMENT, SEGDEF_CONST,
            OMF_GRPDEF_MEMBER_SEGMENT, SEGDEF_CONST2,
            OMF_GRPDEF_MEMBER_SEGMENT, SEGDEF_DATA
        };
        OmfWriteGrpdef(hFile, LNAME_DGROUP, auchMembers, 6);
    }

    /* ---- EXTDEF ---- */

    OmfWriteExtdef(hFile, ppszNames, cEntries);
    free((void *)ppszNames);

    /* ---- LEDATA ---- */

    OmfWriteLedata(hFile, SEGDEF_DATA, 0x0000,
                   puchLedata, usDataLen);
    free(puchLedata);

    /* ---- FIXUPP ---- */

    OmfWriteFixupp(hFile, paFixups, cEntries);
    free(paFixups);

    /* ---- Footer ---- */

    OmfWritePubdef(hFile, 0x01, SEGDEF_DATA, "_imptable", 0x0000);
    OmfWriteComentDefaultLibrary(hFile, "math87s");
    OmfWriteComentDefaultLibrary(hFile, "emu87");
    OmfWriteModend(hFile, 0x00);

    OmfClose(hFile);
}

/* ------------------------------------------------------------------ */
/* Import list                                                         */
/* ------------------------------------------------------------------ */

/*! @brief Add an imported function to the global vector.
 *
 *  Duplicates (same module and same function) are ignored. The
 *  module name and function name are truncated to the fixed field
 *  widths (8 and 20 characters respectively) before storage.
 *
 *  @param[in] mod   Module name. Not NULL.
 *  @param[in] func  Function name. Not NULL.
 *
 *  @return 0 on success, -1 on failure.
 */
int addtolist(char * mod, char * func)
{
    apientry entry;
    ULONG cCount;
    ULONG i;

    if (hvApi == NULLHANDLE) return -1;

    if (VectorGetCount(hvApi, &cCount) != NO_ERROR) return -1;

    for (i = 0; i < cCount; i++) {
        apientry existing;

        if (VectorGetItem(hvApi, i, &existing, sizeof(existing),
                          NULL) != NO_ERROR)
            return -1;
        if (strcmp(existing.mod, mod) == 0 &&
            strcmp(existing.func, func) == 0)
            return 0;
    }

    memset(&entry, 0, sizeof(entry));
    strncpy(entry.mod, mod, sizeof(entry.mod) - 1);
    strncpy(entry.func, func, sizeof(entry.func) - 1);
    entry.ord = 0;

    if (VectorAdd(hvApi, &entry) != NO_ERROR) return -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Bind step                                                           */
/* ------------------------------------------------------------------ */

/*! @brief Bind the DOS stub and the NE image into a FamilyAPI file.
 *
 *  @param[in] fname  Path to the NE executable to bind. Not NULL.
 *
 *  @return 0 on success, 1 on failure.
 */
int bind(char * fname)
{
    char tmpdir[_MAX_PATH];
    char tmpexe[_MAX_PATH];
    APIRET rc;

    if (!mktmpdir(tmpdir)) {
        printf("Error: Cannot create temp directory\n");
        return 1;
    }
    strncpy(tmpexe, tmpdir, sizeof(tmpexe) - 1);
    tmpexe[sizeof(tmpexe) - 1] = '\0';
    strncat(tmpexe, "tmp.exe", sizeof(tmpexe) - strlen(tmpexe) - 1);

    rc = NeBind("fstub.exe", fname, tmpexe);
    if (rc != NO_ERROR) {
        printf("Error: NeBind failed (%lu)\n", (unsigned long)rc);
        return 1;
    }

    remove("fstub.exe");
    remove(options.outfile);
    if (rename(tmpexe, options.outfile) != 0) {
        printf("Error: File rename\n");
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Library search                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Search for a library file along a search path.
 *
 *  @param[in]  libname   Library file name.
 *  @param[out] fullpath  Receives the full path, or an empty string.
 *
 *  @return 1 if found, 0 otherwise.
 */
int searchlib(char * libname, char * fullpath)
{
    _searchenv(libname, ".", fullpath);
    if (fullpath[0] == '\0') {
        _searchenv(libname, options.libpath, fullpath);
        if (fullpath[0] == '\0') {
            _searchenv(libname, "LIB", fullpath);
        }
    }
    return (fullpath[0] == '\0') ? 0 : 1;
}

/* ------------------------------------------------------------------ */
/* Environment check                                                   */
/* ------------------------------------------------------------------ */

/*! @brief Verify that the build environment is usable.
 *
 *  @return 0 if usable, 1 otherwise.
 */
int check_environment(void)
{
    char full_path[_MAX_PATH];

    if (!getenv("WATCOM")) {
        printf("Error: WATCOM environment variable not set\n");
        return 1;
    }

    _searchenv("wlink.lnk", "PATH", full_path);
    if (full_path[0] == '\0') {
        printf("Error: Unable to find wlink.lnk file\n");
        return 1;
    }

    _searchenv("wlink.exe", "PATH", full_path);
    if (full_path[0] == '\0') {
        printf("Error: Unable to find wlink.exe file\n");
        return 1;
    }

    if (!searchlib("os2.lib", full_path)) {
        if (searchlib("doscalls.lib", full_path))
            options.DoscallsLIB = 1;
    }

    if (full_path[0] == '\0') {
        printf("Error: Unable to find nor os2.lib nor doscalls.lib file\n");
        return 1;
    }

    if (!searchlib("api.lib", full_path)) {
        printf("Error: Unable to find api.lib file\n");
        return 1;
    }

    if (!searchlib("dll.lib", full_path)) {
        printf("Error: Unable to find dll.lib file\n");
        return 1;
    }

    if (!searchlib("vios.lib", full_path)) {
        printf("Error: Unable to find vios.lib file\n");
        return 1;
    }

    if (!searchlib("viof.lib", full_path)) {
        printf("Error: Unable to find viof.lib file\n");
        return 1;
    }

    if (!searchlib("mous.lib", full_path)) {
        printf("Error: Unable to find mous.lib file\n");
        return 1;
    }

    if (!searchlib("mouf.lib", full_path)) {
        printf("Error: Unable to find mouf.lib file\n");
        return 1;
    }

    if (!searchlib("kbds.lib", full_path)) {
        printf("Error: Unable to find kbds.lib file\n");
        return 1;
    }

    if (!searchlib("kbdf.lib", full_path)) {
        printf("Error: Unable to find kbdf.lib file\n");
        return 1;
    }

    if (!searchlib("apilmr.obj", full_path)) {
        printf("Error: Unable to find apilmr.obj file\n");
        return 1;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Help                                                                */
/* ------------------------------------------------------------------ */

/*! @brief Print command-line usage information. */
void printhlp(void)
{
    printf("BIND infile [implibs] [linklibs] [options]\n\n");
    printf("/HELP       Displays Help\n");
    printf("/?          Displays Help\n\n");
    printf("/L[IBPATH]  Add to library search path\n");
    printf("/M[AP]      Generates Link Map File\n");
    printf("/N[AMES]    Specifies Protected-Mode Functions\n");
    printf("/NOLOGO     Suppresses Sign-On Banner\n");
    printf("/O[UTFILE]  Specifies Name of Bound Program\n");
    printf("/Q          Quiet\n");
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

/*! @brief Program entry point.
 *
 *  @param[in] argc  Argument count.
 *  @param[in] argv  Argument vector. Not NULL.
 *
 *  @return Process exit status.
 */
int main(int argc, char *argv[])
{
    HNE hNe = NULLHANDLE;
    HOMFLIB hLib = NULLHANDLE;
    HVECTOR hvMods = NULLHANDLE;
    struct exe_hdr MZHeader;
    struct new_exe NEHeader;
    char full_path[_MAX_PATH];
    USHORT usModCount;
    USHORT usSegCount;
    USHORT usRelocCount;
    USHORT k;
    struct new_seg seg;
    struct new_rlc rlc;
    int rc = 1;
    signed char ch;
    APIRET ar;

    options.quiet = 0;
    options.logo = 1;
    options.outfile[0] = 0;
    options.infile[0] = 0;
    options.mapfile[0] = 0;
    options.libpath[0] = 0;
    options.map = 0;
    options.dosformat = 0;
    options.DoscallsLIB = 0;
    options.MouAPI = 0;
    options.KbdAPI = 0;
    options.VioAPI = 0;
    options.DLLAPI = 0;

    if (check_environment()) {
        printf("Environment not configured");
        return 1;
    }

    if (argc == 1) {
        printhlp();
        exit(1);
    }

#ifndef __UNIX__
    if (argc > 1 && argv[1][0] != '-' && argv[1][0] != '/')
#else
    if (argc > 1 && argv[1][0] != '-')
#endif
    {
        strncpy(options.infile, argv[1], sizeof(options.infile) - 1);
        options.infile[sizeof(options.infile) - 1] = '\0';
        options.dosformat = 1;
        optind = 2;
    }

    while ((ch = getopt(argc, argv, "?h:H:m:M:n:N:q:Q:o:O")) != -1) {
        switch (ch) {
        case 'l':
        case 'L':
            if (!strnicmp(optarg, "IBPATH", 6))
                strcpy(options.libpath, argv[optind]);
            else
                strcpy(options.libpath, optarg);
            break;

        case 'm':
        case 'M':
            if (!strnicmp(optarg, "AP", 2))
                strcpy(options.mapfile, argv[optind]);
            else
                strcpy(options.mapfile, optarg);
            options.map = 1;
            break;

        case 'n':
        case 'N':
            if (!strncmp(_strupr(optarg), "OLOGO", 5)) {
                options.logo = 0;
                break;
            }
            if (!strncmp(_strupr(optarg), "AMES", 4)) {
                printf("NAMES\n");
                break;
            }
            exit(1);

        case 'q':
        case 'Q':
            if (!strncmp(_strupr(optarg), "UIET", 4)) {
                options.quiet = 1;
                break;
            }
            if (!strlen(optarg)) {
                options.quiet = 1;
                break;
            }
            exit(1);

        case 'o':
        case 'O':
            if (!strnicmp(optarg, "UTFILE", 6))
                strcpy(options.outfile, argv[optind]);
            else
                strcpy(options.outfile, optarg);
            break;

        case 'h':
        case 'H':
            if (!strncmp(_strupr(optarg), "ELP", 3)) {
                printhlp();
                exit(0);
            }
            /* fall through */

        case '?':
            printhlp();
            exit(0);

        default:
            printf("Unknown option\n");
            exit(1);
        }
    }

    if (options.logo && !options.quiet)
        printf("osFree FamilyAPI Binder v.0.9\n\n");

    if (options.infile[0] == '\0') {
        if (optind < argc) {
            strncpy(options.infile, argv[optind],
                    sizeof(options.infile) - 1);
            options.infile[sizeof(options.infile) - 1] = '\0';
            optind++;
        }
    }

    if (options.infile[0] == '\0') {
        printf("BIND: no input file\n");
        exit(1);
    }

    if (options.outfile[0] == '\0') {
        strncpy(options.outfile, options.infile,
                sizeof(options.outfile) - 1);
        options.outfile[sizeof(options.outfile) - 1] = '\0';
    }

    /* ---- Create CCL vectors ---- */

    ar = VectorCreate(sizeof(apientry), &hvApi);
    if (ar != NO_ERROR) {
        printf("Error: Cannot create import vector (%lu)\n",
               (unsigned long)ar);
        return 1;
    }

    ar = VectorCreate(sizeof(modname), &hvMods);
    if (ar != NO_ERROR) {
        printf("Error: Cannot create module vector (%lu)\n",
               (unsigned long)ar);
        VectorDestroy(hvApi);
        hvApi = NULLHANDLE;
        return 1;
    }

    /* ---- Open NE file ---- */

    ar = NeOpen(options.infile, &hNe);
    if (ar != NO_ERROR) {
        printf("Error: Open input file (%lu)\n", (unsigned long)ar);
        goto error_cleanup;
    }

    ar = NeQueryMZHeader(hNe, &MZHeader);
    if (ar != NO_ERROR) {
        printf("Error: Read MZ Header\n");
        goto error_cleanup;
    }
    ar = NeQueryHeader(hNe, &NEHeader);
    if (ar != NO_ERROR) {
        printf("Error: Read NE Header\n");
        goto error_cleanup;
    }
    if (NE_EXETYP(NEHeader) != NE_OS2) {
        printf("Error: Target OS not OS/2\n");
        goto error_cleanup;
    }

    /* ---- Open library ---- */

    ar = LibOpen(options.DoscallsLIB ? "doscalls.lib" : "os2.lib",
                 &hLib);
    if (ar != NO_ERROR) {
        printf("Error: Open library (%lu)\n", (unsigned long)ar);
        goto error_cleanup;
    }

    /* ---- Read module names into hvMods ---- */

    ar = NeQueryModuleCount(hNe, &usModCount);
    if (ar != NO_ERROR || usModCount == 0) {
        printf("Error: Module count\n");
        goto error_cleanup;
    }

    for (k = 1; k <= usModCount; k++) {
        modname mn;
        char szName[256];

        memset(&mn, 0, sizeof(mn));
        ar = NeQueryModuleName(hNe, k, szName, sizeof(szName));
        if (ar != NO_ERROR) {
            printf("Error: Read module name %u\n", (unsigned)k);
            goto error_cleanup;
        }
        strncpy(mn.name, szName, sizeof(mn.name) - 1);
        if (VectorAdd(hvMods, &mn) != NO_ERROR) {
            printf("Error: VectorAdd module\n");
            goto error_cleanup;
        }
    }

    /* ---- Walk segments and relocations ---- */

    ar = NeQuerySegmentCount(hNe, &usSegCount);
    if (ar != NO_ERROR) {
        printf("Error: Segment count\n");
        goto error_cleanup;
    }

    for (k = 1; k <= usSegCount; k++) {
        USHORT j;

        ar = NeQuerySegment(hNe, k, &seg);
        if (ar != NO_ERROR) {
            printf("Error: Read segment %u\n", (unsigned)k);
            goto error_cleanup;
        }
        ar = NeQueryRelocCount(hNe, k, &usRelocCount);
        if (ar != NO_ERROR) {
            printf("Error: Read relocation table size\n");
            goto error_cleanup;
        }

        for (j = 0; j < usRelocCount; j++) {
            USHORT usMod;
            USHORT usOrd;
            char szFunc[256];
            modname mn;

            ar = NeQueryReloc(hNe, k, j, &rlc);
            if (ar != NO_ERROR) {
                printf("Error: Read relocation entry\n");
                goto error_cleanup;
            }

            if ((rlc.nr_flags & NRRTYP) == NRRNAM) {
                printf("Panic!\n");
                goto error_cleanup;
            }
            if ((rlc.nr_flags & NRRTYP) != NRRORD)
                continue;

            usMod = rlc.nr_union.nr_import.nr_mod;
            usOrd = rlc.nr_union.nr_import.nr_proc;
            if (usMod == 0 || usMod > usModCount)
                continue;

            memset(&mn, 0, sizeof(mn));
            if (VectorGetItem(hvMods, (ULONG)(usMod - 1),
                              &mn, sizeof(mn), NULL) != NO_ERROR)
                continue;

            ar = LibQueryFunction(hLib, mn.name, usOrd,
                                  szFunc, sizeof(szFunc));
            if (ar != NO_ERROR)
                continue;

            addtolist(mn.name, szFunc);

            if (!strncmp(szFunc, "MOU", 3)) options.MouAPI = 1;
            if (!strncmp(szFunc, "KBD", 3)) options.KbdAPI = 1;
            if (!strncmp(szFunc, "VIO", 3)) options.VioAPI = 1;

            if (!strcmp(szFunc, "VIOREGISTER")) {
                options.VioAPI = 2;
                options.DLLAPI = 1;
            }
            if (!strcmp(szFunc, "MOUREGISTER")) {
                options.MouAPI = 2;
                options.DLLAPI = 1;
            }
            if (!strcmp(szFunc, "KBDREGISTER")) {
                options.KbdAPI = 2;
                options.DLLAPI = 1;
            }
            if (!strcmp(szFunc, "DOSLOADMODULE")) options.DLLAPI = 1;
        }
    }

    /* ---- Cleanup library and NE handles ---- */

    LibClose(hLib);
    hLib = NULLHANDLE;
    NeClose(hNe);
    hNe = NULLHANDLE;

    VectorDestroy(hvMods);
    hvMods = NULLHANDLE;

    /* ---- Generate tmp.obj, LNK, run linker ---- */

    generate_imptable();
    generate_lnk();
    system("wlink.exe op q op fullh @bind.lnk");
    remove("bind.lnk");
    remove("tmp.obj");

    /* ---- Bind ---- */

    rc = bind(options.infile);

    VectorDestroy(hvApi);
    hvApi = NULLHANDLE;
    return rc;

error_cleanup:
    if (hLib) LibClose(hLib);
    if (hNe) NeClose(hNe);
    if (hvMods) VectorDestroy(hvMods);
    if (hvApi) VectorDestroy(hvApi);
    hvApi = NULLHANDLE;
    return 1;
}
