/* json.h - JSON parsing, building and serialization (C89) */
#ifndef JSON_H
#define JSON_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file json.h
 * @brief JSON parsing, building and serialization.
 *
 * Conforms to:
 *   - RFC 8259, "The JavaScript Object Notation (JSON) Data
 *     Interchange Format".
 *     https://datatracker.ietf.org/doc/html/rfc8259
 *   - ECMA-404, "The JSON Data Interchange Syntax".
 *     https://www.ecma-international.org/publications-and-standards/standards/ecma-404/
 *
 * Four groups of operations are provided:
 *   - Parsing: JsonParse reads a NUL-terminated text and builds a
 *     document tree.
 *   - Building: JsonNewDoc creates an empty document; JsonNewObject,
 *     JsonNewArray, JsonNewString, JsonNewNumber, JsonNewBoolean and
 *     JsonNewNull create nodes; JsonObjectSet and JsonArrayAppend
 *     attach them to their parents; JsonSetRoot designates the root.
 *   - Reading: JsonRoot, JsonNodeGetType, JsonNodeGetChild,
 *     JsonNodeGetCount, JsonNodeGetElement, JsonNodeGetEntry,
 *     JsonNodeGetString, JsonNodeGetBoolean, JsonNodeGetNumber.
 *   - Modification and cloning: JsonNodeGetKey,
 *     JsonNodeSetValueString, JsonNodeSetString, JsonCloneNode.
 *   - Serialization: JsonFormat writes a node into a caller buffer;
 *     JsonWriteFile writes a node into a file or to stdout.
 *
 * All values are accessed through opaque handles; no internal
 * pointers are exposed. Strings are read into caller-supplied
 * buffers using the size-query convention.
 */

/* ==================================================================
 * Value types
 * ================================================================== */

/** @brief JSON value type tag. */
typedef enum {
    JSON_NULL,      /**< Null value.     */
    JSON_BOOLEAN,   /**< Boolean value.  */
    JSON_NUMBER,    /**< Number value.   */
    JSON_STRING,    /**< String value.   */
    JSON_ARRAY,     /**< Array value.    */
    JSON_OBJECT     /**< Object value.   */
} JsonType;

/**
 * @typedef HJSONDOC
 * @brief Handle to a JSON document.
 */
typedef HANDLE HJSONDOC;

/**
 * @typedef HJSONNODE
 * @brief Handle to a value inside a document.
 *
 * A node handle is valid for the lifetime of the owning document.
 * Nodes do not need to be released individually; JsonClose releases
 * them all.
 */
typedef HANDLE HJSONNODE;

/* ==================================================================
 * Parsing
 * ================================================================== */

/**
 * @brief Parse a NUL-terminated JSON text into a document.
 *
 * On success, @p *phDoc receives a document handle. The input buffer
 * is only read; it is not retained. The caller owns the buffer.
 *
 * @param[in]  pszText  JSON text. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszText or phDoc is NULL.
 * @retval ERROR_INVALID_DATA       Malformed JSON, invalid UTF-8,
 *                                  or trailing bytes after the value.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonParse(PCSZ pszText, HJSONDOC *phDoc);

/**
 * @brief Release a document and all associated nodes.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY JsonClose(HJSONDOC hDoc);

/* ==================================================================
 * Building
 * ================================================================== */

/**
 * @brief Create an empty document for building a tree.
 *
 * Nodes created with JsonNew* belong to the document and are
 * released by JsonClose. Before serialization the caller must
 * designate a root node with JsonSetRoot.
 *
 * @param[out] phDoc  Handle receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewDoc(HJSONDOC *phDoc);

/**
 * @brief Designate the root node of a document.
 *
 * @param[in] hDoc   Document handle. Not NULLHANDLE.
 * @param[in] hNode  Root node. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or hNode is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY JsonSetRoot(HJSONDOC hDoc, HJSONNODE hNode);

/**
 * @brief Create a new object node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewObject(HJSONDOC hDoc, HJSONNODE *phNode);

/**
 * @brief Create a new array node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewArray(HJSONDOC hDoc, HJSONNODE *phNode);

/**
 * @brief Create a new string node.
 *
 * The value is copied into the node.
 *
 * @param[in]  hDoc      Document handle. Not NULLHANDLE.
 * @param[in]  pszValue  String value. Not NULL.
 * @param[out] phNode    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE, pszValue or
 *                                  phNode is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewString(HJSONDOC hDoc, PCSZ pszValue,
                              HJSONNODE *phNode);

/**
 * @brief Create a new number node.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  dValue  Numeric value.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewNumber(HJSONDOC hDoc, double dValue,
                              HJSONNODE *phNode);

/**
 * @brief Create a new boolean node.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  fValue  TRUE_ or FALSE_.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewBoolean(HJSONDOC hDoc, BOOL fValue,
                               HJSONNODE *phNode);

/**
 * @brief Create a new null node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewNull(HJSONDOC hDoc, HJSONNODE *phNode);

/**
 * @brief Attach a value to an object under a key.
 *
 * The key is copied into the object. The value node must belong to
 * the same document as the object.
 *
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] hValue  Value node. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       hObj is not an object.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonObjectSet(HJSONNODE hObj, PCSZ pszKey,
                              HJSONNODE hValue);

/**
 * @brief Append a value to an array.
 *
 * @param[in] hArr    Array handle. Not NULLHANDLE.
 * @param[in] hValue  Value node. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hArr or hValue is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       hArr is not an array.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonArrayAppend(HJSONNODE hArr, HJSONNODE hValue);

/* ==================================================================
 * Reading values
 * ================================================================== */

/**
 * @brief Obtain the root node of a parsed document.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or phNode is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Document has no root.
 */
APIRET APIENTRY JsonRoot(HJSONDOC hDoc, HJSONNODE *phNode);

/**
 * @brief Query the type of a node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of a JsonType value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode or pulType is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY JsonNodeGetType(HJSONNODE hNode, PULONG pulType);

/**
 * @brief Obtain a child of an object node by key.
 *
 * @param[in]  hNode    Node handle (object). Not NULLHANDLE.
 * @param[in]  pszKey   Key. Not NULL.
 * @param[out] phChild  Receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an object.
 * @retval ERROR_FILE_NOT_FOUND     Key not present.
 */
APIRET APIENTRY JsonNodeGetChild(HJSONNODE hNode, PCSZ pszKey,
                                 HJSONNODE *phChild);

/**
 * @brief Number of elements in an array node or entries in an
 *        object node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is neither array nor object.
 */
APIRET APIENTRY JsonNodeGetCount(HJSONNODE hNode, PULONG pulCount);

/**
 * @brief Obtain an element of an array node by index.
 *
 * @param[in]  hNode    Node handle (array). Not NULLHANDLE.
 * @param[in]  ulIndex  Element index.
 * @param[out] phChild  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an array.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY JsonNodeGetElement(HJSONNODE hNode, ULONG ulIndex,
                                   HJSONNODE *phChild);

/**
 * @brief Obtain an entry of an object node by index.
 *
 * @param[in]  hNode       Node handle (object). Not NULLHANDLE.
 * @param[in]  ulIndex     Entry index.
 * @param[out] pszKeyBuf   Key buffer. Not NULL.
 * @param[in]  ulKeySize   Size of pszKeyBuf.
 * @param[out] pulKeyUsed  Optional. May be NULL.
 * @param[out] phChild     Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any required parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an object.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Key buffer too small.
 */
APIRET APIENTRY JsonNodeGetEntry(HJSONNODE hNode, ULONG ulIndex,
                                 PSZ pszKeyBuf, ULONG ulKeySize,
                                 PULONG pulKeyUsed, HJSONNODE *phChild);

/**
 * @brief Read a string value of a node.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied and NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hNode    Node handle (string). Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a string.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonNodeGetString(HJSONNODE hNode,
                                  PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Read a boolean value of a node.
 *
 * @param[in]  hNode    Node handle (boolean). Not NULLHANDLE.
 * @param[out] pfValue  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a boolean.
 */
APIRET APIENTRY JsonNodeGetBoolean(HJSONNODE hNode, PBOOL pfValue);

/**
 * @brief Read a number value of a node.
 *
 * @param[in]  hNode     Node handle (number). Not NULLHANDLE.
 * @param[out] pdValue   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a number.
 */
APIRET APIENTRY JsonNodeGetNumber(HJSONNODE hNode, double *pdValue);

/* ==================================================================
 * Node modification and cloning
 * ================================================================== */

/**
 * @brief Retrieve the key of a node.
 *
 * For a child of an object, this is the field name. For a child of an
 * array, or for the root node, the key is empty.
 *
 * Size-query convention as for JsonNodeGetString.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonNodeGetKey(HJSONNODE hNode,
                               PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Set the string value of an existing string node.
 *
 * The node must be of type JSON_STRING. The new value is copied into
 * the node.
 *
 * @param[in] hNode    Node handle (string). Not NULLHANDLE.
 * @param[in] pszVal   New value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULLHANDLE, or pszVal is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a string.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNodeSetValueString(HJSONNODE hNode, PCSZ pszVal);

/**
 * @brief Set or replace a string field on an object.
 *
 * If the object already has a child with the given key and that child
 * is a string, its value is replaced. Otherwise a new string child
 * is appended.
 *
 * @param[in] hDoc    Document handle. Not NULLHANDLE.
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] pszVal  Field value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       hObj is not an object, or the
 *                                  existing child is not a string.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNodeSetString(HJSONDOC hDoc, HJSONNODE hObj,
                                  PCSZ pszKey, PCSZ pszVal);

/**
 * @brief Deep-clone a subtree into a document.
 *
 * The source node must belong to some document; the destination
 * document receives a full copy of the subtree, with all keys and
 * values. Returns a handle for the new root of the copy.
 *
 * @param[in]  hDst   Destination document. Not NULLHANDLE.
 * @param[in]  hSrc   Source node. Not NULLHANDLE.
 * @param[out] phDst  Receiver for the new node. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonCloneNode(HJSONDOC hDst, HJSONNODE hSrc,
                              HJSONNODE *phDst);

/* ==================================================================
 * Serialization
 * ================================================================== */

/**
 * @brief Serialize a node into a caller-supplied buffer.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: text copied and NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[in]  fIndent   TRUE_ for pretty-printed output with two-space
 *                       indentation; FALSE_ for compact output.
 * @param[out] pszBuf    Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize    Size of pszBuf.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonFormat(HJSONNODE hNode, BOOL fIndent,
                           PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Serialize a node into a file or to stdout.
 *
 * When @p pszPath is NULL, the output is written to stdout.
 *
 * @param[in] hNode    Node handle. Not NULLHANDLE.
 * @param[in] fIndent  TRUE_ for pretty-printed output; FALSE_ for
 *                     compact output.
 * @param[in] pszPath  Output file path, or NULL for stdout.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_OPEN_FAILED        Cannot open output file.
 * @retval ERROR_READ_FAULT         Write error.
 */
APIRET APIENTRY JsonWriteFile(HJSONNODE hNode, BOOL fIndent,
                              PCSZ pszPath);

/**
 * @brief Escape a string for insertion into a JSON value.
 *
 * Applies the escape rules of RFC 8259 §7: quotation mark, reverse
 * solidus and control characters below 0x20 are escaped; the short
 * forms \b, \f, \n, \r, \t are used where available; remaining
 * control characters use \u00XX.
 *
 * Size-query convention as for JsonFormat.
 *
 * @param[in]  pszSrc   Source string. Not NULL. May be empty.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonEscapeString(PCSZ pszSrc,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* JSON_H */
