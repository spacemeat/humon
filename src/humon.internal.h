#pragma once

#include <stdint.h>
#include "humon.h"
#include "ansiColors.h"


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

/// Sets the allocation block size for swagging an endocing.
#ifndef HUMON_SWAG_BLOCKSIZE
#define HUMON_SWAG_BLOCKSIZE		(64)	// checks a cache line at a time
#endif

/// Sets the stack-allocated block size for reading from file.
#ifndef HUMON_FILE_BLOCKSIZE
#define HUMON_FILE_BLOCKSIZE        (1 << 16)
#endif

/// Option to skip parameter checks for faster invocations.
/// Only define HUMON_NO_PARAMETER_CHECKS if all your API calls are successful.
#ifndef HUMON_NO_PARAMETER_CHECKS
#define HUMON_CHECK_PARAMS
#endif

/// Option to examine useful debug reporting. Mainly for Humon development.
//#define HUMON_CAVEPERSON_DEBUGGING


// For determining the max value of a type t. Useful for user-settable types.
// Thanks, SO! https://stackoverflow.com/questions/2053843/min-and-max-value-of-data-type-in-c
#define isSignedType(t) (((t)(-1)) < ((t) 0))

#define uMaxOfType(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                        (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define sMaxOfType(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                        (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define maxOfType(t) ((unsigned long long) (isSignedType(t) ? sMaxOfType(t) : uMaxOfType(t)))


#ifdef __cplusplus
extern "C"
{
#endif

	huIndexSize_t min(huIndexSize_t a, huIndexSize_t b);
	huIndexSize_t max(huIndexSize_t a, huIndexSize_t b);

    void printError(huEnumType_t errorResponse, char const * msg);

    FILE * openFile(char const * path, char const * mode);
    huEnumType_t getFileSize(FILE * fp, huIndexSize_t * fileLen, huEnumType_t errorResponse);

    /// Returns whether a string is contained in another string.
    bool stringInString(char const * haystack, huIndexSize_t haystackLen, char const * needle, huIndexSize_t needleLen);

    /// Initializes a vector to zero size. Vector can count characters but not store them. Does not allocate.
    void initVectorForCounting(huVector * vector);
    /// Initializes a vector with a preallocated buffer. Does not allocate, and cannot grow.
    void initVectorPreallocated(huVector * vector, void * buffer, huIndexSize_t elementSize, huIndexSize_t numElements);
    /// Initializes a vector to zero size. Does not allocate yet.
    void initGrowableVector(huVector * vector, huIndexSize_t elementSize);
    /// Frees the memory owned by a huVector.
    void destroyVector(huVector const * vector);
    /// Returns the number of elements in a huVector.
    huIndexSize_t getVectorSize(huVector const * vector);
    /// Returns a pointer to an element in a huVector.
    void * getVectorElement(huVector const * vector, huIndexSize_t idx);
    /// Resets a vector to its Init* state.
    void resetVector(huVector * vector);
    /// Adds an array of elements to the vector, and returns the number of elements successfully appended.
    huIndexSize_t appendToVector(huVector * vector, void const * data, huIndexSize_t numElements);
    /// Grows a growable vector, and return a pointer to the first element of the appended entries.
    void * growVector(huVector * vector, huIndexSize_t * numElements);

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
        char const * inputStr;
        huIndexSize_t inputStrLen;
        huCursor * curCursor;
        huCursor * nextCursor;
        huCursor cursors[2];
    } huScanner;

    /// Return if the CPU is a big-endian CPU.
    bool isMachineBigEndian();

    /// Move the scanner's character cursor by one.
    void nextCharacter(huScanner * cursor);
    /// Initialize a huScanner.
    void initScanner(huScanner * scanner, huTrove * trove, char const * str, huIndexSize_t strLen);
    /// Move the scanner's character cursor past any whitespace.
    void eatWs(huScanner * cursor, huCol_t tabSize, huLine_t * line, huCol_t * col);

    /// Initialize a huNode object.
    void initNode(huNode * node, huTrove const * trove);
    /// Destroy a huNode object's contents.
    void destroyNode(huNode const * node);

    /// Add a huToken to a trove's token array.
    huToken * allocNewToken(huTrove * trove, huEnumType_t kind, char const * str, huIndexSize_t size, huLine_t line, huCol_t col, huLine_t endLine, huCol_t endCol, char quoteChar);
    /// Add a huNode to a trove's node array.
    huNode * allocNewNode(huTrove * trove, huEnumType_t nodeKind, huToken const * firstToken);

    /// Add a huError to a trove's error array during tokenization.
    void recordTokenizeError(huTrove * trove, huEnumType_t errorCode, huLine_t line, huCol_t col);
    /// Add a huError to a trove's error array during parsing.
    void recordParseError(huTrove * trove, huEnumType_t errorCode, huToken const * pCur);

    /// Attempt to determine the Unicode encoding of a string in memory.
    huEnumType_t swagEncodingFromString(huStringView const * data, huIndexSize_t * numBomChars, huDeserializeOptions * DeserializeOptions);
    /// Attempt to determine the Unicode encoding of a file.
    huEnumType_t swagEncodingFromFile(FILE * fp, huIndexSize_t fileSize, huIndexSize_t * numBomChars, huDeserializeOptions * DeserializeOptions);
    /// Transcode a string in memory from its native encoding to a UTF-8 memory buffer.
    huEnumType_t transcodeToUtf8FromString(char * dest, huIndexSize_t * numBytesEncoded, huStringView const * src, huDeserializeOptions * DeserializeOptions);
    /// Transcode a file from its native encoding to a UTF-8 memory buffer.
    huEnumType_t transcodeToUtf8FromFile(char * dest, huIndexSize_t * numBytesEncoded, FILE * fp, huIndexSize_t srcLen, huDeserializeOptions * DeserializeOptions);

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

        huSerializeOptions * SerializeOptions;

        huIndexSize_t currentDepth;
        bool lastPrintWasNewline;
        bool lastPrintWasIndent;
        bool lastPrintWasUnquotedWord;
        bool lastPrintWasWhitespace;
    } PrintTracker;

    /// This appends a string to a PrintTracker.
    void appendString(PrintTracker * printer, char const * addend, huIndexSize_t size);
    /// This prints a trove to a whitespace-formatted string.
    void troveToPrettyString(huTrove const * trove, huVector * str, huSerializeOptions * SerializeOptions);

#ifdef __cplusplus
} // extern "C"
#endif
