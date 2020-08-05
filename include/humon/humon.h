#pragma once

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    #define HU_NULLTOKEN    (NULL)
    #define HU_NULLNODE     (NULL)
    #define HU_NULLTROVE    (NULL)

    /// Specifies the supported Unicode encodings.
    enum huEncoding
    {
        HU_ENCODING_UTF8,       ///< Specifies UTF-8 encoding.
        HU_ENCODING_UTF16_BE,   ///< Specifies UTF-16 big-endian encoding.
        HU_ENCODING_UTF16_LE,   ///< Specifies UTF-16 little-endian encoding.
        HU_ENCODING_UTF32_BE,   ///< Specifies UTF-32 big-endian encoding.
        HU_ENCODING_UTF32_LE,   ///< Specifies UTF-32 little-endian encoding.
        HU_ENCODING_UNKNOWN     ///< Specifies that the encoding is unknown.
    };

    /// Returns a string representation of a huTokenKind.
    char const * huEncodingToString(int rhs);

    /// Specifies the kind of data represented by a particular huToken.
    enum huTokenKind
    {
        HU_TOKENKIND_NULL,          ///< Invalid token. Malformed, or otherwise nonexistent.
        HU_TOKENKIND_EOF,           ///< The end of the token stream or string.
        HU_TOKENKIND_STARTLIST,     ///< The opening '[' of a list.
        HU_TOKENKIND_ENDLIST,       ///< The closing ']' of a list.
        HU_TOKENKIND_STARTDICT,     ///< The opening '{' of a dict.
        HU_TOKENKIND_ENDDICT,       ///< The closing '}' of a dict.
        HU_TOKENKIND_KEYVALUESEP,   ///< The separating ':' of a key-value pair.
        HU_TOKENKIND_ANNOTATE,      ///< The annotation mark '@'.
        HU_TOKENKIND_WORD,          ///< Any key or value string, quoted or unquoted.
        HU_TOKENKIND_COMMENT        ///< Any comment token. An entire comment is considered one token.
    };

    /// Returns a string representation of a huTokenKind.
    char const * huTokenKindToString(int rhs);

    /// Specifies the kind of node represented by a particular huNode.
    enum huNodeKind
    {
        HU_NODEKIND_NULL,   ///< Invalid node. An invalid address returns a null node.
        HU_NODEKIND_LIST,   ///< List node. The node contains a sequence of unassociated objects in maintained order.
        HU_NODEKIND_DICT,   ///< Dict node. The node contains a sequence of string-associated objects in maintained order.
        HU_NODEKIND_VALUE   ///< Value node. The node contains a string value, and no children.
    };

    /// Returns a string representation of a huNodeKind.
    char const * huNodeKindToString(int rhs);

    /// Specifies the style of whitespacing in Humon text.
    enum huWhitespaceFormat
    {
        HU_WHITESPACEFORMAT_CLONED,     ///< Byte-for-byte copy of the original.
        HU_WHITESPACEFORMAT_MINIMAL,    ///< Reduces as much whitespace as possible.
        HU_WHITESPACEFORMAT_PRETTY      ///< Formats the text in a standard, human-friendly way.
    };

    /// Returns a string representation of a huWhitespaceFormat.
   char const * huWhitespaceFormatToString(int rhs);

    /// Specifies a tokenizing or parsing error code, or lookup error.
    enum huErrorCode
    {
        HU_ERROR_NOERROR,                   ///< No error.
        HU_ERROR_BADENCODING,               ///< The Unicode encoding is malformed.
        HU_ERROR_UNFINISHEDQUOTE,           ///< The quoted text was not endquoted.
        HU_ERROR_UNFINISHEDCSTYLECOMMENT,   ///< The C-style comment was not closed.
        HU_ERROR_UNEXPECTEDEOF,             ///< The text ended early.
        HU_ERROR_TOOMANYROOTS,              ///< There is more than one root node detected.
        HU_ERROR_NONUNIQUEKEY,              ///< A non-unique key was encountered in a dict or annotation.
        HU_ERROR_SYNTAXERROR,               ///< General syntax error.
        HU_ERROR_NOTFOUND,                  ///< No node could be found at the address.
        HU_ERROR_ILLEGAL,                   ///< The address or node was illegal.
        HU_ERROR_BADPARAMETER,              ///< An API parameter is malformed or illegal.
        HU_ERROR_BADFILE,                   ///< An attempt to open or operate on a file failed.
        HU_ERROR_OUTOFMEMORY,               ///< An internal memory allocation failed.
        HU_ERROR_TROVEHASERRORS             ///< The loading function succeeded, but the loaded trove has errors.
    };

    /// Returns a string representation of a huErrorCode.
    char const * huOutputErrorToString(int rhs);

    /// Specifies how a trove responds to errors.
    enum huErrorResponse
    {
        HU_ERRORRESPONSE_MUM,               ///< Do not output errors to any stream.
        HU_ERRORRESPONSE_STDOUT,            ///< Report errors to stdout.
        HU_ERRORRESPONSE_STDERR,            ///< Report errors to stderr.
        HU_ERRORRESPONSE_STDOUTANSICOLOR,   ///< Report errors to stdout with ANSI color codes.
        HU_ERRORRESPONSE_STDERRANSICOLOR,   ///< Report errors to stderr with ANSI color codes.

        HU_ERRORRESPONSE_NUMRESPONSES
    };

    /// Specifies a style ID for colorized printing.
    enum huColorCode
    {
        HU_COLORCODE_TOKENSTREAMBEGIN,          ///< Beginning-of-token stream color code.
        HU_COLORCODE_TOKENSTREAMEND,            ///< End-of-token stream color code.
        HU_COLORCODE_TOKENEND,                  ///< End-of-token color code.
        HU_COLORCODE_PUNCLIST,                  ///< List punctuation style. ([,]) 
        HU_COLORCODE_PUNCDICT,                  ///< Dict punctuation style. ({,})
        HU_COLORCODE_PUNCKEYVALUESEP,           ///< Key-value separator style. (:)
        HU_COLORCODE_PUNCANNOTATE,              ///< Annotation mark style. (@)
        HU_COLORCODE_PUNCANNOTATEDICT,          ///< Annotation dict punctuation style. ({,})
        HU_COLORCODE_PUNCANNOTATEKEYVALUESEP,   ///< Annotation key-value separator style. (:)
        HU_COLORCODE_KEY,                       ///< Key style.
        HU_COLORCODE_VALUE,                     ///< Value style.
        HU_COLORCODE_COMMENT,                   ///< Comment style.
        HU_COLORCODE_ANNOKEY,                   ///< Annotation key style.
        HU_COLORCODE_ANNOVALUE,                 ///< Annotation value style.
        HU_COLORCODE_WHITESPACE,                ///< Whitespace style (including commas).
        HU_COLORCODE_NUMCOLORS                  ///< One past the last style code.
    };

    enum huVectorKind
    {
        HU_VECTORKIND_COUNTING,         ///< The vector is set up to count characters only.
        HU_VECTORKIND_PREALLOCATED,     ///< The vector is set with a preallocated, maximum buffer.
        HU_VECTORKIND_GROWABLE          ///< The vector is set up with an unbounded growable buffer.
    };

    /// Describes and owns an array of memory elements.
    /** Wraps a buffer and operates it as an array, tracking
     * the size and capacity of the array, and the size of its
     * elements. Several Humon functions return huVectors, which 
     * own allocated heap memory to represent an array of
     * arbitrary-sized objects. No functions take them as 
     * parameters, and there is no public functions that
     * initialize a huVector. The huVector object always owns
     * its memory buffer, so when one goes out of scope, the
     * user should free() the buffer pointer.
     * If the elementSize is set to 0, the vector does not track
     * memory in `buffer`; it remains set to NULL.*/
    typedef struct huVector_tag
    {
        int kind;               ///< The kind of vector this is. Determines growth and capacity behavior.
        char * buffer;          ///< The owned buffer for the array.
        int elementSize;        ///< The size of one element of the array. If set to 0, the vector does not manage memory.
        int numElements;        ///< The number of elements currently managed by the array.
        int vectorCapacity;     ///< The maximum capacity of the array.
    } huVector;

    /// Stores a pointer to a string, and its size. Does not own the string.
    /** Represents a view of a string. The string is unlikely to be NULL-terminated.
     * It is also not likely to point directly to an allocation, but to a substring. */
    typedef struct huStringView_tag
    {
        char const * ptr;       ///< The beginning of the string in memory.
        int size;               ///< The size of the string in bytes.
    } huStringView;

    /// Encodes a token read from Humon text.
    /** This structure encodes file location and buffer location information about a
     * particular token in a Humon file. Every token is read and tracked with a huToken. */
    typedef struct huToken_tag
    {
        short kind;             ///< The kind of token this is (huTokenKind).
        char quoteChar;         ///< Whether the token is a quoted string.
        huStringView str;       ///< A view of the token string.
        int line;               ///< The line number in the file where the token begins.
        int col;                ///< The column number in the file where the token begins.
        int endLine;            ///< The line number in the file where the token ends.
        int endCol;             ///< the column number in the file where the token end.
    } huToken;

    /// Encodes an annotation entry for a node or trove.
    /** Nodes and troves can have a plurality of annotations. They are always key-value string
     * pairs, which are referenced by huAnnotation objets. */
    typedef struct huAnnotation_tag
    {
        huToken const * key;    ///< The annotation key token.
        huToken const * value;  ///< The annotation value token.
    } huAnnotation;

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
        int errorCode;                  ///< A huErrorCode value.
        huToken const * token;          ///< The token that seems to be erroneous.
        int line;                       ///< Location info for tokenizer errors.
        int col;                        ///< Location info for tokenizer errors.
    } huError;

    /// Encapsulates a selection of parameters to control how Humon interprets the input for loading.
    typedef struct huDeserializeOptions_tag
    {
        int encoding;
        bool allowOutOfRangeCodePoints;
        bool allowUtf16UnmatchedSurrogates;
        int tabSize;
    } huDeserializeOptions;

    /// Fill in a huDeserializeOptions struct quickly.
    void huInitDeserializeOptions(huDeserializeOptions * params, int encoding, bool strictUnicode, int tabSize);

    /// Encapsulates a selection of parameters to control the serialization of a trove.
    typedef struct huSerializeOptions_tag
    {
        int WhitespaceFormat;
        int indentSize;
        bool indentWithTabs;
        bool usingColors;
        huStringView const * colorTable;
        bool printComments;
        huStringView newline;
        bool printBom;
    } huSerializeOptions;

    struct huTrove_tag;

    /// Fill in a huSerializeOptions struct quickly.
    void huInitSerializeOptionsZ(huSerializeOptions * params, int WhitespaceFormat, int indentSize, bool indentWithTabs, 
        bool usingColors, huStringView const * colorTable,  bool printComments, char const * newline, bool printBom);
    /// Fill in a huSerializeOptions struct quickly.
    void huInitSerializeOptionsN(huSerializeOptions * params, int WhitespaceFormat, int indentSize, bool indentWithTabs, 
        bool usingColors, huStringView const * colorTable,  bool printComments, char const * newline, int newlineSize, bool printBom);

    /// Encodes a Humon data node.
    /** Humon nodes make up a hierarchical structure, stemming from a single root node.
     * Humon troves contain a reference to the root, and store all nodes in an indexable
     * array. A node is either a list, a dict, or a value node. Any number of comments 
     * and annotations can be associated to a node. */
    typedef struct huNode_tag
    {
        struct huTrove_tag const * trove;   ///< The trove tracking this node.
        int nodeIdx;                        ///< The index of this node in its trove's tracking array.
        int kind;                           ///< A huNodeKind value.
        huToken const * firstToken;         ///< The first token which contributes to this node, including any annotation and comment tokens.
        huToken const * keyToken;           ///< The key token if the node is inside a dict.
        huToken const * valueToken;         ///< The first token of this node's actual value; for a container, it points to the opening brac(e|ket).
        huToken const * lastValueToken;     ///< The last token of this node's actual value; for a container, it points to the closing brac(e|ket).
        huToken const * lastToken;          ///< The last token of this node, including any annotation and comment tokens.

        int parentNodeIdx;                  ///< The parent node's index, or -1 if this node is the root.
        int childOrdinal;                   ///< The index of this node vis a vis its sibling nodes (starting at 0).

        huVector childNodeIdxs;             ///< Manages a int []. Stores the node inexes of each child node, if this node is a collection.
        huVector annotations;               ///< Manages a huAnnotation []. Stores the annotations associated to this node.
        huVector comments;                  ///< Manages a huComment []. Stores the comments associated to this node.
    } huNode;

    /// Gets a pointer to a node's parent.
    huNode const * huGetParent(huNode const * node);
    /// Gets the number of children a node has.
    int huGetNumChildren(huNode const * node);
    /// Gets a child of a node by child index.
    huNode const * huGetChildByIndex(huNode const * node, int childOrdinal);
    /// Gets a child of a node by key.
    huNode const * huGetChildByKeyZ(huNode const * node, char const * key);
    /// Gets a child of a node by key.
    huNode const * huGetChildByKeyN(huNode const * node, char const * key, int keyLen);
    /// Gets the first child of node (index 0).
    huNode const * huGetFirstChild(huNode const * node);
    /// Returns the next sibling in the child index order of a node.
    huNode const * huGetNextSibling(huNode const * node);

    /// Looks up a node by relative address to a node.    
    huNode const * huGetRelativeZ(huNode const * node, char const * address);
    /// Looks up a node by relative address to a node.    
    huNode const * huGetRelativeN(huNode const * node, char const * address, int addressLen);

    /// Gets the full address of a node, or the length of that address.
    void huGetAddress(huNode const * node, char * address, int * addressLen);

    /// Returns whether a node has a key token tracked. (If it's a member of a dict.)
    bool huHasKey(huNode const * node);

    /// Returns the entire nested text of a node, including child nodes and associated comments and annotations.
    huStringView huGetTokenStream(huNode const * node);

    /// Returns the number of annotations associated to a node.
    int huGetNumAnnotations(huNode const * node);
    /// Returns an annotation object associated to a node, by index.
    huAnnotation const * huGetAnnotation(huNode const * node, int annotationIdx);

    /// Returns whether there is an annotation associated to a node with a specific key.
    bool huHasAnnotationWithKeyZ(huNode const * node, char const * key);
    /// Returns whether there is an annotation associated to a node with a specific key.
    bool huHasAnnotationWithKeyN(huNode const * node, char const * key, int keyLen);
    /// Returns the value token of an annoation object associated to a node, with the specified key.
    huToken const * huGetAnnotationWithKeyZ(huNode const * node, char const * key);
    /// Returns the value token of an annoation object associated to a node, with the specified key.
    huToken const * huGetAnnotationWithKeyN(huNode const * node, char const * key, int keyLen);

    /// Returns the number of annotations associated to a node with a specific value.
    int huGetNumAnnotationsWithValueZ(huNode const * node, char const * value);
    /// Returns the number of annotations associated to a node with a specific value.
    int huGetNumAnnotationsWithValueN(huNode const * node, char const * value, int valueLen);
    /// Returns the key token of an annotation associated to a node, with the specified value and index.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huToken const * huGetAnnotationWithValueZ(huNode const * node, char const * value, int * cursor);
    /// Returns the key token of an annotation associated to a node, with the specified value and index.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huToken const * huGetAnnotationWithValueN(huNode const * node, char const * value, int valueLen, int * cursor);

    /// Returns the number of comments associated to a node.
    int huGetNumComments(huNode const * node);
    /// Returns a comment token associated to a node, by index.
    huToken const * huGetComment(huNode const * node, int commentIdx);
    /// Returns whether any comment tokens associated to a node contain the specified substring.
    bool huHasCommentsContainingZ(huNode const * node, char const * containedText);
    /// Returns whether any comment tokens associated to a node contain the specified substring.
    bool huHasCommentsContainingN(huNode const * node, char const * containedText, int containedTextLen);
    /// Returns the number of comment tokens associated to a node what contain the specified substring.
    int huGetNumCommentsContainingZ(huNode const * node, char const * containedText);
    /// Returns the number of comment tokens associated to a node what contain the specified substring.
    int huGetNumCommentsContainingN(huNode const * node, char const * containedText, int containedTextLen);
    
    /// Returns each comment token associated to a node which contain the specified substring.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huToken const * huGetCommentsContainingZ(huNode const * node, char const * containedText, int * cursor);
    
    /// Returns each comment token associated to a node which contain the specified substring.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huToken const * huGetCommentsContainingN(huNode const * node, char const * containedText, int containedTextLen, int * cursor);

    /// Encodes a Humon data trove.
    /** A trove stores all the tokens and nodes in a loaded Humon file. It is your main access
     * to the Humon object data. Troves are created by Humon functions that load from file or 
     * string, and can output Humon to file or string as well. */
    typedef struct huTrove_tag
    {
        int encoding;               ///< The input Unicode encoding for loads.
        char const * dataString;    ///< The buffer containing the Humon text as loaded. Owned by the trove. Humon takes care to NULL-terminate this string.
        int dataStringSize;         ///< The size of the buffer.
        huVector tokens;            ///< Manages a huToken []. This is the array of tokens lexed from the Humon text.
        huVector nodes;             ///< Manages a huNode []. This is the array of node objects parsed from tokens.
        huVector errors;            ///< Manages a huError []. This is an array of errors encountered during load.
        int errorResponse;          ///< How the trove respones to errors during load.
        int inputTabSize;           ///< The tab length Humon uses to compute column values for tokens.
        huVector annotations;       ///< Manages a huAnnotation []. Contains the annotations associated to the trove.
        huVector comments;          ///< Manages a huComment[]. Contains the comments associated to the trove.
        huToken const * lastAnnoToken;    ///< Token referencing the last token of any trove annotations.
    } huTrove;

    /// Creates a trove from a NULL-terminated string of Humon text.
    int huDeserializeTroveZ(huTrove const ** trove, char const * data, huDeserializeOptions * DeserializeOptions, int errorResponse);
    /// Creates a trove from a string view of Humon text.
    int huDeserializeTroveN(huTrove const ** trove, char const * data, int dataLen, huDeserializeOptions * DeserializeOptions, int errorResponse);
    /// Creates a trove from a file.
    int huDeserializeTroveFromFileZ(huTrove const ** trove, char const * path, huDeserializeOptions * DeserializeOptions, int errorResponse);
    /// Creates a trove from a file.
    int huDeserializeTroveFromFileN(huTrove const ** trove, char const * path, int pathLen, huDeserializeOptions * DeserializeOptions, int errorResponse);

    /// Reclaims all memory owned by a trove.
    void huDestroyTrove(huTrove const * trove);

    /// Returns the number of tokens in a trove.
    int huGetNumTokens(huTrove const * trove);
    /// Returns a token from a trove by index.
    huToken const * huGetToken(huTrove const * trove, int tokenIdx);

    /// Returns the number of nodes in a trove.
    int huGetNumNodes(huTrove const * trove);
    /// Returns the root node of a trove, if any.
    huNode const * huGetRootNode(huTrove const * trove);
    /// Returns a node from a trove by index.
    huNode const * huGetNodeByIndex(huTrove const * trove, int nodeIdx);

    /// Returns a node by its full address.
    huNode const * huGetNodeByAddressZ(huTrove const * trove, char const * address);
    /// Returns a node by its full address.
    huNode const * huGetNodeByAddressN(huTrove const * trove, char const * address, int addressLen);

    /// Returns the number of errors encountered when loading a trove.
    int huGetNumErrors(huTrove const * trove);
    /// Returns an error from a trove by index.
    huError const * huGetError(huTrove const * trove, int errorIdx);

    /// Returns the number of annotations associated to a trove.
    int huGetNumTroveAnnotations(huTrove const * trove);
    /// Returns an annotation from a trove by index.
    huAnnotation const * huGetTroveAnnotation(huTrove const * trove, int annotationIdx);

    /// Returns whether any annotations are associated to a trove with a specific key.
    bool huTroveHasAnnotationWithKeyZ(huTrove const * trove, char const * key);
    /// Returns whether any annotations are associated to a trove with a specific key.
    bool huTroveHasAnnotationWithKeyN(huTrove const * trove, char const * key, int keyLen);
    /// Returns an annoation object associated to a trove, by key and index.
    huToken const * huGetTroveAnnotationWithKeyZ(huTrove const * trove, char const * key);
    /// Returns an annoation object associated to a trove, by key and index.
    huToken const * huGetTroveAnnotationWithKeyN(huTrove const * trove, char const * key, int keyLen);

    /// Returns the number of annotations associated to a trove with a specific value.
    int huGetNumTroveAnnotationsWithValueZ(huTrove const * trove, char const * value);
    /// Returns the number of annotations associated to a trove with a specific value.
    int huGetNumTroveAnnotationsWithValueN(huTrove const * trove, char const * value, int valueLen);
    /// Returns an annoation object associated to a trove, by value and index.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huToken const * huGetTroveAnnotationWithValueZ(huTrove const * trove, char const * value, int * cursor);
    /// Returns an annoation object associated to a trove, by value and index.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huToken const * huGetTroveAnnotationWithValueN(huTrove const * trove, char const * value, int valueLen, int * cursor);

    /// Returns the number of comments associated to a trove.
    int huGetNumTroveComments(huTrove const * trove);
    /// Returns a comment associated to a trove by index.
    huToken const * huGetTroveComment(huTrove const * trove, int commentIdx);

    /// Returns a collection of all nodes in a trove with a specific annotation key.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationKeyZ(huTrove const * trove, char const * key, int * cursor);
    /// Returns a collection of all nodes in a trove with a specific annotation key.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationKeyN(huTrove const * trove, char const * key, int keyLen, int * cursor);
    /// Returns a collection of all nodes in a trove with a specific annotation value.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationValueZ(huTrove const * trove, char const * value, int * cursor);
    /// Returns a collection of all nodes in a trove with a specific annotation value.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationValueN(huTrove const * trove, char const * value, int valueLen, int * cursor);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value, int * cursor);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationKeyValueNZ(huTrove const * trove, char const * key, int keyLen, char const * value, int * cursor);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen, int * cursor);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    /** Call this function continually to iterate over all the annotations. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesWithAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen, int * cursor);
    /// Returns a collection of all nodes in a trove with a comment which contains specific text.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText, int * cursor);
    /// Returns a collection of all nodes in a trove with a comment which contains specific text.
    /** Call this function continually to iterate over all the comments. For `cursor`, be sure 
     * to pass the address of an integer whose value is 0 for the first call; subsequent calls 
     * must use the same integer for `cursor`; the value is otherwise opaque, and has no meaning
     * to the caller.*/
    huNode const * huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, int containedTextLen, int * cursor);

    /// Returns the entire token stream of a trove (its text), including all nodes and all comments and annotations.
    /** This function returns the stored text as a view. It does not allocate or copy memory, 
     * and cannot format the string.*/
    huStringView huGetTroveTokenStream(huTrove const * trove);

    /// Serializes a trove to text.
    /** This function makes a new copy of the token steram, optionally with formatting options.*/
    int huSerializeTrove(huTrove const * trove, char * dest, int * destLength, huSerializeOptions * SerializeOptions);

    /// Serializes a trove to file.
    /** This function stores a new copy of the token steram to file, optionally with formatting options.*/
    int huSerializeTroveToFileZ(huTrove const * trove, char const * path, int * destLength, huSerializeOptions * SerializeOptions);
    /// Serializes a trove to file.
    /** This function stores a new copy of the token steram to file, optionally with formatting options.*/
    int huSerializeTroveToFileN(huTrove const * trove, char const * path, int pathLen, int * destLength, huSerializeOptions * SerializeOptions);

    /// Fills an array of HU_COLORCODE_NUMCOLORS huStringViews with ANSI terminal color codes for printing to console.
    void huFillAnsiColorTable(huStringView table[]);

#ifdef __cplusplus
} // extern "C"
#endif
