
/*
 *@@sourcefile xmlparse.c:
 *      contains the API of the expat XML parser 1.95.2, as released on
 *      http://sourceforge.net/projects/expat/ on July 26, 2001.
 *
 *      V1.95.1 was ported to and integrated with the xwphelpers
 *      for V0.9.9 (2001-02-10) [umoeller]. V1.95.2 was re-ported
 *      with V0.9.14 (2001-08-09) [umoeller].
 *
 *      See xml.c for a general introduction to @XML support in the
 *      xwphelpers.
 *
 *      Expat is a library, written in C, for parsing XML @documents. It's
 *      the underlying XML parser for the open source Mozilla project,
 *      perl's XML::Parser, and other open-source XML parsers.
 *
 *      Expat was written by James Clark, who also wrote groff (an nroff
 *      look-alike), Jade (an implemention of ISO's DSSSL stylesheet
 *      language for SGML), XP (a Java XML parser package), XT (a Java
 *      XSL engine). He was also the technical lead on the XML Working
 *      Group at W3 that produced the XML specification.
 *
 *      Most of this documentation is the original documentation
 *      that comes with expat as an HTML file. I (umoeller)
 *      have integrated the docs into the sources and added a
 *      couple of remarks while trying to implement parsers.
 *
 *      See XML_ParserCreate for instructions how to parse XML.
 *
 *      Note that expat is a non-validating XML processor. Validation
 *      is supported by the top layer of xwphelpers XML support. See
 *      xml.c.
 *
 *@@added V0.9.9 (2001-02-10) [umoeller]
 *@@header "expat\expat.h"
 */

/*
 *      Copyright (C) 2001 Ulrich M”ller.
 *      Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd.
 *                                     and Clark Cooper.
 *
 *      Permission is hereby granted, free of charge, to any person obtaining
 *      a copy of this software and associated documentation files (the
 *      "Software"), to deal in the Software without restriction, including
 *      without limitation the rights to use, copy, modify, merge, publish,
 *      distribute, sublicense, and/or sell copies of the Software, and to
 *      permit persons to whom the Software is furnished to do so, subject to
 *      the following conditions:
 *
 *      The above copyright notice and this permission notice shall be included
 *      in all copies or substantial portions of the Software.
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *      EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *      MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *      CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *      TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 *@@category: Helpers\XML\expat
 *      expat XML parser. See xmlparse.c.
 */

#include "setup.h"

#pragma info(norea, nogen)
        // disable "statement unreachable" and "missing break statement"
        // this code generates those options HEAVILY


/* #ifdef COMPILED_FROM_DSP
 * #  include "winconfig.h"
 * #  define XMLPARSEAPI(type) __declspec(dllexport) type __cdecl
 * #  include "expat.h"
 * #  undef XMLPARSEAPI
 * #else
 * #include <config.h> */

#ifdef __declspec
#define XMLPARSEAPI(type) __declspec(dllexport) type __cdecl
#endif

#include "expat/expat.h"

#ifdef __declspec
#undef XMLPARSEAPI
#endif
// #endif /* ndef COMPILED_FROM_DSP */

#include <stddef.h>
#include <string.h>

#ifdef XML_UNICODE
#define XML_ENCODE_MAX XML_UTF16_ENCODE_MAX
#define XmlConvert XmlUtf16Convert
#define XmlGetInternalEncoding XmlGetUtf16InternalEncoding
#define XmlGetInternalEncodingNS XmlGetUtf16InternalEncodingNS
#define XmlEncode XmlUtf16Encode
#define MUST_CONVERT(enc, s) (!(enc)->isUtf16 || (((unsigned long)s) & 1))
typedef unsigned short ICHAR;

#else
#define XML_ENCODE_MAX XML_UTF8_ENCODE_MAX
#define XmlConvert XmlUtf8Convert
#define XmlGetInternalEncoding XmlGetUtf8InternalEncoding
#define XmlGetInternalEncodingNS XmlGetUtf8InternalEncodingNS
#define XmlEncode XmlUtf8Encode
#define MUST_CONVERT(enc, s) (!(enc)->isUtf8)
typedef char ICHAR;

#endif


#ifndef XML_NS

#define XmlInitEncodingNS XmlInitEncoding
#define XmlInitUnknownEncodingNS XmlInitUnknownEncoding
#undef XmlGetInternalEncodingNS
#define XmlGetInternalEncodingNS XmlGetInternalEncoding
#define XmlParseXmlDeclNS XmlParseXmlDecl

#endif

#ifdef XML_UNICODE_WCHAR_T
#define XML_T(x) L ## x
#else
#define XML_T(x) x
#endif

/* Round up n to be a multiple of sz, where sz is a power of 2. */
#define ROUND_UP(n, sz) (((n) + ((sz) - 1)) & ~((sz) - 1))

#include "expat\xmltok.h"
#include "expat\xmlrole.h"

typedef const XML_Char *KEY;

typedef struct
{
    KEY name;
}
NAMED;

typedef struct
{
    NAMED **v;
    size_t size;
    size_t used;
    size_t usedLim;
    XML_Memory_Handling_Suite *mem;
}
HASH_TABLE;

typedef struct
{
    NAMED **p;
    NAMED **end;
}
HASH_TABLE_ITER;

#define INIT_TAG_BUF_SIZE 32    /* must be a multiple of sizeof(XML_Char) */
#define INIT_DATA_BUF_SIZE 1024
#define INIT_ATTS_SIZE 16
#define INIT_BLOCK_SIZE 1024
#define INIT_BUFFER_SIZE 1024

#define EXPAND_SPARE 24

typedef struct binding
{
    struct prefix *prefix;
    struct binding *nextTagBinding;
    struct binding *prevPrefixBinding;
    const struct attribute_id *attId;
    XML_Char *uri;
    int uriLen;
    int uriAlloc;
}
BINDING;

typedef struct prefix
{
    const XML_Char *name;
    BINDING *binding;
}
PREFIX;

typedef struct
{
    const XML_Char *str;
    const XML_Char *localPart;
    int uriLen;
}
TAG_NAME;

typedef struct tag
{
    struct tag *parent;
    const char *rawName;
    int rawNameLength;
    TAG_NAME name;
    char *buf;
    char *bufEnd;
    BINDING *bindings;
}
TAG;

typedef struct
{
    const XML_Char *name;
    const XML_Char *textPtr;
    int textLen;
    const XML_Char *systemId;
    const XML_Char *base;
    const XML_Char *publicId;
    const XML_Char *notation;
    char open;
    char is_param;
}
ENTITY;

typedef struct
{
    enum XML_Content_Type type;
    enum XML_Content_Quant quant;
    const XML_Char *name;
    int firstchild;
    int lastchild;
    int childcnt;
    int nextsib;
}
CONTENT_SCAFFOLD;

typedef struct block
{
    struct block *next;
    int size;
    XML_Char s[1];
}
BLOCK;

typedef struct
{
    BLOCK *blocks;
    BLOCK *freeBlocks;
    const XML_Char *end;
    XML_Char *ptr;
    XML_Char *start;
    XML_Memory_Handling_Suite *mem;
}
STRING_POOL;

/* The XML_Char before the name is used to determine whether
 * an attribute has been specified. */
typedef struct attribute_id
{
    XML_Char *name;
    PREFIX *prefix;
    char maybeTokenized;
    char xmlns;
}
ATTRIBUTE_ID;

typedef struct
{
    const ATTRIBUTE_ID *id;
    char isCdata;
    const XML_Char *value;
}
DEFAULT_ATTRIBUTE;

typedef struct
{
    const XML_Char *name;
    PREFIX *prefix;
    const ATTRIBUTE_ID *idAtt;
    int nDefaultAtts;
    int allocDefaultAtts;
    DEFAULT_ATTRIBUTE *defaultAtts;
}
ELEMENT_TYPE;

typedef struct
{
    HASH_TABLE generalEntities;
    HASH_TABLE elementTypes;
    HASH_TABLE attributeIds;
    HASH_TABLE prefixes;
    STRING_POOL pool;
    int complete;
    int standalone;
#ifdef XML_DTD
    HASH_TABLE paramEntities;
#endif                          /* XML_DTD */
    PREFIX defaultPrefix;
    /* === scaffolding for building content model === */
    int in_eldecl;
    CONTENT_SCAFFOLD *scaffold;
    unsigned contentStringLen;
    unsigned scaffSize;
    unsigned scaffCount;
    int scaffLevel;
    int *scaffIndex;
}
DTD;

typedef struct open_internal_entity
{
    const char *internalEventPtr;
    const char *internalEventEndPtr;
    struct open_internal_entity *next;
    ENTITY *entity;
}
OPEN_INTERNAL_ENTITY;

typedef XMLERROR Processor(XML_Parser parser,
                           const char *start,
                           const char *end,
                           const char **endPtr);

STATIC Processor prologProcessor;
STATIC Processor prologInitProcessor;
STATIC Processor contentProcessor;
STATIC Processor cdataSectionProcessor;

#ifdef XML_DTD
STATIC Processor ignoreSectionProcessor;

#endif /* XML_DTD */
STATIC Processor epilogProcessor;
STATIC Processor errorProcessor;
STATIC Processor externalEntityInitProcessor;
STATIC Processor externalEntityInitProcessor2;
STATIC Processor externalEntityInitProcessor3;
STATIC Processor externalEntityContentProcessor;

STATIC XMLERROR
 handleUnknownEncoding(XML_Parser parser, const XML_Char * encodingName);
STATIC XMLERROR
 processXmlDecl(XML_Parser parser, int isGeneralTextEntity, const char *, const char *);
STATIC XMLERROR
 initializeEncoding(XML_Parser parser);
STATIC XMLERROR
 doProlog(XML_Parser parser, const ENCODING * enc, const char *s,
          const char *end, int tok, const char *next, const char **nextPtr);
STATIC XMLERROR
 processInternalParamEntity(XML_Parser parser, ENTITY * entity);
STATIC XMLERROR
 doContent(XML_Parser parser, int startTagLevel, const ENCODING * enc,
           const char *start, const char *end, const char **endPtr);
STATIC XMLERROR
 doCdataSection(XML_Parser parser, const ENCODING *, const char **startPtr, const char *end, const char **nextPtr);

#ifdef XML_DTD
STATIC XMLERROR
 doIgnoreSection(XML_Parser parser, const ENCODING *, const char **startPtr, const char *end, const char **nextPtr);

#endif /* XML_DTD */
STATIC XMLERROR storeAtts(XML_Parser parser, const ENCODING *, const char *s,
                          TAG_NAME * tagNamePtr, BINDING ** bindingsPtr);
STATIC
int addBinding(XML_Parser parser, PREFIX * prefix, const ATTRIBUTE_ID * attId, const XML_Char * uri, BINDING ** bindingsPtr);

STATIC int
 defineAttribute(ELEMENT_TYPE * type, ATTRIBUTE_ID *,
                 int isCdata, int isId, const XML_Char * dfltValue,
                 XML_Parser parser);

STATIC XMLERROR
 storeAttributeValue(XML_Parser parser, const ENCODING *, int isCdata, const char *, const char *,
                     STRING_POOL *);
STATIC XMLERROR
 appendAttributeValue(XML_Parser parser, const ENCODING *, int isCdata, const char *, const char *,
                      STRING_POOL *);
STATIC ATTRIBUTE_ID *
 getAttributeId(XML_Parser parser, const ENCODING * enc, const char *start, const char *end);
STATIC int setElementTypePrefix(XML_Parser parser, ELEMENT_TYPE *);
STATIC XMLERROR
 storeEntityValue(XML_Parser parser, const ENCODING * enc, const char *start, const char *end);
STATIC int
 reportProcessingInstruction(XML_Parser parser, const ENCODING * enc, const char *start, const char *end);
STATIC int
 reportComment(XML_Parser parser, const ENCODING * enc, const char *start, const char *end);
STATIC void
 reportDefault(XML_Parser parser, const ENCODING * enc, const char *start, const char *end);

STATIC const XML_Char *getContext(XML_Parser parser);
STATIC int setContext(XML_Parser parser, const XML_Char * context);
STATIC void normalizePublicId(XML_Char * s);
STATIC int dtdInit(DTD *, XML_Parser parser);

STATIC void dtdDestroy(DTD *, XML_Parser parser);

STATIC int dtdCopy(DTD * newDtd, const DTD * oldDtd, XML_Parser parser);

STATIC int copyEntityTable(HASH_TABLE *, STRING_POOL *, const HASH_TABLE *,
                           XML_Parser parser);

#ifdef XML_DTD
STATIC void dtdSwap(DTD *, DTD *);

#endif /* XML_DTD */

STATIC NAMED *lookup(HASH_TABLE * table, KEY name, size_t createSize);

STATIC void hashTableInit(HASH_TABLE *, XML_Memory_Handling_Suite * ms);

STATIC void hashTableDestroy(HASH_TABLE *);
STATIC void hashTableIterInit(HASH_TABLE_ITER *, const HASH_TABLE *);
STATIC NAMED *hashTableIterNext(HASH_TABLE_ITER *);
STATIC void poolInit(STRING_POOL *, XML_Memory_Handling_Suite * ms);
STATIC void poolClear(STRING_POOL *);
STATIC void poolDestroy(STRING_POOL *);
STATIC XML_Char *poolAppend(STRING_POOL *pool,
                            const ENCODING *enc,
                            const char *ptr,
                            const char *end,
                            unsigned long *pulOfs); // V0.9.14 (2001-08-09) [umoeller]
STATIC XML_Char *poolStoreString(STRING_POOL *pool,
                                 const ENCODING *enc,
                                 const char *ptr,
                                 const char *end,
                                 unsigned long *pulOfs); // V0.9.14 (2001-08-09) [umoeller]

STATIC int poolGrow(STRING_POOL * pool);

STATIC int nextScaffoldPart(XML_Parser parser);
STATIC XMLCONTENT *build_model(XML_Parser parser);

STATIC const XML_Char *poolCopyString(STRING_POOL * pool, const XML_Char * s);
STATIC const XML_Char *poolCopyStringN(STRING_POOL * pool, const XML_Char * s, int n);
STATIC const XML_Char *poolAppendString(STRING_POOL * pool, const XML_Char * s);
STATIC ELEMENT_TYPE *getElementType(XML_Parser Paraser,
                                    const ENCODING * enc,
                                    const char *ptr,
                                    const char *end);

#define poolStart(pool) ((pool)->start)
#define poolEnd(pool) ((pool)->ptr)
#define poolLength(pool) ((pool)->ptr - (pool)->start)
#define poolChop(pool) ((void)--(pool->ptr))
#define poolLastChar(pool) (((pool)->ptr)[-1])
#define poolDiscard(pool) ((pool)->ptr = (pool)->start)
#define poolFinish(pool) ((pool)->start = (pool)->ptr)
#define poolAppendChar(pool, c) \
  (((pool)->ptr == (pool)->end && !poolGrow(pool)) \
   ? 0 \
   : ((*((pool)->ptr)++ = c), 1))

typedef struct
{
    /* The first member must be userData so that the XML_GetUserData macro works. */
    void *m_userData;
    void *m_handlerArg;
    char *m_buffer;
    XML_Memory_Handling_Suite m_mem;
    /* first character to be parsed */
    const char *m_bufferPtr;
    /* past last character to be parsed */
    char *m_bufferEnd;
    /* allocated end of buffer */
    const char *m_bufferLim;
    long m_parseEndByteIndex;
    const char *m_parseEndPtr;
    XML_Char *m_dataBuf;
    XML_Char *m_dataBufEnd;
    XML_StartElementHandler m_startElementHandler;
    XML_EndElementHandler m_endElementHandler;
    XML_CharacterDataHandler m_characterDataHandler;
    XML_ProcessingInstructionHandler m_processingInstructionHandler;
    XML_CommentHandler m_commentHandler;
    XML_StartCdataSectionHandler m_startCdataSectionHandler;
    XML_EndCdataSectionHandler m_endCdataSectionHandler;
    XML_DefaultHandler m_defaultHandler;
    XML_StartDoctypeDeclHandler m_startDoctypeDeclHandler;
    XML_EndDoctypeDeclHandler m_endDoctypeDeclHandler;
    XML_UnparsedEntityDeclHandler m_unparsedEntityDeclHandler;
    XML_NotationDeclHandler m_notationDeclHandler;
    XML_StartNamespaceDeclHandler m_startNamespaceDeclHandler;
    XML_EndNamespaceDeclHandler m_endNamespaceDeclHandler;
    XML_NotStandaloneHandler m_notStandaloneHandler;
    XML_ExternalEntityRefHandler m_externalEntityRefHandler;
    void *m_externalEntityRefHandlerArg;
    XML_UnknownEncodingHandler m_unknownEncodingHandler;
    XML_ElementDeclHandler m_elementDeclHandler;
    XML_AttlistDeclHandler m_attlistDeclHandler;
    XML_EntityDeclHandler m_entityDeclHandler;
    XML_XmlDeclHandler m_xmlDeclHandler;
    const ENCODING *m_encoding;
    INIT_ENCODING m_initEncoding;
    const ENCODING *m_internalEncoding;
    const XML_Char *m_protocolEncodingName;
    int m_ns;
    int m_ns_triplets;
    void *m_unknownEncodingMem;
    void *m_unknownEncodingData;
    void *m_unknownEncodingHandlerData;
    void (*m_unknownEncodingRelease) (void *);
    PROLOG_STATE m_prologState;
    Processor *m_processor;
    XMLERROR m_errorCode;
    const char *m_eventPtr;
    const char *m_eventEndPtr;
    const char *m_positionPtr;
    OPEN_INTERNAL_ENTITY *m_openInternalEntities;
    int m_defaultExpandInternalEntities;
    int m_tagLevel;
    ENTITY *m_declEntity;
    const XML_Char *m_doctypeName;
    const XML_Char *m_doctypeSysid;
    const XML_Char *m_doctypePubid;
    const XML_Char *m_declAttributeType;
    const XML_Char *m_declNotationName;
    const XML_Char *m_declNotationPublicId;
    ELEMENT_TYPE *m_declElementType;
    ATTRIBUTE_ID *m_declAttributeId;
    char m_declAttributeIsCdata;
    char m_declAttributeIsId;
    DTD m_dtd;
    const XML_Char *m_curBase;
    TAG *m_tagStack;
    TAG *m_freeTagList;
    BINDING *m_inheritedBindings;
    BINDING *m_freeBindingList;
    int m_attsSize;
    int m_nSpecifiedAtts;
    int m_idAttIndex;
    ATTRIBUTE *m_atts;
    POSITION m_position;
    STRING_POOL m_tempPool;
    STRING_POOL m_temp2Pool;
    char *m_groupConnector;
    unsigned m_groupSize;
    int m_hadExternalDoctype;
    XML_Char m_namespaceSeparator;
#ifdef XML_DTD
    enum XML_ParamEntityParsing m_paramEntityParsing;
    XML_Parser m_parentParser;
#endif
}
Parser;

#define MALLOC(s) (((Parser *)parser)->m_mem.malloc_fcn((s)))
#define REALLOC(p,s) (((Parser *)parser)->m_mem.realloc_fcn((p),(s)))
#define FREE(p) (((Parser *)parser)->m_mem.free_fcn((p)))

#define userData (((Parser *)parser)->m_userData)
#define handlerArg (((Parser *)parser)->m_handlerArg)
#define startElementHandler (((Parser *)parser)->m_startElementHandler)
#define endElementHandler (((Parser *)parser)->m_endElementHandler)
#define characterDataHandler (((Parser *)parser)->m_characterDataHandler)
#define processingInstructionHandler (((Parser *)parser)->m_processingInstructionHandler)
#define commentHandler (((Parser *)parser)->m_commentHandler)
#define startCdataSectionHandler (((Parser *)parser)->m_startCdataSectionHandler)
#define endCdataSectionHandler (((Parser *)parser)->m_endCdataSectionHandler)
#define defaultHandler (((Parser *)parser)->m_defaultHandler)
#define startDoctypeDeclHandler (((Parser *)parser)->m_startDoctypeDeclHandler)
#define endDoctypeDeclHandler (((Parser *)parser)->m_endDoctypeDeclHandler)
#define unparsedEntityDeclHandler (((Parser *)parser)->m_unparsedEntityDeclHandler)
#define notationDeclHandler (((Parser *)parser)->m_notationDeclHandler)
#define startNamespaceDeclHandler (((Parser *)parser)->m_startNamespaceDeclHandler)
#define endNamespaceDeclHandler (((Parser *)parser)->m_endNamespaceDeclHandler)
#define notStandaloneHandler (((Parser *)parser)->m_notStandaloneHandler)
#define externalEntityRefHandler (((Parser *)parser)->m_externalEntityRefHandler)
#define externalEntityRefHandlerArg (((Parser *)parser)->m_externalEntityRefHandlerArg)
#define internalEntityRefHandler (((Parser *)parser)->m_internalEntityRefHandler)
#define unknownEncodingHandler (((Parser *)parser)->m_unknownEncodingHandler)
#define elementDeclHandler (((Parser *)parser)->m_elementDeclHandler)
#define attlistDeclHandler (((Parser *)parser)->m_attlistDeclHandler)
#define entityDeclHandler (((Parser *)parser)->m_entityDeclHandler)
#define xmlDeclHandler (((Parser *)parser)->m_xmlDeclHandler)
#define encoding (((Parser *)parser)->m_encoding)
#define initEncoding (((Parser *)parser)->m_initEncoding)
#define internalEncoding (((Parser *)parser)->m_internalEncoding)
#define unknownEncodingMem (((Parser *)parser)->m_unknownEncodingMem)
#define unknownEncodingData (((Parser *)parser)->m_unknownEncodingData)
#define unknownEncodingHandlerData \
  (((Parser *)parser)->m_unknownEncodingHandlerData)
#define unknownEncodingRelease (((Parser *)parser)->m_unknownEncodingRelease)
#define protocolEncodingName (((Parser *)parser)->m_protocolEncodingName)
#define ns (((Parser *)parser)->m_ns)
#define ns_triplets (((Parser *)parser)->m_ns_triplets)
#define prologState (((Parser *)parser)->m_prologState)
#define processor (((Parser *)parser)->m_processor)
#define errorCode (((Parser *)parser)->m_errorCode)
#define eventPtr (((Parser *)parser)->m_eventPtr)
#define eventEndPtr (((Parser *)parser)->m_eventEndPtr)
#define positionPtr (((Parser *)parser)->m_positionPtr)
#define position (((Parser *)parser)->m_position)
#define openInternalEntities (((Parser *)parser)->m_openInternalEntities)
#define defaultExpandInternalEntities (((Parser *)parser)->m_defaultExpandInternalEntities)
#define tagLevel (((Parser *)parser)->m_tagLevel)
#define buffer (((Parser *)parser)->m_buffer)
#define bufferPtr (((Parser *)parser)->m_bufferPtr)
#define bufferEnd (((Parser *)parser)->m_bufferEnd)
#define parseEndByteIndex (((Parser *)parser)->m_parseEndByteIndex)
#define parseEndPtr (((Parser *)parser)->m_parseEndPtr)
#define bufferLim (((Parser *)parser)->m_bufferLim)
#define dataBuf (((Parser *)parser)->m_dataBuf)
#define dataBufEnd (((Parser *)parser)->m_dataBufEnd)
#define dtd (((Parser *)parser)->m_dtd)
#define curBase (((Parser *)parser)->m_curBase)
#define declEntity (((Parser *)parser)->m_declEntity)
#define doctypeName (((Parser *)parser)->m_doctypeName)
#define doctypeSysid (((Parser *)parser)->m_doctypeSysid)
#define doctypePubid (((Parser *)parser)->m_doctypePubid)
#define declAttributeType (((Parser *)parser)->m_declAttributeType)
#define declNotationName (((Parser *)parser)->m_declNotationName)
#define declNotationPublicId (((Parser *)parser)->m_declNotationPublicId)
#define declElementType (((Parser *)parser)->m_declElementType)
#define declAttributeId (((Parser *)parser)->m_declAttributeId)
#define declAttributeIsCdata (((Parser *)parser)->m_declAttributeIsCdata)
#define declAttributeIsId (((Parser *)parser)->m_declAttributeIsId)
#define freeTagList (((Parser *)parser)->m_freeTagList)
#define freeBindingList (((Parser *)parser)->m_freeBindingList)
#define inheritedBindings (((Parser *)parser)->m_inheritedBindings)
#define tagStack (((Parser *)parser)->m_tagStack)
#define atts (((Parser *)parser)->m_atts)
#define attsSize (((Parser *)parser)->m_attsSize)
#define nSpecifiedAtts (((Parser *)parser)->m_nSpecifiedAtts)
#define idAttIndex (((Parser *)parser)->m_idAttIndex)
#define tempPool (((Parser *)parser)->m_tempPool)
#define temp2Pool (((Parser *)parser)->m_temp2Pool)
#define groupConnector (((Parser *)parser)->m_groupConnector)
#define groupSize (((Parser *)parser)->m_groupSize)
#define hadExternalDoctype (((Parser *)parser)->m_hadExternalDoctype)
#define namespaceSeparator (((Parser *)parser)->m_namespaceSeparator)
#ifdef XML_DTD
#define parentParser (((Parser *)parser)->m_parentParser)
#define paramEntityParsing (((Parser *)parser)->m_paramEntityParsing)
#endif /* XML_DTD */

#ifdef COMPILED_FROM_DSP
BOOL WINAPI DllMain(HINSTANCE h, DWORD r, LPVOID p)
{
    return TRUE;
}
#endif /* def COMPILED_FROM_DSP */

#ifdef _MSC_VER
#ifdef _DEBUG
Parser *asParser(XML_Parser parser)
{
    return parser;
}
#endif
#endif

/*
 *@@ XML_ParserCreate:
 *      constructs a new parser. If encoding is non-null, it specifies
 *      a character encoding to use for the document (see below).
 *      This overrides the document encoding declaration.
 *
 *      Expat is a stream-oriented parser. You register callback (or
 *      handler) functions with the parser and then start feeding it
 *      the document. As the parser recognizes parts of the document,
 *      it will call the appropriate handler for that part (if you've
 *      registered one). The document is fed to the parser in pieces,
 *      so you can start parsing before you have all the document.
 *      This also allows you to parse really huge documents that won't
 *      fit into memory.
 *
 *      Expat can be intimidating due to the many kinds of handlers
 *      and options you can set. But you only need to learn four
 *      functions in order to do 90% of what you'll want to do with it:
 *
 *      -- XML_ParserCreate: Create a new parser object.
 *
 *      -- XML_SetElementHandler: Set handlers for start and end tags.
 *
 *      -- XML_SetCharacterDataHandler: Set handler for text.
 *
 *      -- XML_Parse: Pass a buffer full of document to the parser.
 *
 *      So the first step in parsing an XML document with expat is
 *      to create a parser object. There are three functions in the
 *      expat API for creating a parser object. However, only two of
 *      these (XML_ParserCreate and XML_ParserCreateNS) can be used
 *      for constructing a parser for a top-level document. The object
 *      returned by these functions is an opaque pointer (i.e. expat.h
 *      declares it as void*) to data with further internal structure.
 *      In order to free the memory associated with this object you
 *      must call XML_ParserFree. Note that if you have provided
 *      any user data that gets stored in the parser, then your
 *      application is responsible for freeing it prior to calling
 *      XML_ParserFree.
 *
 *      The objects returned by the parser creation functions are
 *      good for parsing only one XML document or external parsed
 *      entity. If your application needs to parse many XML documents,
 *      then it needs to create a parser object for each one. The
 *      best way to deal with this is to create a higher level object
 *      that contains all the default initialization you want for
 *      your parser objects.
 *
 *      Walking through a document hierarchy with a stream oriented
 *      parser will require a good stack mechanism in order to keep
 *      track of current context. For instance, to answer the simple
 *      question, "What element does this text belong to?" requires
 *      a stack, since the parser may have descended into other
 *      elements that are children of the current one and has
 *      encountered this text on the way out.
 *
 *      The things you're likely to want to keep on a stack are the
 *      currently opened element and its attributes. You push this
 *      information onto the stack in the start handler and you pop
 *      it off in the end handler.
 *
 *      For some tasks, it is sufficient to just keep information on
 *      what the depth of the stack is (or would be if you had one).
 *      The outline program shown above presents one example. Another
 *      such task would be skipping over a complete element. When you
 *      see the start tag for the element you want to skip, you set a
 *      skip flag and record the depth at which the element started.
 *      When the end tag handler encounters the same depth, the
 *      skipped element has ended and the flag may be cleared. If you
 *      follow the convention that the root element starts at 1, then
 *      you can use the same variable for skip flag and skip depth.
 *
 *      <B>Namespace Processing</B>
 *
 *      When the parser is created using the XML_ParserCreateNS,
 *      function, expat performs namespace processing. See that
 *      function for details.
 *
 *      <B>Character Encodings</B>
 *
 *      While XML is based on Unicode, and every XML processor is
 *      required to recognized UTF-8 and UTF-16 (1 and 2 byte
 *      encodings of Unicode), other @encodings may be declared in
 *      XML documents or entities. For the main document, an XML
 *      declaration may contain an encoding declaration in its
 *      @text_declaration. See @encodings for additional
 *      information.
 *
 *      With expat, you may also specify an encoding at the time
 *      of creating a parser. This is useful when the encoding
 *      information may come from a source outside the document
 *      itself (like a higher level protocol).
 *
 *      See XML_SetUnknownEncodingHandler for encodings directly
 *      supported by expat and for how to handle other encodings.
 *
 *      One pitfall that novice expat users are likely to fall into is
 *      that although expat may accept input in various encodings, the
 *      strings that it passes to the handlers are always encoded in
 *      UTF-8. Your application is responsible for any translation of
 *      these strings into other encodings.
 *
 *      <B>Handling External Entity References</B>
 *
 *      Expat does not read or parse @external_entities directly. Note that
 *      any external @DTD is a special case of an external entity. For how to
 *      handle external entities, see XML_SetExternalEntityRefHandler.
 *
 *      <B>Parsing Parameter Entities</B>
 *
 *      In order to parse @parameter_entities, before starting the parse,
 *      you must call XML_SetParamEntityParsing.
 */

XML_Parser XML_ParserCreate(const XML_Char * encodingName)
{
    return XML_ParserCreate_MM(encodingName, NULL, NULL);
}

/*
 *@@ XML_ParserCreateNS:
 *      constructs a new parser that has namespace processing
 *      in effect. Namespace expanded element names and attribute
 *      names are returned as a concatenation of the namespace URI,
 *      sep, and the local part of the name. This means that you
 *      should pick a character for sep that can't be part of a
 *      legal URI.
 *
 *      Under namespace processing, expat consumes xmlns and xmlns:...
 *      attributes, which declare namespaces for the scope of the
 *      element in which they occur. This means that your start
 *      handler will not see these attributes. Your application can
 *      still be informed of these declarations by setting namespace
 *      declaration handlers with XML_SetNamespaceDeclHandler.
 *
 *      Element type and attribute names that belong to a given
 *      namespace are passed to the appropriate handler in expanded
 *      form. By default this expanded form is a concatenation of the
 *      namespace URI, the separator character (which is the 2nd
 *      argument to XML_ParserCreateNS), and the local name (i.e.
 *      the part after the colon). Names with undeclared prefixes
 *      are passed through to the handlers unchanged, with the prefix
 *      and colon still attached. Unprefixed attribute names are never
 *      expanded, and unprefixed element names are only expanded when
 *      they are in the scope of a default namespace.
 *
 *      However if XML_SetReturnNSTriplet has been called with a
 *      non-zero do_nst parameter, then the expanded form for names
 *      with an explicit prefix is a concatenation of: URI, separator,
 *      local name, separator, prefix.
 *
 *      You can set handlers for the start of a namespace declaration
 *      and for the end of a scope of a declaration with the
 *      XML_SetNamespaceDeclHandler function. The
 *      StartNamespaceDeclHandler is called prior to the start
 *      tag handler and the EndNamespaceDeclHandler is called before
 *      the corresponding end tag that ends the namespace's scope.
 *      The namespace start handler gets passed the prefix and URI
 *      for the namespace. For a default namespace declaration
 *      (xmlns='...'), the prefix will be null. The URI will be null
 *      for the case where the default namespace is being unset. The
 *      namespace end handler just gets the prefix for the closing
 *      scope.
 *
 *      These handlers are called for each declaration. So if, for
 *      instance, a start tag had three namespace declarations, then
 *      the StartNamespaceDeclHandler would be called three times
 *      before the start tag handler is called, once for each
 *      declaration.
 *
 *      The namespace.c example demonstrates the use of these
 *      features. Like outline.c, it produces an outline, but in
 *      addition it annotates when a namespace scope starts and
 *      when it ends. This example also demonstrates use of
 *      application user data.
 */

XML_Parser XML_ParserCreateNS(const XML_Char * encodingName,
                              XML_Char nsSep)
{
    XML_Char tmp[2];

    *tmp = nsSep;
    return XML_ParserCreate_MM(encodingName, NULL, tmp);
}

void myfree( void *__ptr )
{
	free(__ptr);
};

void *mymalloc( size_t __size )
{
	return malloc(__size);
};

void *myrealloc( void *__ptr, size_t __size )
{
	return realloc(__ptr, __size);
};

/*
 *@@ XML_ParserCreate_MM:
 *      constructs a new parser using the suite of memory handling
 *      functions specified in ms. If ms is NULL, then use the
 *      standard set of memory management functions. If sep is
 *      non-NULL, then namespace processing is enabled in the
 *      created parser and the character pointed at by sep is
 *      used as the separator between the namespace URI and the
 *      local part of the name.
 */

XML_Parser XML_ParserCreate_MM(const XML_Char * encodingName,
                               const XML_Memory_Handling_Suite * memsuite,
                               const XML_Char * nameSep)
{

    XML_Parser parser;
    static
    const XML_Char implicitContext[] =
    {
        XML_T('x'), XML_T('m'), XML_T('l'), XML_T('='),
        XML_T('h'), XML_T('t'), XML_T('t'), XML_T('p'), XML_T(':'),
        XML_T('/'), XML_T('/'), XML_T('w'), XML_T('w'), XML_T('w'),
        XML_T('.'), XML_T('w'), XML_T('3'),
        XML_T('.'), XML_T('o'), XML_T('r'), XML_T('g'),
        XML_T('/'), XML_T('X'), XML_T('M'), XML_T('L'),
        XML_T('/'), XML_T('1'), XML_T('9'), XML_T('9'), XML_T('8'),
        XML_T('/'), XML_T('n'), XML_T('a'), XML_T('m'), XML_T('e'),
        XML_T('s'), XML_T('p'), XML_T('a'), XML_T('c'), XML_T('e'),
        XML_T('\0')
    };


    if (memsuite)
    {
        XML_Memory_Handling_Suite *mtemp;

        parser = memsuite->malloc_fcn(sizeof(Parser));
        mtemp = &(((Parser *) parser)->m_mem);
        mtemp->malloc_fcn = memsuite->malloc_fcn;
        mtemp->realloc_fcn = memsuite->realloc_fcn;
        mtemp->free_fcn = memsuite->free_fcn;
    }
    else
    {
        XML_Memory_Handling_Suite *mtemp;

        parser = malloc(sizeof(Parser));
        mtemp = &(((Parser *) parser)->m_mem);
        mtemp->malloc_fcn = mymalloc;
        mtemp->realloc_fcn = myrealloc;
        mtemp->free_fcn = myfree;
    }

    if (!parser)
        return parser;
    processor = prologInitProcessor;
    XmlPrologStateInit(&prologState);
    userData = 0;
    handlerArg = 0;
    startElementHandler = 0;
    endElementHandler = 0;
    characterDataHandler = 0;
    processingInstructionHandler = 0;
    commentHandler = 0;
    startCdataSectionHandler = 0;
    endCdataSectionHandler = 0;
    defaultHandler = 0;
    startDoctypeDeclHandler = 0;
    endDoctypeDeclHandler = 0;
    unparsedEntityDeclHandler = 0;
    notationDeclHandler = 0;
    startNamespaceDeclHandler = 0;
    endNamespaceDeclHandler = 0;
    notStandaloneHandler = 0;
    externalEntityRefHandler = 0;
    externalEntityRefHandlerArg = parser;
    unknownEncodingHandler = 0;
    elementDeclHandler = 0;
    attlistDeclHandler = 0;
    entityDeclHandler = 0;
    xmlDeclHandler = 0;
    buffer = 0;
    bufferPtr = 0;
    bufferEnd = 0;
    parseEndByteIndex = 0;
    parseEndPtr = 0;
    bufferLim = 0;
    declElementType = 0;
    declAttributeId = 0;
    declEntity = 0;
    doctypeName = 0;
    doctypeSysid = 0;
    doctypePubid = 0;
    declAttributeType = 0;
    declNotationName = 0;
    declNotationPublicId = 0;
    memset(&position, 0, sizeof(POSITION));
    errorCode = ERROR_EXPAT_NONE;
    eventPtr = 0;
    eventEndPtr = 0;
    positionPtr = 0;
    openInternalEntities = 0;
    tagLevel = 0;
    tagStack = 0;
    freeTagList = 0;
    freeBindingList = 0;
    inheritedBindings = 0;
    attsSize = INIT_ATTS_SIZE;
    atts = (ATTRIBUTE*)MALLOC(attsSize * sizeof(ATTRIBUTE));
    nSpecifiedAtts = 0;
    dataBuf = (XML_Char*)MALLOC(INIT_DATA_BUF_SIZE * sizeof(XML_Char));
    groupSize = 0;
    groupConnector = 0;
    hadExternalDoctype = 0;
    unknownEncodingMem = 0;
    unknownEncodingRelease = 0;
    unknownEncodingData = 0;
    unknownEncodingHandlerData = 0;
    namespaceSeparator = '!';
#ifdef XML_DTD
    parentParser = 0;
    paramEntityParsing = XML_PARAM_ENTITY_PARSING_NEVER;
#endif
    ns = 0;
    ns_triplets = 0;
    poolInit(&tempPool, &(((Parser *) parser)->m_mem));
    poolInit(&temp2Pool, &(((Parser *) parser)->m_mem));
    protocolEncodingName = encodingName ? poolCopyString(&tempPool, encodingName) : 0;
    curBase = 0;
    if (!dtdInit(&dtd, parser) || !atts || !dataBuf
        || (encodingName && !protocolEncodingName))
    {
        XML_ParserFree(parser);
        return 0;
    }
    dataBufEnd = dataBuf + INIT_DATA_BUF_SIZE;

    if (nameSep)
    {
        XmlInitEncodingNS(&initEncoding, &encoding, 0);
        ns = 1;
        internalEncoding = XmlGetInternalEncodingNS();
        namespaceSeparator = *nameSep;

        if (!setContext(parser, implicitContext))
        {
            XML_ParserFree(parser);
            return 0;
        }
    }
    else
    {
        XmlInitEncoding(&initEncoding, &encoding, 0);
        internalEncoding = XmlGetInternalEncoding();
    }

    return parser;
}                               /* End XML_ParserCreate_MM */

/*
 *@@ XML_SetEncoding:
 *      sets the encoding to be used by the parser. It is
 *      equivalent to passing a non-null encoding argument
 *      to the parser creation functions. It must not be
 *      called after XML_Parser or XML_ParseBuffer have
 *      been called on the given parser.
 */

int XML_SetEncoding(XML_Parser parser,
                    const XML_Char * encodingName)
{
    if (!encodingName)
        protocolEncodingName = 0;
    else
    {
        protocolEncodingName = poolCopyString(&tempPool, encodingName);
        if (!protocolEncodingName)
            return 0;
    }
    return 1;
}

/*
 *@@ XML_ExternalEntityParserCreate:
 *      constructs a new XML_Parser object for parsing an external
 *      general entity. Context is the context argument passed in a
 *      call to a ExternalEntityRefHandler. Other state information
 *      such as handlers, user data, namespace processing is
 *      inherited from the parser passed as the 1st argument. So you
 *      shouldn't need to call any of the behavior changing
 *      functions on this parser (unless you want it to act
 *      differently than the parent parser.)
 */

XML_Parser XML_ExternalEntityParserCreate(XML_Parser oldParser,
                                          const XML_Char * context,
                                          const XML_Char * encodingName)
{
    XML_Parser parser = oldParser;
    DTD *oldDtd = &dtd;
    XML_StartElementHandler oldStartElementHandler = startElementHandler;
    XML_EndElementHandler oldEndElementHandler = endElementHandler;
    XML_CharacterDataHandler oldCharacterDataHandler = characterDataHandler;
    XML_ProcessingInstructionHandler oldProcessingInstructionHandler = processingInstructionHandler;
    XML_CommentHandler oldCommentHandler = commentHandler;
    XML_StartCdataSectionHandler oldStartCdataSectionHandler = startCdataSectionHandler;
    XML_EndCdataSectionHandler oldEndCdataSectionHandler = endCdataSectionHandler;
    XML_DefaultHandler oldDefaultHandler = defaultHandler;
    XML_UnparsedEntityDeclHandler oldUnparsedEntityDeclHandler = unparsedEntityDeclHandler;
    XML_NotationDeclHandler oldNotationDeclHandler = notationDeclHandler;
    XML_StartNamespaceDeclHandler oldStartNamespaceDeclHandler = startNamespaceDeclHandler;
    XML_EndNamespaceDeclHandler oldEndNamespaceDeclHandler = endNamespaceDeclHandler;
    XML_NotStandaloneHandler oldNotStandaloneHandler = notStandaloneHandler;
    XML_ExternalEntityRefHandler oldExternalEntityRefHandler = externalEntityRefHandler;
    XML_UnknownEncodingHandler oldUnknownEncodingHandler = unknownEncodingHandler;
    XML_ElementDeclHandler oldElementDeclHandler = elementDeclHandler;
    XML_AttlistDeclHandler oldAttlistDeclHandler = attlistDeclHandler;
    XML_EntityDeclHandler oldEntityDeclHandler = entityDeclHandler;
    XML_XmlDeclHandler oldXmlDeclHandler = xmlDeclHandler;
    ELEMENT_TYPE *oldDeclElementType = declElementType;

    void *oldUserData = userData;
    void *oldHandlerArg = handlerArg;
    int oldDefaultExpandInternalEntities = defaultExpandInternalEntities;
    void *oldExternalEntityRefHandlerArg = externalEntityRefHandlerArg;

#ifdef XML_DTD
    enum XML_ParamEntityParsing oldParamEntityParsing = paramEntityParsing;

#endif
    int oldns_triplets = ns_triplets;

    if (ns)
    {
        XML_Char tmp[2];

        *tmp = namespaceSeparator;
        parser = XML_ParserCreate_MM(encodingName, &((Parser *) parser)->m_mem,
                                     tmp);
    }
    else
    {
        parser = XML_ParserCreate_MM(encodingName, &((Parser *) parser)->m_mem,
                                     NULL);
    }

    if (!parser)
        return 0;

    startElementHandler = oldStartElementHandler;
    endElementHandler = oldEndElementHandler;
    characterDataHandler = oldCharacterDataHandler;
    processingInstructionHandler = oldProcessingInstructionHandler;
    commentHandler = oldCommentHandler;
    startCdataSectionHandler = oldStartCdataSectionHandler;
    endCdataSectionHandler = oldEndCdataSectionHandler;
    defaultHandler = oldDefaultHandler;
    unparsedEntityDeclHandler = oldUnparsedEntityDeclHandler;
    notationDeclHandler = oldNotationDeclHandler;
    startNamespaceDeclHandler = oldStartNamespaceDeclHandler;
    endNamespaceDeclHandler = oldEndNamespaceDeclHandler;
    notStandaloneHandler = oldNotStandaloneHandler;
    externalEntityRefHandler = oldExternalEntityRefHandler;
    unknownEncodingHandler = oldUnknownEncodingHandler;
    elementDeclHandler = oldElementDeclHandler;
    attlistDeclHandler = oldAttlistDeclHandler;
    entityDeclHandler = oldEntityDeclHandler;
    xmlDeclHandler = oldXmlDeclHandler;
    declElementType = oldDeclElementType;
    userData = oldUserData;
    if (oldUserData == oldHandlerArg)
        handlerArg = userData;
    else
        handlerArg = parser;
    if (oldExternalEntityRefHandlerArg != oldParser)
        externalEntityRefHandlerArg = oldExternalEntityRefHandlerArg;
    defaultExpandInternalEntities = oldDefaultExpandInternalEntities;
    ns_triplets = oldns_triplets;
#ifdef XML_DTD
    paramEntityParsing = oldParamEntityParsing;
    if (context)
    {
#endif /* XML_DTD */
        if (!dtdCopy(&dtd, oldDtd, parser) || !setContext(parser, context))
        {
            XML_ParserFree(parser);
            return 0;
        }
        processor = externalEntityInitProcessor;
#ifdef XML_DTD
    }
    else
    {
        dtdSwap(&dtd, oldDtd);
        parentParser = oldParser;
        XmlPrologStateInitExternalEntity(&prologState);
        dtd.complete = 1;
        hadExternalDoctype = 1;
    }
#endif /* XML_DTD */
    return parser;
}

STATIC void destroyBindings(BINDING * bindings,
                            XML_Parser parser)
{
    for (;;)
    {
        BINDING *b = bindings;

        if (!b)
            break;
        bindings = b->nextTagBinding;
        FREE(b->uri);
        FREE(b);
    }
}

/*
 *@@ XML_ParserFree:
 *      free memory used by the parser. Your application is
 *      responsible for freeing any memory associated
 *      with UserData.
 */

void XML_ParserFree(XML_Parser parser)
{
    for (;;)
    {
        TAG *p;

        if (tagStack == 0)
        {
            if (freeTagList == 0)
                break;
            tagStack = freeTagList;
            freeTagList = 0;
        }
        p = tagStack;
        tagStack = tagStack->parent;
        FREE(p->buf);
        destroyBindings(p->bindings, parser);
        FREE(p);
    }
    destroyBindings(freeBindingList, parser);
    destroyBindings(inheritedBindings, parser);
    poolDestroy(&tempPool);
    poolDestroy(&temp2Pool);
#ifdef XML_DTD
    if (parentParser)
    {
        if (hadExternalDoctype)
            dtd.complete = 0;
        dtdSwap(&dtd, &((Parser *) parentParser)->m_dtd);
    }
#endif /* XML_DTD */
    dtdDestroy(&dtd, parser);
    FREE((void *)atts);
    if (groupConnector)
        FREE(groupConnector);
    if (buffer)
        FREE(buffer);
    FREE(dataBuf);
    if (unknownEncodingMem)
        FREE(unknownEncodingMem);
    if (unknownEncodingRelease)
        unknownEncodingRelease(unknownEncodingData);
    FREE(parser);
}

/*
 *@@ XML_UseParserAsHandlerArg:
 *      after this is called, handlers receive the parser in the
 *      userData argument (see XML_SetUserData). The userData
 *      information can still be obtained using the XML_GetUserData
 *      function.
 */

void XML_UseParserAsHandlerArg(XML_Parser parser)
{
    handlerArg = parser;
}

/*
 *@@ XML_SetReturnNSTriplet:
 *      this function only has an effect when using a parser
 *      created with XML_ParserCreateNS, i.e. when namespace
 *      processing is in effect. The do_nst sets whether or
 *      not prefixes are returned with names qualified with
 *      a namespace prefix. If this function is called with
 *      do_nst non-zero, then afterwards namespace qualified
 *      names (that is qualified with a prefix as opposed to
 *      belonging to a default namespace) are returned as a
 *      triplet with the three parts separated by the namespace
 *      separator specified when the parser was created. The
 *      order of returned parts is URI, local name, and prefix.
 *
 *      If do_nst is zero, then namespaces are reported in
 *      the default manner, URI then local_name separated by
 *      the namespace separator.
 */

void XML_SetReturnNSTriplet(XML_Parser parser,
                            int do_nst)
{
    ns_triplets = do_nst;
}

/*
 *@@ XML_SetUserData:
 *      sets the user data pointer that gets passed to handlers.
 *
 *      It overwrites any previous value for this pointer. Note that
 *      the application is responsible for freeing the memory
 *      associated with userData when it is finished with the parser.
 *      So if you call this when there's already a pointer there, and
 *      you haven't freed the memory associated with it, then you've
 *      probably just leaked memory.
 *
 *      Also see XML_UseParserAsHandlerArg.
 */

void XML_SetUserData(XML_Parser parser,
                     void *p)
{
    if (handlerArg == userData)
        handlerArg = userData = p;
    else
        userData = p;
}

/*
 *@@ XML_SetBase:
 *      set the base to be used for resolving relative URIs in
 *      system identifiers. The return value is 0 if there's no
 *      memory to store base, otherwise it's non-zero.
 */

int XML_SetBase(XML_Parser parser,
                const XML_Char * p)
{
    if (p)
    {
        p = poolCopyString(&dtd.pool, p);
        if (!p)
            return 0;
        curBase = p;
    }
    else
        curBase = 0;
    return 1;
}

/*
 *@@ XML_GetBase:
 *      returns the base for resolving relative URIs.
 *      See XML_SetBase.
 */

const XML_Char *XML_GetBase(XML_Parser parser)
{
    return curBase;
}

/*
 *@@ XML_GetSpecifiedAttributeCount:
 *      when attributes are reported to the start handler in the
 *      atts vector, attributes that were explicitly set in the
 *      element occur before any attributes that receive their
 *      value from default information in an ATTLIST declaration.
 *      This function returns the number of attributes that were
 *      explicitly set times two, thus giving the offset in the
 *      atts array passed to the start tag handler of the first
 *      attribute set due to defaults. It supplies information
 *      for the last call to a start handler. If called inside a
 *      start handler, then that means the current call.
 */

int XML_GetSpecifiedAttributeCount(XML_Parser parser)
{
    return nSpecifiedAtts;
}

/*
 *@@ XML_GetIdAttributeIndex:
 *      returns the index of the ID attribute passed in the atts
 *      array in the last call to XML_StartElementHandler, or -1
 *      if there is no ID attribute. If called inside a start
 *      handler, then that means the current call.
 */

int XML_GetIdAttributeIndex(XML_Parser parser)
{
    return idAttIndex;
}

/*
 *@@ XML_SetElementHandler:
 *      sets handlers for start and end tags with one call.
 *
 *      See XML_SetStartElementHandler and XML_SetEndElementHandler
 *      for details.
 */

void XML_SetElementHandler(XML_Parser parser,
                           XML_StartElementHandler start,
                           XML_EndElementHandler end)
{
    startElementHandler = start;
    endElementHandler = end;
}

/*
 *@@ XML_SetStartElementHandler:
 *      sets handler for start (and empty) tags.
 *
 *      This handler must have the following prototype:
 *
 +          void EXPATENTRY StartElementHandler(void *pUserData,
 +                                              const XML_Char *name,
 +                                              const XML_Char **atts);
 +
 *      "data" is the user data pointer set with XML_SetUserData.
 *
 *      "name" is the element name.
 *
 *      Attributes are passed to the start handler as a pointer
 *      to a vector of char pointers. Each attribute seen in a
 *      start (or empty) tag occupies 2 consecutive places in
 *      this vector: the attribute name followed by the attribute
 *      value. These pairs are terminated by a null pointer.
 *
 *      Note that an empty tag generates a call to both start
 *      and end handlers (in that order).
 *
 *      Although handlers are typically set prior to parsing and left
 *      alone, an application may choose to set or change the handler
 *      for a parsing event while the parse is in progress. For
 *      instance, your application may choose to ignore all text not
 *      descended from a para element. One way it could do this is to
 *      set the character handler when a para start tag is seen, and
 *      unset it for the corresponding end tag.
 *
 *      A handler may be unset by providing a NULL pointer to the
 *      appropriate handler setter. None of the handler setting
 *      functions have a return value.
 *
 *      Your handlers will be receiving strings in arrays of type
 *      XML_Char. This type is defined in expat.h as char* and
 *      contains bytes encoding UTF-8. Note that you'll receive them
 *      in this form independent of the original encoding of the
 *      document.
 */

void XML_SetStartElementHandler(XML_Parser parser,
                                XML_StartElementHandler start)
{
    startElementHandler = start;
}

/*
 *@@ XML_SetEndElementHandler:
 *      sets handler for end (and empty) tags. As noted above, an
 *      empty tag generates a call to both start and end handlers.
 *
 *      Handler prototype:
 +
 +          void EXPATENTRY EndElementHandler(void *pUserData,
 +                                            const XML_Char *name);
 */

void XML_SetEndElementHandler(XML_Parser parser,
                              XML_EndElementHandler end)
{
    endElementHandler = end;
}

/*
 *@@ XML_SetCharacterDataHandler:
 *      sets a character data (text, @content) handler.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY CharacterDataHandler(void *pUserData,
 +                                               const XML_Char *s,
 +                                               int len);
 +
 *      The string your handler receives is NOT zero terminated.
 *      You have to use the length argument to deal with the end
 *      of the string. A single block of contiguous text free of
 *      markup may still result in a sequence of calls to this
 *      handler. In other words, if you're searching for a pattern
 *      in the text, it may be split across calls to this handler.
 */

void XML_SetCharacterDataHandler(XML_Parser parser,
                                 XML_CharacterDataHandler handler)
{
    characterDataHandler = handler;
}

/*
 *@@ XML_SetProcessingInstructionHandler:
 *      sets a handler for processing instructions.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY ProcessingInstructionHandler(void *pUserData,
 +                                                       const XML_Char *target,
 +                                                       const XML_Char *data);
 +
 *      The target is the first word in the processing instruction. The data
 *      is the rest of the characters in it after skipping all
 *      whitespace after the initial word.
 */

void XML_SetProcessingInstructionHandler(XML_Parser parser,
                                     XML_ProcessingInstructionHandler handler)
{
    processingInstructionHandler = handler;
}

/*
 *@@ XML_SetCommentHandler:
 *      sets a handler for comments. The data is all text inside
 *      the comment delimiters.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY CommentHandler(void *pUserData,
 +                                         const XML_Char *data);
 *
 */

void XML_SetCommentHandler(XML_Parser parser,
                           XML_CommentHandler handler)
{
    commentHandler = handler;
}

/*
 *@@ XML_SetCdataSectionHandler:
 *      combination of XML_SetStartCdataSectionHandler and
 *      XML_SetEndCdataSectionHandler in one call.
 */

void XML_SetCdataSectionHandler(XML_Parser parser,
                                XML_StartCdataSectionHandler start,
                                XML_EndCdataSectionHandler end)
{
    startCdataSectionHandler = start;
    endCdataSectionHandler = end;
}

/*
 *@@ XML_SetCdataSectionHandler:
 *      sets a handler that gets called at the beginning of a CDATA
 *      section.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY StartCdataSectionHandler(void *pUserData);
 *
 */

void XML_SetStartCdataSectionHandler(XML_Parser parser,
                                     XML_StartCdataSectionHandler start)
{
    startCdataSectionHandler = start;
}

/*
 *@@ XML_SetCdataSectionHandler:
 *      sets a handler that gets called at the end of a CDATA
 *      section.
 *
 *      Handler prototype:
 *
 *          void EXPATENTRY EndCdataSectionHandler(void *pUserData);
 */

void XML_SetEndCdataSectionHandler(XML_Parser parser,
                                   XML_EndCdataSectionHandler end)
{
    endCdataSectionHandler = end;
}

/*
 *@@ XML_SetDefaultHandler:
 *      sets a handler for any characters in the document for
 *      which there is no applicable handler.
 *
 *      This includes both data for which no handlers can be set
 *      (like some kinds of DTD declarations) and data which could
 *      be reported but for which there has no handler supplied.
 *
 *      Handler prototype:
 +
 +          void EXPATENTRY DefaultHandler(void *pUserData,
 +                                         const XML_Char *s,
 +                                         int len);
 *
 *      The characters are passed exactly as they were in the XML
 *      document except that they will be encoded in UTF-8.  Line
 *      boundaries are not normalized.
 *
 *      Note that a byte order mark character is not passed to the
 *      default handler.
 *
 *      Note that a contiguous piece of data that is destined to
 *      be reported to the default handler may actually be reported
 *      over several calls to the handler.
 *
 *      Setting the handler with this call has the side effect of
 *      turning off expansion of references to internally defined
 *      general entities. Instead these references are passed to
 *      the default handler. To avoid that, use XML_SetDefaultHandlerExpand.
 */

void XML_SetDefaultHandler(XML_Parser parser,
                           XML_DefaultHandler handler)
{
    defaultHandler = handler;
    defaultExpandInternalEntities = 0;
}

/*
 *@@ XML_SetDefaultHandlerExpand:
 *      this sets a default handler, but doesn't affect expansion of
 *      internal entity references.
 *
 *      See XML_SetDefaultHandler.
 */

void XML_SetDefaultHandlerExpand(XML_Parser parser,
                                 XML_DefaultHandler handler)
{
    defaultHandler = handler;
    defaultExpandInternalEntities = 1;
}

/*
 *@@ XML_SetDoctypeDeclHandler:
 *      combination of XML_SetStartDoctypeDeclHandler and
 *      XML_SetEndDoctypeDeclHandler.
 */

void XML_SetDoctypeDeclHandler(XML_Parser parser,
                               XML_StartDoctypeDeclHandler start,
                               XML_EndDoctypeDeclHandler end)
{
    startDoctypeDeclHandler = start;
    endDoctypeDeclHandler = end;
}

/*
 *@@ XML_SetStartDoctypeDeclHandler:
 *      sets a handler that is called at the start of a DOCTYPE
 *      declaration, before any external or internal subset is
 *      parsed.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY StartDoctypeDeclHandler(void *pUserData,
 +                                                  const XML_Char *pcszDoctypeName,
 +                                                  const XML_Char *pcszSysid,
 +                                                  const XML_Char *pcszPubid,
 +                                                  int fHasInternalSubset);
 *
 *      Both pcszSysid and pcszPubid may be NULL. "fHasInternalSubset"
 *      will be non-zero if the DOCTYPE declaration has an internal subset.
 */

void XML_SetStartDoctypeDeclHandler(XML_Parser parser,
                                    XML_StartDoctypeDeclHandler start)
{
    startDoctypeDeclHandler = start;
}

/*
 *@@ XML_SetEndDoctypeDeclHandler:
 *      sets a handler that is called at the end of a DOCTYPE
 *      declaration, after parsing any external subset.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY EndDoctypeDeclHandler(void *pUserData);
 *
 */

void XML_SetEndDoctypeDeclHandler(XML_Parser parser,
                                  XML_EndDoctypeDeclHandler end)
{
    endDoctypeDeclHandler = end;
}

/*
 *@@ XML_SetUnparsedEntityDeclHandler:
 *      sets a handler that receives declarations of unparsed
 *      entities. These are entity declarations that have a
 *      notation (NDATA) field:
 *
 +           <!ENTITY logo SYSTEM "images/logo.gif" NDATA gif>
 *
 *      This handler is obsolete and is provided for backwards
 *      compatibility. Use instead XML_SetEntityDeclHandler.
 */

void XML_SetUnparsedEntityDeclHandler(XML_Parser parser,
                                      XML_UnparsedEntityDeclHandler handler)
{
    unparsedEntityDeclHandler = handler;
}

/*
 *@@ XML_SetNotationDeclHandler:
 *      sets a handler that receives notation declarations.
 *
 *      Handler prototype:
 +
 +      void EXPATENTRY NotationDeclHandler(void *pUserData,
 +                                          const XML_Char *pcszNotationName,
 +                                          const XML_Char *pcszBase,
 +                                          const XML_Char *pcszSystemId,
 +                                          const XML_Char *pcszPublicId);
 +
 */

void XML_SetNotationDeclHandler(XML_Parser parser,
                                XML_NotationDeclHandler handler)
{
    notationDeclHandler = handler;
}

/*
 *@@ XML_SetNamespaceDeclHandler:
 *      combination of XML_SetEndNamespaceDeclHandler and
 *      XML_SetEndNamespaceDeclHandler.
 */

void XML_SetNamespaceDeclHandler(XML_Parser parser,
                                 XML_StartNamespaceDeclHandler start,
                                 XML_EndNamespaceDeclHandler end)
{
    startNamespaceDeclHandler = start;
    endNamespaceDeclHandler = end;
}

/*
 *@@ XML_SetStartNamespaceDeclHandler:
 *      sets a handler to be called when a namespace is declared.
 *      Namespace declarations occur inside start tags. But the
 *      namespace declaration start handler is called before the
 *      start tag handler for each namespace declared in that start
 *      tag.
 *
 *      Handler prototype:
 *
 +      void EXPATENTRY StartNamespaceDeclHandler(void *pUserData,
 +                                                const XML_Char *prefix,
 +                                                const XML_Char *uri);
 +
 */

void XML_SetStartNamespaceDeclHandler(XML_Parser parser,
                                      XML_StartNamespaceDeclHandler start)
{
    startNamespaceDeclHandler = start;
}

/*
 *@@ XML_SetEndNamespaceDeclHandler:
 *      sets a handler to be called when leaving the scope of a
 *      namespace declaration. This will be called, for each
 *      namespace declaration, after the handler for the end tag
 *      of the element in which the namespace was declared.
 *
 *      Handler prototype:
 +
 +      void EXPATENTRY EndNamespaceDeclHandler(void *pUserData,
 +                                              const XML_Char *prefix);
 */

void XML_SetEndNamespaceDeclHandler(XML_Parser parser,
                                    XML_EndNamespaceDeclHandler end)
{
    endNamespaceDeclHandler = end;
}


/*
 *@@ XML_SetNotStandaloneHandler:
 *      sets a handler that is called if the document is not
 *      "standalone". This happens when there is an external
 *      subset or a reference to a parameter entity, but does
 *      not have standalone set to "yes" in an XML declaration.
 *      If this handler returns 0, then the parser will throw an
 *      ERROR_EXPAT_NOT_STANDALONE error.
 *
 *      Handler prototype:
 *
 +          int EXPATENTRY NotStandaloneHandler(void *pUserData);
 */

void XML_SetNotStandaloneHandler(XML_Parser parser,
                                 XML_NotStandaloneHandler handler)
{
    notStandaloneHandler = handler;
}

/*
 *@@ XML_SetExternalEntityRefHandler:
 *      sets a handler for references to @external_entities.
 *
 *      This handler is also called for processing an external DTD
 *      subset if parameter entity parsing is in effect.
 *      (See XML_SetParamEntityParsing.)
 *
 *      Warning: If you have set no ExternalEntityRefHandler, then
 *      external entity references are silently ignored. Otherwise,
 *      they trigger a call to your handler with the information
 *      needed to read and parse the external entity.
 *
 *      Handler prototype:
 +
 +      int EXPATENTRY ExternalEntityRefHandler(void *pUser,
 +                                              XML_Parser parser,
 +                                              const XML_Char *pcszContext,
 +                                              const XML_Char *pcszBase,
 +                                              const XML_Char *pcszSystemId,
 +                                              const XML_Char *pcszPublicId);
 +
 *      The pcszContext argument specifies the parsing context in the
 *      format expected by the context argument to
 *      XML_ExternalEntityParserCreate; pcszContext is valid only until
 *      the handler returns, so if the referenced entity is to be
 *      parsed later, it must be copied.
 *
 *      The pcszBase parameter is the base to use for relative system
 *      identifiers. It is set by XML_SetBase and may be null.
 *
 *      The pcszPublicId parameter is the public id given in the entity
 *      declaration and may be null.
 *
 *      The pcszSystemId is the system identifier specified in the
 *      entity declaration and is never null.
 *
 *      There are a couple of ways in which this handler differs
 *      from others. First, this handler returns an integer. A
 *      non-zero value should be returned for successful handling
 *      of the external entity reference. Returning a zero indicates
 *      failure, and causes the calling parser to return an
 *      ERROR_EXPAT_EXTERNAL_ENTITY_HANDLING error.
 *
 *      Second, instead of having pUserData as its first argument,
 *      it receives the parser that encountered the entity reference.
 *      This, along with the context parameter, may be used as
 *      arguments to a call to XML_ExternalEntityParserCreate.
 *      Using the returned parser, the body of the external entity
 *      can be recursively parsed.
 *
 *      Since this handler may be called recursively, it should not
 *      be saving information into global or static variables.
 *
 *      Your handler isn't actually responsible for parsing the entity,
 *      but it is responsible for creating a subsidiary parser with
 *      XML_ExternalEntityParserCreate that will do the job. That returns
 *      an instance of XML_Parser that has handlers and other data
 *      structures initialized from the parent parser. You may then use
 *      XML_Parse or XML_ParseBuffer calls against that parser. Since
 *      external entities may refer to other external entities, your
 *      handler should be prepared to be called recursively.
 *
 *@@changed V0.9.14 (2001-08-09) [umoeller]: changed prototype to contain user data
 */

void XML_SetExternalEntityRefHandler(XML_Parser parser,
                                     XML_ExternalEntityRefHandler handler)
{
    externalEntityRefHandler = handler;
}

void XML_SetExternalEntityRefHandlerArg(XML_Parser parser, void *arg)
{
    if (arg)
        externalEntityRefHandlerArg = arg;
    else
        externalEntityRefHandlerArg = parser;
}

/*
 *@@ XML_SetUnknownEncodingHandler:
 *      sets a handler to deal with @encodings other than the
 *      built-in set.
 *
 *      There are four built-in encodings in expat:
 *
 *      --   UTF-8: 8-bit encoding of Unicode.
 *
 *      --   UTF-16: 16-bit encoding of Unicode.
 *
 *      --   ISO-8859-1: that's "latin 1".
 *
 *      --   US-ASCII
 *
 *      Anything else discovered in an encoding declaration or in
 *      the protocol encoding specified in the parser constructor
 *      triggers a call to the UnknownEncodingHandler.
 *
 *      Handler prototype:
 *
 +          int EXPATENTRY UnknownEncodingHandler(void *encodingHandlerData,
 +                                                const XML_Char *name,
 +                                                XML_Encoding *info);
 +
 *      The encodingHandlerData argument is that which was passed as the
 *      second argument to XML_SetUnknownEncodingHandler.
 *
 *      The name argument gives the name of the encoding as specified in
 *      the encoding declaration.
 *
 *      If the callback can provide information about the encoding,
 *      it must fill in the XML_Encoding structure, and return 1.
 *      Otherwise it must return 0.
 *      If info does not describe a suitable encoding,
 *      then the parser will return an XML_UNKNOWN_ENCODING error.
 *
 *      See _XML_Encoding for more details about that structure and
 *      what to do with it, and restrictions imposed by expat.
 *
 */

void XML_SetUnknownEncodingHandler(XML_Parser parser,
                                   XML_UnknownEncodingHandler handler,
                                   void *data)
{
    unknownEncodingHandler = handler;
    unknownEncodingHandlerData = data;
}

/*
 *@@ XML_SetElementDeclHandler:
 *      sets a handler for an @element_declaration in a @DTD. The
 *      handler gets called with the name of the element in
 *      the declaration and a pointer to a structure that contains
 *      the element model. It is the application's responsibility
 *      to free this data structure.
 *
 *      This handler must have the following prototype:
 *
 +          void EXPATENTRY ElementDeclHandler(void *pUserData,
 +                                             const XML_Char *name,
 +                                             XMLCONTENT *model);
 *
 *      See _XMLCONTENT for details.
 */

void XML_SetElementDeclHandler(XML_Parser parser,
                               XML_ElementDeclHandler eldecl)
{
    elementDeclHandler = eldecl;
}

/*
 *@@ XML_SetAttlistDeclHandler:
 *      sets a handler for an @attribute_declaration in the @DTD.
 *
 *      This handler must have the following prototype:
 *
 +          void EXPATENTRY AttlistDeclHandler(void *pUserData,
 +                                             const XML_Char *pcszElementName,
 +                                             const XML_Char *pcszAttribName,
 +                                             const XML_Char *pcszAttribType,
 +                                             const XML_Char *pcszDefault,
 +                                             int fIsRequired);
 *
 *      This handler is called for each attribute. So a single attlist
 *      declaration with multiple attributes declared will generate
 *      multiple calls to this handler.
 *
 *      --  pcszElementName is the name of the  element for which the
 *          attribute is being declared.
 *
 *      --  pcszAttribName has the attribute name being declared.
 *
 *      --  pcszAttribType is the attribute type.
 *          It is the string representing the type in the declaration
 *          with whitespace removed.
 *
 *      --  pcszDefault holds the default value. It will be
 *          NULL in the case of "#IMPLIED" or "#REQUIRED" attributes.
 *          You can distinguish these two cases by checking the
 *          fIsRequired parameter, which will be true in the case of
 *          "#REQUIRED" attributes. Attributes which are "#FIXED"
 *          will have also have a TRUE fIsRequired, but they will have
 *          the non-NULL fixed value in the pcszDefault parameter.
 */

void XML_SetAttlistDeclHandler(XML_Parser parser,
                               XML_AttlistDeclHandler attdecl)
{
    attlistDeclHandler = attdecl;
}

/*
 *@@ XML_SetEntityDeclHandler:
 *      sets a handler that will be called for all entity declarations.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY EntityDeclHandler(void *pUserData,
 +                                            const XML_Char *pcszEntityName,
 +                                            int fIsParameterEntity,
 +                                            const XML_Char *pcszValue,
 +                                            int iValueLength,
 +                                            const XML_Char *pcszBase,
 +                                            const XML_Char *pcszSystemId,
 +                                            const XML_Char *pcszPublicId,
 +                                            const XML_Char *pcszNotationName);
 +
 *      The fIsParameterEntity argument will be non-zero in the case
 *      of parameter entities and zero otherwise.
 *
 *      For internal entities (<!ENTITY foo "bar">), pcszValue will be
 *      non-NULL and pcszSystemId, pcszPublicId, and pcszNotationName
 *      will all be NULL. The value string is not NULL terminated; the
 *      lengthis provided in the iValueLength parameter. Do not use
 *      iValueLength to test for internal entities, since it is legal
 *      to have zero-length values. Instead check for whether or not
 *      pcszValue is NULL.
 *
 *      The pcszNotationName argument will have a non-NULL value only
 *      for unparsed entity declarations.
 */

void XML_SetEntityDeclHandler(XML_Parser parser,
                              XML_EntityDeclHandler handler)
{
    entityDeclHandler = handler;
}

/*
 *@@ XML_SetXmlDeclHandler:
 *      sets a handler that is called for XML declarations and also
 *      for text declarations discovered in external entities.
 *
 *      Handler prototype:
 *
 +          void EXPATENTRY XmlDeclHandler(void *pUserData,
 +                                         const XML_Char *pcszVersion,
 +                                         const XML_Char *pcszEncoding,
 +                                         int standalone);
 *
 *      The way to distinguish is that the version parameter will
 *      be NULL for text declarations. The encoding parameter may
 *      be NULL for an XML declaration. The standalone argument will
 *      contain -1, 0, or 1 indicating respectively that there was no
 *      standalone parameter in the declaration, that it was given
 *      as no, or that it was given as yes.
 */

void XML_SetXmlDeclHandler(XML_Parser parser,
                           XML_XmlDeclHandler handler)
{
    xmlDeclHandler = handler;
}

/*
 *@@ XML_SetParamEntityParsing:
 *      this enables parsing of @parameter_entities, including the
 *      external parameter entity that is the external @DTD subset,
 *      according to code.
 *
 *      If parsing of parameter entities is enabled, then references
 *      to external parameter entities (including the external DTD
 *      subset) will be passed to the handler set with
 *      XML_SetExternalEntityRefHandler. The context passed will be 0.
 *
 *      Unlike external general entities, external parameter entities
 *      can only be parsed synchronously. If the external parameter
 *      entity is to be parsed, it must be parsed during the call to
 *      the external entity ref handler: the complete sequence of
 *      XML_ExternalEntityParserCreate, XML_Parse/XML_ParseBuffer and
 *      XML_ParserFree calls must be made during this call.
 *
 *      After XML_ExternalEntityParserCreate has been called to create
 *      the parser for the external parameter entity (context must be 0
 *      for this call), it is illegal to make any calls on the old parser
 *      until XML_ParserFree has been called on the newly created parser.
 *      If the library has been compiled without support for parameter
 *      entity parsing (ie without XML_DTD being defined), then
 *      XML_SetParamEntityParsing will return 0 if parsing of parameter
 *      entities is requested; otherwise it will return non-zero.
 *
 *      NOTE: The xwphelpers implementation #defines XML_DTD in
 *      include\expat\expat_setup.h, which is included in all expat
 *      files.
 *
 *      The choices for code are:
 *
 *      --  XML_PARAM_ENTITY_PARSING_NEVER:
 *          Don't parse parameter entities or the external subset.
 *          This is the only choice if XML_DTD is not defined.
 *
 *      --  XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE:
 *          Parse parameter entities and the external subset unless
 *          standalone was set to "yes" in the XML declaration.
 *
 *      --  XML_PARAM_ENTITY_PARSING_ALWAYS
 *          Always parse parameter entities and the external subset.
 *
 *      In order to read an external DTD, you also have to set an
 *      external entity reference handler (XML_SetExternalEntityRefHandler).
 */

int XML_SetParamEntityParsing(XML_Parser parser,
                              enum XML_ParamEntityParsing parsing)
{
#ifdef XML_DTD
    paramEntityParsing = parsing;
    return 1;
#else
    return parsing == XML_PARAM_ENTITY_PARSING_NEVER;
#endif
}

/*
 *@@ XML_Parse:
 *      parses some more of the document.
 *
 *      The string s is a buffer containing part (or perhaps all)
 *      of the document. The number of bytes of s that are part
 *      of the document is indicated by len. This means that s
 *      doesn't have to be null terminated. It also means that
 *      if len is larger than the number of bytes in the block
 *      of memory that s points at, then a memory fault is
 *      likely. The isFinal parameter informs the parser that
 *      this is the last piece of the document. Frequently, the
 *      last piece is empty (i.e. len is zero.)
 *
 *      If a parse error occurred, it returns 0. Otherwise it
 *      returns a non-zero value.
 */

int XML_Parse(XML_Parser parser,
              const char *s,
              int len,
              int isFinal)
{
    if (len == 0)
    {
        if (!isFinal)
            return 1;
        positionPtr = bufferPtr;
        errorCode = processor(parser, bufferPtr, parseEndPtr = bufferEnd, 0);
        if (errorCode == ERROR_EXPAT_NONE)
            return 1;
        eventEndPtr = eventPtr;
        processor = errorProcessor;
        return 0;
    }
#ifndef XML_CONTEXT_BYTES
    else if (bufferPtr == bufferEnd)
    {
        const char *end;
        int nLeftOver;

        parseEndByteIndex += len;
        positionPtr = s;
        if (isFinal)
        {
            errorCode = processor(parser, s, parseEndPtr = s + len, 0);
            if (errorCode == ERROR_EXPAT_NONE)
                return 1;
            eventEndPtr = eventPtr;
            processor = errorProcessor;
            return 0;
        }
        errorCode = processor(parser, s, parseEndPtr = s + len, &end);
        if (errorCode != ERROR_EXPAT_NONE)
        {
            eventEndPtr = eventPtr;
            processor = errorProcessor;
            return 0;
        }
        XmlUpdatePosition(encoding, positionPtr, end, &position);
        nLeftOver = s + len - end;
        if (nLeftOver)
        {
            if (buffer == 0 || nLeftOver > bufferLim - buffer)
            {
                /* FIXME avoid integer overflow */
                buffer = buffer == 0 ? MALLOC(len * 2) : REALLOC(buffer, len * 2);
                /* FIXME storage leak if realloc fails */
                if (!buffer)
                {
                    errorCode = ERROR_EXPAT_NO_MEMORY;
                    eventPtr = eventEndPtr = 0;
                    processor = errorProcessor;
                    return 0;
                }
                bufferLim = buffer + len * 2;
            }
            memcpy(buffer, end, nLeftOver);
            bufferPtr = buffer;
            bufferEnd = buffer + nLeftOver;
        }
        return 1;
    }
#endif /* not defined XML_CONTEXT_BYTES */
    else
    {
        memcpy(XML_GetBuffer(parser, len), s, len);
        return XML_ParseBuffer(parser, len, isFinal);
    }
}

/*
 *@@ XML_ParseBuffer:
 *      this is just like XML_Parse, except in this case expat
 *      provides the buffer. By obtaining the buffer from expat
 *      with the XML_GetBuffer function, the application can
 *      avoid double copying of the input.
 */

int XML_ParseBuffer(XML_Parser parser,
                    int len,
                    int isFinal)
{
    const char *start = bufferPtr;

    positionPtr = start;
    bufferEnd += len;
    parseEndByteIndex += len;
    errorCode = processor(parser, start, parseEndPtr = bufferEnd,
                          isFinal ? (const char **)0 : &bufferPtr);
    if (errorCode == ERROR_EXPAT_NONE)
    {
        if (!isFinal)
            XmlUpdatePosition(encoding, positionPtr, bufferPtr, &position);
        return 1;
    }
    else
    {
        eventEndPtr = eventPtr;
        processor = errorProcessor;
        return 0;
    }
}

/*
 *@@ XML_GetBuffer:
 *      obtains a buffer of size len to read a piece of the
 *      document into. A NULL value is returned if expat can't
 *      allocate enough memory for this buffer. This has to be
 *      called prior to every call to XML_ParseBuffer.
 *
 *      A typical use would look like this:
 *
 +      for (;;)
 +      {
 +          int bytes_read;
 +          void *buff = XML_GetBuffer(p, BUFF_SIZE);
 +          if (buff == NULL)
 +              // handle error
 +
 +          bytes_read = read(docfd, buff, BUFF_SIZE);
 +          if (bytes_read < 0)
 +              // handle error
 +
 +          if (!XML_ParseBuffer(p, bytes_read, bytes_read == 0))
 +              // handle parse error
 +
 +          if (bytes_read == 0)
 +              break;
 +      }
 *
 */

void *XML_GetBuffer(XML_Parser parser,
                    int len)
{
    if (len > bufferLim - bufferEnd)
    {
        /* FIXME avoid integer overflow */
        int neededSize = len + (bufferEnd - bufferPtr);

#ifdef XML_CONTEXT_BYTES
        int keep = bufferPtr - buffer;

        if (keep > XML_CONTEXT_BYTES)
            keep = XML_CONTEXT_BYTES;
        neededSize += keep;
#endif /* defined XML_CONTEXT_BYTES */
        if (neededSize <= bufferLim - buffer)
        {
#ifdef XML_CONTEXT_BYTES
            if (keep < bufferPtr - buffer)
            {
                int offset = (bufferPtr - buffer) - keep;

                memmove(buffer, &buffer[offset], bufferEnd - bufferPtr + keep);
                bufferEnd -= offset;
                bufferPtr -= offset;
            }
#else
            memmove(buffer, bufferPtr, bufferEnd - bufferPtr);
            bufferEnd = buffer + (bufferEnd - bufferPtr);
            bufferPtr = buffer;
#endif /* not defined XML_CONTEXT_BYTES */
        }
        else
        {
            char *newBuf;
            int bufferSize = bufferLim - bufferPtr;

            if (bufferSize == 0)
                bufferSize = INIT_BUFFER_SIZE;
            do
            {
                bufferSize *= 2;
            }
            while (bufferSize < neededSize);
            newBuf = (char*)MALLOC(bufferSize);
            if (newBuf == 0)
            {
                errorCode = ERROR_EXPAT_NO_MEMORY;
                return 0;
            }
            bufferLim = newBuf + bufferSize;
#ifdef XML_CONTEXT_BYTES
            if (bufferPtr)
            {
                int keep = bufferPtr - buffer;

                if (keep > XML_CONTEXT_BYTES)
                    keep = XML_CONTEXT_BYTES;
                memcpy(newBuf, &bufferPtr[-keep], bufferEnd - bufferPtr + keep);
                FREE(buffer);
                buffer = newBuf;
                bufferEnd = buffer + (bufferEnd - bufferPtr) + keep;
                bufferPtr = buffer + keep;
            }
            else
            {
                bufferEnd = newBuf + (bufferEnd - bufferPtr);
                bufferPtr = buffer = newBuf;
            }
#else
            if (bufferPtr)
            {
                memcpy(newBuf, bufferPtr, bufferEnd - bufferPtr);
                FREE(buffer);
            }
            bufferEnd = newBuf + (bufferEnd - bufferPtr);
            bufferPtr = buffer = newBuf;
#endif /* not defined XML_CONTEXT_BYTES */
        }
    }
    return bufferEnd;
}

/*
 *@@ XML_GetErrorCode:
 *      returns what type of error has occurred.
 *
 *      To be called when the parse functions return 0 (i.e. a
 *      parse error has ocurred), although the position reporting
 *      functions are useful outside of errors. The position reported
 *      is the byte position (in the original document or entity
 *      encoding) of the first of the sequence of characters that
 *      generated the current event (or the error that caused the
 *      parse functions to return 0.)
 *
 *      The position reporting functions are accurate only outside
 *      of the DTD. In other words, they usually return bogus
 *      information when called from within a DTD declaration handler.
 */

XMLERROR XML_GetErrorCode(XML_Parser parser)
{
    return errorCode;
}

/*
 *@@ XML_GetCurrentByteIndex:
 *      returns the byte offset of the position.
 *
 *      To be called on parser errors. See XML_GetErrorCode.
 */

long XML_GetCurrentByteIndex(XML_Parser parser)
{
    if (eventPtr)
        return parseEndByteIndex - (parseEndPtr - eventPtr);
    return -1;
}

/*
 *@@ XML_GetCurrentByteCount:
 *      return the number of bytes in the current event. Returns
 *      0 if the event is inside a reference to an internal entity.
 *
 *      To be called on parser errors. See XML_GetErrorCode.
 */

int XML_GetCurrentByteCount(XML_Parser parser)
{
    if (eventEndPtr && eventPtr)
        return eventEndPtr - eventPtr;
    return 0;
}

/*
 *@@ XML_GetInputContext:
 *      returns the parser's input buffer, sets the integer pointed
 *      at by offset to the offset within this buffer of the current
 *      parse position, and set the integer pointed at by size to the
 *      size of the returned buffer.
 *
 *      This should only be called from within a handler during an
 *      active parse and the returned buffer should only be referred
 *      to from within the handler that made the call. This input
 *      buffer contains the untranslated bytes of the input.
 *
 *      Only a limited amount of context is kept, so if the event
 *      triggering a call spans over a very large amount of input,
 *      the actual parse position may be before the beginning of the buffer.
 */

const char *XML_GetInputContext(XML_Parser parser,
                                int *offset,
                                int *size)
{
#ifdef XML_CONTEXT_BYTES
    if (eventPtr && buffer)
    {
        *offset = eventPtr - buffer;
        *size = bufferEnd - buffer;
        return buffer;
    }
#endif /* defined XML_CONTEXT_BYTES */
    return (char *)0;
}

/*
 *@@ XML_GetCurrentLineNumber:
 *      returns the line number of the position.
 *
 *      Can be called on parser errors. See XML_GetErrorCode.
 */

int XML_GetCurrentLineNumber(XML_Parser parser)
{
    if (eventPtr)
    {
        XmlUpdatePosition(encoding, positionPtr, eventPtr, &position);
        positionPtr = eventPtr;
    }
    return position.lineNumber + 1;
}

/*
 *@@ XML_GetCurrentColumnNumber:
 *      returns the offset, from the beginning of the current
 *      line, of the position.
 *
 *      Can be called on parser errors. See XML_GetErrorCode.
 */

int XML_GetCurrentColumnNumber(XML_Parser parser)
{
    if (eventPtr)
    {
        XmlUpdatePosition(encoding, positionPtr, eventPtr, &position);
        positionPtr = eventPtr;
    }
    return position.columnNumber;
}

/*
 *@@ XML_DefaultCurrent:
 *      not in the expat docs.
 */

void XML_DefaultCurrent(XML_Parser parser)
{
    if (defaultHandler)
    {
        if (openInternalEntities)
            reportDefault(parser,
                          internalEncoding,
                          openInternalEntities->internalEventPtr,
                          openInternalEntities->internalEventEndPtr);
        else
            reportDefault(parser, encoding, eventPtr, eventEndPtr);
    }
}

const XML_LChar *XML_ErrorString(int code)
{
    static const XML_LChar *message[] =
    {
        0,
        XML_T("out of memory"),
        XML_T("syntax error"),
        XML_T("no element found"),
        XML_T("not well-formed (invalid token)"),
        XML_T("unclosed token"),
        XML_T("unclosed token"),
        XML_T("mismatched tag"),
        XML_T("duplicate attribute"),
        XML_T("junk after document element"),
        XML_T("illegal parameter entity reference"),
        XML_T("undefined entity"),
        XML_T("recursive entity reference"),
        XML_T("asynchronous entity"),
        XML_T("reference to invalid character number"),
        XML_T("reference to binary entity"),
        XML_T("reference to external entity in attribute"),
        XML_T("xml processing instruction not at start of external entity"),
        XML_T("unknown encoding"),
        XML_T("encoding specified in XML declaration is incorrect"),
        XML_T("unclosed CDATA section"),
        XML_T("error in processing external entity reference"),
        XML_T("document is not standalone"),
        XML_T("unexpected parser state - please send a bug report")
    };

    if (code > 0 && code < sizeof(message) / sizeof(message[0]))
        return message[code];
    return 0;
}

/* const XML_LChar *
 * XML_ExpatVersion(void) {
 * return VERSION;
 * }
 *
 * XML_Expat_Version
 * XML_ExpatVersionInfo(void) {
 * XML_Expat_Version version;
 *
 * version.major = XML_MAJOR_VERSION;
 * version.minor = XML_MINOR_VERSION;
 * version.micro = XML_MICRO_VERSION;
 *
 * return version;
 * } */

STATIC XMLERROR contentProcessor(XML_Parser parser,
                                 const char *start,
                                 const char *end,
                                 const char **endPtr)
{
    return doContent(parser, 0, encoding, start, end, endPtr);
}

STATIC XMLERROR externalEntityInitProcessor(XML_Parser parser,
                                            const char *start,
                                            const char *end,
                                            const char **endPtr)
{
    XMLERROR result = initializeEncoding(parser);

    if (result != ERROR_EXPAT_NONE)
        return result;
    processor = externalEntityInitProcessor2;
    return externalEntityInitProcessor2(parser, start, end, endPtr);
}

STATIC XMLERROR externalEntityInitProcessor2(XML_Parser parser,
                                             const char *start,
                                             const char *end,
                                             const char **endPtr)
{
    const char *next;
    int tok = XmlContentTok(encoding, start, end, &next);

    switch (tok)
    {
        case XML_TOK_BOM:
            start = next;
            break;
        case XML_TOK_PARTIAL:
            if (endPtr)
            {
                *endPtr = start;
                return ERROR_EXPAT_NONE;
            }
            eventPtr = start;
            return ERROR_EXPAT_UNCLOSED_TOKEN;
        case XML_TOK_PARTIAL_CHAR:
            if (endPtr)
            {
                *endPtr = start;
                return ERROR_EXPAT_NONE;
            }
            eventPtr = start;
            return ERROR_EXPAT_PARTIAL_CHAR;
    }
    processor = externalEntityInitProcessor3;
    return externalEntityInitProcessor3(parser, start, end, endPtr);
}

STATIC XMLERROR externalEntityInitProcessor3(XML_Parser parser,
                                             const char *start,
                                             const char *end,
                                             const char **endPtr)
{
    const char *next;
    int tok = XmlContentTok(encoding, start, end, &next);

    switch (tok)
    {
        case XML_TOK_XML_DECL:
            {
                XMLERROR result = processXmlDecl(parser, 1, start, next);

                if (result != ERROR_EXPAT_NONE)
                    return result;
                start = next;
            }
            break;
        case XML_TOK_PARTIAL:
            if (endPtr)
            {
                *endPtr = start;
                return ERROR_EXPAT_NONE;
            }
            eventPtr = start;
            return ERROR_EXPAT_UNCLOSED_TOKEN;
        case XML_TOK_PARTIAL_CHAR:
            if (endPtr)
            {
                *endPtr = start;
                return ERROR_EXPAT_NONE;
            }
            eventPtr = start;
            return ERROR_EXPAT_PARTIAL_CHAR;
    }
    processor = externalEntityContentProcessor;
    tagLevel = 1;
    return doContent(parser, 1, encoding, start, end, endPtr);
}

STATIC XMLERROR externalEntityContentProcessor(XML_Parser parser,
                                               const char *start,
                                               const char *end,
                                               const char **endPtr)
{
    return doContent(parser, 1, encoding, start, end, endPtr);
}

/*
 *@@ doContent:
 *
 *@@changed V0.9.14 (2001-08-09) [umoeller]: fixed ERROR_EXPAT_UNDEFINED_ENTITY with callback-defined encodings
 */

STATIC XMLERROR doContent(XML_Parser parser,
                          int startTagLevel,
                          const ENCODING * enc,
                          const char *s,
                          const char *end,
                          const char **nextPtr)
{
    const char **eventPP;
    const char **eventEndPP;

    if (enc == encoding)
    {
        eventPP = &eventPtr;
        eventEndPP = &eventEndPtr;
    }
    else
    {
        eventPP = &(openInternalEntities->internalEventPtr);
        eventEndPP = &(openInternalEntities->internalEventEndPtr);
    }
    *eventPP = s;
    for (;;)
    {
        const char *next = s;   /* XmlContentTok doesn't always set the last arg */
        int tok = XmlContentTok(enc, s, end, &next);

        *eventEndPP = next;
        switch (tok)
        {
            case XML_TOK_TRAILING_CR:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                *eventEndPP = end;
                if (characterDataHandler)
                {
                    XML_Char c = 0xA;

                    characterDataHandler(handlerArg, &c, 1);
                }
                else if (defaultHandler)
                    reportDefault(parser, enc, s, end);
                if (startTagLevel == 0)
                    return ERROR_EXPAT_NO_ELEMENTS;
                if (tagLevel != startTagLevel)
                    return ERROR_EXPAT_ASYNC_ENTITY;
                return ERROR_EXPAT_NONE;
            case XML_TOK_NONE:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                if (startTagLevel > 0)
                {
                    if (tagLevel != startTagLevel)
                        return ERROR_EXPAT_ASYNC_ENTITY;
                    return ERROR_EXPAT_NONE;
                }
                return ERROR_EXPAT_NO_ELEMENTS;
            case XML_TOK_INVALID:
                *eventPP = next;
                return ERROR_EXPAT_INVALID_TOKEN;
            case XML_TOK_PARTIAL:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                return ERROR_EXPAT_UNCLOSED_TOKEN;
            case XML_TOK_PARTIAL_CHAR:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                return ERROR_EXPAT_PARTIAL_CHAR;
            case XML_TOK_ENTITY_REF:
                {
                    const XML_Char *name;
                    ENTITY *entity;
                    unsigned long ulOfs; // V0.9.14 (2001-08-09) [umoeller]
                    XML_Char ch = XmlPredefinedEntityName(enc,
                                                          s + enc->minBytesPerChar,
                                                          next - enc->minBytesPerChar);

                    if (ch)
                    {
                        if (characterDataHandler)
                            characterDataHandler(handlerArg, &ch, 1);
                        else if (defaultHandler)
                            reportDefault(parser, enc, s, next);
                        break;
                    }
                    name = poolStoreString(&dtd.pool,
                                           enc,
                                           s + enc->minBytesPerChar,
                                           next - enc->minBytesPerChar,
                                           &ulOfs);
                    if (!name)
                        return ERROR_EXPAT_NO_MEMORY;
                    entity = (ENTITY*)lookup(&dtd.generalEntities,
                                             name + ulOfs,  // V0.9.14 (2001-08-09) [umoeller]
                                             0);
                    poolDiscard(&dtd.pool);
                    if (!entity)
                    {
                        if (dtd.complete || dtd.standalone)
                            return ERROR_EXPAT_UNDEFINED_ENTITY;
                        if (defaultHandler)
                            reportDefault(parser, enc, s, next);
                        break;
                    }
                    if (entity->open)
                        return ERROR_EXPAT_RECURSIVE_ENTITY_REF;
                    if (entity->notation)
                        return ERROR_EXPAT_BINARY_ENTITY_REF;
                    if (entity)
                    {
                        if (entity->textPtr)
                        {
                            XMLERROR result;
                            OPEN_INTERNAL_ENTITY openEntity;

                            if (defaultHandler && !defaultExpandInternalEntities)
                            {
                                reportDefault(parser, enc, s, next);
                                break;
                            }
                            entity->open = 1;
                            openEntity.next = openInternalEntities;
                            openInternalEntities = &openEntity;
                            openEntity.entity = entity;
                            openEntity.internalEventPtr = 0;
                            openEntity.internalEventEndPtr = 0;
                            result = doContent(parser,
                                               tagLevel,
                                               internalEncoding,
                                               (char *)entity->textPtr,
                                  (char *)(entity->textPtr + entity->textLen),
                                               0);
                            entity->open = 0;
                            openInternalEntities = openEntity.next;
                            if (result)
                                return result;
                        }
                        else if (externalEntityRefHandler)
                        {
                            const XML_Char *context;

                            entity->open = 1;
                            context = getContext(parser);
                            entity->open = 0;
                            if (!context)
                                return ERROR_EXPAT_NO_MEMORY;
                            if (!externalEntityRefHandler(handlerArg,   // V0.9.14 (2001-08-09) [umoeller]
                                                          externalEntityRefHandlerArg,
                                                          context,
                                                          entity->base,
                                                          entity->systemId,
                                                          entity->publicId))
                                return ERROR_EXPAT_EXTERNAL_ENTITY_HANDLING;
                            poolDiscard(&tempPool);
                        }
                        else if (defaultHandler)
                            reportDefault(parser, enc, s, next);
                    }
                    break;
                }
            case XML_TOK_START_TAG_WITH_ATTS:
                if (!startElementHandler)
                {
                    XMLERROR result = storeAtts(parser, enc, s, 0, 0);

                    if (result)
                        return result;
                }
                /* fall through */
            case XML_TOK_START_TAG_NO_ATTS:
                {
                    TAG *tag;

                    if (freeTagList)
                    {
                        tag = freeTagList;
                        freeTagList = freeTagList->parent;
                    }
                    else
                    {
                        tag = (TAG*)MALLOC(sizeof(TAG));
                        if (!tag)
                            return ERROR_EXPAT_NO_MEMORY;
                        tag->buf = (char*)MALLOC(INIT_TAG_BUF_SIZE);
                        if (!tag->buf)
                            return ERROR_EXPAT_NO_MEMORY;
                        tag->bufEnd = tag->buf + INIT_TAG_BUF_SIZE;
                    }
                    tag->bindings = 0;
                    tag->parent = tagStack;
                    tagStack = tag;
                    tag->name.localPart = 0;
                    tag->rawName = s + enc->minBytesPerChar;
                    tag->rawNameLength = XmlNameLength(enc, tag->rawName);
                    if (nextPtr)
                    {
                        /* Need to guarantee that:
                         * tag->buf + ROUND_UP(tag->rawNameLength, sizeof(XML_Char)) <= tag->bufEnd - sizeof(XML_Char) */
                        if (tag->rawNameLength + (int)(sizeof(XML_Char) - 1) + (int)sizeof(XML_Char) > tag->bufEnd - tag->buf)
                        {
                            int bufSize = tag->rawNameLength * 4;

                            bufSize = ROUND_UP(bufSize, sizeof(XML_Char));
                            tag->buf = (char*)REALLOC(tag->buf, bufSize);
                            if (!tag->buf)
                                return ERROR_EXPAT_NO_MEMORY;
                            tag->bufEnd = tag->buf + bufSize;
                        }
                        memcpy(tag->buf, tag->rawName, tag->rawNameLength);
                        tag->rawName = tag->buf;
                    }
                    ++tagLevel;
                    if (startElementHandler)
                    {
                        XMLERROR result;
                        XML_Char *toPtr;

                        for (;;)
                        {
                            const char *rawNameEnd = tag->rawName + tag->rawNameLength;
                            const char *fromPtr = tag->rawName;
                            int bufSize;

                            if (nextPtr)
                                toPtr = (XML_Char *) (tag->buf + ROUND_UP(tag->rawNameLength, sizeof(XML_Char)));
                            else
                                toPtr = (XML_Char *) tag->buf;
                            tag->name.str = toPtr;
                            XmlConvert(enc,
                                       &fromPtr, rawNameEnd,
                               (ICHAR **) & toPtr, (ICHAR *) tag->bufEnd - 1);
                            if (fromPtr == rawNameEnd)
                                break;
                            bufSize = (tag->bufEnd - tag->buf) << 1;
                            tag->buf = (char*)REALLOC(tag->buf, bufSize);
                            if (!tag->buf)
                                return ERROR_EXPAT_NO_MEMORY;
                            tag->bufEnd = tag->buf + bufSize;
                            if (nextPtr)
                                tag->rawName = tag->buf;
                        }
                        *toPtr = XML_T('\0');
                        result = storeAtts(parser, enc, s, &(tag->name), &(tag->bindings));
                        if (result)
                            return result;
                        startElementHandler(handlerArg, tag->name.str, (const XML_Char **)atts);
                        poolClear(&tempPool);
                    }
                    else
                    {
                        tag->name.str = 0;
                        if (defaultHandler)
                            reportDefault(parser, enc, s, next);
                    }
                    break;
                }
            case XML_TOK_EMPTY_ELEMENT_WITH_ATTS:
                if (!startElementHandler)
                {
                    XMLERROR result = storeAtts(parser, enc, s, 0, 0);

                    if (result)
                        return result;
                }
                /* fall through */
            case XML_TOK_EMPTY_ELEMENT_NO_ATTS:
                if (startElementHandler || endElementHandler)
                {
                    const char *rawName = s + enc->minBytesPerChar;
                    XMLERROR result;
                    BINDING *bindings = 0;
                    TAG_NAME name;

                    name.str = poolStoreString(&tempPool,
                                               enc,
                                               rawName,
                                               rawName + XmlNameLength(enc, rawName),
                                               NULL);
                    if (!name.str)
                        return ERROR_EXPAT_NO_MEMORY;
                    poolFinish(&tempPool);
                    result = storeAtts(parser, enc, s, &name, &bindings);
                    if (result)
                        return result;
                    poolFinish(&tempPool);
                    if (startElementHandler)
                        startElementHandler(handlerArg, name.str, (const XML_Char **)atts);
                    if (endElementHandler)
                    {
                        if (startElementHandler)
                            *eventPP = *eventEndPP;
                        endElementHandler(handlerArg, name.str);
                    }
                    poolClear(&tempPool);
                    while (bindings)
                    {
                        BINDING *b = bindings;

                        if (endNamespaceDeclHandler)
                            endNamespaceDeclHandler(handlerArg, b->prefix->name);
                        bindings = bindings->nextTagBinding;
                        b->nextTagBinding = freeBindingList;
                        freeBindingList = b;
                        b->prefix->binding = b->prevPrefixBinding;
                    }
                }
                else if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                if (tagLevel == 0)
                    return epilogProcessor(parser, next, end, nextPtr);
                break;
            case XML_TOK_END_TAG:
                if (tagLevel == startTagLevel)
                    return ERROR_EXPAT_ASYNC_ENTITY;
                else
                {
                    int len;
                    const char *rawName;
                    TAG *tag = tagStack;

                    tagStack = tag->parent;
                    tag->parent = freeTagList;
                    freeTagList = tag;
                    rawName = s + enc->minBytesPerChar * 2;
                    len = XmlNameLength(enc, rawName);
                    if (len != tag->rawNameLength
                        || memcmp(tag->rawName, rawName, len) != 0)
                    {
                        *eventPP = rawName;
                        return ERROR_EXPAT_TAG_MISMATCH;
                    }
                    --tagLevel;
                    if (endElementHandler && tag->name.str)
                    {
                        if (tag->name.localPart)
                        {
                            XML_Char *to = (XML_Char *) tag->name.str + tag->name.uriLen;
                            const XML_Char *from = tag->name.localPart;

                            while ((*to++ = *from++) != 0)
                                ;
                        }
                        endElementHandler(handlerArg, tag->name.str);
                    }
                    else if (defaultHandler)
                        reportDefault(parser, enc, s, next);
                    while (tag->bindings)
                    {
                        BINDING *b = tag->bindings;

                        if (endNamespaceDeclHandler)
                            endNamespaceDeclHandler(handlerArg, b->prefix->name);
                        tag->bindings = tag->bindings->nextTagBinding;
                        b->nextTagBinding = freeBindingList;
                        freeBindingList = b;
                        b->prefix->binding = b->prevPrefixBinding;
                    }
                    if (tagLevel == 0)
                        return epilogProcessor(parser, next, end, nextPtr);
                }
                break;
            case XML_TOK_CHAR_REF:
                {
                    int n = XmlCharRefNumber(enc, s);

                    if (n < 0)
                        return ERROR_EXPAT_BAD_CHAR_REF;
                    if (characterDataHandler)
                    {
                        XML_Char buf[XML_ENCODE_MAX];

                        characterDataHandler(handlerArg, buf, XmlEncode(n, (ICHAR *) buf));
                    }
                    else if (defaultHandler)
                        reportDefault(parser, enc, s, next);
                }
                break;
            case XML_TOK_XML_DECL:
                return ERROR_EXPAT_MISPLACED_XML_PI;
            case XML_TOK_DATA_NEWLINE:
                if (characterDataHandler)
                {
                    XML_Char c = 0xA;

                    characterDataHandler(handlerArg, &c, 1);
                }
                else if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                break;
            case XML_TOK_CDATA_SECT_OPEN:
                {
                    XMLERROR result;

                    if (startCdataSectionHandler)
                        startCdataSectionHandler(handlerArg);
#if 0
                    /* Suppose you doing a transformation on a document that involves
                     * changing only the character data.  You set up a defaultHandler
                     * and a characterDataHandler.  The defaultHandler simply copies
                     * characters through.  The characterDataHandler does the transformation
                     * and writes the characters out escaping them as necessary.  This case
                     * will fail to work if we leave out the following two lines (because &
                     * and < inside CDATA sections will be incorrectly escaped).
                     *
                     * However, now we have a start/endCdataSectionHandler, so it seems
                     * easier to let the user deal with this. */

                    else if (characterDataHandler)
                        characterDataHandler(handlerArg, dataBuf, 0);
#endif
                    else if (defaultHandler)
                        reportDefault(parser, enc, s, next);
                    result = doCdataSection(parser, enc, &next, end, nextPtr);
                    if (!next)
                    {
                        processor = cdataSectionProcessor;
                        return result;
                    }
                }
                break;
            case XML_TOK_TRAILING_RSQB:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                if (characterDataHandler)
                {
                    if (MUST_CONVERT(enc, s))
                    {
                        ICHAR *dataPtr = (ICHAR *) dataBuf;

                        XmlConvert(enc, &s, end, &dataPtr, (ICHAR *) dataBufEnd);
                        characterDataHandler(handlerArg, dataBuf, dataPtr - (ICHAR *) dataBuf);
                    }
                    else
                        characterDataHandler(handlerArg,
                                             (XML_Char *) s,
                                           (XML_Char *) end - (XML_Char *) s);
                }
                else if (defaultHandler)
                    reportDefault(parser, enc, s, end);
                if (startTagLevel == 0)
                {
                    *eventPP = end;
                    return ERROR_EXPAT_NO_ELEMENTS;
                }
                if (tagLevel != startTagLevel)
                {
                    *eventPP = end;
                    return ERROR_EXPAT_ASYNC_ENTITY;
                }
                return ERROR_EXPAT_NONE;
            case XML_TOK_DATA_CHARS:
                if (characterDataHandler)
                {
                    if (MUST_CONVERT(enc, s))
                    {
                        for (;;)
                        {
                            ICHAR *dataPtr = (ICHAR *) dataBuf;

                            XmlConvert(enc, &s, next, &dataPtr, (ICHAR *) dataBufEnd);
                            *eventEndPP = s;
                            characterDataHandler(handlerArg, dataBuf, dataPtr - (ICHAR *) dataBuf);
                            if (s == next)
                                break;
                            *eventPP = s;
                        }
                    }
                    else
                        characterDataHandler(handlerArg,
                                             (XML_Char *) s,
                                          (XML_Char *) next - (XML_Char *) s);
                }
                else if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                break;
            case XML_TOK_PI:
                if (!reportProcessingInstruction(parser, enc, s, next))
                    return ERROR_EXPAT_NO_MEMORY;
                break;
            case XML_TOK_COMMENT:
                if (!reportComment(parser, enc, s, next))
                    return ERROR_EXPAT_NO_MEMORY;
                break;
            default:
                if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                break;
        }
        *eventPP = s = next;
    }
    /* not reached */
}

/* If tagNamePtr is non-null, build a real list of attributes,
 * otherwise just check the attributes for well-formedness. */

STATIC XMLERROR storeAtts(XML_Parser parser,
                          const ENCODING * enc,
                          const char *attStr,
                          TAG_NAME * tagNamePtr,
                          BINDING ** bindingsPtr)
{
    ELEMENT_TYPE *elementType = 0;
    int nDefaultAtts = 0;
    const XML_Char **appAtts;   /* the attribute list to pass to the application */
    int attIndex = 0;
    int i;
    int n;
    int nPrefixes = 0;
    BINDING *binding;
    const XML_Char *localPart;

    /* lookup the element type name */
    if (tagNamePtr)
    {
        elementType = (ELEMENT_TYPE *) lookup(&dtd.elementTypes, tagNamePtr->str, 0);
        if (!elementType)
        {
            tagNamePtr->str = poolCopyString(&dtd.pool, tagNamePtr->str);
            if (!tagNamePtr->str)
                return ERROR_EXPAT_NO_MEMORY;
            elementType = (ELEMENT_TYPE *) lookup(&dtd.elementTypes, tagNamePtr->str, sizeof(ELEMENT_TYPE));
            if (!elementType)
                return ERROR_EXPAT_NO_MEMORY;
            if (ns && !setElementTypePrefix(parser, elementType))
                return ERROR_EXPAT_NO_MEMORY;
        }
        nDefaultAtts = elementType->nDefaultAtts;
    }
    /* get the attributes from the tokenizer */
    n = XmlGetAttributes(enc, attStr, attsSize, atts);
    if (n + nDefaultAtts > attsSize)
    {
        int oldAttsSize = attsSize;

        attsSize = n + nDefaultAtts + INIT_ATTS_SIZE;
        atts = (ATTRIBUTE*)REALLOC((void *)atts, attsSize * sizeof(ATTRIBUTE));
        if (!atts)
            return ERROR_EXPAT_NO_MEMORY;
        if (n > oldAttsSize)
            XmlGetAttributes(enc, attStr, n, atts);
    }
    appAtts = (const XML_Char **)atts;
    for (i = 0; i < n; i++)
    {
        /* add the name and value to the attribute list */
        ATTRIBUTE_ID *attId = getAttributeId(parser, enc, atts[i].name,
                                             atts[i].name
                                          + XmlNameLength(enc, atts[i].name));

        if (!attId)
            return ERROR_EXPAT_NO_MEMORY;
        /* detect duplicate attributes */
        if ((attId->name)[-1])
        {
            if (enc == encoding)
                eventPtr = atts[i].name;
            return ERROR_EXPAT_DUPLICATE_ATTRIBUTE;
        }
        (attId->name)[-1] = 1;
        appAtts[attIndex++] = attId->name;
        if (!atts[i].normalized)
        {
            XMLERROR result;
            int isCdata = 1;

            /* figure out whether declared as other than CDATA */
            if (attId->maybeTokenized)
            {
                int j;

                for (j = 0; j < nDefaultAtts; j++)
                {
                    if (attId == elementType->defaultAtts[j].id)
                    {
                        isCdata = elementType->defaultAtts[j].isCdata;
                        break;
                    }
                }
            }

            /* normalize the attribute value */
            result = storeAttributeValue(parser, enc, isCdata,
                                         atts[i].valuePtr, atts[i].valueEnd,
                                         &tempPool);
            if (result)
                return result;
            if (tagNamePtr)
            {
                appAtts[attIndex] = poolStart(&tempPool);
                poolFinish(&tempPool);
            }
            else
                poolDiscard(&tempPool);
        }
        else if (tagNamePtr)
        {
            /* the value did not need normalizing */
            appAtts[attIndex] = poolStoreString(&tempPool,
                                                enc,
                                                atts[i].valuePtr,
                                                atts[i].valueEnd,
                                                NULL);
            if (appAtts[attIndex] == 0)
                return ERROR_EXPAT_NO_MEMORY;
            poolFinish(&tempPool);
        }
        /* handle prefixed attribute names */
        if (attId->prefix && tagNamePtr)
        {
            if (attId->xmlns)
            {
                /* deal with namespace declarations here */
                if (!addBinding(parser, attId->prefix, attId, appAtts[attIndex], bindingsPtr))
                    return ERROR_EXPAT_NO_MEMORY;
                --attIndex;
            }
            else
            {
                /* deal with other prefixed names later */
                attIndex++;
                nPrefixes++;
                (attId->name)[-1] = 2;
            }
        }
        else
            attIndex++;
    }
    if (tagNamePtr)
    {
        int j;

        nSpecifiedAtts = attIndex;
        if (elementType->idAtt && (elementType->idAtt->name)[-1])
        {
            for (i = 0; i < attIndex; i += 2)
                if (appAtts[i] == elementType->idAtt->name)
                {
                    idAttIndex = i;
                    break;
                }
        }
        else
            idAttIndex = -1;
        /* do attribute defaulting */
        for (j = 0; j < nDefaultAtts; j++)
        {
            const DEFAULT_ATTRIBUTE *da = elementType->defaultAtts + j;

            if (!(da->id->name)[-1] && da->value)
            {
                if (da->id->prefix)
                {
                    if (da->id->xmlns)
                    {
                        if (!addBinding(parser, da->id->prefix, da->id, da->value, bindingsPtr))
                            return ERROR_EXPAT_NO_MEMORY;
                    }
                    else
                    {
                        (da->id->name)[-1] = 2;
                        nPrefixes++;
                        appAtts[attIndex++] = da->id->name;
                        appAtts[attIndex++] = da->value;
                    }
                }
                else
                {
                    (da->id->name)[-1] = 1;
                    appAtts[attIndex++] = da->id->name;
                    appAtts[attIndex++] = da->value;
                }
            }
        }
        appAtts[attIndex] = 0;
    }
    i = 0;
    if (nPrefixes)
    {
        /* expand prefixed attribute names */
        for (; i < attIndex; i += 2)
        {
            if (appAtts[i][-1] == 2)
            {
                ATTRIBUTE_ID *id;

                ((XML_Char *) (appAtts[i]))[-1] = 0;
                id = (ATTRIBUTE_ID *) lookup(&dtd.attributeIds, appAtts[i], 0);
                if (id->prefix->binding)
                {
                    int j;
                    const BINDING *b = id->prefix->binding;
                    const XML_Char *s = appAtts[i];

                    for (j = 0; j < b->uriLen; j++)
                    {
                        if (!poolAppendChar(&tempPool, b->uri[j]))
                            return ERROR_EXPAT_NO_MEMORY;
                    }
                    while (*s++ != ':')
                        ;
                    do
                    {
                        if (!poolAppendChar(&tempPool, *s))
                            return ERROR_EXPAT_NO_MEMORY;
                    }
                    while (*s++);
                    if (ns_triplets)
                    {
                        tempPool.ptr[-1] = namespaceSeparator;
                        s = b->prefix->name;
                        do
                        {
                            if (!poolAppendChar(&tempPool, *s))
                                return ERROR_EXPAT_NO_MEMORY;
                        }
                        while (*s++);
                    }

                    appAtts[i] = poolStart(&tempPool);
                    poolFinish(&tempPool);
                }
                if (!--nPrefixes)
                    break;
            }
            else
                ((XML_Char *) (appAtts[i]))[-1] = 0;
        }
    }
    /* clear the flags that say whether attributes were specified */
    for (; i < attIndex; i += 2)
        ((XML_Char *) (appAtts[i]))[-1] = 0;
    if (!tagNamePtr)
        return ERROR_EXPAT_NONE;
    for (binding = *bindingsPtr; binding; binding = binding->nextTagBinding)
        binding->attId->name[-1] = 0;
    /* expand the element type name */
    if (elementType->prefix)
    {
        binding = elementType->prefix->binding;
        if (!binding)
            return ERROR_EXPAT_NONE;
        localPart = tagNamePtr->str;
        while (*localPart++ != XML_T(':'))
            ;
    }
    else if (dtd.defaultPrefix.binding)
    {
        binding = dtd.defaultPrefix.binding;
        localPart = tagNamePtr->str;
    }
    else
        return ERROR_EXPAT_NONE;
    tagNamePtr->localPart = localPart;
    tagNamePtr->uriLen = binding->uriLen;
    for (i = 0; localPart[i++];)
        ;
    n = i + binding->uriLen;
    if (n > binding->uriAlloc)
    {
        TAG *p;
        XML_Char *uri = (XML_Char*)MALLOC((n + EXPAND_SPARE) * sizeof(XML_Char));

        if (!uri)
            return ERROR_EXPAT_NO_MEMORY;
        binding->uriAlloc = n + EXPAND_SPARE;
        memcpy(uri, binding->uri, binding->uriLen * sizeof(XML_Char));
        for (p = tagStack; p; p = p->parent)
            if (p->name.str == binding->uri)
                p->name.str = uri;
        FREE(binding->uri);
        binding->uri = uri;
    }
    memcpy(binding->uri + binding->uriLen, localPart, i * sizeof(XML_Char));
    tagNamePtr->str = binding->uri;
    return ERROR_EXPAT_NONE;
}

STATIC int addBinding(XML_Parser parser,
                      PREFIX * prefix,
                      const ATTRIBUTE_ID * attId,
                      const XML_Char * uri,
                      BINDING ** bindingsPtr)
{
    BINDING *b;
    int len;

    for (len = 0; uri[len]; len++)
        ;
    if (namespaceSeparator)
        len++;
    if (freeBindingList)
    {
        b = freeBindingList;
        if (len > b->uriAlloc)
        {
            b->uri = (XML_Char*)REALLOC(b->uri, sizeof(XML_Char) * (len + EXPAND_SPARE));
            if (!b->uri)
                return 0;
            b->uriAlloc = len + EXPAND_SPARE;
        }
        freeBindingList = b->nextTagBinding;
    }
    else
    {
        b = (BINDING*)MALLOC(sizeof(BINDING));
        if (!b)
            return 0;
        b->uri = (XML_Char*)MALLOC(sizeof(XML_Char) * (len + EXPAND_SPARE));
        if (!b->uri)
        {
            FREE(b);
            return 0;
        }
        b->uriAlloc = len + EXPAND_SPARE;
    }
    b->uriLen = len;
    memcpy(b->uri, uri, len * sizeof(XML_Char));
    if (namespaceSeparator)
        b->uri[len - 1] = namespaceSeparator;
    b->prefix = prefix;
    b->attId = attId;
    b->prevPrefixBinding = prefix->binding;
    if (*uri == XML_T('\0') && prefix == &dtd.defaultPrefix)
        prefix->binding = 0;
    else
        prefix->binding = b;
    b->nextTagBinding = *bindingsPtr;
    *bindingsPtr = b;
    if (startNamespaceDeclHandler)
        startNamespaceDeclHandler(handlerArg, prefix->name,
                                  prefix->binding ? uri : 0);
    return 1;
}

/* The idea here is to avoid using stack for each CDATA section when
 * the whole file is parsed with one call. */

STATIC XMLERROR cdataSectionProcessor(XML_Parser parser,
                                      const char *start,
                                      const char *end,
                                      const char **endPtr)
{
    XMLERROR result = doCdataSection(parser, encoding, &start, end, endPtr);

    if (start)
    {
        processor = contentProcessor;
        return contentProcessor(parser, start, end, endPtr);
    }
    return result;
}

/* startPtr gets set to non-null is the section is closed, and to null if
 * the section is not yet closed. */

STATIC XMLERROR doCdataSection(XML_Parser parser,
                               const ENCODING * enc,
                               const char **startPtr,
                               const char *end,
                               const char **nextPtr)
{
    const char *s = *startPtr;
    const char **eventPP;
    const char **eventEndPP;

    if (enc == encoding)
    {
        eventPP = &eventPtr;
        *eventPP = s;
        eventEndPP = &eventEndPtr;
    }
    else
    {
        eventPP = &(openInternalEntities->internalEventPtr);
        eventEndPP = &(openInternalEntities->internalEventEndPtr);
    }
    *eventPP = s;
    *startPtr = 0;
    for (;;)
    {
        const char *next;
        int tok = XmlCdataSectionTok(enc, s, end, &next);

        *eventEndPP = next;
        switch (tok)
        {
            case XML_TOK_CDATA_SECT_CLOSE:
                if (endCdataSectionHandler)
                    endCdataSectionHandler(handlerArg);
#if 0
                /* see comment under XML_TOK_CDATA_SECT_OPEN */
                else if (characterDataHandler)
                    characterDataHandler(handlerArg, dataBuf, 0);
#endif
                else if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                *startPtr = next;
                return ERROR_EXPAT_NONE;
            case XML_TOK_DATA_NEWLINE:
                if (characterDataHandler)
                {
                    XML_Char c = 0xA;

                    characterDataHandler(handlerArg, &c, 1);
                }
                else if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                break;
            case XML_TOK_DATA_CHARS:
                if (characterDataHandler)
                {
                    if (MUST_CONVERT(enc, s))
                    {
                        for (;;)
                        {
                            ICHAR *dataPtr = (ICHAR *) dataBuf;

                            XmlConvert(enc, &s, next, &dataPtr, (ICHAR *) dataBufEnd);
                            *eventEndPP = next;
                            characterDataHandler(handlerArg, dataBuf, dataPtr - (ICHAR *) dataBuf);
                            if (s == next)
                                break;
                            *eventPP = s;
                        }
                    }
                    else
                        characterDataHandler(handlerArg,
                                             (XML_Char *) s,
                                          (XML_Char *) next - (XML_Char *) s);
                }
                else if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                break;
            case XML_TOK_INVALID:
                *eventPP = next;
                return ERROR_EXPAT_INVALID_TOKEN;
            case XML_TOK_PARTIAL_CHAR:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                return ERROR_EXPAT_PARTIAL_CHAR;
            case XML_TOK_PARTIAL:
            case XML_TOK_NONE:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                return ERROR_EXPAT_UNCLOSED_CDATA_SECTION;
            default:
                *eventPP = next;
                return ERROR_EXPAT_UNEXPECTED_STATE;
        }
        *eventPP = s = next;
    }
    /* not reached */
}

#ifdef XML_DTD

/* The idea here is to avoid using stack for each IGNORE section when
 * the whole file is parsed with one call. */

STATIC XMLERROR ignoreSectionProcessor(XML_Parser parser,
                                       const char *start,
                                       const char *end,
                                       const char **endPtr)
{
    XMLERROR result = doIgnoreSection(parser, encoding, &start, end, endPtr);

    if (start)
    {
        processor = prologProcessor;
        return prologProcessor(parser, start, end, endPtr);
    }
    return result;
}

/* startPtr gets set to non-null is the section is closed, and to null if
 * the section is not yet closed. */

STATIC XMLERROR doIgnoreSection(XML_Parser parser,
                                const ENCODING * enc,
                                const char **startPtr,
                                const char *end,
                                const char **nextPtr)
{
    const char *next;
    int tok;
    const char *s = *startPtr;
    const char **eventPP;
    const char **eventEndPP;

    if (enc == encoding)
    {
        eventPP = &eventPtr;
        *eventPP = s;
        eventEndPP = &eventEndPtr;
    }
    else
    {
        eventPP = &(openInternalEntities->internalEventPtr);
        eventEndPP = &(openInternalEntities->internalEventEndPtr);
    }
    *eventPP = s;
    *startPtr = 0;
    tok = XmlIgnoreSectionTok(enc, s, end, &next);
    *eventEndPP = next;
    switch (tok)
    {
        case XML_TOK_IGNORE_SECT:
            if (defaultHandler)
                reportDefault(parser, enc, s, next);
            *startPtr = next;
            return ERROR_EXPAT_NONE;
        case XML_TOK_INVALID:
            *eventPP = next;
            return ERROR_EXPAT_INVALID_TOKEN;
        case XML_TOK_PARTIAL_CHAR:
            if (nextPtr)
            {
                *nextPtr = s;
                return ERROR_EXPAT_NONE;
            }
            return ERROR_EXPAT_PARTIAL_CHAR;
        case XML_TOK_PARTIAL:
        case XML_TOK_NONE:
            if (nextPtr)
            {
                *nextPtr = s;
                return ERROR_EXPAT_NONE;
            }
            return ERROR_EXPAT_SYNTAX;  /* ERROR_EXPAT_UNCLOSED_IGNORE_SECTION */
        default:
            *eventPP = next;
            return ERROR_EXPAT_UNEXPECTED_STATE;
    }
    /* not reached */
}

#endif /* XML_DTD */

STATIC XMLERROR initializeEncoding(XML_Parser parser)
{
    const char *s;

#ifdef XML_UNICODE
    char encodingBuf[128];

    if (!protocolEncodingName)
        s = 0;
    else
    {
        int i;

        for (i = 0; protocolEncodingName[i]; i++)
        {
            if (i == sizeof(encodingBuf) - 1
                || (protocolEncodingName[i] & ~0x7f) != 0)
            {
                encodingBuf[0] = '\0';
                break;
            }
            encodingBuf[i] = (char)protocolEncodingName[i];
        }
        encodingBuf[i] = '\0';
        s = encodingBuf;
    }
#else
    s = protocolEncodingName;
#endif
    if ((ns ? XmlInitEncodingNS : XmlInitEncoding) (&initEncoding, &encoding, s))
        return ERROR_EXPAT_NONE;
    return handleUnknownEncoding(parser, protocolEncodingName);
}

STATIC XMLERROR  processXmlDecl(XML_Parser parser,
                                int isGeneralTextEntity,
                                const char *s,
                                const char *next)
{
    const char *encodingName = 0;
    const char *storedEncName = 0;
    const ENCODING *newEncoding = 0;
    const char *version = 0;
    const char *versionend;
    const char *storedversion = 0;
    int standalone = -1;

    if (!(ns
          ? XmlParseXmlDeclNS
          : XmlParseXmlDecl) (isGeneralTextEntity,
                              encoding,
                              s,
                              next,
                              &eventPtr,
                              &version,
                              &versionend,
                              &encodingName,
                              &newEncoding,
                              &standalone))
        return ERROR_EXPAT_SYNTAX;
    if (!isGeneralTextEntity && standalone == 1)
    {
        dtd.standalone = 1;
#ifdef XML_DTD
        if (paramEntityParsing == XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE)
            paramEntityParsing = XML_PARAM_ENTITY_PARSING_NEVER;
#endif /* XML_DTD */
    }
    if (xmlDeclHandler)
    {
        if (encodingName)
        {
            storedEncName = poolStoreString(&temp2Pool,
                                            encoding,
                                            encodingName,
                                            encodingName
                                            + XmlNameLength(encoding, encodingName),
                                            NULL);
            if (!storedEncName)
                return ERROR_EXPAT_NO_MEMORY;
            poolFinish(&temp2Pool);
        }
        if (version)
        {
            storedversion = poolStoreString(&temp2Pool,
                                            encoding,
                                            version,
                                            versionend - encoding->minBytesPerChar,
                                            NULL);
            if (!storedversion)
                return ERROR_EXPAT_NO_MEMORY;
        }
        xmlDeclHandler(handlerArg, storedversion, storedEncName, standalone);
    }
    else if (defaultHandler)
        reportDefault(parser, encoding, s, next);
    if (!protocolEncodingName)
    {
        if (newEncoding)
        {
            if (newEncoding->minBytesPerChar != encoding->minBytesPerChar)
            {
                eventPtr = encodingName;
                return ERROR_EXPAT_INCORRECT_ENCODING;
            }
            encoding = newEncoding;
        }
        else if (encodingName)
        {
            XMLERROR result;

            if (!storedEncName)
            {
                storedEncName = poolStoreString(&temp2Pool,
                                                encoding,
                                                encodingName,
                                                encodingName
                                                + XmlNameLength(encoding, encodingName),
                                                NULL);
                if (!storedEncName)
                    return ERROR_EXPAT_NO_MEMORY;
            }
            result = handleUnknownEncoding(parser, storedEncName);
            poolClear(&tempPool);
            if (result == ERROR_EXPAT_UNKNOWN_ENCODING)
                eventPtr = encodingName;
            return result;
        }
    }

    if (storedEncName || storedversion)
        poolClear(&temp2Pool);

    return ERROR_EXPAT_NONE;
}

STATIC XMLERROR  handleUnknownEncoding(XML_Parser parser,
                                       const XML_Char * encodingName)
{
    if (unknownEncodingHandler)
    {
        XML_Encoding info;
        int i;

        for (i = 0; i < 256; i++)
            info.map[i] = -1;
        info.convert = 0;
        info.data = 0;
        info.release = 0;
        if (unknownEncodingHandler(unknownEncodingHandlerData, encodingName, &info))
        {
            ENCODING *enc;

            unknownEncodingMem = MALLOC(XmlSizeOfUnknownEncoding());
            if (!unknownEncodingMem)
            {
                if (info.release)
                    info.release(info.data);
                return ERROR_EXPAT_NO_MEMORY;
            }
            enc = (ns
                   ? XmlInitUnknownEncodingNS
                   : XmlInitUnknownEncoding) (unknownEncodingMem,
                                              info.map,
                                              info.convert,
                                              info.data);
            if (enc)
            {
                unknownEncodingData = info.data;
                unknownEncodingRelease = (void(*)(void*))info.release;
                encoding = enc;
                return ERROR_EXPAT_NONE;
            }
        }
        if (info.release)
            info.release(info.data);
    }
    return ERROR_EXPAT_UNKNOWN_ENCODING;
}

STATIC XMLERROR prologInitProcessor(XML_Parser parser,
                                    const char *s,
                                    const char *end,
                                    const char **nextPtr)
{
    XMLERROR result = initializeEncoding(parser);

    if (result != ERROR_EXPAT_NONE)
        return result;
    processor = prologProcessor;
    return prologProcessor(parser, s, end, nextPtr);
}

STATIC XMLERROR prologProcessor(XML_Parser parser,
                                const char *s,
                                const char *end,
                                const char **nextPtr)
{
    const char *next;
    int tok = XmlPrologTok(encoding, s, end, &next);

    return doProlog(parser, encoding, s, end, tok, next, nextPtr);
}

STATIC XMLERROR doProlog(XML_Parser parser,
                         const ENCODING * enc,
                         const char *s,
                         const char *end,
                         int tok,
                         const char *next,
                         const char **nextPtr)
{
#ifdef XML_DTD
    static const XML_Char externalSubsetName[] =
    {'#', '\0'};

#endif /* XML_DTD */

    const char **eventPP;
    const char **eventEndPP;
    enum XML_Content_Quant quant;

    if (enc == encoding)
    {
        eventPP = &eventPtr;
        eventEndPP = &eventEndPtr;
    }
    else
    {
        eventPP = &(openInternalEntities->internalEventPtr);
        eventEndPP = &(openInternalEntities->internalEventEndPtr);
    }
    for (;;)
    {
        int role;

        *eventPP = s;
        *eventEndPP = next;
        if (tok <= 0)
        {
            if (nextPtr != 0 && tok != XML_TOK_INVALID)
            {
                *nextPtr = s;
                return ERROR_EXPAT_NONE;
            }
            switch (tok)
            {
                case XML_TOK_INVALID:
                    *eventPP = next;
                    return ERROR_EXPAT_INVALID_TOKEN;
                case XML_TOK_PARTIAL:
                    return ERROR_EXPAT_UNCLOSED_TOKEN;
                case XML_TOK_PARTIAL_CHAR:
                    return ERROR_EXPAT_PARTIAL_CHAR;
                case XML_TOK_NONE:
#ifdef XML_DTD
                    if (enc != encoding)
                        return ERROR_EXPAT_NONE;
                    if (parentParser)
                    {
                        if (XmlTokenRole(&prologState, XML_TOK_NONE, end, end, enc)
                            == XML_ROLE_ERROR)
                            return ERROR_EXPAT_SYNTAX;
                        hadExternalDoctype = 0;
                        return ERROR_EXPAT_NONE;
                    }
#endif /* XML_DTD */
                    return ERROR_EXPAT_NO_ELEMENTS;
                default:
                    tok = -tok;
                    next = end;
                    break;
            }
        }
        role = XmlTokenRole(&prologState, tok, s, next, enc);
        switch (role)
        {
            case XML_ROLE_XML_DECL:
                {
                    XMLERROR result = processXmlDecl(parser, 0, s, next);

                    if (result != ERROR_EXPAT_NONE)
                        return result;
                    enc = encoding;
                }
                break;
            case XML_ROLE_DOCTYPE_NAME:
                if (startDoctypeDeclHandler)
                {
                    doctypeName = poolStoreString(&tempPool, enc, s, next, NULL);
                    if (!doctypeName)
                        return ERROR_EXPAT_NO_MEMORY;
                    poolFinish(&tempPool);
                    doctypeSysid = 0;
                    doctypePubid = 0;
                }
                break;
            case XML_ROLE_DOCTYPE_INTERNAL_SUBSET:
                if (startDoctypeDeclHandler)
                {
                    startDoctypeDeclHandler(handlerArg, doctypeName, doctypeSysid,
                                            doctypePubid, 1);
                    doctypeName = 0;
                    poolClear(&tempPool);
                }
                break;
#ifdef XML_DTD
            case XML_ROLE_TEXT_DECL:
                {
                    XMLERROR result = processXmlDecl(parser, 1, s, next);

                    if (result != ERROR_EXPAT_NONE)
                        return result;
                    enc = encoding;
                }
                break;
#endif /* XML_DTD */
            case XML_ROLE_DOCTYPE_PUBLIC_ID:
                if (startDoctypeDeclHandler)
                {
                    doctypePubid = poolStoreString(&tempPool, enc, s + 1, next - 1, NULL);
                    if (!doctypePubid)
                        return ERROR_EXPAT_NO_MEMORY;
                    poolFinish(&tempPool);
                }
#ifdef XML_DTD
                declEntity = (ENTITY *) lookup(&dtd.paramEntities,
                                               externalSubsetName,
                                               sizeof(ENTITY));
                if (!declEntity)
                    return ERROR_EXPAT_NO_MEMORY;
#endif /* XML_DTD */
                /* fall through */
            case XML_ROLE_ENTITY_PUBLIC_ID:
                if (!XmlIsPublicId(enc, s, next, eventPP))
                    return ERROR_EXPAT_SYNTAX;
                if (declEntity)
                {
                    XML_Char *tem = poolStoreString(&dtd.pool,
                                                    enc,
                                                    s + enc->minBytesPerChar,
                                                    next - enc->minBytesPerChar,
                                                    NULL);

                    if (!tem)
                        return ERROR_EXPAT_NO_MEMORY;
                    normalizePublicId(tem);
                    declEntity->publicId = tem;
                    poolFinish(&dtd.pool);
                }
                break;
            case XML_ROLE_DOCTYPE_CLOSE:
                if (doctypeName)
                {
                    startDoctypeDeclHandler(handlerArg, doctypeName,
                                            doctypeSysid, doctypePubid, 0);
                    poolClear(&tempPool);
                }
                if (dtd.complete && hadExternalDoctype)
                {
                    dtd.complete = 0;
#ifdef XML_DTD
                    if (paramEntityParsing && externalEntityRefHandler)
                    {
                        ENTITY *entity = (ENTITY *) lookup(&dtd.paramEntities,
                                                           externalSubsetName,
                                                           0);

                        if (!externalEntityRefHandler(handlerArg,   // V0.9.14 (2001-08-09) [umoeller]
                                                      externalEntityRefHandlerArg,
                                                      0,
                                                      entity->base,
                                                      entity->systemId,
                                                      entity->publicId))
                            return ERROR_EXPAT_EXTERNAL_ENTITY_HANDLING;
                    }
#endif /* XML_DTD */
                    if (!dtd.complete
                        && !dtd.standalone
                        && notStandaloneHandler
                        && !notStandaloneHandler(handlerArg))
                        return ERROR_EXPAT_NOT_STANDALONE;
                }
                if (endDoctypeDeclHandler)
                    endDoctypeDeclHandler(handlerArg);
                break;
            case XML_ROLE_INSTANCE_START:
                processor = contentProcessor;
                return contentProcessor(parser, s, end, nextPtr);
            case XML_ROLE_ATTLIST_ELEMENT_NAME:
                declElementType = getElementType(parser, enc, s, next);
                if (!declElementType)
                    return ERROR_EXPAT_NO_MEMORY;
                break;
            case XML_ROLE_ATTRIBUTE_NAME:
                declAttributeId = getAttributeId(parser, enc, s, next);
                if (!declAttributeId)
                    return ERROR_EXPAT_NO_MEMORY;
                declAttributeIsCdata = 0;
                declAttributeType = 0;
                declAttributeIsId = 0;
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_CDATA:
                declAttributeIsCdata = 1;
                declAttributeType = "CDATA";
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_ID:
                declAttributeIsId = 1;
                declAttributeType = "ID";
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_IDREF:
                declAttributeType = "IDREF";
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_IDREFS:
                declAttributeType = "IDREFS";
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_ENTITY:
                declAttributeType = "ENTITY";
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_ENTITIES:
                declAttributeType = "ENTITIES";
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_NMTOKEN:
                declAttributeType = "NMTOKEN";
                break;
            case XML_ROLE_ATTRIBUTE_TYPE_NMTOKENS:
                declAttributeType = "NMTOKENS";
                break;

            case XML_ROLE_ATTRIBUTE_ENUM_VALUE:
            case XML_ROLE_ATTRIBUTE_NOTATION_VALUE:
                if (attlistDeclHandler)
                {
                    char *prefix;

                    if (declAttributeType)
                    {
                        prefix = "|";
                    }
                    else
                    {
                        prefix = (role == XML_ROLE_ATTRIBUTE_NOTATION_VALUE
                                  ? "NOTATION("
                                  : "(");
                    }
                    if (!poolAppendString(&tempPool, prefix))
                        return ERROR_EXPAT_NO_MEMORY;
                    if (!poolAppend(&tempPool, enc, s, next, NULL))
                        return ERROR_EXPAT_NO_MEMORY;
                    declAttributeType = tempPool.start;
                }
                break;
            case XML_ROLE_IMPLIED_ATTRIBUTE_VALUE:
            case XML_ROLE_REQUIRED_ATTRIBUTE_VALUE:
                if (dtd.complete
                    && !defineAttribute(declElementType, declAttributeId,
                                   declAttributeIsCdata, declAttributeIsId, 0,
                                        parser))
                    return ERROR_EXPAT_NO_MEMORY;
                if (attlistDeclHandler && declAttributeType)
                {
                    if (*declAttributeType == '('
                        || (*declAttributeType == 'N' && declAttributeType[1] == 'O'))
                    {
                        /* Enumerated or Notation type */
                        if (!poolAppendChar(&tempPool, ')')
                            || !poolAppendChar(&tempPool, '\0'))
                            return ERROR_EXPAT_NO_MEMORY;
                        declAttributeType = tempPool.start;
                        poolFinish(&tempPool);
                    }
                    *eventEndPP = s;
                    attlistDeclHandler(handlerArg, declElementType->name,
                                     declAttributeId->name, declAttributeType,
                                0, role == XML_ROLE_REQUIRED_ATTRIBUTE_VALUE);
                    poolClear(&tempPool);
                }
                break;
            case XML_ROLE_DEFAULT_ATTRIBUTE_VALUE:
            case XML_ROLE_FIXED_ATTRIBUTE_VALUE:
                {
                    const XML_Char *attVal;
                    XMLERROR result
                    = storeAttributeValue(parser, enc, declAttributeIsCdata,
                                          s + enc->minBytesPerChar,
                                          next - enc->minBytesPerChar,
                                          &dtd.pool);

                    if (result)
                        return result;
                    attVal = poolStart(&dtd.pool);
                    poolFinish(&dtd.pool);
                    if (dtd.complete
                    /* ID attributes aren't allowed to have a default */
                        && !defineAttribute(declElementType, declAttributeId, declAttributeIsCdata, 0, attVal, parser))
                        return ERROR_EXPAT_NO_MEMORY;
                    if (attlistDeclHandler && declAttributeType)
                    {
                        if (*declAttributeType == '('
                            || (*declAttributeType == 'N' && declAttributeType[1] == 'O'))
                        {
                            /* Enumerated or Notation type */
                            if (!poolAppendChar(&tempPool, ')')
                                || !poolAppendChar(&tempPool, '\0'))
                                return ERROR_EXPAT_NO_MEMORY;
                            declAttributeType = tempPool.start;
                            poolFinish(&tempPool);
                        }
                        *eventEndPP = s;
                        attlistDeclHandler(handlerArg, declElementType->name,
                                     declAttributeId->name, declAttributeType,
                                           attVal,
                                      role == XML_ROLE_FIXED_ATTRIBUTE_VALUE);
                        poolClear(&tempPool);
                    }
                    break;
                }
            case XML_ROLE_ENTITY_VALUE:
                {
                    XMLERROR result = storeEntityValue(parser, enc,
                                                     s + enc->minBytesPerChar,
                                                 next - enc->minBytesPerChar);

                    if (declEntity)
                    {
                        declEntity->textPtr = poolStart(&dtd.pool);
                        declEntity->textLen = poolLength(&dtd.pool);
                        poolFinish(&dtd.pool);
                        if (entityDeclHandler)
                        {
                            *eventEndPP = s;
                            entityDeclHandler(handlerArg,
                                              declEntity->name,
                                              declEntity->is_param,
                                              declEntity->textPtr,
                                              declEntity->textLen,
                                              curBase, 0, 0, 0);
                        }
                    }
                    else
                        poolDiscard(&dtd.pool);
                    if (result != ERROR_EXPAT_NONE)
                        return result;
                }
                break;
            case XML_ROLE_DOCTYPE_SYSTEM_ID:
                if (startDoctypeDeclHandler)
                {
                    doctypeSysid = poolStoreString(&tempPool,
                                                   enc,
                                                   s + 1,
                                                   next - 1,
                                                   NULL);
                    if (!doctypeSysid)
                        return ERROR_EXPAT_NO_MEMORY;
                    poolFinish(&tempPool);
                }
                if (!dtd.standalone
#ifdef XML_DTD
                    && !paramEntityParsing
#endif /* XML_DTD */
                    && notStandaloneHandler
                    && !notStandaloneHandler(handlerArg))
                    return ERROR_EXPAT_NOT_STANDALONE;
                hadExternalDoctype = 1;
#ifndef XML_DTD
                break;
#else /* XML_DTD */
                if (!declEntity)
                {
                    declEntity = (ENTITY *) lookup(&dtd.paramEntities,
                                                   externalSubsetName,
                                                   sizeof(ENTITY));
                    declEntity->publicId = 0;
                    if (!declEntity)
                        return ERROR_EXPAT_NO_MEMORY;
                }
                /* fall through */
#endif /* XML_DTD */
            case XML_ROLE_ENTITY_SYSTEM_ID:
                if (declEntity)
                {
                    declEntity->systemId = poolStoreString(&dtd.pool,
                                                           enc,
                                                           s + enc->minBytesPerChar,
                                                           next - enc->minBytesPerChar,
                                                           NULL);
                    if (!declEntity->systemId)
                        return ERROR_EXPAT_NO_MEMORY;
                    declEntity->base = curBase;
                    poolFinish(&dtd.pool);
                }
                break;
            case XML_ROLE_ENTITY_COMPLETE:
                if (declEntity && entityDeclHandler)
                {
                    *eventEndPP = s;
                    entityDeclHandler(handlerArg,
                                      declEntity->name,
                                      0, 0, 0,
                                      declEntity->base,
                                      declEntity->systemId,
                                      declEntity->publicId,
                                      0);
                }
                break;
            case XML_ROLE_ENTITY_NOTATION_NAME:
                if (declEntity)
                {
                    declEntity->notation = poolStoreString(&dtd.pool,
                                                           enc,
                                                           s,
                                                           next,
                                                           NULL);
                    if (!declEntity->notation)
                        return ERROR_EXPAT_NO_MEMORY;
                    poolFinish(&dtd.pool);
                    if (unparsedEntityDeclHandler)
                    {
                        *eventEndPP = s;
                        unparsedEntityDeclHandler(handlerArg,
                                                  declEntity->name,
                                                  declEntity->base,
                                                  declEntity->systemId,
                                                  declEntity->publicId,
                                                  declEntity->notation);
                    }
                    else if (entityDeclHandler)
                    {
                        *eventEndPP = s;
                        entityDeclHandler(handlerArg,
                                          declEntity->name,
                                          0, 0, 0,
                                          declEntity->base,
                                          declEntity->systemId,
                                          declEntity->publicId,
                                          declEntity->notation);
                    }
                }
                break;
            case XML_ROLE_GENERAL_ENTITY_NAME:
                {
                    const XML_Char *name;

                    if (XmlPredefinedEntityName(enc, s, next))
                    {
                        declEntity = 0;
                        break;
                    }
                    name = poolStoreString(&dtd.pool,
                                           enc,
                                           s,
                                           next,
                                           NULL);
                    if (!name)
                        return ERROR_EXPAT_NO_MEMORY;
                    if (dtd.complete)
                    {
                        declEntity = (ENTITY *) lookup(&dtd.generalEntities, name, sizeof(ENTITY));
                        if (!declEntity)
                            return ERROR_EXPAT_NO_MEMORY;
                        if (declEntity->name != name)
                        {
                            poolDiscard(&dtd.pool);
                            declEntity = 0;
                        }
                        else
                        {
                            poolFinish(&dtd.pool);
                            declEntity->publicId = 0;
                            declEntity->is_param = 0;
                        }
                    }
                    else
                    {
                        poolDiscard(&dtd.pool);
                        declEntity = 0;
                    }
                }
                break;
            case XML_ROLE_PARAM_ENTITY_NAME:
#ifdef XML_DTD
                if (dtd.complete)
                {
                    const XML_Char *name = poolStoreString(&dtd.pool, enc, s, next, NULL);

                    if (!name)
                        return ERROR_EXPAT_NO_MEMORY;
                    declEntity = (ENTITY *) lookup(&dtd.paramEntities,
                                                   name, sizeof(ENTITY));
                    if (!declEntity)
                        return ERROR_EXPAT_NO_MEMORY;
                    if (declEntity->name != name)
                    {
                        poolDiscard(&dtd.pool);
                        declEntity = 0;
                    }
                    else
                    {
                        poolFinish(&dtd.pool);
                        declEntity->publicId = 0;
                        declEntity->is_param = 1;
                    }
                }
#else /* not XML_DTD */
                declEntity = 0;
#endif /* not XML_DTD */
                break;
            case XML_ROLE_NOTATION_NAME:
                declNotationPublicId = 0;
                declNotationName = 0;
                if (notationDeclHandler)
                {
                    declNotationName = poolStoreString(&tempPool, enc, s, next, NULL);
                    if (!declNotationName)
                        return ERROR_EXPAT_NO_MEMORY;
                    poolFinish(&tempPool);
                }
                break;
            case XML_ROLE_NOTATION_PUBLIC_ID:
                if (!XmlIsPublicId(enc, s, next, eventPP))
                    return ERROR_EXPAT_SYNTAX;
                if (declNotationName)
                {
                    XML_Char *tem = poolStoreString(&tempPool,
                                                    enc,
                                                    s + enc->minBytesPerChar,
                                                    next - enc->minBytesPerChar,
                                                    NULL);

                    if (!tem)
                        return ERROR_EXPAT_NO_MEMORY;
                    normalizePublicId(tem);
                    declNotationPublicId = tem;
                    poolFinish(&tempPool);
                }
                break;
            case XML_ROLE_NOTATION_SYSTEM_ID:
                if (declNotationName && notationDeclHandler)
                {
                    const XML_Char *systemId
                    = poolStoreString(&tempPool, enc,
                                      s + enc->minBytesPerChar,
                                      next - enc->minBytesPerChar,
                                      NULL);

                    if (!systemId)
                        return ERROR_EXPAT_NO_MEMORY;
                    *eventEndPP = s;
                    notationDeclHandler(handlerArg,
                                        declNotationName,
                                        curBase,
                                        systemId,
                                        declNotationPublicId);
                }
                poolClear(&tempPool);
                break;
            case XML_ROLE_NOTATION_NO_SYSTEM_ID:
                if (declNotationPublicId && notationDeclHandler)
                {
                    *eventEndPP = s;
                    notationDeclHandler(handlerArg,
                                        declNotationName,
                                        curBase,
                                        0,
                                        declNotationPublicId);
                }
                poolClear(&tempPool);
                break;
            case XML_ROLE_ERROR:
                switch (tok)
                {
                    case XML_TOK_PARAM_ENTITY_REF:
                        return ERROR_EXPAT_PARAM_ENTITY_REF;
                    case XML_TOK_XML_DECL:
                        return ERROR_EXPAT_MISPLACED_XML_PI;
                    default:
                        return ERROR_EXPAT_SYNTAX;
                }
#ifdef XML_DTD
            case XML_ROLE_IGNORE_SECT:
                {
                    XMLERROR result;

                    if (defaultHandler)
                        reportDefault(parser, enc, s, next);
                    result = doIgnoreSection(parser, enc, &next, end, nextPtr);
                    if (!next)
                    {
                        processor = ignoreSectionProcessor;
                        return result;
                    }
                }
                break;
#endif /* XML_DTD */
            case XML_ROLE_GROUP_OPEN:
                if (prologState.level >= groupSize)
                {
                    if (groupSize)
                    {
                        groupConnector = (char*)REALLOC(groupConnector, groupSize *= 2);
                        if (dtd.scaffIndex)
                            dtd.scaffIndex = (int*)REALLOC(dtd.scaffIndex, groupSize * sizeof(int));
                    }
                    else
                        groupConnector = (char*)MALLOC(groupSize = 32);
                    if (!groupConnector)
                        return ERROR_EXPAT_NO_MEMORY;
                }
                groupConnector[prologState.level] = 0;
                if (dtd.in_eldecl)
                {
                    int myindex = nextScaffoldPart(parser);

                    if (myindex < 0)
                        return ERROR_EXPAT_NO_MEMORY;
                    dtd.scaffIndex[dtd.scaffLevel] = myindex;
                    dtd.scaffLevel++;
                    dtd.scaffold[myindex].type = XML_CTYPE_SEQ;
                }
                break;
            case XML_ROLE_GROUP_SEQUENCE:
                if (groupConnector[prologState.level] == '|')
                    return ERROR_EXPAT_SYNTAX;
                groupConnector[prologState.level] = ',';
                break;
            case XML_ROLE_GROUP_CHOICE:
                if (groupConnector[prologState.level] == ',')
                    return ERROR_EXPAT_SYNTAX;
                if (dtd.in_eldecl
                    && !groupConnector[prologState.level]
                    && dtd.scaffold[dtd.scaffIndex[dtd.scaffLevel - 1]].type != XML_CTYPE_MIXED
                    )
                {
                    dtd.scaffold[dtd.scaffIndex[dtd.scaffLevel - 1]].type = XML_CTYPE_CHOICE;
                }
                groupConnector[prologState.level] = '|';
                break;
            case XML_ROLE_PARAM_ENTITY_REF:
#ifdef XML_DTD
            case XML_ROLE_INNER_PARAM_ENTITY_REF:
                if (paramEntityParsing
                 && (dtd.complete || role == XML_ROLE_INNER_PARAM_ENTITY_REF))
                {
                    const XML_Char *name;
                    ENTITY *entity;

                    name = poolStoreString(&dtd.pool, enc,
                                           s + enc->minBytesPerChar,
                                           next - enc->minBytesPerChar,
                                           NULL);
                    if (!name)
                        return ERROR_EXPAT_NO_MEMORY;
                    entity = (ENTITY *) lookup(&dtd.paramEntities, name, 0);
                    poolDiscard(&dtd.pool);
                    if (!entity)
                    {
                        /* FIXME what to do if !dtd.complete? */
                        return ERROR_EXPAT_UNDEFINED_ENTITY;
                    }
                    if (entity->open)
                        return ERROR_EXPAT_RECURSIVE_ENTITY_REF;
                    if (entity->textPtr)
                    {
                        XMLERROR result;

                        result = processInternalParamEntity(parser, entity);
                        if (result != ERROR_EXPAT_NONE)
                            return result;
                        break;
                    }
                    if (role == XML_ROLE_INNER_PARAM_ENTITY_REF)
                        return ERROR_EXPAT_PARAM_ENTITY_REF;
                    if (externalEntityRefHandler)
                    {
                        dtd.complete = 0;
                        entity->open = 1;
                        if (!externalEntityRefHandler(handlerArg,       // V0.9.14 (2001-08-09) [umoeller]
                                                      externalEntityRefHandlerArg,
                                                      0,
                                                      entity->base,
                                                      entity->systemId,
                                                      entity->publicId))
                        {
                            entity->open = 0;
                            return ERROR_EXPAT_EXTERNAL_ENTITY_HANDLING;
                        }
                        entity->open = 0;
                        if (dtd.complete)
                            break;
                    }
                }
#endif /* XML_DTD */
                if (!dtd.standalone
                    && notStandaloneHandler
                    && !notStandaloneHandler(handlerArg))
                    return ERROR_EXPAT_NOT_STANDALONE;
                dtd.complete = 0;
                if (defaultHandler)
                    reportDefault(parser, enc, s, next);
                break;

                /* Element declaration stuff */

            case XML_ROLE_ELEMENT_NAME:
                if (elementDeclHandler)
                {
                    declElementType = getElementType(parser, enc, s, next);
                    if (!declElementType)
                        return ERROR_EXPAT_NO_MEMORY;
                    dtd.scaffLevel = 0;
                    dtd.scaffCount = 0;
                    dtd.in_eldecl = 1;
                }
                break;

            case XML_ROLE_CONTENT_ANY:
            case XML_ROLE_CONTENT_EMPTY:
                if (dtd.in_eldecl)
                {
                    if (elementDeclHandler)
                    {
                        XMLCONTENT *content = (XMLCONTENT *) MALLOC(sizeof(XMLCONTENT));

                        if (!content)
                            return ERROR_EXPAT_NO_MEMORY;
                        content->quant = XML_CQUANT_NONE;
                        content->name = 0;
                        content->numchildren = 0;
                        content->children = 0;
                        content->type = ((role == XML_ROLE_CONTENT_ANY) ?
                                         XML_CTYPE_ANY :
                                         XML_CTYPE_EMPTY);
                        *eventEndPP = s;
                        elementDeclHandler(handlerArg, declElementType->name, content);
                    }
                    dtd.in_eldecl = 0;
                }
                break;

            case XML_ROLE_CONTENT_PCDATA:
                if (dtd.in_eldecl)
                {
                    dtd.scaffold[dtd.scaffIndex[dtd.scaffLevel - 1]].type = XML_CTYPE_MIXED;
                }
                break;

            case XML_ROLE_CONTENT_ELEMENT:
                quant = XML_CQUANT_NONE;
                goto elementContent;
            case XML_ROLE_CONTENT_ELEMENT_OPT:
                quant = XML_CQUANT_OPT;
                goto elementContent;
            case XML_ROLE_CONTENT_ELEMENT_REP:
                quant = XML_CQUANT_REP;
                goto elementContent;
            case XML_ROLE_CONTENT_ELEMENT_PLUS:
                quant = XML_CQUANT_PLUS;
              elementContent:
                if (dtd.in_eldecl)
                {
                    ELEMENT_TYPE *el;
                    const char *nxt = quant == XML_CQUANT_NONE ? next : next - 1;
                    int myindex = nextScaffoldPart(parser);

                    if (myindex < 0)
                        return ERROR_EXPAT_NO_MEMORY;
                    dtd.scaffold[myindex].type = XML_CTYPE_NAME;
                    dtd.scaffold[myindex].quant = quant;
                    el = getElementType(parser, enc, s, nxt);
                    if (!el)
                        return ERROR_EXPAT_NO_MEMORY;
                    dtd.scaffold[myindex].name = el->name;
                    dtd.contentStringLen += nxt - s + 1;
                }
                break;

            case XML_ROLE_GROUP_CLOSE:
                quant = XML_CQUANT_NONE;
                goto closeGroup;
            case XML_ROLE_GROUP_CLOSE_OPT:
                quant = XML_CQUANT_OPT;
                goto closeGroup;
            case XML_ROLE_GROUP_CLOSE_REP:
                quant = XML_CQUANT_REP;
                goto closeGroup;
            case XML_ROLE_GROUP_CLOSE_PLUS:
                quant = XML_CQUANT_PLUS;
              closeGroup:
                if (dtd.in_eldecl)
                {
                    dtd.scaffLevel--;
                    dtd.scaffold[dtd.scaffIndex[dtd.scaffLevel]].quant = quant;
                    if (dtd.scaffLevel == 0)
                    {
                        if (elementDeclHandler)
                        {
                            XMLCONTENT *model = build_model(parser);

                            if (!model)
                                return ERROR_EXPAT_NO_MEMORY;
                            *eventEndPP = s;
                            elementDeclHandler(handlerArg, declElementType->name, model);
                        }
                        dtd.in_eldecl = 0;
                        dtd.contentStringLen = 0;
                    }
                }
                break;
                /* End element declaration stuff */

            case XML_ROLE_NONE:
                switch (tok)
                {
                    case XML_TOK_PI:
                        if (!reportProcessingInstruction(parser, enc, s, next))
                            return ERROR_EXPAT_NO_MEMORY;
                        break;
                    case XML_TOK_COMMENT:
                        if (!reportComment(parser, enc, s, next))
                            return ERROR_EXPAT_NO_MEMORY;
                        break;
                }
                break;
        }
        if (defaultHandler)
        {
            switch (tok)
            {
                case XML_TOK_PI:
                case XML_TOK_COMMENT:
                case XML_TOK_BOM:
                case XML_TOK_XML_DECL:
#ifdef XML_DTD
                case XML_TOK_IGNORE_SECT:
#endif /* XML_DTD */
                case XML_TOK_PARAM_ENTITY_REF:
                    break;
                default:
#ifdef XML_DTD
                    if (role != XML_ROLE_IGNORE_SECT)
#endif /* XML_DTD */
                        reportDefault(parser, enc, s, next);
            }
        }
        s = next;
        tok = XmlPrologTok(enc, s, end, &next);
    }
    /* not reached */
}

STATIC XMLERROR epilogProcessor(XML_Parser parser,
                                const char *s,
                                const char *end,
                                const char **nextPtr)
{
    processor = epilogProcessor;
    eventPtr = s;
    for (;;)
    {
        const char *next;
        int tok = XmlPrologTok(encoding, s, end, &next);

        eventEndPtr = next;
        switch (tok)
        {
            case -XML_TOK_PROLOG_S:
                if (defaultHandler)
                {
                    eventEndPtr = end;
                    reportDefault(parser, encoding, s, end);
                }
                /* fall through */
            case XML_TOK_NONE:
                if (nextPtr)
                    *nextPtr = end;
                return ERROR_EXPAT_NONE;
            case XML_TOK_PROLOG_S:
                if (defaultHandler)
                    reportDefault(parser, encoding, s, next);
                break;
            case XML_TOK_PI:
                if (!reportProcessingInstruction(parser, encoding, s, next))
                    return ERROR_EXPAT_NO_MEMORY;
                break;
            case XML_TOK_COMMENT:
                if (!reportComment(parser, encoding, s, next))
                    return ERROR_EXPAT_NO_MEMORY;
                break;
            case XML_TOK_INVALID:
                eventPtr = next;
                return ERROR_EXPAT_INVALID_TOKEN;
            case XML_TOK_PARTIAL:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                return ERROR_EXPAT_UNCLOSED_TOKEN;
            case XML_TOK_PARTIAL_CHAR:
                if (nextPtr)
                {
                    *nextPtr = s;
                    return ERROR_EXPAT_NONE;
                }
                return ERROR_EXPAT_PARTIAL_CHAR;
            default:
                return ERROR_EXPAT_JUNK_AFTER_DOC_ELEMENT;
        }
        eventPtr = s = next;
    }
}

#ifdef XML_DTD

STATIC XMLERROR processInternalParamEntity(XML_Parser parser,
                                           ENTITY * entity)
{
    const char *s, *end, *next;
    int tok;
    XMLERROR result;
    OPEN_INTERNAL_ENTITY openEntity;

    entity->open = 1;
    openEntity.next = openInternalEntities;
    openInternalEntities = &openEntity;
    openEntity.entity = entity;
    openEntity.internalEventPtr = 0;
    openEntity.internalEventEndPtr = 0;
    s = (char *)entity->textPtr;
    end = (char *)(entity->textPtr + entity->textLen);
    tok = XmlPrologTok(internalEncoding, s, end, &next);
    result = doProlog(parser, internalEncoding, s, end, tok, next, 0);
    entity->open = 0;
    openInternalEntities = openEntity.next;
    return result;
}

#endif /* XML_DTD */

STATIC XMLERROR errorProcessor(XML_Parser parser,
                               const char *s,
                               const char *end,
                               const char **nextPtr)
{
    return errorCode;
}

STATIC XMLERROR storeAttributeValue(XML_Parser parser,
                                    const ENCODING * enc,
                                    int isCdata,
                                    const char *ptr,
                                    const char *end,
                                    STRING_POOL * pool)
{
    XMLERROR result = appendAttributeValue(parser, enc, isCdata, ptr, end, pool);

    if (result)
        return result;
    if (!isCdata && poolLength(pool) && poolLastChar(pool) == 0x20)
        poolChop(pool);
    if (!poolAppendChar(pool, XML_T('\0')))
        return ERROR_EXPAT_NO_MEMORY;
    return ERROR_EXPAT_NONE;
}


/*
 *@@ appendAttributeValue:
 *
 *@@changed V0.9.14 (2001-08-09) [umoeller]: fixed ERROR_EXPAT_UNDEFINED_ENTITY with callback-defined encodings
 */

STATIC XMLERROR appendAttributeValue(XML_Parser parser,
                                     const ENCODING * enc,
                                     int isCdata,
                                     const char *ptr,
                                     const char *end,
                                     STRING_POOL * pool)
{
    for (;;)
    {
        const char *next;
        int tok = XmlAttributeValueTok(enc, ptr, end, &next);

        switch (tok)
        {
            case XML_TOK_NONE:
                return ERROR_EXPAT_NONE;
            case XML_TOK_INVALID:
                if (enc == encoding)
                    eventPtr = next;
                return ERROR_EXPAT_INVALID_TOKEN;
            case XML_TOK_PARTIAL:
                if (enc == encoding)
                    eventPtr = ptr;
                return ERROR_EXPAT_INVALID_TOKEN;
            case XML_TOK_CHAR_REF:
                {
                    XML_Char buf[XML_ENCODE_MAX];
                    int i;
                    int n = XmlCharRefNumber(enc, ptr);

                    if (n < 0)
                    {
                        if (enc == encoding)
                            eventPtr = ptr;
                        return ERROR_EXPAT_BAD_CHAR_REF;
                    }
                    if (!isCdata
                        && n == 0x20    /* space */
                     && (poolLength(pool) == 0 || poolLastChar(pool) == 0x20))
                        break;
                    n = XmlEncode(n, (ICHAR *) buf);
                    if (!n)
                    {
                        if (enc == encoding)
                            eventPtr = ptr;
                        return ERROR_EXPAT_BAD_CHAR_REF;
                    }
                    for (i = 0; i < n; i++)
                    {
                        if (!poolAppendChar(pool, buf[i]))
                            return ERROR_EXPAT_NO_MEMORY;
                    }
                }
                break;
            case XML_TOK_DATA_CHARS:
                if (!poolAppend(pool, enc, ptr, next, NULL))
                    return ERROR_EXPAT_NO_MEMORY;
                break;
                break;
            case XML_TOK_TRAILING_CR:
                next = ptr + enc->minBytesPerChar;
                /* fall through */
            case XML_TOK_ATTRIBUTE_VALUE_S:
            case XML_TOK_DATA_NEWLINE:
                if (!isCdata && (poolLength(pool) == 0 || poolLastChar(pool) == 0x20))
                    break;
                if (!poolAppendChar(pool, 0x20))
                    return ERROR_EXPAT_NO_MEMORY;
                break;
            case XML_TOK_ENTITY_REF:
                {
                    const XML_Char *name;
                    ENTITY *entity;
                    unsigned long ulOfs; // V0.9.14 (2001-08-09) [umoeller]
                    XML_Char ch = XmlPredefinedEntityName(enc,
                                                   ptr + enc->minBytesPerChar,
                                                 next - enc->minBytesPerChar);

                    if (ch)
                    {
                        if (!poolAppendChar(pool, ch))
                            return ERROR_EXPAT_NO_MEMORY;
                        break;
                    }
                    name = poolStoreString(&temp2Pool, enc,
                                           ptr + enc->minBytesPerChar,
                                           next - enc->minBytesPerChar,
                                           &ulOfs); // V0.9.14 (2001-08-09) [umoeller]
                    if (!name)
                        return ERROR_EXPAT_NO_MEMORY;
                    entity = (ENTITY *) lookup(&dtd.generalEntities,
                                               name + ulOfs, // V0.9.14 (2001-08-09) [umoeller]
                                               0);
                    poolDiscard(&temp2Pool);
                    if (!entity)
                    {
                        if (dtd.complete)
                        {
                            if (enc == encoding)
                                eventPtr = ptr;
                            return ERROR_EXPAT_UNDEFINED_ENTITY;
                        }
                    }
                    else if (entity->open)
                    {
                        if (enc == encoding)
                            eventPtr = ptr;
                        return ERROR_EXPAT_RECURSIVE_ENTITY_REF;
                    }
                    else if (entity->notation)
                    {
                        if (enc == encoding)
                            eventPtr = ptr;
                        return ERROR_EXPAT_BINARY_ENTITY_REF;
                    }
                    else if (!entity->textPtr)
                    {
                        if (enc == encoding)
                            eventPtr = ptr;
                        return ERROR_EXPAT_ATTRIBUTE_EXTERNAL_ENTITY_REF;
                    }
                    else
                    {
                        XMLERROR result;
                        const XML_Char *textEnd = entity->textPtr + entity->textLen;

                        entity->open = 1;
                        result = appendAttributeValue(parser, internalEncoding, isCdata, (char *)entity->textPtr, (char *)textEnd, pool);
                        entity->open = 0;
                        if (result)
                            return result;
                    }
                }
                break;
            default:
                if (enc == encoding)
                    eventPtr = ptr;
                return ERROR_EXPAT_UNEXPECTED_STATE;
        }
        ptr = next;
    }
    /* not reached */
}

STATIC XMLERROR storeEntityValue(XML_Parser parser,
                                 const ENCODING * enc,
                                 const char *entityTextPtr,
                                 const char *entityTextEnd)
{
    STRING_POOL *pool = &(dtd.pool);

    for (;;)
    {
        const char *next;
        int tok = XmlEntityValueTok(enc, entityTextPtr, entityTextEnd, &next);

        switch (tok)
        {
            case XML_TOK_PARAM_ENTITY_REF:
#ifdef XML_DTD
                if (parentParser || enc != encoding)
                {
                    XMLERROR result;
                    const XML_Char *name;
                    ENTITY *entity;

                    name = poolStoreString(&tempPool, enc,
                                           entityTextPtr + enc->minBytesPerChar,
                                           next - enc->minBytesPerChar,
                                           NULL);
                    if (!name)
                        return ERROR_EXPAT_NO_MEMORY;
                    entity = (ENTITY *) lookup(&dtd.paramEntities, name, 0);
                    poolDiscard(&tempPool);
                    if (!entity)
                    {
                        if (enc == encoding)
                            eventPtr = entityTextPtr;
                        return ERROR_EXPAT_UNDEFINED_ENTITY;
                    }
                    if (entity->open)
                    {
                        if (enc == encoding)
                            eventPtr = entityTextPtr;
                        return ERROR_EXPAT_RECURSIVE_ENTITY_REF;
                    }
                    if (entity->systemId)
                    {
                        if (enc == encoding)
                            eventPtr = entityTextPtr;
                        return ERROR_EXPAT_PARAM_ENTITY_REF;
                    }
                    entity->open = 1;
                    result = storeEntityValue(parser,
                                              internalEncoding,
                                              (char *)entity->textPtr,
                                 (char *)(entity->textPtr + entity->textLen));
                    entity->open = 0;
                    if (result)
                        return result;
                    break;
                }
#endif /* XML_DTD */
                eventPtr = entityTextPtr;
                return ERROR_EXPAT_SYNTAX;
            case XML_TOK_NONE:
                return ERROR_EXPAT_NONE;
            case XML_TOK_ENTITY_REF:
            case XML_TOK_DATA_CHARS:
                if (!poolAppend(pool, enc, entityTextPtr, next, NULL))
                    return ERROR_EXPAT_NO_MEMORY;
                break;
            case XML_TOK_TRAILING_CR:
                next = entityTextPtr + enc->minBytesPerChar;
                /* fall through */
            case XML_TOK_DATA_NEWLINE:
                if (pool->end == pool->ptr && !poolGrow(pool))
                    return ERROR_EXPAT_NO_MEMORY;
                *(pool->ptr)++ = 0xA;
                break;
            case XML_TOK_CHAR_REF:
                {
                    XML_Char buf[XML_ENCODE_MAX];
                    int i;
                    int n = XmlCharRefNumber(enc, entityTextPtr);

                    if (n < 0)
                    {
                        if (enc == encoding)
                            eventPtr = entityTextPtr;
                        return ERROR_EXPAT_BAD_CHAR_REF;
                    }
                    n = XmlEncode(n, (ICHAR *) buf);
                    if (!n)
                    {
                        if (enc == encoding)
                            eventPtr = entityTextPtr;
                        return ERROR_EXPAT_BAD_CHAR_REF;
                    }
                    for (i = 0; i < n; i++)
                    {
                        if (pool->end == pool->ptr && !poolGrow(pool))
                            return ERROR_EXPAT_NO_MEMORY;
                        *(pool->ptr)++ = buf[i];
                    }
                }
                break;
            case XML_TOK_PARTIAL:
                if (enc == encoding)
                    eventPtr = entityTextPtr;
                return ERROR_EXPAT_INVALID_TOKEN;
            case XML_TOK_INVALID:
                if (enc == encoding)
                    eventPtr = next;
                return ERROR_EXPAT_INVALID_TOKEN;
            default:
                if (enc == encoding)
                    eventPtr = entityTextPtr;
                return ERROR_EXPAT_UNEXPECTED_STATE;
        }
        entityTextPtr = next;
    }
    /* not reached */
}

STATIC void  normalizeLines(XML_Char * s)
{
    XML_Char *p;

    for (;; s++)
    {
        if (*s == XML_T('\0'))
            return;
        if (*s == 0xD)
            break;
    }
    p = s;
    do
    {
        if (*s == 0xD)
        {
            *p++ = 0xA;
            if (*++s == 0xA)
                s++;
        }
        else
            *p++ = *s++;
    }
    while (*s);
    *p = XML_T('\0');
}

STATIC int reportProcessingInstruction(XML_Parser parser,
                                       const ENCODING * enc,
                                       const char *start,
                                       const char *end)
{
    const XML_Char *target;
    XML_Char *data;
    const char *tem;

    if (!processingInstructionHandler)
    {
        if (defaultHandler)
            reportDefault(parser, enc, start, end);
        return 1;
    }
    start += enc->minBytesPerChar * 2;
    tem = start + XmlNameLength(enc, start);
    target = poolStoreString(&tempPool, enc, start, tem, NULL);
    if (!target)
        return 0;
    poolFinish(&tempPool);
    data = poolStoreString(&tempPool, enc,
                           XmlSkipS(enc, tem),
                           end - enc->minBytesPerChar * 2,
                           NULL);
    if (!data)
        return 0;
    normalizeLines(data);
    processingInstructionHandler(handlerArg, target, data);
    poolClear(&tempPool);
    return 1;
}

STATIC int reportComment(XML_Parser parser,
                         const ENCODING * enc,
                         const char *start,
                         const char *end)
{
    XML_Char *data;

    if (!commentHandler)
    {
        if (defaultHandler)
            reportDefault(parser, enc, start, end);
        return 1;
    }
    data = poolStoreString(&tempPool,
                           enc,
                           start + enc->minBytesPerChar * 4,
                           end - enc->minBytesPerChar * 3,
                           NULL);
    if (!data)
        return 0;
    normalizeLines(data);
    commentHandler(handlerArg, data);
    poolClear(&tempPool);
    return 1;
}

STATIC void reportDefault(XML_Parser parser,
                          const ENCODING * enc,
                          const char *s,
                          const char *end)
{
    if (MUST_CONVERT(enc, s))
    {
        const char **eventPP;
        const char **eventEndPP;

        if (enc == encoding)
        {
            eventPP = &eventPtr;
            eventEndPP = &eventEndPtr;
        }
        else
        {
            eventPP = &(openInternalEntities->internalEventPtr);
            eventEndPP = &(openInternalEntities->internalEventEndPtr);
        }
        do
        {
            ICHAR *dataPtr = (ICHAR *) dataBuf;

            XmlConvert(enc, &s, end, &dataPtr, (ICHAR *) dataBufEnd);
            *eventEndPP = s;
            defaultHandler(handlerArg, dataBuf, dataPtr - (ICHAR *) dataBuf);
            *eventPP = s;
        }
        while (s != end);
    }
    else
        defaultHandler(handlerArg, (XML_Char *) s, (XML_Char *) end - (XML_Char *) s);
}


STATIC int defineAttribute(ELEMENT_TYPE * type,
                           ATTRIBUTE_ID * attId,
                           int isCdata,
                           int isId,
                           const XML_Char * value,
                           XML_Parser parser)
{
    DEFAULT_ATTRIBUTE *att;

    if (value || isId)
    {
        /* The handling of default attributes gets messed up if we have
         * a default which duplicates a non-default. */
        int i;

        for (i = 0; i < type->nDefaultAtts; i++)
            if (attId == type->defaultAtts[i].id)
                return 1;
        if (isId && !type->idAtt && !attId->xmlns)
            type->idAtt = attId;
    }
    if (type->nDefaultAtts == type->allocDefaultAtts)
    {
        if (type->allocDefaultAtts == 0)
        {
            type->allocDefaultAtts = 8;
            type->defaultAtts = (DEFAULT_ATTRIBUTE*)MALLOC(type->allocDefaultAtts * sizeof(DEFAULT_ATTRIBUTE));
        }
        else
        {
            type->allocDefaultAtts *= 2;
            type->defaultAtts = (DEFAULT_ATTRIBUTE*)REALLOC(type->defaultAtts,
                          type->allocDefaultAtts * sizeof(DEFAULT_ATTRIBUTE));
        }
        if (!type->defaultAtts)
            return 0;
    }
    att = type->defaultAtts + type->nDefaultAtts;
    att->id = attId;
    att->value = value;
    att->isCdata = isCdata;
    if (!isCdata)
        attId->maybeTokenized = 1;
    type->nDefaultAtts += 1;
    return 1;
}

STATIC int setElementTypePrefix(XML_Parser parser,
                                ELEMENT_TYPE * elementType)
{
    const XML_Char *name;

    for (name = elementType->name; *name; name++)
    {
        if (*name == XML_T(':'))
        {
            PREFIX *prefix;
            const XML_Char *s;

            for (s = elementType->name; s != name; s++)
            {
                if (!poolAppendChar(&dtd.pool, *s))
                    return 0;
            }
            if (!poolAppendChar(&dtd.pool, XML_T('\0')))
                return 0;
            prefix = (PREFIX *) lookup(&dtd.prefixes, poolStart(&dtd.pool), sizeof(PREFIX));
            if (!prefix)
                return 0;
            if (prefix->name == poolStart(&dtd.pool))
                poolFinish(&dtd.pool);
            else
                poolDiscard(&dtd.pool);
            elementType->prefix = prefix;

        }
    }
    return 1;
}

STATIC ATTRIBUTE_ID * getAttributeId(XML_Parser parser,
                                     const ENCODING * enc,
                                     const char *start,
                                     const char *end)
{
    ATTRIBUTE_ID *id;
    const XML_Char *name;

    if (!poolAppendChar(&dtd.pool, XML_T('\0')))
        return 0;
    name = poolStoreString(&dtd.pool, enc, start, end, NULL);
    if (!name)
        return 0;
    ++name;
    id = (ATTRIBUTE_ID *) lookup(&dtd.attributeIds, name, sizeof(ATTRIBUTE_ID));
    if (!id)
        return 0;
    if (id->name != name)
        poolDiscard(&dtd.pool);
    else
    {
        poolFinish(&dtd.pool);
        if (!ns)
            ;
        else if (name[0] == 'x'
                 && name[1] == 'm'
                 && name[2] == 'l'
                 && name[3] == 'n'
                 && name[4] == 's'
                 && (name[5] == XML_T('\0') || name[5] == XML_T(':')))
        {
            if (name[5] == '\0')
                id->prefix = &dtd.defaultPrefix;
            else
                id->prefix = (PREFIX *) lookup(&dtd.prefixes, name + 6, sizeof(PREFIX));
            id->xmlns = 1;
        }
        else
        {
            int i;

            for (i = 0; name[i]; i++)
            {
                if (name[i] == XML_T(':'))
                {
                    int j;

                    for (j = 0; j < i; j++)
                    {
                        if (!poolAppendChar(&dtd.pool, name[j]))
                            return 0;
                    }
                    if (!poolAppendChar(&dtd.pool, XML_T('\0')))
                        return 0;
                    id->prefix = (PREFIX *) lookup(&dtd.prefixes, poolStart(&dtd.pool), sizeof(PREFIX));
                    if (id->prefix->name == poolStart(&dtd.pool))
                        poolFinish(&dtd.pool);
                    else
                        poolDiscard(&dtd.pool);
                    break;
                }
            }
        }
    }
    return id;
}

#define CONTEXT_SEP XML_T('\f')

STATIC const XML_Char *getContext(XML_Parser parser)
{
    HASH_TABLE_ITER iter;
    int needSep = 0;

    if (dtd.defaultPrefix.binding)
    {
        int i;
        int len;

        if (!poolAppendChar(&tempPool, XML_T('=')))
            return 0;
        len = dtd.defaultPrefix.binding->uriLen;
        if (namespaceSeparator != XML_T('\0'))
            len--;
        for (i = 0; i < len; i++)
            if (!poolAppendChar(&tempPool, dtd.defaultPrefix.binding->uri[i]))
                return 0;
        needSep = 1;
    }

    hashTableIterInit(&iter, &(dtd.prefixes));
    for (;;)
    {
        int i;
        int len;
        const XML_Char *s;
        PREFIX *prefix = (PREFIX *) hashTableIterNext(&iter);

        if (!prefix)
            break;
        if (!prefix->binding)
            continue;
        if (needSep && !poolAppendChar(&tempPool, CONTEXT_SEP))
            return 0;
        for (s = prefix->name; *s; s++)
            if (!poolAppendChar(&tempPool, *s))
                return 0;
        if (!poolAppendChar(&tempPool, XML_T('=')))
            return 0;
        len = prefix->binding->uriLen;
        if (namespaceSeparator != XML_T('\0'))
            len--;
        for (i = 0; i < len; i++)
            if (!poolAppendChar(&tempPool, prefix->binding->uri[i]))
                return 0;
        needSep = 1;
    }


    hashTableIterInit(&iter, &(dtd.generalEntities));
    for (;;)
    {
        const XML_Char *s;
        ENTITY *e = (ENTITY *) hashTableIterNext(&iter);

        if (!e)
            break;
        if (!e->open)
            continue;
        if (needSep && !poolAppendChar(&tempPool, CONTEXT_SEP))
            return 0;
        for (s = e->name; *s; s++)
            if (!poolAppendChar(&tempPool, *s))
                return 0;
        needSep = 1;
    }

    if (!poolAppendChar(&tempPool, XML_T('\0')))
        return 0;
    return tempPool.start;
}

STATIC int setContext(XML_Parser parser,
                      const XML_Char * context)
{
    const XML_Char *s = context;

    while (*context != XML_T('\0'))
    {
        if (*s == CONTEXT_SEP || *s == XML_T('\0'))
        {
            ENTITY *e;

            if (!poolAppendChar(&tempPool, XML_T('\0')))
                return 0;
            e = (ENTITY *) lookup(&dtd.generalEntities, poolStart(&tempPool), 0);
            if (e)
                e->open = 1;
            if (*s != XML_T('\0'))
                s++;
            context = s;
            poolDiscard(&tempPool);
        }
        else if (*s == '=')
        {
            PREFIX *prefix;

            if (poolLength(&tempPool) == 0)
                prefix = &dtd.defaultPrefix;
            else
            {
                if (!poolAppendChar(&tempPool, XML_T('\0')))
                    return 0;
                prefix = (PREFIX *) lookup(&dtd.prefixes, poolStart(&tempPool), sizeof(PREFIX));
                if (!prefix)
                    return 0;
                if (prefix->name == poolStart(&tempPool))
                {
                    prefix->name = poolCopyString(&dtd.pool, prefix->name);
                    if (!prefix->name)
                        return 0;
                }
                poolDiscard(&tempPool);
            }
            for (context = s + 1; *context != CONTEXT_SEP && *context != XML_T('\0'); context++)
                if (!poolAppendChar(&tempPool, *context))
                    return 0;
            if (!poolAppendChar(&tempPool, XML_T('\0')))
                return 0;
            if (!addBinding(parser, prefix, 0, poolStart(&tempPool), &inheritedBindings))
                return 0;
            poolDiscard(&tempPool);
            if (*context != XML_T('\0'))
                ++context;
            s = context;
        }
        else
        {
            if (!poolAppendChar(&tempPool, *s))
                return 0;
            s++;
        }
    }
    return 1;
}


STATIC void normalizePublicId(XML_Char * publicId)
{
    XML_Char *p = publicId;
    XML_Char *s;

    for (s = publicId; *s; s++)
    {
        switch (*s)
        {
            case 0x20:
            case 0xD:
            case 0xA:
                if (p != publicId && p[-1] != 0x20)
                    *p++ = 0x20;
                break;
            default:
                *p++ = *s;
        }
    }
    if (p != publicId && p[-1] == 0x20)
        --p;
    *p = XML_T('\0');
}

STATIC int dtdInit(DTD * p,
                   XML_Parser parser)
{
    XML_Memory_Handling_Suite *ms = &((Parser *) parser)->m_mem;

    poolInit(&(p->pool), ms);
    hashTableInit(&(p->generalEntities), ms);
    hashTableInit(&(p->elementTypes), ms);
    hashTableInit(&(p->attributeIds), ms);
    hashTableInit(&(p->prefixes), ms);
    p->complete = 1;
    p->standalone = 0;
#ifdef XML_DTD
    hashTableInit(&(p->paramEntities), ms);
#endif /* XML_DTD */
    p->defaultPrefix.name = 0;
    p->defaultPrefix.binding = 0;

    p->in_eldecl = 0;
    p->scaffIndex = 0;
    p->scaffLevel = 0;
    p->scaffold = 0;
    p->contentStringLen = 0;
    p->scaffSize = 0;
    p->scaffCount = 0;

    return 1;
}

#ifdef XML_DTD

STATIC void dtdSwap(DTD * p1, DTD * p2)
{
    DTD tem;

    memcpy(&tem, p1, sizeof(DTD));
    memcpy(p1, p2, sizeof(DTD));
    memcpy(p2, &tem, sizeof(DTD));
}

#endif /* XML_DTD */

STATIC void dtdDestroy(DTD * p, XML_Parser parser)
{
    HASH_TABLE_ITER iter;

    hashTableIterInit(&iter, &(p->elementTypes));
    for (;;)
    {
        ELEMENT_TYPE *e = (ELEMENT_TYPE *) hashTableIterNext(&iter);

        if (!e)
            break;
        if (e->allocDefaultAtts != 0)
            FREE(e->defaultAtts);
    }
    hashTableDestroy(&(p->generalEntities));
#ifdef XML_DTD
    hashTableDestroy(&(p->paramEntities));
#endif /* XML_DTD */
    hashTableDestroy(&(p->elementTypes));
    hashTableDestroy(&(p->attributeIds));
    hashTableDestroy(&(p->prefixes));
    poolDestroy(&(p->pool));
    if (p->scaffIndex)
        FREE(p->scaffIndex);
    if (p->scaffold)
        FREE(p->scaffold);
}

/* Do a deep copy of the DTD.  Return 0 for out of memory; non-zero otherwise.
 * The new DTD has already been initialized. */

STATIC int dtdCopy(DTD * newDtd, const DTD * oldDtd, XML_Parser parser)
{
    HASH_TABLE_ITER iter;

    /* Copy the prefix table. */

    hashTableIterInit(&iter, &(oldDtd->prefixes));
    for (;;)
    {
        const XML_Char *name;
        const PREFIX *oldP = (PREFIX *) hashTableIterNext(&iter);

        if (!oldP)
            break;
        name = poolCopyString(&(newDtd->pool), oldP->name);
        if (!name)
            return 0;
        if (!lookup(&(newDtd->prefixes), name, sizeof(PREFIX)))
            return 0;
    }

    hashTableIterInit(&iter, &(oldDtd->attributeIds));

    /* Copy the attribute id table. */

    for (;;)
    {
        ATTRIBUTE_ID *newA;
        const XML_Char *name;
        const ATTRIBUTE_ID *oldA = (ATTRIBUTE_ID *) hashTableIterNext(&iter);

        if (!oldA)
            break;
        /* Remember to allocate the scratch byte before the name. */
        if (!poolAppendChar(&(newDtd->pool), XML_T('\0')))
            return 0;
        name = poolCopyString(&(newDtd->pool), oldA->name);
        if (!name)
            return 0;
        ++name;
        newA = (ATTRIBUTE_ID *) lookup(&(newDtd->attributeIds), name, sizeof(ATTRIBUTE_ID));
        if (!newA)
            return 0;
        newA->maybeTokenized = oldA->maybeTokenized;
        if (oldA->prefix)
        {
            newA->xmlns = oldA->xmlns;
            if (oldA->prefix == &oldDtd->defaultPrefix)
                newA->prefix = &newDtd->defaultPrefix;
            else
                newA->prefix = (PREFIX *) lookup(&(newDtd->prefixes), oldA->prefix->name, 0);
        }
    }

    /* Copy the element type table. */

    hashTableIterInit(&iter, &(oldDtd->elementTypes));

    for (;;)
    {
        int i;
        ELEMENT_TYPE *newE;
        const XML_Char *name;
        const ELEMENT_TYPE *oldE = (ELEMENT_TYPE *) hashTableIterNext(&iter);

        if (!oldE)
            break;
        name = poolCopyString(&(newDtd->pool), oldE->name);
        if (!name)
            return 0;
        newE = (ELEMENT_TYPE *) lookup(&(newDtd->elementTypes), name, sizeof(ELEMENT_TYPE));
        if (!newE)
            return 0;
        if (oldE->nDefaultAtts)
        {
            newE->defaultAtts = (DEFAULT_ATTRIBUTE *) MALLOC(oldE->nDefaultAtts * sizeof(DEFAULT_ATTRIBUTE));
            if (!newE->defaultAtts)
                return 0;
        }
        if (oldE->idAtt)
            newE->idAtt = (ATTRIBUTE_ID *) lookup(&(newDtd->attributeIds), oldE->idAtt->name, 0);
        newE->allocDefaultAtts = newE->nDefaultAtts = oldE->nDefaultAtts;
        if (oldE->prefix)
            newE->prefix = (PREFIX *) lookup(&(newDtd->prefixes), oldE->prefix->name, 0);
        for (i = 0; i < newE->nDefaultAtts; i++)
        {
            newE->defaultAtts[i].id = (ATTRIBUTE_ID *) lookup(&(newDtd->attributeIds), oldE->defaultAtts[i].id->name, 0);
            newE->defaultAtts[i].isCdata = oldE->defaultAtts[i].isCdata;
            if (oldE->defaultAtts[i].value)
            {
                newE->defaultAtts[i].value = poolCopyString(&(newDtd->pool), oldE->defaultAtts[i].value);
                if (!newE->defaultAtts[i].value)
                    return 0;
            }
            else
                newE->defaultAtts[i].value = 0;
        }
    }

    /* Copy the entity tables. */
    if (!copyEntityTable(&(newDtd->generalEntities),
                         &(newDtd->pool),
                         &(oldDtd->generalEntities), parser))
        return 0;

#ifdef XML_DTD
    if (!copyEntityTable(&(newDtd->paramEntities),
                         &(newDtd->pool),
                         &(oldDtd->paramEntities), parser))
        return 0;
#endif /* XML_DTD */

    newDtd->complete = oldDtd->complete;
    newDtd->standalone = oldDtd->standalone;

    /* Don't want deep copying for scaffolding */
    newDtd->in_eldecl = oldDtd->in_eldecl;
    newDtd->scaffold = oldDtd->scaffold;
    newDtd->contentStringLen = oldDtd->contentStringLen;
    newDtd->scaffSize = oldDtd->scaffSize;
    newDtd->scaffLevel = oldDtd->scaffLevel;
    newDtd->scaffIndex = oldDtd->scaffIndex;

    return 1;
}                               /* End dtdCopy */

STATIC int copyEntityTable(HASH_TABLE * newTable,
                           STRING_POOL * newPool,
                           const HASH_TABLE * oldTable,
                           XML_Parser parser)
{
    HASH_TABLE_ITER iter;
    const XML_Char *cachedOldBase = 0;
    const XML_Char *cachedNewBase = 0;

    hashTableIterInit(&iter, oldTable);

    for (;;)
    {
        ENTITY *newE;
        const XML_Char *name;
        const ENTITY *oldE = (ENTITY *) hashTableIterNext(&iter);

        if (!oldE)
            break;
        name = poolCopyString(newPool, oldE->name);
        if (!name)
            return 0;
        newE = (ENTITY *) lookup(newTable, name, sizeof(ENTITY));
        if (!newE)
            return 0;
        if (oldE->systemId)
        {
            const XML_Char *tem = poolCopyString(newPool, oldE->systemId);

            if (!tem)
                return 0;
            newE->systemId = tem;
            if (oldE->base)
            {
                if (oldE->base == cachedOldBase)
                    newE->base = cachedNewBase;
                else
                {
                    cachedOldBase = oldE->base;
                    tem = poolCopyString(newPool, cachedOldBase);
                    if (!tem)
                        return 0;
                    cachedNewBase = newE->base = tem;
                }
            }
        }
        else
        {
            const XML_Char *tem = poolCopyStringN(newPool, oldE->textPtr, oldE->textLen);

            if (!tem)
                return 0;
            newE->textPtr = tem;
            newE->textLen = oldE->textLen;
        }
        if (oldE->notation)
        {
            const XML_Char *tem = poolCopyString(newPool, oldE->notation);

            if (!tem)
                return 0;
            newE->notation = tem;
        }
    }
    return 1;
}

#define INIT_SIZE 64

STATIC int keyeq(KEY s1, KEY s2)
{
    for (; *s1 == *s2; s1++, s2++)
        if (*s1 == 0)
            return 1;
    return 0;
}

STATIC unsigned long hash(KEY s)
{
    unsigned long h = 0;

    while (*s)
        h = (h << 5) + h + (unsigned char)*s++;
    return h;
}

STATIC NAMED *lookup(HASH_TABLE * table,
                     KEY name,
                     size_t createSize)
{
    size_t i;

    if (table->size == 0)
    {
        size_t tsize;

        if (!createSize)
            return 0;
        tsize = INIT_SIZE * sizeof(NAMED *);
        table->v = (NAMED**)table->mem->malloc_fcn(tsize);
        if (!table->v)
            return 0;
        memset(table->v, 0, tsize);
        table->size = INIT_SIZE;
        table->usedLim = INIT_SIZE / 2;
        i = hash(name) & (table->size - 1);
    }
    else
    {
        unsigned long h = hash(name);

        for (i = h & (table->size - 1);
             table->v[i];
             i == 0 ? i = table->size - 1 : --i)
        {
            if (keyeq(name, table->v[i]->name))
                return table->v[i];
        }
        if (!createSize)
            return 0;
        if (table->used == table->usedLim)
        {
            /* check for overflow */
            size_t newSize = table->size * 2;
            size_t tsize = newSize * sizeof(NAMED *);
            NAMED **newV = (NAMED**)table->mem->malloc_fcn(tsize);

            if (!newV)
                return 0;
            memset(newV, 0, tsize);
            for (i = 0; i < table->size; i++)
                if (table->v[i])
                {
                    size_t j;

                    for (j = hash(table->v[i]->name) & (newSize - 1);
                         newV[j];
                         j == 0 ? j = newSize - 1 : --j)
                        ;
                    newV[j] = table->v[i];
                }
            table->mem->free_fcn(table->v);
            table->v = newV;
            table->size = newSize;
            table->usedLim = newSize / 2;
            for (i = h & (table->size - 1);
                 table->v[i];
                 i == 0 ? i = table->size - 1 : --i)
                ;
        }
    }
    table->v[i] = (NAMED*)table->mem->malloc_fcn(createSize);
    if (!table->v[i])
        return 0;
    memset(table->v[i], 0, createSize);
    table->v[i]->name = name;
    (table->used)++;
    return table->v[i];
}

STATIC void hashTableDestroy(HASH_TABLE * table)
{
    size_t i;

    for (i = 0; i < table->size; i++)
    {
        NAMED *p = table->v[i];

        if (p)
            table->mem->free_fcn(p);
    }
    if (table->v)
        table->mem->free_fcn(table->v);
}

STATIC void hashTableInit(HASH_TABLE * p, XML_Memory_Handling_Suite * ms)
{
    p->size = 0;
    p->usedLim = 0;
    p->used = 0;
    p->v = 0;
    p->mem = ms;
}

STATIC void hashTableIterInit(HASH_TABLE_ITER * iter, const HASH_TABLE * table)
{
    iter->p = table->v;
    iter->end = iter->p + table->size;
}

STATIC NAMED *hashTableIterNext(HASH_TABLE_ITER * iter)
{
    while (iter->p != iter->end)
    {
        NAMED *tem = *(iter->p)++;

        if (tem)
            return tem;
    }
    return 0;
}


STATIC void poolInit(STRING_POOL * pool, XML_Memory_Handling_Suite * ms)
{
    pool->blocks = 0;
    pool->freeBlocks = 0;
    pool->start = 0;
    pool->ptr = 0;
    pool->end = 0;
    pool->mem = ms;
}

STATIC void poolClear(STRING_POOL * pool)
{
    if (!pool->freeBlocks)
        pool->freeBlocks = pool->blocks;
    else
    {
        BLOCK *p = pool->blocks;

        while (p)
        {
            BLOCK *tem = p->next;

            p->next = pool->freeBlocks;
            pool->freeBlocks = p;
            p = tem;
        }
    }
    pool->blocks = 0;
    pool->start = 0;
    pool->ptr = 0;
    pool->end = 0;
}

STATIC void poolDestroy(STRING_POOL * pool)
{
    BLOCK *p = pool->blocks;

    while (p)
    {
        BLOCK *tem = p->next;

        pool->mem->free_fcn(p);
        p = tem;
    }
    pool->blocks = 0;
    p = pool->freeBlocks;
    while (p)
    {
        BLOCK *tem = p->next;

        pool->mem->free_fcn(p);
        p = tem;
    }
    pool->freeBlocks = 0;
    pool->ptr = 0;
    pool->start = 0;
    pool->end = 0;
}

/*
 *@@ poolAppend:
 *      appends a new string to a pool.
 *      Returns pool->start if the pool is valid,
 *      or NULL if storing failed.
 *
 *      V0.9.14: I added the pulOfs parameter which, after
 *      the string was stored, receives the offset from
 *      pool->start at which the string was stored in the
 *      pool. I am not quite sure how all this works, but
 *      if a user-defined encoding is in place (via
 *      XML_SetUnknownEncodingHandler), the first entry
 *      in the pool is always the encoding name from the
 *      xml header, and all entity references failed here
 *      because the lookup would always be done for
 *      the encoding name instead of the entity name
 *      (which then is the second entry in the pool).
 *      Whatever.
 *
 *@@changed V0.9.14 (2001-08-09) [umoeller]: added pulOfs param
 */

STATIC XML_Char* poolAppend(STRING_POOL * pool,
                            const ENCODING * enc,
                            const char *ptr,
                            const char *end,
                            unsigned long *pulOfs)  // out: offset of beginning of stored
                                                    // string from pool->start
                                                    // V0.9.14 (2001-08-09) [umoeller]
{
    if (!pool->ptr && !poolGrow(pool))
        return 0;
    for (;;)
    {
        char *pOldStart = pool->ptr;
        XmlConvert(enc,
                   &ptr,                    // fromP
                   end,
                   (ICHAR**)&(pool->ptr),   // toP
                   (ICHAR *)pool->end);
                // expands to:
                // (((enc)->utf8Convert)(enc, fromP, fromLim, toP, toLim))
                // -- for cp850, we end up in
                // -- for Latin1, we end up in
        if (pulOfs)
            *pulOfs = pOldStart - pool->start; // V0.9.14 (2001-08-09) [umoeller]

        if (ptr == end)
            break;
        if (!poolGrow(pool))
            return 0;
    }
    return pool->start;
}

STATIC const XML_Char* poolCopyString(STRING_POOL *pool,
                                      const XML_Char *s)
{
    do
    {
        if (!poolAppendChar(pool, *s))
            return 0;
    }
    while (*s++);
    s = pool->start;
    poolFinish(pool);
    return s;
}

STATIC const XML_Char *poolCopyStringN(STRING_POOL * pool, const XML_Char * s, int n)
{
    if (!pool->ptr && !poolGrow(pool))
        return 0;
    for (; n > 0; --n, s++)
    {
        if (!poolAppendChar(pool, *s))
            return 0;

    }
    s = pool->start;
    poolFinish(pool);
    return s;
}

STATIC const XML_Char* poolAppendString(STRING_POOL *pool,
                                        const XML_Char *s)
{
    while (*s)
    {
        if (!poolAppendChar(pool, *s))
            return 0;
        s++;
    }
    return pool->start;
}                               /* End poolAppendString */

/*
 *@@ poolStoreString:
 *
 *@@changed V0.9.14 (2001-08-09) [umoeller]: added pulOfs param
 */

STATIC XML_Char* poolStoreString(STRING_POOL *pool,
                                 const ENCODING *enc,
                                 const char *ptr,
                                 const char *end,
                                 unsigned long *pulOfs) // V0.9.14 (2001-08-09) [umoeller]
{
    if (!poolAppend(pool,
                    enc,
                    ptr,
                    end,
                    pulOfs))    // V0.9.14 (2001-08-09) [umoeller]
        return 0;
    if (    (pool->ptr == pool->end)
         && (!poolGrow(pool))
       )
        return 0;
    *(pool->ptr)++ = 0;
    return pool->start;
}

STATIC int poolGrow(STRING_POOL *pool)
{
    if (pool->freeBlocks)
    {
        if (pool->start == 0)
        {
            pool->blocks = pool->freeBlocks;
            pool->freeBlocks = pool->freeBlocks->next;
            pool->blocks->next = 0;
            pool->start = pool->blocks->s;
            pool->end = pool->start + pool->blocks->size;
            pool->ptr = pool->start;
            return 1;
        }
        if (pool->end - pool->start < pool->freeBlocks->size)
        {
            BLOCK *tem = pool->freeBlocks->next;

            pool->freeBlocks->next = pool->blocks;
            pool->blocks = pool->freeBlocks;
            pool->freeBlocks = tem;
            memcpy(pool->blocks->s,
                   pool->start,
                   (pool->end - pool->start) * sizeof(XML_Char));
            pool->ptr = pool->blocks->s + (pool->ptr - pool->start);
            pool->start = pool->blocks->s;
            pool->end = pool->start + pool->blocks->size;
            return 1;
        }
    }
    if (pool->blocks && pool->start == pool->blocks->s)
    {
        int blockSize = (pool->end - pool->start) * 2;

        pool->blocks = (BLOCK*)pool->mem->realloc_fcn(pool->blocks, offsetof(BLOCK, s) + blockSize * sizeof(XML_Char));
        if (!pool->blocks)
            return 0;
        pool->blocks->size = blockSize;
        pool->ptr = pool->blocks->s + (pool->ptr - pool->start);
        pool->start = pool->blocks->s;
        pool->end = pool->start + blockSize;
    }
    else
    {
        BLOCK *tem;
        int blockSize = pool->end - pool->start;

        if (blockSize < INIT_BLOCK_SIZE)
            blockSize = INIT_BLOCK_SIZE;
        else
            blockSize *= 2;
        tem = (BLOCK*)pool->mem->malloc_fcn(offsetof(BLOCK, s) + blockSize * sizeof(XML_Char));
        if (!tem)
            return 0;
        tem->size = blockSize;
        tem->next = pool->blocks;
        pool->blocks = tem;
        if (pool->ptr != pool->start)
            memcpy(tem->s, pool->start, (pool->ptr - pool->start) * sizeof(XML_Char));
        pool->ptr = tem->s + (pool->ptr - pool->start);
        pool->start = tem->s;
        pool->end = tem->s + blockSize;
    }
    return 1;
}

STATIC int nextScaffoldPart(XML_Parser parser)
{
    CONTENT_SCAFFOLD *me;
    int next;

    if (!dtd.scaffIndex)
    {
        dtd.scaffIndex = (int*)MALLOC(groupSize * sizeof(int));

        if (!dtd.scaffIndex)
            return -1;
        dtd.scaffIndex[0] = 0;
    }

    if (dtd.scaffCount >= dtd.scaffSize)
    {
        if (dtd.scaffold)
        {
            dtd.scaffSize *= 2;
            dtd.scaffold = (CONTENT_SCAFFOLD *) REALLOC(dtd.scaffold,
                                    dtd.scaffSize * sizeof(CONTENT_SCAFFOLD));
        }
        else
        {
            dtd.scaffSize = 32;
            dtd.scaffold = (CONTENT_SCAFFOLD *) MALLOC(dtd.scaffSize * sizeof(CONTENT_SCAFFOLD));
        }
        if (!dtd.scaffold)
            return -1;
    }
    next = dtd.scaffCount++;
    me = &dtd.scaffold[next];
    if (dtd.scaffLevel)
    {
        CONTENT_SCAFFOLD *parent = &dtd.scaffold[dtd.scaffIndex[dtd.scaffLevel - 1]];

        if (parent->lastchild)
        {
            dtd.scaffold[parent->lastchild].nextsib = next;
        }
        if (!parent->childcnt)
            parent->firstchild = next;
        parent->lastchild = next;
        parent->childcnt++;
    }
    me->firstchild = me->lastchild = me->childcnt = me->nextsib = 0;
    return next;
}                               /* End nextScaffoldPart */

STATIC void  build_node(XML_Parser parser,
                        int src_node,
                        XMLCONTENT * dest,
                        XMLCONTENT ** contpos,
                        char **strpos)
{
    dest->type = dtd.scaffold[src_node].type;
    dest->quant = dtd.scaffold[src_node].quant;
    if (dest->type == XML_CTYPE_NAME)
    {
        const char *src;

        dest->name = *strpos;
        src = dtd.scaffold[src_node].name;
        for (;;)
        {
            *(*strpos)++ = *src;
            if (!*src)
                break;
            src++;
        }
        dest->numchildren = 0;
        dest->children = 0;
    }
    else
    {
        unsigned int i;
        int cn;

        dest->numchildren = dtd.scaffold[src_node].childcnt;
        dest->children = *contpos;
        *contpos += dest->numchildren;
        for (i = 0, cn = dtd.scaffold[src_node].firstchild;
             i < dest->numchildren;
             i++, cn = dtd.scaffold[cn].nextsib)
        {
            build_node(parser, cn, &(dest->children[i]), contpos, strpos);
        }
        dest->name = 0;
    }
}                               /* End build_node */

STATIC XMLCONTENT * build_model(XML_Parser parser)
{
    XMLCONTENT *ret;
    XMLCONTENT *cpos;
    char *str;
    int allocsize = dtd.scaffCount * sizeof(XMLCONTENT) + dtd.contentStringLen;

    ret = (XMLCONTENT*)MALLOC(allocsize);
    if (!ret)
        return 0;

    str = (char *)(&ret[dtd.scaffCount]);
    cpos = &ret[1];

    build_node(parser, 0, ret, &cpos, &str);
    return ret;
}                               /* End build_model */

STATIC ELEMENT_TYPE * getElementType(XML_Parser parser,
                                     const ENCODING * enc,
                                     const char *ptr,
                                     const char *end)
{
    const XML_Char *name = poolStoreString(&dtd.pool, enc, ptr, end, NULL);
    ELEMENT_TYPE *ret;

    if (!name)
        return 0;
    ret = (ELEMENT_TYPE *) lookup(&dtd.elementTypes, name, sizeof(ELEMENT_TYPE));
    if (!ret)
        return 0;
    if (ret->name != name)
        poolDiscard(&dtd.pool);
    else
    {
        poolFinish(&dtd.pool);
        if (!setElementTypePrefix(parser, ret))
            return 0;
    }
    return ret;
}                               /* End getElementType */
