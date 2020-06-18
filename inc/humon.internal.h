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
    void huInitVectorPreallocated(huVector * vector, int elementSize, void * buffer, int numElements);

    /// Frees the memory owned by a huVector.
    void huDestroyVector(huVector const * vector);
    /// Returns the number of elements in a huVector.
    int huGetVectorSize(huVector const * vector);
    /// Returns a pointer to an element in a huVector.
    void * huGetVectorElement(huVector const * vector, int idx);

    void huResetVector(huVector * vector);
    void * huGrowVector(huVector * vector, int numElements);

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

    huToken * allocNewToken(huTrove * trove, int tokenKind, char const * str, int size, int line, int col, int endLine, int endCol);

    huNode * allocNewNode(huTrove * trove, int nodeKind, huToken * firstToken);

    void recordError(huTrove * trove, int errorCode, huToken const * pCur);

    void huTokenizeTrove(huTrove * trove);
    void huParseTrove(huTrove * trove);

    // printing
    void appendString(huVector * str, char const * addend, int size);
    void appendWs(huVector * str, int numChars);
    void appendColor(huVector * str, huStringView const * colorTable, int colorCode);
    void endColor(huVector * str, huStringView const * colorTable);
    void appendColoredString(huVector * str, char const * addend, int size, huStringView const * colorTable, int colorCode);
    void printComment(huToken const * comment, huVector * str, huStringView const * colorTable);
    int printSameLineComments(huNode const * node, bool firstToken, int startingCommentIdx, huVector * str, huStringView const* colorTable);
    void printAnnotations(huAnnotation const * annos, int numAnno, bool troveOwned, huVector * str, huStringView const * colorTable);
    void printTroveAnnotations(huTrove const * trove, huVector * str, huStringView const * colorTable);
    void printNodeAnnotations(huNode const * node, huVector * str, huStringView const * colorTable);
    void printKey(huToken const * keyToken, huVector * str, huStringView const * colorTable);
    void printValue(huToken const * valueToken, huVector * str, huStringView const * colorTable);
    void troveToPrettyStringRec(huNode const * node, huVector * str, int depth, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable);
    void troveToPrettyString(huTrove const * trove, huVector * str, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable);





#ifdef __cplusplus
} // extern "C"
#endif
