/** @file
 *  @brief This is the main header for the Humon C API. \#include this from your C code.
 **/

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include "version.h"

#if defined(_WIN32) || defined(_WIN64)		//	if we're on Windows
#if defined (HUMON_BUILDING_DLL)			//		if we're building the DLL
#define HUMON_PUBLIC __declspec(dllexport)
#elif defined(HUMON_USING_DLL)				//		else if we're building an application with the DLL's import lib
#define HUMON_PUBLIC __declspec(dllimport)
#else										//		else we're building an application with the static lib
#define HUMON_PUBLIC
#endif
#else										//	else we're on unix-like, using the static lib or .so
#define HUMON_PUBLIC
#endif

#if defined(_WIN32) || defined(_WIN64)
#if defined(_WIN64)
#define HUMON_ENV64BIT
#else
#define HUMON_ENV32BIT
#endif
#endif

#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__ppc64__)
#define HUMON_ENV64BIT
#else
#define HUMON_ENV32BIT
#endif
#endif

#define HU_NULLTOKEN        NULL
#define HU_NULLNODE         NULL
#define HU_NULLTROVE        NULL

#ifndef HUMON_ENUM_TYPE
#define HUMON_ENUM_TYPE     char
#endif
#ifndef HUMON_LINE_TYPE
#define HUMON_LINE_TYPE     long
#endif
#ifndef HUMON_COL_TYPE
#define HUMON_COL_TYPE      long
#endif

// For proper operation, this type must be signed.
#ifndef HUMON_SIZE_TYPE
#if defined(HUMON_ENV64BIT)
#define HUMON_SIZE_TYPE     long long
#else
#define HUMON_SIZE_TYPE     long
#endif
#endif

#ifdef __cplusplus
#ifdef HUMON_USENAMESPACE
/// If this file is #included from humon.hpp, we wrap the C API in a namespace
/// to avoid polluting global.
namespace hu { namespace capi {
#endif
extern "C"
{
#endif

    typedef HUMON_LINE_TYPE         huLine_t;
    typedef HUMON_COL_TYPE          huCol_t;
    typedef HUMON_SIZE_TYPE         huSize_t;

    /// Specifies the supported Unicode encodings.
    typedef enum huEncoding_tag
    {
        HU_ENCODING_UTF8,       ///< Specifies UTF-8 encoding.
        HU_ENCODING_UTF16_BE,   ///< Specifies UTF-16 big-endian encoding.
        HU_ENCODING_UTF16_LE,   ///< Specifies UTF-16 little-endian encoding.
        HU_ENCODING_UTF32_BE,   ///< Specifies UTF-32 big-endian encoding.
        HU_ENCODING_UTF32_LE,   ///< Specifies UTF-32 little-endian encoding.
        HU_ENCODING_UNKNOWN     ///< Specifies that the encoding is unknown.
    } huEncoding;

    /// Returns a string representation of a huTokenKind.
	HUMON_PUBLIC char const * huEncodingToString(huEncoding rhs);

    /// Specifies the kind of data represented by a particular huToken.
    typedef enum huTokenKind_tag
    {
        HU_TOKENKIND_NULL,          ///< Invalid token. Malformed, or otherwise nonexistent.
        HU_TOKENKIND_EOF,           ///< The end of the source text.
        HU_TOKENKIND_STARTLIST,     ///< The opening '[' of a list.
        HU_TOKENKIND_ENDLIST,       ///< The closing ']' of a list.
        HU_TOKENKIND_STARTDICT,     ///< The opening '{' of a dict.
        HU_TOKENKIND_ENDDICT,       ///< The closing '}' of a dict.
        HU_TOKENKIND_KEYVALUESEP,   ///< The separating ':' of a key-value pair.
        HU_TOKENKIND_METATAG,      ///< The metatag mark '@'.
        HU_TOKENKIND_WORD,          ///< Any key or value string, quoted or unquoted.
        HU_TOKENKIND_COMMENT        ///< Any comment token. An entire comment is considered one token.
    } huTokenKind;

    /// Returns a string representation of a huTokenKind.
	HUMON_PUBLIC char const * huTokenKindToString(huTokenKind rhs);

    /// Specifies the kind of node represented by a particular huNode.
    typedef enum huNodeKind_tag
    {
        HU_NODEKIND_NULL,   ///< Invalid node. An invalid address returns a null node.
        HU_NODEKIND_LIST,   ///< List node. The node contains a sequence of unassociated objects in maintained order.
        HU_NODEKIND_DICT,   ///< Dict node. The node contains a sequence of string-associated objects in maintained order.
        HU_NODEKIND_VALUE   ///< Value node. The node contains a string value, and no children.
    } huNodeKind;

    /// Returns a string representation of a huNodeKind.
	HUMON_PUBLIC char const * huNodeKindToString(huNodeKind rhs);

    /// Specifies the style of whitespacing in Humon text.
    typedef enum huWhitespaceFormat_tag
    {
        HU_WHITESPACEFORMAT_CLONED,     ///< Byte-for-byte copy of the original.
        HU_WHITESPACEFORMAT_MINIMAL,    ///< Reduces as much whitespace as possible.
        HU_WHITESPACEFORMAT_PRETTY      ///< Formats the text in a standard, human-friendly way.
    } huWhitespaceFormat;

    /// Returns a string representation of a huWhitespaceFormat.
	HUMON_PUBLIC char const * huWhitespaceFormatToString(huWhitespaceFormat rhs);

    /// Specifies a tokenizing or parsing error code, or lookup error.
    typedef enum huErrorCode_tag
    {
        HU_ERROR_NOERROR,                   ///< No error.
        HU_ERROR_BADENCODING,               ///< The Unicode encoding is malformed.
        HU_ERROR_UNFINISHEDQUOTE,           ///< The quoted text was not endquoted.
        HU_ERROR_UNFINISHEDCSTYLECOMMENT,   ///< The C-style comment was not closed.
        HU_ERROR_UNEXPECTEDEOF,             ///< The text ended early.
        HU_ERROR_TOOMANYROOTS,              ///< There is more than one root node detected.
        HU_ERROR_SYNTAXERROR,               ///< General syntax error.
        HU_ERROR_NOTFOUND,                  ///< No node could be found at the address.
        HU_ERROR_ILLEGAL,                   ///< The address or node was illegal.
        HU_ERROR_BADPARAMETER,              ///< An API parameter is malformed or illegal.
        HU_ERROR_BADFILE,                   ///< An attempt to open or operate on a file failed.
        HU_ERROR_OUTOFMEMORY,               ///< An internal memory allocation failed.
        HU_ERROR_TROVEHASERRORS             ///< The loading function succeeded, but the loaded trove has errors.
    } huErrorCode;

    /// Returns a string representation of a huErrorCode.
	HUMON_PUBLIC char const * huOutputErrorToString(huErrorCode rhs);

    /// Specifies how a trove responds to errors.
    typedef enum huErrorResponse_tag
    {
        HU_ERRORRESPONSE_MUM,               ///< Do not output errors to any stream.
        HU_ERRORRESPONSE_STDOUT,            ///< Report errors to stdout.
        HU_ERRORRESPONSE_STDERR,            ///< Report errors to stderr.
        HU_ERRORRESPONSE_STDOUTANSICOLOR,   ///< Report errors to stdout with ANSI color codes.
        HU_ERRORRESPONSE_STDERRANSICOLOR,   ///< Report errors to stderr with ANSI color codes.
        HU_ERRORRESPONSE_NUMRESPONSES       ///< One past the last.
    } huErrorResponse;

    /// Specifies a style ID for colorized printing.
    typedef enum huColorCode_tag
    {
        HU_COLORCODE_TOKENSTREAMBEGIN,          ///< Beginning-of-source text color code.
        HU_COLORCODE_TOKENSTREAMEND,            ///< End-of-source text color code.
        HU_COLORCODE_TOKENEND,                  ///< End-of-token color code.
        HU_COLORCODE_PUNCLIST,                  ///< List punctuation style. ([,])
        HU_COLORCODE_PUNCDICT,                  ///< Dict punctuation style. ({,})
        HU_COLORCODE_PUNCKEYVALUESEP,           ///< Key-value separator style. (:)
        HU_COLORCODE_PUNCMETATAG,               ///< Metatag mark style. (@)
        HU_COLORCODE_PUNCMETATAGDICT,           ///< Metatag dict punctuation style. ({,})
        HU_COLORCODE_PUNCMETATAGKEYVALUESEP,    ///< Metatag key-value separator style. (:)
        HU_COLORCODE_KEY,                       ///< Key style.
        HU_COLORCODE_VALUE,                     ///< Value style.
        HU_COLORCODE_COMMENT,                   ///< Comment style.
        HU_COLORCODE_METATAGKEY,                ///< Metatag key style.
        HU_COLORCODE_METATAGVALUE,              ///< Metatag value style.
        HU_COLORCODE_WHITESPACE,                ///< Whitespace style (including commas).
        HU_COLORCODE_NUMCOLORS                  ///< One past the last.
    } huColorCode;

    typedef enum huVectorKind_tag
    {
        HU_VECTORKIND_COUNTING,         ///< The vector is set up to count characters only.
        HU_VECTORKIND_PREALLOCATED,     ///< The vector is set with a preallocated, maximum buffer.
        HU_VECTORKIND_GROWABLE          ///< The vector is set up with an unbounded growable buffer.
    } huVectorKind;

    typedef enum huBufferManagement_tag
    {
        HU_BUFFERMANAGEMENT_COPYANDOWN, ///< The trove should copy the input buffer, and free the copy when destroyed.
        HU_BUFFERMANAGEMENT_MOVEANDOWN, ///< The trove should use the input buffer without copying it, and free it when destroyed.
        HU_BUFFERMANAGEMENT_MOVE        ///< The trove should use the input buffer without copying it, and do nothing to it when destroyed.
    } huBufferManagement;

    typedef void * (*huMemAlloc)(void * manager, size_t len);               ///< Custom memory allocator.
    typedef void * (*huMemRealloc)(void * allocator, void *, size_t len);   ///< Custom memory reallocator.
    typedef void (*huMemFree)(void * manager, void * alloc);                ///< Custom memory deallocator.

    /// Describes memory allocation context and functions that Humon can use in lieu of malloc/realloc/free.
    /** You can create and pass one of these to the deserialize functions, and
     * the trove will use them instead of malloc, realloc, and free from stdlib.h.
     * The allocator is an opaque pointer, which is simply passed to the memory
     * functions whenever they're called, and is otehrwise unused by Humon.*/
    typedef struct huAllocator_tag
    {
        void * manager;                             ///< Custom memory manager. This is passed as the first parameter to the custom memory functions.
        huMemAlloc memAlloc;                        ///< Custom memory allocator.
        huMemRealloc memRealloc;                    ///< Custom memory reallocator.
        huMemFree memFree;                          ///< Custom memory deallocator.
    } huAllocator;

    /// Describes and owns an array of memory elements.
    /** Owns and manages a buffer and operates it as an array, tracking the
     * size and capacity of the array, and the size of its elements. It's for
     * internal API storage, and thus there are no public functions to use it.*/
    typedef struct huVector_tag
    {
        huVectorKind kind;                          ///< The kind of vector this is. Determines growth and capacity behavior.
        huAllocator const * allocator;              ///< A custom memory allocator.
        char * buffer;                              ///< The owned buffer for the array.
        huSize_t elementSize;                       ///< The size of one element of the array. If set to 0, the vector does not manage memory.
        huSize_t numElements;                       ///< The number of elements currently managed by the array.
        huSize_t vectorCapacity;                    ///< The maximum capacity of the array.
    } huVector;

    /// Stores a pointer to a string, and its size. Does not own the string.
    /** Represents a view of a string. The string is unlikely to be NULL-terminated.
     * It is also not likely to point directly to an allocation, but to a substring. */
    typedef struct huStringView_tag
    {
        char const * ptr;     ///< The beginning of the string in memory.
        huSize_t size;        ///< The size of the string in bytes.
    } huStringView;

    /// Encodes a token read from Humon text.
    /** This structure encodes file location and buffer location information about a
     * particular token in a Humon file. Every token is read and tracked with a huToken. */
    typedef struct huToken_tag huToken;

	/// Gets the kind of a token.
	HUMON_PUBLIC huTokenKind huGetTokenKind(huToken const * token);

	/// Gets the full raw string of a token.
	HUMON_PUBLIC huStringView const * huGetRawString(huToken const * token);

	/// Gets the logical string of a token.
	HUMON_PUBLIC huStringView const * huGetString(huToken const * token);

	/// Gets the starting line number of a token.
	HUMON_PUBLIC huLine_t huGetLine(huToken const * token);

	/// Gets the starting column number of token.
	HUMON_PUBLIC huCol_t huGetColumn(huToken const * token);

	/// Gets the ending line number of a token.
	HUMON_PUBLIC huLine_t huGetEndLine(huToken const * token);

	/// Gets the ending column number of a token.
	HUMON_PUBLIC huCol_t huGetEndColumn(huToken const * token);

    /// Encodes a metatag entry for a node or trove.
    /** Nodes and troves can have a plurality of metatags. They are always key-value string
     * pairs, which are referenced by huMetatag objets. */
    typedef struct huMetatag_tag
    {
        huToken const * key;    ///< The metatag key token.
        huToken const * value;  ///< The metatag value token.
    } huMetatag;

    struct huNode_tag;

    /// Encodes a comment.
    /** A comment object includes the comment token, and the node to which it is associated. If
     * node is NULL, the comment is associated to the trove. */
    typedef struct huComment_tag
    {
        huToken const * token;             ///< The comment token.
        struct huNode_tag const * node;    ///< The associated node. NULL indicated association to a trove.
    } huComment;

    /// Encodes an error tracked by the tokenizer or parser.
    /** Errors are tracked for reporting to the user. */
    typedef struct huError_tag
    {
        huErrorCode errorCode;         ///< A huErrorCode value.
        huToken const * token;          ///< The token that seems to be erroneous.
        huLine_t line;                  ///< Location info for tokenizer errors.
        huCol_t col;                    ///< Location info for tokenizer errors.
    } huError;

    /// Encapsulates a selection of parameters to control how Humon interprets the input for loading.
    typedef struct huDeserializeOptions_tag
    {
        huEncoding encoding;                      ///< The Unicode encoding of the input. Can be `HU_ENCODING_UNKNOWN`.
        bool allowOutOfRangeCodePoints;             ///< Whether to check whether input code points are outside legal ranges.
        bool allowUtf16UnmatchedSurrogates;         ///< Whether to check whether UTF-16 input code points are unmatched surrogates.
        huCol_t tabSize;                            ///< The tab size to assume for the input, for the purposes of reporting token column data.
        huAllocator allocator;                      ///< A memory allocator.
        huBufferManagement bufferManagement;              ///< How to manage the input buffer, if it is a string. (One of huBufferManagement.)
    } huDeserializeOptions;

    /// Fill in a huDeserializeOptions struct quickly. You can pass NULL for the allocator, in which case stdlib will be used.
	HUMON_PUBLIC void huInitDeserializeOptions(huDeserializeOptions * params, huEncoding encoding,
		bool strictUnicode, huCol_t tabSize, huAllocator const * allocator, 
		huBufferManagement bufferManagement);

    /// Encapsulates a selection of parameters to control the serialization of a trove.
    typedef struct huSerializeOptions_tag
    {
        huWhitespaceFormat whitespaceFormat;      ///< The desired whitespace format of the output.
        huCol_t indentSize;                 ///< The number of spaces to indent if not using tabs.
        bool indentWithTabs;                ///< Whether to use `\t` for indentation.
        bool usingColors;                   ///< Whether to inject color codes into the output.
        huStringView const * colorTable;    ///< The color table to use, or NULL;
        bool printComments;                 ///< Whether to print or skip printing comments.
        huStringView newline;               ///< The string to use for newlines.
        huEncoding encoding;              ///< The Unicode encoding of the output.
        bool printBom;                      ///< Whether to print the UTF-8 BOM.
    } huSerializeOptions;

    struct huTrove_tag;

    /// Fill in a huSerializeOptions struct quickly.
	HUMON_PUBLIC void huInitSerializeOptionsZ(huSerializeOptions * params,
		huWhitespaceFormat whitespaceFormat, huCol_t indentSize, bool indentWithTabs,
        bool usingColors, huStringView const * colorTable,  bool printComments,
		char const * newline, huEncoding encoding, bool printBom);

    /// Fill in a huSerializeOptions struct quickly.
	HUMON_PUBLIC void huInitSerializeOptionsN(huSerializeOptions * params,
		huWhitespaceFormat whitespaceFormat, huCol_t indentSize, bool indentWithTabs,
        bool usingColors, huStringView const * colorTable,  bool printComments,
		char const * newline, huSize_t newlineSize, huEncoding encoding, bool printBom);

    /// Encodes a Humon data node.
    /** Humon nodes make up a hierarchical structure, stemming from a single root node.
     * Humon troves contain a reference to the root, and store all nodes in an indexable
     * array. A node is either a list, a dict, or a value node. Any number of comments
     * and metatags can be associated to a node. */
    typedef struct huNode_tag huNode;

	/// Gets the kind of a node.
	HUMON_PUBLIC huNodeKind huGetNodeKind(huNode const * node);

	/// Gets the index of a node in the trove's node list.
	HUMON_PUBLIC huSize_t huGetNodeIndex(huNode const * node);

	/// Gets the first token of a node.
	HUMON_PUBLIC huToken const * huGetFirstToken(huNode const * node);

	/// Gets the token containing the key in a dict entry node.
	HUMON_PUBLIC huToken const * huGetKeyToken(huNode const * node);

	/// Gets the token containing the value of a ndoe.
	HUMON_PUBLIC huToken const * huGetValueToken(huNode const * node);

	/// Gets the last token containing the value of a node.
	HUMON_PUBLIC huToken const * huGetLastValueToken(huNode const * node);

	/// Gets the last token of a node.
	HUMON_PUBLIC huToken const * huGetLastToken(huNode const * node);

	/// Gets the child index of a node.
	HUMON_PUBLIC huSize_t huGetChildIndex(huNode const * node);

    /// Gets a pointer to a node's parent.
	HUMON_PUBLIC huNode const * huGetParent(huNode const * node);
    /// Gets the number of children a node has.
	HUMON_PUBLIC huSize_t huGetNumChildren(huNode const * node);
    /// Gets a child of a node by child index.
	HUMON_PUBLIC huNode const * huGetChildByIndex(huNode const * node, huSize_t childIndex);
    /// Gets a child of a node by key.
	HUMON_PUBLIC huNode const * huGetChildByKeyZ(huNode const * node, char const * key);
    /// Gets a child of a node by key.
	HUMON_PUBLIC huNode const * huGetChildByKeyN(huNode const * node, char const * key,
											     huSize_t keyLen);
    /// Gets the first child of node (index 0).
	HUMON_PUBLIC huNode const * huGetFirstChild(huNode const * node);
    /// Returns the next sibling in the child index order of a node.
	HUMON_PUBLIC huNode const * huGetNextSibling(huNode const * node);
    /// Gets the first child of node with a specific key.
	HUMON_PUBLIC huNode const * huGetFirstChildWithKeyZ(huNode const * node, char const * key);
    /// Gets the first child of node with a specific key.
	HUMON_PUBLIC huNode const * huGetFirstChildWithKeyN(huNode const * node, char const * key,
		huSize_t keyLen);
    /// Returns the next sibling with a specific key in the child index order of a node.
	HUMON_PUBLIC huNode const * huGetNextSiblingWithKeyZ(huNode const * node, char const * key);
    /// Returns the next sibling with a specific key in the child index order of a node.
	HUMON_PUBLIC huNode const * huGetNextSiblingWithKeyN(huNode const * node, char const * key,
		huSize_t keyLen);

    /// Looks up a node by relative address to a node.
	HUMON_PUBLIC huNode const * huGetNodeByRelativeAddressZ(huNode const * node,
		char const * address);
    /// Looks up a node by relative address to a node.
	HUMON_PUBLIC huNode const * huGetNodeByRelativeAddressN(huNode const * node,
		char const * address, huSize_t addressLen);

    /// Gets the full address of a node, or the length of that address.
	HUMON_PUBLIC void huGetAddress(huNode const * node, char * address, huSize_t * addressLen);

    /// Returns whether a node has a key token tracked. (If it's a member of a dict.)
	HUMON_PUBLIC bool huHasKey(huNode const * node);

    /// Returns the key token for this node, or NULL if this node is not a child of a dict.
	HUMON_PUBLIC huToken const * huGetKey(huNode const * node);

	/// Returns the shaerd key index for this node.
	HUMON_PUBLIC huSize_t huGetSharedKeyIndex(huNode const * node);

    /// Returns the value token for this node.
	HUMON_PUBLIC huToken const * huGetValue(huNode const * node);

    /// Returns the entire nested text of a node, including child nodes and associated comments and metatags.
	HUMON_PUBLIC huStringView huGetSourceText(huNode const * node);

    /// Returns the number of metatags associated to a node.
	HUMON_PUBLIC huSize_t huGetNumMetatags(huNode const * node);
    /// Returns a metatag object associated to a node, by index.
	HUMON_PUBLIC huMetatag const * huGetMetatag(huNode const * node, huSize_t metatagIdx);

    /// Returns the number of metatags associated to a node with a specific value.
	HUMON_PUBLIC huSize_t huGetNumMetatagsWithKeyZ(huNode const * node, char const * key);
    /// Returns the number of metatags associated to a node with a specific value.
	HUMON_PUBLIC huSize_t huGetNumMetatagsWithKeyN(huNode const * node, char const * key,
		huSize_t valueLen);

    /// Returns the value token of a metatag object associated to a node, with the specified key.
	HUMON_PUBLIC huToken const * huGetMetatagWithKeyZ(huNode const * node, char const * key,
		huSize_t * cursor);
    /// Returns the value token of a metatag object associated to a node, with the specified key.
	HUMON_PUBLIC huToken const * huGetMetatagWithKeyN(huNode const * node, char const * key,
		huSize_t keyLen, huSize_t * cursor);

    /// Returns the number of metatags associated to a node with a specific value.
	HUMON_PUBLIC huSize_t huGetNumMetatagsWithValueZ(huNode const * node, char const * value);
    /// Returns the number of metatags associated to a node with a specific value.
	HUMON_PUBLIC huSize_t huGetNumMetatagsWithValueN(huNode const * node, char const * value,
		huSize_t valueLen);
    /// Returns the key token of a metatag associated to a node, with the specified value and index.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetMetatagWithValueZ(huNode const * node, char const * value,
		huSize_t * cursor);
    /// Returns the key token of a metatag associated to a node, with the specified value and index.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetMetatagWithValueN(huNode const * node, char const * value,
		huSize_t valueLen, huSize_t * cursor);

    /// Returns the number of comments associated to a node.
	HUMON_PUBLIC huSize_t huGetNumComments(huNode const * node);
    /// Returns a comment token associated to a node, by index.
	HUMON_PUBLIC huToken const * huGetComment(huNode const * node, huSize_t commentIdx);
    /// Returns whether any comment tokens associated to a node contain the specified substring.
	HUMON_PUBLIC bool huHasCommentsContainingZ(huNode const * node, char const * containedText);
    /// Returns whether any comment tokens associated to a node contain the specified substring.
	HUMON_PUBLIC bool huHasCommentsContainingN(huNode const * node, char const * containedText,
		huSize_t containedTextLen);
    /// Returns the number of comment tokens associated to a node what contain the specified substring.
	HUMON_PUBLIC huSize_t huGetNumCommentsContainingZ(huNode const * node,
	    char const * containedText);
    /// Returns the number of comment tokens associated to a node what contain the specified substring.
	HUMON_PUBLIC huSize_t huGetNumCommentsContainingN(huNode const * node,
	    char const * containedText, huSize_t containedTextLen);

    /// Returns each comment token associated to a node which contain the specified substring.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetCommentsContainingZ(huNode const * node,
	    char const * containedText, huSize_t * cursor);

    /// Returns each comment token associated to a node which contain the specified substring.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetCommentsContainingN(huNode const * node,
	    char const * containedText, huSize_t containedTextLen, huSize_t * cursor);

    typedef enum huChangeKind_tag
    {
        HU_CHANGEKIND_REPLACE,
        HU_CHANGEKIND_INSERT,
        HU_CHANGEKIND_NUMKINDS
    } huChangeKind;

    /// Encodes a Humon data trove.
    /** A trove stores all the tokens and nodes in a loaded Humon file. It is your main access
     * to the Humon object data. Troves are created by Humon functions that load from file or
     * string, and can output Humon to file or string as well. */
    typedef struct huTrove_tag huTrove;

    /// Creates a trove from a NULL-terminated string of Humon text.
	HUMON_PUBLIC huErrorCode huDeserializeTroveZ(huTrove ** trove, char const * data,
		huDeserializeOptions * deserializeOptions, huErrorResponse errorResponse);
    /// Creates a trove from a string view of Humon text.
	HUMON_PUBLIC huErrorCode huDeserializeTroveN(huTrove ** trove, char const * data,
		huSize_t dataLen, huDeserializeOptions * deserializeOptions, huErrorResponse errorResponse);
    /// Creates a trove from a file.
	HUMON_PUBLIC huErrorCode huDeserializeTroveFromFile(huTrove ** trove, char const * path,
	    huDeserializeOptions * deserializeOptions, huErrorResponse errorResponse);

    /// Reclaims all memory owned by a trove.
	HUMON_PUBLIC void huDestroyTrove(huTrove * trove);

	/// Gets the allocator owned by this trove.
	HUMON_PUBLIC huAllocator const * huGetAllocator(huTrove const * trove);

    /// Returns the number of tokens in a trove.
	HUMON_PUBLIC huSize_t huGetNumTokens(huTrove const * trove);
    /// Returns a token from a trove by index.
	HUMON_PUBLIC huToken const * huGetToken(huTrove const * trove, huSize_t tokenIdx);

    /// Returns the number of nodes in a trove.
	HUMON_PUBLIC huSize_t huGetNumNodes(huTrove const * trove);
    /// Returns the root node of a trove, if any.
	HUMON_PUBLIC huNode const * huGetRootNode(huTrove const * trove);
    /// Returns a node from a trove by index.
	HUMON_PUBLIC huNode const * huGetNodeByIndex(huTrove const * trove, huSize_t nodeIdx);

    /// Returns a node by its full address.
	HUMON_PUBLIC huNode const * huGetNodeByAddressZ(huTrove const * trove, char const * address);
    /// Returns a node by its full address.
	HUMON_PUBLIC huNode const * huGetNodeByAddressN(huTrove const * trove, char const * address,
	    huSize_t addressLen);

    /// Returns the number of errors encountered when loading a trove.
	HUMON_PUBLIC huSize_t huGetNumErrors(huTrove const * trove);
    /// Returns an error from a trove by index.
	HUMON_PUBLIC huError const * huGetError(huTrove const * trove, huSize_t errorIdx);

    /// Returns the number of metatags associated to a trove.
	HUMON_PUBLIC huSize_t huGetNumTroveMetatags(huTrove const * trove);
    /// Returns a metatag from a trove by index.
	HUMON_PUBLIC huMetatag const * huGetTroveMetatag(huTrove const * trove,
		huSize_t metatagIdx);

    /// Returns the number of metatags associated to a trove with a specific value.
	HUMON_PUBLIC huSize_t huGetNumTroveMetatagsWithKeyZ(huTrove const * trove, char const * key);
    /// Returns the number of metatags associated to a trove with a specific value.
	HUMON_PUBLIC huSize_t huGetNumTroveMetatagsWithKeyN(huTrove const * trove, char const * key,
		huSize_t keyLen);
    /// Returns a metatag value associated to a trove, by key and index.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetTroveMetatagWithKeyZ(huTrove const * trove,
	    char const * key, huSize_t * cursor);
    /// Returns a metatag value associated to a trove, by key and index.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetTroveMetatagWithKeyN(huTrove const * trove,
	    char const * key, huSize_t keyLen, huSize_t * cursor);

    /// Returns the number of metatags associated to a trove with a specific value.
	HUMON_PUBLIC huSize_t huGetNumTroveMetatagsWithValueZ(huTrove const * trove,
	    char const * value);
    /// Returns the number of metatags associated to a trove with a specific value.
	HUMON_PUBLIC huSize_t huGetNumTroveMetatagsWithValueN(huTrove const * trove,
	    char const * value, huSize_t valueLen);
    /// Returns a metatag key associated to a trove, by value and index.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetTroveMetatagWithValueZ(huTrove const * trove,
		char const * value, huSize_t * cursor);
    /// Returns a metatag key associated to a trove, by value and index.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huToken const * huGetTroveMetatagWithValueN(huTrove const * trove,
		char const * value, huSize_t valueLen, huSize_t * cursor);

    /// Returns the number of comments associated to a trove.
	HUMON_PUBLIC huSize_t huGetNumTroveComments(huTrove const * trove);
    /// Returns a comment associated to a trove by index.
	HUMON_PUBLIC huToken const * huGetTroveComment(huTrove const * trove, huSize_t commentIdx);

    /// Returns a collection of all nodes in a trove with a specific metatag key.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagKeyZ(huTrove const * trove,
		char const * key, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a specific metatag key.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagKeyN(huTrove const * trove,
		char const * key, huSize_t keyLen, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a specific metatag value.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagValueZ(huTrove const * trove,
		char const * value, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a specific metatag value.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagValueN(huTrove const * trove,
		char const * value, huSize_t valueLen, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a specific metatag key and value.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagKeyValueZZ(huTrove const * trove,
		char const * key, char const * value, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a specific metatag key and value.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagKeyValueNZ(huTrove const * trove,
		char const * key, huSize_t keyLen, char const * value, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a specific metatag key and value.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagKeyValueZN(huTrove const * trove,
		char const * key, char const * value, huSize_t valueLen, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a specific metatag key and value.
    /** Call this function continually to iterate over all the metatags. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesWithMetatagKeyValueNN(huTrove const * trove,
		char const * key, huSize_t keyLen, char const * value, huSize_t valueLen,
		huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a comment which contains specific text.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesByCommentContainingZ(huTrove const * trove,
		char const * containedText, huSize_t * cursor);
    /// Returns a collection of all nodes in a trove with a comment which contains specific text.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
	HUMON_PUBLIC huNode const * huFindNodesByCommentContainingN(huTrove const * trove,
		char const * containedText, huSize_t containedTextLen, huSize_t * cursor);

    /// Returns the entire source text of a trove, including all nodes and all comments and metatags.
    /** This function returns the stored text as a view. It does not allocate or copy memory,
     * and cannot format the string.*/
	HUMON_PUBLIC huStringView huGetTroveSourceText(huTrove const * trove);

    /// Serializes a trove to text.
    /** This function makes a new copy of the token steram, optionally with formatting options.*/
	HUMON_PUBLIC huErrorCode huSerializeTrove(huTrove const * trove, char * dest,
		huSize_t * destLength, huSerializeOptions * serializeOptions);
    /// Serializes a trove to file.
    /** This function stores a new copy of the token steram to file, optionally with formatting options.*/
	HUMON_PUBLIC huErrorCode huSerializeTroveToFile(huTrove const * trove, char const * path,
		huSize_t * destLength, huSerializeOptions * serializeOptions);

    /// Fills an array of HU_COLORCODE_NUMCOLORS huStringViews with ANSI terminal color codes for printing to console.
	HUMON_PUBLIC void huFillAnsiColorTable(huStringView table[]);

    /// Change entry for making modified troves.
    typedef struct huChange_tag
    {
        huChangeKind changeKind;
        huSize_t nodeIdx;
        huSize_t childIdx;
        huStringView newString;
    } huChange;

    typedef struct huChangeSet_tag
    {
        huVector changes;               ///< Manages a huChange[]. Contains pending changes to the trove.
        huVector changeStrings;         ///< manages a char[]. Contains pending change strings.
    } huChangeSet;

    HUMON_PUBLIC void huInitChangeSet(huChangeSet * changeSet, huAllocator const * allocator);
    HUMON_PUBLIC void huDestroyChangeSet(huChangeSet * changeSet);

    /// Record a node replacement / removal.
	HUMON_PUBLIC huSize_t huReplaceNodeZ(huChangeSet * changeSet, huNode const * node,
		char const * newString);
    /// Record a node replacement / removal.
	HUMON_PUBLIC huSize_t huReplaceNodeN(huChangeSet * changeSet, huNode const * node,
		char const * newString, huSize_t newStringLength);

    /// Record appending to a list or dict.
	HUMON_PUBLIC huSize_t huAppendZ(huChangeSet * changeSet, huNode const * node,
		char const * newString);
    /// Record appending to a list or dict.
	HUMON_PUBLIC huSize_t huAppendN(huChangeSet * changeSet, huNode const * node,
		char const * newString, huSize_t newStringLength);

    /// Record inserting into a list or dict.
	HUMON_PUBLIC huSize_t huInsertAtIndexZ(huChangeSet * changeSet, huNode const * node,
		huSize_t idx, char const * newString);
    /// Record inserting into a list or dict.
	HUMON_PUBLIC huSize_t huInsertAtIndexN(huChangeSet * changeSet, huNode const * node,
		huSize_t idx, char const * newString, huSize_t newStringLength);

    /// Create a new trove, using recorded changes.
    HUMON_PUBLIC huErrorCode huMakeChangedTrove(huTrove ** newTrove, huTrove const * srcTrove,
		huChangeSet * changeSet);

#ifdef __cplusplus
} // extern "C"
#ifdef HUMON_USENAMESPACE
}} // hu::capi::
#endif
#endif
