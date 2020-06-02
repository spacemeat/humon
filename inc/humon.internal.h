#pragma once

#include <stdint.h>
#include "humon.h"
#include "ansiColors.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool stringInString(char const * haystack, int haystackLen, char const * needle, int needleLen);

    void huInitVector(huVector_t * vector, int elementSize);
    void huDestroyVector(huVector_t * vector);
    void huResetVector(huVector_t * vector);
    void * huGrowVector(huVector_t * vector, int numElements);

    typedef struct cursor
    {
            huTrove_t * trove;
            char const * character;
            uint8_t codepointLength;
            uint8_t ws_col;            // set if space-like whitespace
            uint8_t ws_line;           // set if newline-like whitespace
    } cursor_t;


    int getCodepointLength(char const * cur);
    void nextCharacter(cursor_t * cursor);
    void analyzeWhitespace(cursor_t * cursor);
    void eatWs(cursor_t * cursor, int tabSize, int * line, int * col);

    void huInitNode(huNode_t * node, huTrove_t * trove);
    void huDestroyNode(huNode_t * node);

    huToken_t * allocNewToken(huTrove_t * trove, int tokenKind, char const * str, int size, int line, int col, int endLine, int endCol);

    huNode_t * allocNewNode(huTrove_t * trove, int nodeKind, huToken_t * firstToken);

    void recordError(huTrove_t * trove, int errorCode, huToken_t * pCur);

    void huTokenizeTrove(huTrove_t * trove);
    void huParseTrove(huTrove_t * trove);
    

#ifdef __cplusplus
} // extern "C"
#endif
