#pragma once

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /// Specifies the kind of data represented by a particular huToken.
    enum huTokenKind
    {
        HU_TOKENKIND_NULL,          ///< Invalid token. Either malformed, or otherwise nonexistent.
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

    /// Return a string representation of a huTokenKind.
    char const * huTokenKindToString(int rhs);

    /// Specifies the kind of node represented by a particular huNode.
    enum huNodeKind
    {
        HU_NODEKIND_NULL,   ///< Invalid node. And invalid address returns a null node.
        HU_NODEKIND_LIST,   ///< List node. The node ontains a sequence of unassociated objects in maintained order.
        HU_NODEKIND_DICT,   ///< Dict node. The node ontains a sequence of string-associated objects in maintained order.
        HU_NODEKIND_VALUE   ///< Value node. The node ontains a string value, and no children.
    };

    /// Return a string representation of a huNodeKind.
    char const * huNodeKindToString(int rhs);

    /// Specifies the style of whitespacing in humon text.
    enum huOutputFormat
    {
        HU_OUTPUTFORMAT_PRESERVED,  ///< Preserves the original whitespacing as loaded.
        HU_OUTPUTFORMAT_MINIMAL,    ///< Reduces as much whitespace as possible.
        HU_OUTPUTFORMAT_PRETTY      ///< Formats the text in a standard, human-friendly way.
    };

    /// Return a string representation of a huOutputFormat.
   char const * huOutputFormatToString(int rhs);

    /// Specifies a tokizing or parsing error code, or lookup error.
    enum huErrorCode
    {
        HU_ERROR_NO_ERROR,                  ///< No error.
        HU_ERROR_UNEXPECTED_EOF,            ///< The text ended early.
        HU_ERROR_UNFINISHED_QUOTE,          ///< The quoted text was not endquoted.
        HU_ERROR_UNFINISHED_CSTYLECOMMENT,  ///< The C-style comment was not closed.
        HU_ERROR_SYNTAX_ERROR,              ///< General syntax error.
        HU_ERROR_START_END_MISMATCH,        ///< Braces ({,}) or brackets ([,]) are not properly nested.
        HU_ERROR_NOTFOUND                   ///< No node could be found at the address.
    };

    /// Return a string representation of a huErrorCode.
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
     * user should free() the buffer pointer. */
    typedef struct huVector_tag
    {
        void * buffer;          ///< The owned buffer for the array.
        int elementSize;        ///< The size of one element of the array.
        int numElements;        ///< The number of elements currently managed by the array.
        int vectorCapacity;     ///< The maximum capacity of the array.
    } huVector;

    /// Frees the memory owned by a huVector.
    void huDestroyVector(huVector const * vector);
    /// Returns the number of elements in a huVector.
    int huGetVectorSize(huVector const * vector);
    /// Returns a pointer to an element in a huVector.
    void * huGetVectorElement(huVector const * vector, int idx);

    /// Stores a pointer to a string, and its size. May or may not own the string.
    /** Represents a view of a string. The string is unlikely to be NULL-terminated.
     * It is also not likely to point directly to an allocation, but to a substring.
     * If the string memory is owned by the huStringView object, documentation will
     * specify that the user must use huDestroyStringView(). if not, a huStringView
     * object must simply be descoped. */
    typedef struct huStringView_tag
    {
        char const * str;       ///< The beginning of the string in memory.
        int size;               ///< The size of the string in bytes.
    } huStringView;

    /// Frees the memory owned by a huStringView.
    void huDestroyStringView(huStringView const * string);

    /// Encodes a token read from humon text.
    /** This structure encodes file location and buffer location information about a
     * particular token in a humon file. Every token is read and tracked with a huToken. */
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

    /// Encodes a humon data node.
    /** Humon nodes make up a heirarchical structure, stemming from a single root node.
     * Humon troves contain a reference to the root, and store all nodes in an indexable
     * array. A node is either a list, a dict, or a value node. Any number of comments 
     * and annotations can be associated to a node. */
    typedef struct huNode_tag
    {
        huTrove const * trove;                ///< The trove tracking this node.
        int nodeIdx;                    ///< The index of this node in the tracking array.
        int kind;                       ///< A huNodeKind value.
        huToken const * firstToken;           ///< The first token which contributes to this node, including any annotation and comment tokens.
        huToken const * keyToken;             ///< The key token if the node is inside a dict.
        huToken const * firstValueToken;      ///< The first token of this node's actual value; for a container, it points to the opening brac(e|ket).
        huToken const * lastValueToken;       ///< The last token of this node's actual value; for a container, it points to the closing brac(e|ket).
        huToken const * lastToken;            ///< The last token of this node, including any annotation and comment tokens.

        int parentNodeIdx;              ///< The parent node's index, or -1 if this node is the root.
        int childIdx;                   ///< The index of this node vis a vis its sibling nodes (starting at 0).

        huVector childNodeIdxs;         ///< Manages a int []. Stores the node inexes of each child node, if this node is a collection.
        huVector childDictKeys;         ///< Manages a huDictEntry []. Stores the key-index associations for a dict.
        huVector annotations;           ///< Manages a huAnnotation []. Stores the annotations associated to this node.
        huVector comments;              ///< Manages a huComment []. Stores the comments associated to this node.
    } huNode;

    /// Get a pointer to a node's parent.
    huNode const * huGetParentNode(huNode const * node);
    /// Get the number of children a node has.
    int huGetNumChildren(huNode const * node);
    /// Get a child node by child index.
    huNode const * huGetChildNodeByIndex(huNode const * node, int childIdx);
    /// Get a child node by NULL-terminated key.
    huNode const * huGetChildNodeByKeyZ(huNode const * node, char const * key);
    /// Get a child node by string view key.
    huNode const * huGetChildNodeByKeyN(huNode const * node, char const * key, int keyLen);

    /// Return if a node has a key token tracked. (If it's a member of a dict.)
    bool huHasKey(huNode const * node);
    /// Return the token containing the key string for a node.
    huToken const * huGetKey(huNode const * node);

    /// Return if a node has a value token tracked. (In a valid state, it always should.)
    bool huHasValue(huNode const * node);
    /// Return the token containing the first value elements.
    huToken const * huGetValue(huNode const * node);

    /// Return the first token which contributes to this node, including annotations and comments.
    huToken const * huGetStartToken(huNode const * node);
    /// Return the last token which contributes to this node, including annotations and comments.
    huToken const * huGetEndToken(huNode const * node);

    /// Return the next sibling in the child index order of a node.
    huNode const * huNextSibling(huNode const * node);

    /// Return the number of annotations associated to a node.
    int huGetNumAnnotations(huNode const * node);
    /// Return an annotation object associated to a node, by index.
    huAnnotation const * huGetAnnotation(huNode const * node, int annotationIdx);

    /// Return the number of annotations associated to a node with a specific key.
    int huGetNumAnnotationsByKeyZ(huNode const * node, char const * key);
    /// Return the number of annotations associated to a node with a specific key.
    int huGetNumAnnotationsByKeyN(huNode const * node, char const * key, int keyLen);
    /// Return an annoation object associated to a node, by key and index.
    huAnnotation const * huGetAnnotationByKeyZ(huNode const * node, char const * key, int annotationIdx);
    /// Return an annoation object associated to a node, by key and index.
    huAnnotation const * huGetAnnotationByKeyN(huNode const * node, char const * key, int keyLen, int annotationIdx);

    /// Return the number of annotations associated to a node with a specific value.
    int huGetNumAnnotationsByValueZ(huNode const * node, char const * value);
    /// Return the number of annotations associated to a node with a specific value.
    int huGetNumAnnotationsByValueN(huNode const * node, char const * value, int valueLen);
    /// Return an annoation object associated to a node, by value and index.
    huAnnotation const * huGetAnnotationByValueZ(huNode const * node, char const * value, int annotationIdx);
    /// Return an annoation object associated to a node, by value and index.
    huAnnotation const * huGetAnnotationByValueN(huNode const * node, char const * value, int valueLen, int annotationIdx);

    /// Return the number of comments associated to a node.
    int huGetNumComments(huNode const * node);
    /// Return a comment associated to a node, by index.
    huComment const * huGetComment(huNode const * node, int commentIdx);

    /// Look up a node by relative address to a node.    
    huNode const * huGetNodeByRelativeAddressZ(huNode const * node, char const * address, int * error);
    /// Look up a node by relative address to a node.    
    huNode const * huGetNodeByRelativeAddressN(huNode const * node, char const * address, int addressLen, int * error);

    /// Return the full address for a node.
    /// @note: user must DestroyStringView(retval).
    huStringView huGetNodeAddress(huNode const * node);

    /// Encodes a humon data trove.
    /** A trove stores all the tokens and nodes in a loaded humon file. It is your main access
     * to the humon object data. Troves are created by humon functions that load from file or 
     * string, and can output humon to file or string as well. */
    typedef struct huTrove_tag
    {
        char const * dataString;      ///< The buffer containing the humon text as loaded. Owned by the trove. Humon takes care to NULL-terminate this string.
        int dataStringSize;     ///< The size of the buffer.
        huVector tokens;        ///< Manages a huToken []. This is the array of tokens lexed from the humon text.
        huVector nodes;         ///< Manages a huNode []. This is the array of node objects parsed from tokens.
        huVector errors;        ///< Manages a huError []. This is an array of errors encountered during load.
        int inputTabSize;       ///< The tab length humon uses to compute column values for tokens.
        int outputTabSize;      ///< The tab length used when printing humon text.
        huVector annotations;   ///< Manages a huAnnotation []. Contains the annotations associated to the trove.
        huVector comments;      ///< Manages a huComment[]. Contains the comments associated to the trove.
    } huTrove;

    /// Creates a trove from a NULL-terminated string of humon text.
    huTrove const * huMakeTroveFromStringZ(char const * data, int inputTabSize, int outputTabSize);
    /// Creates a trove from a string view of humon text.
    huTrove const * huMakeTroveFromStringN(char const * data, int dataLen, int inputTabSize, int outputTabSize);
    /// Creates a trove from a file.
    huTrove const * huMakeTroveFromFile(char const * path, int inputTabSize, int outputTabSize);

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

    /// Return the number of annotations associated to a trove with a specific key.
    int huGetNumTroveAnnotationsByKeyZ(huTrove const * trove, char const * key);
    /// Return the number of annotations associated to a trove with a specific key.
    int huGetNumTroveAnnotationsByKeyN(huTrove const * trove, char const * key, int keyLen);
    /// Return an annoation object associated to a trove, by key and index.
    huAnnotation const * huGetTroveAnnotationByKeyZ(huTrove const * trove, char const * key, int annotationIdx);
    /// Return an annoation object associated to a trove, by key and index.
    huAnnotation const * huGetTroveAnnotationByKeyN(huTrove const * trove, char const * key, int keyLen, int annotationIdx);

    /// Return the number of annotations associated to a trove with a specific value.
    int huGetNumTroveAnnotationsByValueZ(huTrove const * trove, char const * value);
    /// Return the number of annotations associated to a trove with a specific value.
    int huGetNumTroveAnnotationsByValueN(huTrove const * trove, char const * value, int valueLen);
    /// Return an annoation object associated to a trove, by value and index.
    huAnnotation const * huGetTroveAnnotationByValueZ(huTrove const * trove, char const * value, int annotationIdx);
    /// Return an annoation object associated to a trove, by value and index.
    huAnnotation const * huGetTroveAnnotationByValueN(huTrove const * trove, char const * value, int valueLen, int annotationIdx);

    /// Return the number of comments associated to a trove.
    int huGetNumTroveComments(huTrove const * trove);
    /// Return a comment associated to a trove by index.
    huComment const * huGetTroveComment(huTrove const * trove, int errorIdx);

    /// Return a node by its full address.
    huNode const * huGetNodeByFullAddressZ(huTrove const * trove, char const * address, int * error);
    /// Return a node by its full address.
    huNode const * huGetNodeByFullAddressN(huTrove const * trove, char const * address, int addressLen, int * error);

    /// Return a collection of all nodes in a trove with a specific annotation key.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationKeyZ(huTrove const * trove, char const * key);
    /// Return a collection of all nodes in a trove with a specific annotation key.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationKeyN(huTrove const * trove, char const * key, int keyLen);
    /// Return a collection of all nodes in a trove with a specific annotation value.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationValueZ(huTrove const * trove, char const * value);
    /// Return a collection of all nodes in a trove with a specific annotation value.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationValueN(huTrove const * trove, char const * value, int valueLen);
    /// Return a collection of all nodes in a trove with a specific annotation key and value.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value);
    /// Return a collection of all nodes in a trove with a specific annotation key and value.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationKeyValueNZ(huTrove const * trove, char const * key, int keyLen, char const * value);
    /// Return a collection of all nodes in a trove with a specific annotation key and value.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen);
    /// Return a collection of all nodes in a trove with a specific annotation key and value.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen);

    /// Return a collection of all nodes in a trove with a comment which contains specific text.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByCommentZ(huTrove const * trove, char const * text);
    /// Return a collection of all nodes in a trove with a comment which contains specific text.
    /// @note: User must huDestroyVector(retval.buffer);
    huVector huFindNodesByCommentN(huTrove const * trove, char const * text, int textLen);    

    /// Serialize a trove to text.
    /// @note: User must huDestroyStringView(retval.str);
    huStringView huTroveToString(huTrove const * trove, int outputFormat, bool excludeComments, huStringView const * colorTable);

    /// Serialize a trove to file.
    size_t huTroveToFile(huTrove const * trove, char const * path, int outputFormat, bool excludeComments, huStringView const * colorTable);

    /// Global null token object. Functions that return null tokens reference this.
    extern huToken humon_nullToken;
    /// Global null node object. Functions that return null nodes reference this.
    extern huNode humon_nullNode;
    /// Global null trove object. Functions that return null troves reference this.
    extern huTrove humon_nullTrove;

#ifdef __cplusplus
} // extern "C"
#endif
