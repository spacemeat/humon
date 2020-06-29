#pragma once

#include <stdint.h>
#include "humon.h"
#include "ansiColors.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool stringInString(char const * haystack, int haystackLen, char const * needle, int needleLen);

    /// Initializes a vector to zero size. Does not allocate.
    void huInitVector(huVector * vector, int elementSize);
    /// Initializes a vector with a preallocated buffer. Can still grow.
    void huInitVectorPreallocated(huVector * vector, int elementSize, void * buffer, int numElements, bool canStillGrow);

    /// Frees the memory owned by a huVector.
    void huDestroyVector(huVector const * vector);
    /// Returns the number of elements in a huVector.
    int huGetVectorSize(huVector const * vector);
    /// Returns a pointer to an element in a huVector.
    void * huGetVectorElement(huVector const * vector, int idx);

    void huResetVector(huVector * vector);
    int huAppendToVector(huVector * vector, void const * data, int numElements);
    void * huGrowVector(huVector * vector, int * numElements);

    typedef struct cursor_tag
    {
            huTrove * trove;
            char const * character;
            uint8_t charLength;
            uint8_t ws_col;            // boolean - set if space-like whitespace
            uint8_t ws_line;           // boolean - set if newline-like whitespace
    } huCursor;


    int getcharLength(char const * cur);
    void nextCharacter(huCursor * cursor);
    void analyzeWhitespace(huCursor * cursor);
    void eatWs(huCursor * cursor, int tabSize, int * line, int * col);

    void huInitNode(huNode * node, huTrove const * trove);
    void huDestroyNode(huNode const * node);

    huToken * allocNewToken(huTrove * trove, int kind, char const * str, int size, int line, int col, int endLine, int endCol, char quoteChar);
    huNode * allocNewNode(huTrove * trove, int nodeKind, huToken const * firstToken);

    void recordTokenizeError(huTrove * trove, int errorCode, int line, int col);
    void recordError(huTrove * trove, int errorCode, huToken const * pCur);

    void huTokenizeTrove(huTrove * trove);
    void huParseTrove(huTrove * trove);

    int min(int a, int b);
    int max(int a, int b);

    // printing token streams and node addresses
    typedef struct PrintTracker_tag
    {
        huTrove const * trove;
        huVector * str;

        int format;
        bool printComments;
        int tabSize;
        char const * newline;
        int newlineSize;
        huStringView const * colorTable;

        int currentDepth;
        int currentLine;
        int currentCol;
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
    void troveToPrettyString(huTrove const * trove, huVector * str, int outputFormat, bool printComments, int outputTabSize, char const * newline, int newlineSize, huStringView const * colorTable);


    /*
    void appendString(huVector * str, char const * addend, int size);
    void appendWs(huVector * str, int numChars);
    void appendColor(huVector * str, huStringView const * colorTable, int colorCode);
    void endColor(huVector * str, huStringView const * colorTable);
    void appendColoredString(huVector * str, char const * addend, int size, huStringView const * colorTable, int colorCode);
    void printComment(huToken const * comment, huVector * str, char const * newline, int newlineSize, huStringView const * colorTable);
    int printSameLineComments(huNode const * node, bool firstToken, int startingCommentIdx, huVector * str, char const * newline, int newlineSize, huStringView const* colorTable);
    void printAnnotations(huAnnotation const * annos, int numAnno, bool troveOwned, huVector * str, huStringView const * colorTable);
    void printTroveAnnotations(huTrove const * trove, huVector * str, huStringView const * colorTable);
    void printNodeAnnotations(huNode const * node, huVector * str, huStringView const * colorTable);
    void printKey(huToken const * keyToken, huVector * str, huStringView const * colorTable);
    void printValue(huToken const * valueToken, huVector * str, huStringView const * colorTable);
    void troveToPrettyStringRec(huNode const * node, huVector * str, int depth, int outputFormat, bool excludeComments, int outputTabSize, char const * newline, int newlineSize, huStringView const * colorTable);
    void troveToPrettyString(huTrove const * trove, huVector * str, int outputFormat, bool excludeComments, int outputTabSize, char const * newline, int newlineSize, huStringView const * colorTable);
    */

#ifdef __cplusplus
} // extern "C"
#endif
