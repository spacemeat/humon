#pragma once

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef HUMON_CHECK_PARAMS
#if defined(DEBUG) && !defined(HUMON_FORCE_NO_CHECK_PARAMS)
#define HUMON_CHECK_PARAMS
#endif
#endif

    static int const MIN_INPUT_TAB_SIZE = 1;
    static int const MAX_INPUT_TAB_SIZE = 1024;

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
        HU_NODEKIND_NULL,   ///< Invalid node. And invalid address returns a null node.
        HU_NODEKIND_LIST,   ///< List node. The node ontains a sequence of unassociated objects in maintained order.
        HU_NODEKIND_DICT,   ///< Dict node. The node ontains a sequence of string-associated objects in maintained order.
        HU_NODEKIND_VALUE   ///< Value node. The node ontains a string value, and no children.
    };

    /// Returns a string representation of a huNodeKind.
    char const * huNodeKindToString(int rhs);

    /// Specifies the style of whitespacing in Humon text.
    enum huOutputFormat
    {
        HU_OUTPUTFORMAT_PRESERVED,  ///< Preserves the original whitespacing as loaded.
        HU_OUTPUTFORMAT_MINIMAL,    ///< Reduces as much whitespace as possible.
        HU_OUTPUTFORMAT_PRETTY      ///< Formats the text in a standard, human-friendly way.
    };

    /// Returns a string representation of a huOutputFormat.
   char const * huOutputFormatToString(int rhs);

    /// Specifies a tokenizing or parsing error code, or lookup error.
    enum huErrorCode
    {
        HU_ERROR_NO_ERROR,                  ///< No error.
        HU_ERROR_UNEXPECTED_EOF,            ///< The text ended early.
        HU_ERROR_UNFINISHED_QUOTE,          ///< The quoted text was not endquoted.
        HU_ERROR_UNFINISHED_CSTYLECOMMENT,  ///< The C-style comment was not closed.
        HU_ERROR_SYNTAX_ERROR,              ///< General syntax error.
        HU_ERROR_START_END_MISMATCH,        ///< Braces ({,}) or brackets ([,]) are not properly nested.
        HU_ERROR_NOTFOUND,                  ///< No node could be found at the address.
        HU_ERROR_ILLEGAL                    ///< The address or node was illegal.
    };

    /// Returns a string representation of a huErrorCode.
    char const * huOutputErrorToString(int rhs);

    /// Specifies a style ID for colorized printing.
    enum huColorKind
    {
        HU_COLORKIND_NONE = 0,                  ///< No color
        HU_COLORKIND_END,                       ///< End-of-color code.
        HU_COLORKIND_PUNCLIST,                  ///< List punctuation style. ([,]) 
        HU_COLORKIND_PUNCDICT,                  ///< Dict punctuation style. ({,})
        HU_COLORKIND_PUNCKEYVALUESEP,           ///< Key-value separator style. (:)
        HU_COLORKIND_PUNCANNOTATE,              ///< Annotation mark style. (@)
        HU_COLORKIND_PUNCANNOTATEDICT,          ///< Annotation dict punctuation style. ({,})
        HU_COLORKIND_PUNCANNOTATEKEYVALUESEP,   ///< Annotation key-value separator style. (:)
        HU_COLORKIND_KEY,                       ///< Key style.
        HU_COLORKIND_VALUE,                     ///< Value style.
        HU_COLORKIND_COMMENT,                   ///< Comment style.
        HU_COLORKIND_ANNOKEY,                   ///< Annotation key style.
        HU_COLORKIND_ANNOVALUE,                 ///< Annotation value style.
        HU_COLORKIND_WHITESPACE,                ///< Whitespace style (including commas).

        HU_COLORKIND_NUMCOLORKINDS              ///< One past the last style code.
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
        void * buffer;          ///< The owned buffer for the array.
        int elementSize;        ///< The size of one element of the array. If set to 0, the vector does not manage memory.
        int numElements;        ///< The number of elements currently managed by the array.
        int vectorCapacity;     ///< The maximum capacity of the array.
    } huVector;

    /// Stores a pointer to a string, and its size. Does not own the string.
    /** Represents a view of a string. The string is unlikely to be NULL-terminated.
     * It is also not likely to point directly to an allocation, but to a substring. */
    typedef struct huStringView_tag
    {
        char const * str;       ///< The beginning of the string in memory.
        int size;               ///< The size of the string in bytes.
    } huStringView;

    /// Encodes a token read from Humon text.
    /** This structure encodes file location and buffer location information about a
     * particular token in a Humon file. Every token is read and tracked with a huToken. */
    typedef struct huToken_tag
    {
        int tokenKind;          ///< A huTokenKind value.
        huStringView value;     ///< A view of the token string.
        int line;               ///< The line number in the file where the token begins.
        int col;                ///< The column number in the file where the token begins.
        int endLine;            ///< The line number in the file where the token ends.
        int endCol;             ///< the column number in the file where the token end.
    } huToken;

    /// Encodes a dict entry's key-index association.
    /** Dicts store their child nodes in order like lists, but also maintain a lookup table
     * of huDictEntry objects for quicker access by key. */
    typedef struct huDictEntry_tag
    {
        huToken const * key;      ///< The key token owned by the child node.
        int idx;            ///< The index of the node in the child node array.
    } huDictEntry;

    /// Encodes an annotation entry for a node or trove.
    /** Nodes and troves can have a plurality of annotations. They are always key-value string
     * pairs, which are referenced by huAnnotation objets. */
    typedef struct huAnnotation_tag
    {
        huToken const * key;      ///< The annotation key token.
        huToken const * value;    ///< The annotation value token.
    } huAnnotation;

    typedef struct huNode_tag huNode;

    /// Encodes a comment.
    /** A comment object includes the comment token, and the node to which it is associated. If
     * owner is NULL, the comment is associated to the trove. */
    typedef struct huComment_tag
    {
        huToken const * commentToken;     ///< The comment token.
        huNode const * owner;             ///< The associated node. NULL indicated association to a trove.
    } huComment;

    /// Encodes an error tracked by the tokenizer or parser.
    /** Errors are tracked for reporting to the user. */
    typedef struct huError_tag
    {
        int errorCode;              ///< A huErrorCode value.
        huToken const * errorToken;       ///< The token that seems to be erroneous.
    } huError;


    typedef struct huTrove_tag huTrove;

    /// Encodes a Humon data node.
    /** Humon nodes make up a heirarchical structure, stemming from a single root node.
     * Humon troves contain a reference to the root, and store all nodes in an indexable
     * array. A node is either a list, a dict, or a value node. Any number of comments 
     * and annotations can be associated to a node. */
    typedef struct huNode_tag
    {
        huTrove const * trove;              ///< The trove tracking this node.
        int nodeIdx;                        ///< The index of this node in the tracking array.
        int kind;                           ///< A huNodeKind value.
        huToken const * firstToken;         ///< The first token which contributes to this node, including any annotation and comment tokens.
        huToken const * keyToken;           ///< The key token if the node is inside a dict.
        huToken const * firstValueToken;    ///< The first token of this node's actual value; for a container, it points to the opening brac(e|ket).
        huToken const * lastValueToken;     ///< The last token of this node's actual value; for a container, it points to the closing brac(e|ket).
        huToken const * lastToken;          ///< The last token of this node, including any annotation and comment tokens.

        int parentNodeIdx;                  ///< The parent node's index, or -1 if this node is the root.
        int childIdx;                       ///< The index of this node vis a vis its sibling nodes (starting at 0).

        huVector childNodeIdxs;             ///< Manages a int []. Stores the node inexes of each child node, if this node is a collection.
        huVector childDictKeys;             ///< Manages a huDictEntry []. Stores the key-index associations for a dict.
        huVector annotations;               ///< Manages a huAnnotation []. Stores the annotations associated to this node.
        huVector comments;                  ///< Manages a huComment []. Stores the comments associated to this node.
    } huNode;

    /// Gets a pointer to a node's parent.
    huNode const * huGetParentNode(huNode const * node);
    /// Gets the number of children a node has.
    int huGetNumChildren(huNode const * node);
    /// Gets a child node by child index.
    huNode const * huGetChildNodeByIndex(huNode const * node, int childIdx);
    /// Gets a child node by NULL-terminated key.
    huNode const * huGetChildNodeByKeyZ(huNode const * node, char const * key);
    /// Gets a child node by string view key.
    huNode const * huGetChildNodeByKeyN(huNode const * node, char const * key, int keyLen);

    /// Returns if a node has a key token tracked. (If it's a member of a dict.)
    bool huHasKey(huNode const * node);
    /// Returns the token containing the key string for a node.
    huToken const * huGetKey(huNode const * node);

    /// Returns if a node has a value token tracked. (All non-null nodes always should.)
    bool huHasValue(huNode const * node);

    /// Returns the next sibling in the child index order of a node.
    huNode const * huNextSibling(huNode const * node);

    /// Returns the number of annotations associated to a node.
    int huGetNumAnnotations(huNode const * node);
    /// Returns an annotation object associated to a node, by index.
    huAnnotation const * huGetAnnotation(huNode const * node, int annotationIdx);

    /// Returns whether there is an annotation associated to a node with a specific key.
    bool huHasAnnotationWithKeyZ(huNode const * node, char const * key);
    /// Returns whether there is an annotation associated to a node with a specific key.
    bool huHasAnnotationWithKeyN(huNode const * node, char const * key, int keyLen);
    /// Returns the value token of an annoation object associated to a node, with the specified key.
    huToken const * huGetAnnotationByKeyZ(huNode const * node, char const * key);
    /// Returns the value token of an annoation object associated to a node, with the specified key.
    huToken const * huGetAnnotationByKeyN(huNode const * node, char const * key, int keyLen);

    /// Returns the number of annotations associated to a node with a specific value.
    int huGetNumAnnotationsByValueZ(huNode const * node, char const * value);
    /// Returns the number of annotations associated to a node with a specific value.
    int huGetNumAnnotationsByValueN(huNode const * node, char const * value, int valueLen);
    /// Returns the key token of an annotation associated to a node, with the specified value and index.
    huToken const * huGetAnnotationByValueZ(huNode const * node, char const * value, int annotationIdx);
    /// Returns the key token of an annotation associated to a node, with the specified value and index.
    huToken const * huGetAnnotationByValueN(huNode const * node, char const * value, int valueLen, int annotationIdx);

    /// Returns the number of comments associated to a node.
    int huGetNumComments(huNode const * node);
    /// Returns a comment token associated to a node, by index.
    huToken const * huGetComment(huNode const * node, int commentIdx);
    /// Returns all comment tokens associated to a node which contain the specified substring.
    huToken const * huGetCommentsContainingZ(huNode const * node, char const * containedText, huToken const * startWith);
    /// Returns all comment tokens associated to a node which contain the specified substring.
    huToken const * huGetCommentsContainingN(huNode const * node, char const * containedText, int containedTextLen, huToken const * startWith);

    /// Looks up a node by relative address to a node.    
    huNode const * huGetNodeByRelativeAddressZ(huNode const * node, char const * address);
    /// Looks up a node by relative address to a node.    
    huNode const * huGetNodeByRelativeAddressN(huNode const * node, char const * address, int addressLen);

    /// Gets the full address of a node, or the length of that address.
    void huGetNodeAddress(huNode const * node, char * address, int * addressLen);

    /// Encodes a Humon data trove.
    /** A trove stores all the tokens and nodes in a loaded Humon file. It is your main access
     * to the Humon object data. Troves are created by Humon functions that load from file or 
     * string, and can output Humon to file or string as well. */
    typedef struct huTrove_tag
    {
        char const * dataString;      ///< The buffer containing the Humon text as loaded. Owned by the trove. Humon takes care to NULL-terminate this string.
        int dataStringSize;     ///< The size of the buffer.
        huVector tokens;        ///< Manages a huToken []. This is the array of tokens lexed from the Humon text.
        huVector nodes;         ///< Manages a huNode []. This is the array of node objects parsed from tokens.
        huVector errors;        ///< Manages a huError []. This is an array of errors encountered during load.
        int inputTabSize;       ///< The tab length Humon uses to compute column values for tokens.
        huVector annotations;   ///< Manages a huAnnotation []. Contains the annotations associated to the trove.
        huVector comments;      ///< Manages a huComment[]. Contains the comments associated to the trove.
    } huTrove;

    /// Creates a trove from a NULL-terminated string of Humon text.
    huTrove const * huMakeTroveFromStringZ(char const * data, int inputTabSize);
    /// Creates a trove from a string view of Humon text.
    huTrove const * huMakeTroveFromStringN(char const * data, int dataLen, int inputTabSize);
    /// Creates a trove from a file.
    huTrove const * huMakeTroveFromFileZ(char const * path, int inputTabSize);
    /// Creates a trove from a file.
    huTrove const * huMakeTroveFromFileN(char const * path, int pathLen, int inputTabSize);

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
    huNode const * huGetNode(huTrove const * trove, int nodeIdx);

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
    int huGetNumTroveAnnotationsByValueZ(huTrove const * trove, char const * value);
    /// Returns the number of annotations associated to a trove with a specific value.
    int huGetNumTroveAnnotationsByValueN(huTrove const * trove, char const * value, int valueLen);
    /// Returns an annoation object associated to a trove, by value and index.
    huToken const * huGetTroveAnnotationByValueZ(huTrove const * trove, char const * value, int annotationIdx);
    /// Returns an annoation object associated to a trove, by value and index.
    huToken const * huGetTroveAnnotationByValueN(huTrove const * trove, char const * value, int valueLen, int annotationIdx);

    /// Returns the number of comments associated to a trove.
    int huGetNumTroveComments(huTrove const * trove);
    /// Returns a comment associated to a trove by index.
    huToken const * huGetTroveComment(huTrove const * trove, int errorIdx);

    /// Returns a node by its full address.
    huNode const * huGetNodeByFullAddressZ(huTrove const * trove, char const * address);
    /// Returns a node by its full address.
    huNode const * huGetNodeByFullAddressN(huTrove const * trove, char const * address, int addressLen);

    /// Returns a collection of all nodes in a trove with a specific annotation key.
    huNode const * huFindNodesByAnnotationKeyZ(huTrove const * trove, char const * key, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a specific annotation key.
    huNode const * huFindNodesByAnnotationKeyN(huTrove const * trove, char const * key, int keyLen, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a specific annotation value.
    huNode const * huFindNodesByAnnotationValueZ(huTrove const * trove, char const * value, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a specific annotation value.
    huNode const * huFindNodesByAnnotationValueN(huTrove const * trove, char const * value, int valueLen, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    huNode const * huFindNodesByAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    huNode const * huFindNodesByAnnotationKeyValueNZ(huTrove const * trove, char const * key, int keyLen, char const * value, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    huNode const * huFindNodesByAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a specific annotation key and value.
    huNode const * huFindNodesByAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen, huNode const * startWith);

    /// Returns a collection of all nodes in a trove with a comment which contains specific text.
    huNode const * huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText, huNode const * startWith);
    /// Returns a collection of all nodes in a trove with a comment which contains specific text.
    huNode const * huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, int containedTextLen, huNode const * startWith);

    /// Serializes a trove to text.
    void huTroveToString(huTrove const * trove, char * dest, int * destLength, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable);

    /// Serializes a trove to file.
    size_t huTroveToFileZ(huTrove const * trove, char const * path, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable);
    /// Serializes a trove to file.
    size_t huTroveToFileN(huTrove const * trove, char const * path, int pathLen, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable);

    /// Global null token object. Functions that return null tokens reference this.
    extern huToken const humon_nullToken;
    /// Global null node object. Functions that return null nodes reference this.
    extern huNode const humon_nullNode;
    /// Global null trove object. Functions that return null troves reference this.
    extern huTrove const humon_nullTrove;

#ifdef __cplusplus
} // extern "C"
#endif
