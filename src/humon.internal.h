#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "humon/humon.h"
#include "humon/ansiColors.h"


// Even <stdlib.h> defines these macros in Visual Studio 2017 unless you
// switch off language extensions, in which case the compiler uses C89.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// The following are switches that can be set when building Humon. They
// are not referenced in public headers, so they don't need to be set by
// a project that uses the Humon library.

/// Sets the stack-allocated block size for swagging an endocing.
#ifndef HUMON_SWAG_BLOCKSIZE
#define HUMON_SWAG_BLOCKSIZE		(64)	// checks a cache line at a time
#endif

/// Sets the stack-allocated block size for reading from file.
#ifndef HUMON_TRANSCODE_BLOCKSIZE
#define HUMON_TRANSCODE_BLOCKSIZE   (1 << 16)
#endif

/// Sets the stack-allocated block size for translating an address component.
#ifndef HUMON_ADDRESS_BLOCKSIZE
#define HUMON_ADDRESS_BLOCKSIZE     (64)
#endif

/// Option to skip parameter checks for faster invocations.
/// Only define HUMON_NO_PARAMETER_CHECKS if all your API calls are successful.
#ifndef HUMON_NO_PARAMETER_CHECKS
#define HUMON_CHECK_PARAMS
#endif

/// Option to examine useful debug reporting. Mainly for Humon development.
//#define HUMON_CAVEPERSON_DEBUGGING

/// Option to select terminal colors in console output.
#define HUMON_TERMINALCOLORS_NOCOLOR 1
#define HUMON_TERMINALCOLORS_ANSI    2
#ifndef HUMON_TERMINALCOLORS
#define HUMON_TERMINALCOLORS HUMON_TERMINALCOLORS_ANSI
#endif

// For determining the max value of a type t. Useful for user-settable types.
// Thanks, SO! https://stackoverflow.com/questions/2053843/min-and-max-value-of-data-type-in-c
#define isSignedType(t) (((t)(-1)) < ((t) 0))

#define uMaxOfType(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                        (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define sMaxOfType(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                        (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define maxOfType(t) ((unsigned long long) (isSignedType(t) ? sMaxOfType(t) : uMaxOfType(t)))

// Shifty. This passes compilers complaining about neg-testing unsigned ints.
// Since some types are user-defineable and should work as unsigned types, the
// parameter checking sometimes needs to check for negative, and sometimes not.
#define isNegative(v) ((v) < 1 && (v) != 0)

#ifdef __cplusplus
extern "C"
{
#endif

	huSize_t min(huSize_t a, huSize_t b);
	huSize_t max(huSize_t a, huSize_t b);

    void * sysAlloc(void * allocator, size_t len);
    void * sysRealloc(void * allocator, void * alloc, size_t len);
    void sysFree(void * allocator, void * alloc);

    void * ourAlloc(huAllocator const * allocator, size_t len);
    void * ourRealloc(huAllocator const * allocator, void * alloc, size_t len);
    void ourFree(huAllocator const * allocator, void * alloc);

    void printError(huErrorResponse errorResponse, char const * msg);

    FILE * openFile(char const * path, char const * mode);
    huErrorCode getFileSize(FILE * fp, huSize_t * fileLen, huErrorResponse errorResponse);

    /// Returns whether a string is contained in another string.
    bool stringInString(char const * haystack, huSize_t haystackLen, char const * needle, huSize_t needleLen);

    /// Initializes a vector to zero size. Vector can count characters but not store them. Does not allocate.
    void initVectorForCounting(huVector * vector);
    /// Initializes a vector with a preallocated buffer. Does not allocate, and cannot grow.
    void initVectorPreallocated(huVector * vector, void * buffer, huSize_t elementSize, huSize_t numElements);
    /// Initializes a vector to zero size. Does not allocate yet.
    void initGrowableVector(huVector * vector, huSize_t elementSize, huAllocator const * allocator);
    /// Frees the memory owned by a huVector.
    void destroyVector(huVector const * vector);
    /// Returns the number of elements in a huVector.
    huSize_t getVectorSize(huVector const * vector);
    /// Returns a pointer to an element in a huVector.
    void * getVectorElement(huVector const * vector, huSize_t idx);
    /// Resets a vector to its Init* state.
    void resetVector(huVector * vector);
    /// Adds an array of elements to the vector, and returns the number of elements successfully appended.
    huSize_t appendToVector(huVector * vector, void const * data, huSize_t numElements);
    /// Grows a growable vector, and return a pointer to the first element of the appended entries.
    void * growVector(huVector * vector, huSize_t * numElements);
    /// Shrinks a growable vector from the end.
    void shrinkVector(huVector * vector, huSize_t numElements);

    typedef struct huCursor_tag
    {
        char const * character;     // pointer into memory
        uint8_t charLength;         // length of the character
        uint32_t codePoint;         // Unicode code point
        bool isEof;                 // set if end of string
        bool isSpace;               // set if space-like whitespace
        bool isTab;                 // set if tab-like whitespace
        bool isNewline;             // set if newline-like whitespace
        bool isError;               // set if encoding was erroneous
    } huCursor;

    typedef struct huScanner_tag
    {
        huTrove * trove;
        huCol_t tabLen;
        char const * inputStr;
        huSize_t inputStrLen;
        huCursor * curCursor;
        huCursor * nextCursor;
        huCursor cursors[2];
        huLine_t line;
        huCol_t col;
        huSize_t len;
    } huScanner;

    /// Return if the CPU is a big-endian CPU.
    bool isMachineBigEndian();

    /// Move the scanner's character cursor by one.
    void nextCharacter(huScanner * cursor);
    /// Initialize a huScanner.
    void initScanner(huScanner * scanner, huTrove * trove, huCol_t tabLen, char const * str, huSize_t strLen);
    /// Move the scanner's character cursor past any whitespace.
    void eatWs(huScanner * cursor);

    /// Initialize a huNode object.
    void initNode(huNode * node, huTrove const * trove);
    /// Destroy a huNode object's contents.
    void destroyNode(huNode const * node);

    /// Add a huToken to a trove's token array.
    huToken * allocNewToken(huTrove * trove, huTokenKind kind, char const * str, huSize_t size,
        huLine_t line, huCol_t col, huLine_t endLine, huCol_t endCol,
        huSize_t offsetIn, huSize_t offsetOut, char quoteChar);
    /// Add a huNode to a trove's node array.
    huNode * allocNewNode(huTrove * trove, huNodeKind nodeKind, huToken const * firstToken);

    /// Add a huError to a trove's error array during tokenization.
    void recordTokenizeError(huTrove * trove, huErrorCode errorCode, huLine_t line, huCol_t col);
    /// Add a huError to a trove's error array during parsing.
    void recordParseError(huTrove * trove, huErrorCode errorCode, huToken const * pCur);

    /// Attempt to determine the Unicode encoding of a string in memory.
    huEncoding swagEncodingFromString(huStringView const * data, huSize_t * numBomChars, huDeserializeOptions * deserializeOptions);
    /// Attempt to determine the Unicode encoding of a file.
    huEncoding swagEncodingFromFile(FILE * fp, huSize_t fileSize, huSize_t * numBomChars, huDeserializeOptions * deserializeOptions);
    /// Transcode a string in memory from its native encoding to a UTF-8 memory buffer.
    huErrorCode transcodeToUtf8FromString(char * dest, huSize_t * numBytesEncoded, huStringView const * src, huDeserializeOptions * deserializeOptions);
    /// Transcode a file from its native encoding to a UTF-8 memory buffer.
    huErrorCode transcodeToUtf8FromFile(char * dest, huSize_t * numBytesEncoded, FILE * fp, huSize_t srcLen, huDeserializeOptions * deserializeOptions);

    /// Extracts the tokens from a token stream.
    void tokenizeTrove(huTrove * trove);
    /// Extracts the nodes from a token array.
    void parseTrove(huTrove * trove);

    /// Manages printing a dynamic string.
    /** An object of this class manages a memory string which
     * is printed small bits at a time. It also tracks the kind
     * of text printed, so as to properly format the whitespace
     * according to settings. */
    typedef struct PrintTracker_tag
    {
        huTrove const * trove;
        huVector * str;

        huSerializeOptions * serializeOptions;

        huSize_t currentDepth;
        bool lastPrintWasNewline;
        bool lastPrintWasIndent;
        bool lastPrintWasUnquotedWord;
        bool lastPrintWasWhitespace;
    } PrintTracker;

    /// This appends a string to a PrintTracker.
    void appendString(PrintTracker * printer, char const * addend, huSize_t size);
    /// This prints a trove to a whitespace-formatted string.
    void troveToPrettyString(huTrove const * trove, huVector * str, huSerializeOptions * serializeOptions);


    /// Encodes a token read from Humon text.
    /** This structure encodes file location and buffer location information about a
     * particular token in a Humon file. Every token is read and tracked with a huToken. */
    struct huToken_tag
    {
        short kind;                 ///< The kind of token this is (huTokenKind).
        char quoteChar;             ///< Whether the token is a quoted string.
        huStringView rawStr;        ///< A view of the token raw string.
        huStringView str;           ///< A view of the token unenquoted string.
        huLine_t line;              ///< The line number in the file where the token begins.
        huCol_t col;                ///< The column number in the file where the token begins.
        huLine_t endLine;           ///< The line number in the file where the token ends.
        huCol_t endCol;             ///< The column number in the file where the token end.
    };

    /// Encodes a Humon data node.
    /** Humon nodes make up a hierarchical structure, stemming from a single root node.
     * Humon troves contain a reference to the root, and store all nodes in an indexable
     * array. A node is either a list, a dict, or a value node. Any number of comments
     * and metatags can be associated to a node. */
    struct huNode_tag
    {
        struct huTrove_tag const * trove;   ///< The trove tracking this node.
        huSize_t nodeIdx;                   ///< The index of this node in its trove's tracking array.
        huNodeKind kind;                  ///< A huNodeKind value.
        huToken const * firstToken;         ///< The first token which contributes to this node, including any metatag and comment tokens.
        huToken const * keyToken;           ///< The key token if the node is inside a dict.
		huSize_t sharedKeyIdx;				///< The index of the node with the same key as other nodes, if inside a dict.
        huToken const * valueToken;         ///< The first token of this node's actual value; for a container, it points to the opening brac(e|ket).
        huToken const * lastValueToken;     ///< The last token of this node's actual value; for a container, it points to the closing brac(e|ket).
        huToken const * lastToken;          ///< The last token of this node, including any metatag and comment tokens.

        huSize_t parentNodeIdx;             ///< The parent node's index, or -1 if this node is the root.
        huSize_t childIndex;              ///< The index of this node vis a vis its sibling nodes (starting at 0).

        huVector childNodeIdxs;             ///< Manages a huSize_t []. Stores the node inexes of each child node, if this node is a collection.
        huVector metatags;               ///< Manages a huMetatag []. Stores the metatags associated to this node.
        huVector comments;                  ///< Manages a huComment []. Stores the comments associated to this node.
    };

    /// Encodes a Humon data trove.
    /** A trove stores all the tokens and nodes in a loaded Humon file. It is your main access
     * to the Humon object data. Troves are created by Humon functions that load from file or
     * string, and can output Humon to file or string as well. */
    struct huTrove_tag
    {
        huEncoding encoding;                      ///< The input Unicode encoding for loads.
        char const * dataString;                    ///< The buffer containing the Humon text as loaded. Owned by the trove. Humon takes care to NULL-terminate this string.
        huSize_t dataStringSize;                    ///< The size of the buffer.
        huAllocator allocator;                      ///< A custom memory allocator.
        huVector tokens;                            ///< Manages a huToken []. This is the array of tokens lexed from the Humon text.
        huVector nodes;                             ///< Manages a huNode []. This is the array of node objects parsed from tokens.
        huVector errors;                            ///< Manages a huError []. This is an array of errors encountered during load.
        huErrorResponse errorResponse;                 ///< How the trove respones to errors during load.
		huCol_t inputTabSize;			            ///< The tab length Humon uses to compute column values for tokens.
        huVector metatags;                       ///< Manages a huMetatag []. Contains the metatags associated to the trove.
        huVector comments;                          ///< Manages a huComment[]. Contains the comments associated to the trove.
        huToken const * lastMetatagToken;              ///< Token referencing the last token of any trove metatags.
        huBufferManagement bufferManagement;              ///< How to manage the input buffer. (One of huBufferManagement.)
    };

#ifdef __cplusplus
} // extern "C"
#endif
