#pragma once

#include <stdint.h>
#include "humon.h"
#include "ansiColors.h"


// The following are options you can set before #include <humon.h>

/// Sets the stack-allocated block size for reading from file.
#ifndef HUMON_FILE_BLOCK_SIZE
#define HUMON_FILE_BLOCK_SIZE       (1 << 16)
#endif

/// Option to skip parameter checks for faster invocations.
/// Only define HUMON_NO_PARAMETER_CHECKS if all your API calls are successful.
#ifndef HUMON_NO_PARAMETER_CHECKS
#define HUMON_CHECK_PARAMS
#endif

/// Option to examine useful debug reporting. Mainly for Humon development.
//#define HUMON_CAVEPERSON_DEBUGGING

/// Option to silence error reporting to STDERR.
#ifndef HUMON_SUPPRESS_ERROR_TO_STDERR
#define HUMON_ERRORS_TO_STDERR
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    bool stringInString(char const * haystack, int haystackLen, char const * needle, int needleLen);

    /// Initializes a vector to zero size. Vector can count characters but not store them. Does not allocate.
    void huInitVectorForCounting(huVector * vector);
    /// Initializes a vector with a preallocated buffer. Does not allocate, and cannot grow.
    void huInitVectorPreallocated(huVector * vector, void * buffer, int elementSize, int numElements);
    /// Initializes a vector to zero size. Does not allocate yet.
    void huInitGrowableVector(huVector * vector, int elementSize);

    /// Frees the memory owned by a huVector.
    void huDestroyVector(huVector const * vector);
    /// Returns the number of elements in a huVector.
    int huGetVectorSize(huVector const * vector);
    /// Returns a pointer to an element in a huVector.
    void * huGetVectorElement(huVector const * vector, int idx);

    void huResetVector(huVector * vector);
    int huAppendToVector(huVector * vector, void const * data, int numElements);
    void * huGrowVector(huVector * vector, int * numElements);

    typedef struct huCursor_tag
    {
        char const * character;     // pointer into memory
        uint8_t charLength;         // length of the character
        uint32_t codePoint;         // Unicode code point
        bool isEof;                 // set if end of string
        bool isSpace;               // set if space-like whitespace
        bool isTab;                 // set if tab-like whitespace
        bool isNewline;             // set if newline-like whitespace
    } huCursor;

    typedef struct huScanner_tag
    {
        huTrove * trove;
        char const * inputStr;
        int inputStrLen;
        huCursor * curCursor;
        huCursor * nextCursor;
        huCursor cursors[2];
    } huScanner;

    bool isMachineBigEndian();

    void huInitScanner(huScanner * scanner, huTrove * trove, char const * str, int strLen);
    int getcharLength(char const * cur);
    void nextCharacter(huScanner * cursor);
    void analyzeCharacter(huScanner * cursor);
    void analyzeWhitespace(huScanner * cursor);
    void eatWs(huScanner * cursor, int tabSize, int * line, int * col);
    bool parseInt(huScanner * scanner, int encoding, int * integer);

    void huInitNode(huNode * node, huTrove const * trove);
    void huDestroyNode(huNode const * node);

    huToken * allocNewToken(huTrove * trove, int kind, char const * str, int size, int line, int col, int endLine, int endCol, char quoteChar);
    huNode * allocNewNode(huTrove * trove, int nodeKind, huToken const * firstToken);

    void recordTokenizeError(huTrove * trove, int errorCode, int line, int col);
    void recordError(huTrove * trove, int errorCode, huToken const * pCur);

    int swagEncodingFromString(huStringView const * data, size_t * numBomChars, huLoadParams * loadParams);
    int swagEncodingFromFile(FILE * fp, int fileSize, size_t * numBomChars, huLoadParams * loadParams);
    int transcodeToUtf8FromString(char * dest, size_t * numBytesEncoded, huStringView const * src, huLoadParams * loadParams);
    int transcodeToUtf8FromFile(char * dest, size_t * numBytesEncoded, FILE * fp, int srcLen, huLoadParams * loadParams);

    void huTokenizeTrove(huTrove * trove);
    void huParseTrove(huTrove * trove);

    int min(int a, int b);
    int max(int a, int b);

    // printing token streams and node addresses
    typedef struct PrintTracker_tag
    {
        huTrove const * trove;
        huVector * str;

        huStoreParams * storeParams;

        int currentDepth;
        bool lastPrintWasNewline;
        bool lastPrintWasIndent;
        bool lastPrintWasUnquotedWord;
        bool lastPrintWasWhitespace;
    } PrintTracker;

    void appendString(PrintTracker * printer, char const * addend, int size);
    void appendWs(PrintTracker * printer, int numChars);
    void appendIndent(PrintTracker * printer);
    void appendNewline(PrintTracker * printer);
    void appendColor(PrintTracker * printer, int colorCode);
    void appendColoredString(PrintTracker * printer, char const * addend, int size, int colorCode);
    void appendColoredToken(PrintTracker * printer, huToken const * tok, int colorCode);
    void printForwardComment(PrintTracker * printer, huToken const * tok);
    void printTrailingComment(PrintTracker * printer, huToken const * tok);
    int printAllPrecedingComments(PrintTracker * printer, huNode const * node, huToken const * tok, int startingWith);
    int printAllTrailingComments(PrintTracker * printer, huNode const * node, huToken const * tok, int startingWith);
    void printAnnotations(PrintTracker * printer, huVector const * annotations, bool isTroveAnnotations);
    void printNode(PrintTracker * printer, huNode const * node);
    void troveToPrettyString(huTrove const * trove, huVector * str, huStoreParams * storeParams);

#ifdef __cplusplus
} // extern "C"
#endif
