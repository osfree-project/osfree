
/*
 *@@sourcefile
 *
 *
 *@@added V0.9.9 (2001-02-10) [umoeller]
 */

#if __cplusplus
extern "C" {
#endif

#ifndef ENC_BASE_HEADER_INCLUDED
    #define ENC_BASE_HEADER_INCLUDED

    #pragma pack(1)         // V0.9.20 (2002-07-03) [umoeller]

    /*
     *@@ XWPENCODINGMAP:
     *      entry in a codepage-to-Unicode conversion table.
     */

    typedef struct _XWPENCODINGMAP
    {
        unsigned short      usCP;
        unsigned short      usUni;
    } XWPENCODINGMAP, *PXWPENCODINGMAP;

    #pragma pack()

    /*
     *@@ ENCID:
     *      enum identifying each encoding set which is
     *      generally supported. Each ID corresponds to
     *      one header file in include\encodings\.
     */

    typedef enum _ENCID
    {
        UNSUPPORTED,
        enc_cp437,
        enc_cp737,
        enc_cp775,
        enc_cp850,
        enc_cp852,
        enc_cp855,
        enc_cp857,
        enc_cp860,
        enc_cp861,
        enc_cp862,
        enc_cp863,
        enc_cp864,
        enc_cp865,
        enc_cp866,
        enc_cp869,
        enc_cp874,
        enc_cp932,
        enc_cp936,
        enc_cp949,
        enc_cp950,
        enc_cp1004,     // added V0.9.18 (2002-03-08) [umoeller]
        enc_cp1250,
        enc_cp1251,
        enc_cp1252,
        enc_cp1253,
        enc_cp1254,
        enc_cp1255,
        enc_cp1256,
        enc_cp1257,
        enc_cp1258,
        enc_cp1386,  // WarpIN V1.0.16 (2008-02-19) [pr]
        enc_iso8859_1,
        enc_iso8859_2,
        enc_iso8859_3,
        enc_iso8859_4,
        enc_iso8859_5,
        enc_iso8859_6,
        enc_iso8859_7,
        enc_iso8859_8,
        enc_iso8859_9,
        enc_iso8859_10,
        enc_iso8859_13,
        enc_iso8859_14,
        enc_iso8859_15
    } ENCID;

    /*
     *@@ CONVERSION:
     *
     *@@added V0.9.18 (2002-03-08) [umoeller]
     */

    typedef struct _CONVERSION
    {
        ENCID   EncodingID;

        // to get Unicode for character 123 in the specific
        // encoding, do pTable->ausEntriesUniFromCP[123].
        // If you get 0xFFFF, the encoding is undefined.
        unsigned short  usHighestCP;
        unsigned short  *ausEntriesUniFromCP;  // usHighestCP + 1 entries

        // to get codepage for unicode character 123,
        // do pTable->ausEntriesCPFromUni[123].
        // If you get 0xFFFF, the encoding is undefined.
        unsigned short  usHighestUni;
        unsigned short  *ausEntriesCPFromUni;   // usHighestUni + 1 entries

    } CONVERSION, *PCONVERSION;

    typedef enum _ENCBYTECOUNT
    {
        SINGLE,
        DOUBLE,
        EBCDIC,
        MULTI_UNICODE,
        UNKNOWN
    } ENCBYTECOUNT;

    int encGetTable(ENCID id,
                    PXWPENCODINGMAP *ppMap,
                    unsigned long *pcEntries);

    ENCID encFindIdForCodepage(unsigned short usCodepage,
                               const char **ppcszDescription,
                               ENCBYTECOUNT *pByteCount);

    PCONVERSION encCreateCodec(ENCID id);

    void encFreeCodec(PCONVERSION *ppTable);

    unsigned long encChar2Uni(PCONVERSION pTable,
                              unsigned short c);

    unsigned short encUni2Char(PCONVERSION pTable,
                               unsigned long ulUni);

    unsigned long encDecodeUTF8(const char **ppch);

    int encInitCase(void);

    unsigned long encToUpper(unsigned long ulUni);

    int encicmp(const char *pcsz1,
                const char *pcsz2);

#endif

#if __cplusplus
}
#endif

