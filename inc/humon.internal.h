#pragma once

#include <stdint.h>
#include "humon.h"
#include "ansiColors.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool stringInString(char const * haystack, int haystackLen, char const * needle, int needleLen);

    void huInitVector(huVector * vector, int elementSize);
    void huDestroyVector(huVector * vector);
    void huResetVector(huVector * vector);
    void * huGrowVector(huVector * vector, int numElements);

    typedef struct cursor_tag
    {
            huTrove * trove;
            char const * character;
            uint8_t codepointLength;
            uint8_t ws_col;            // set if space-like whitespace
            uint8_t ws_line;           // set if newline-like whitespace
    } huCursor;


    int getCodepointLength(char const * cur);
    void nextCharacter(huCursor * cursor);
    void analyzeWhitespace(huCursor * cursor);
    void eatWs(huCursor * cursor, int tabSize, int * line, int * col);

    void huInitNode(huNode * node, huTrove * trove);
    void huDestroyNode(huNode * node);

    huToken * allocNewToken(huTrove * trove, int tokenKind, char const * str, int size, int line, int col, int endLine, int endCol);

    huNode * allocNewNode(huTrove * trove, int nodeKind, huToken * firstToken);

    void recordError(huTrove * trove, int errorCode, huToken * pCur);

    void huTokenizeTrove(huTrove * trove);
    void huParseTrove(huTrove * trove);
    

#ifdef __cplusplus
} // extern "C"
#endif
