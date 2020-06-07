#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


bool stringInString(char const * haystack, int haystackLen, char const * needle, int needleLen)
{
    // I'm unconcerned about O(m*n).
    for (int i = 0; i < haystackLen - needleLen; ++i)
    {
        if (strncmp(haystack + i, needle, needleLen) == 0)
            { return true; }
    }

    return false;
}


char const * huTokenKindToString(int rhs)
{
    switch(rhs)
    {
    case HU_TOKENKIND_EOF: return "eof";
    case HU_TOKENKIND_STARTLIST: return "startList";
    case HU_TOKENKIND_ENDLIST: return "endList";
    case HU_TOKENKIND_STARTDICT: return "startDict";
    case HU_TOKENKIND_ENDDICT: return "endDict";
    case HU_TOKENKIND_KEYVALUESEP: return "keyValueSep";
    case HU_TOKENKIND_ANNOTATE: return "annotate";
    case HU_TOKENKIND_WORD: return "word";
    case HU_TOKENKIND_COMMENT: return "comment";
    default: return "!!unknown!!";
    }
}


char const * huNodeKindToString(int rhs)
{
    switch(rhs)
    {
    case HU_NODEKIND_NULL: return "null";
    case HU_NODEKIND_LIST: return "list";
    case HU_NODEKIND_DICT: return "dict";
    case HU_NODEKIND_VALUE: return "value";
    default: return "!!unknown!!";
    }
}

    char const * huOutputFormatToString(int rhs)
{
    switch(rhs)
    {
    case HU_OUTPUTFORMAT_PRESERVED: return "preserved";
    case HU_OUTPUTFORMAT_MINIMAL: return "minimal";
    case HU_OUTPUTFORMAT_PRETTY: return "pretty";
    default: return "!!unknown!!";
    }
}

char const * huOutputErrorToString(int rhs)
{
    switch(rhs)
    {
    case HU_ERROR_UNEXPECTED_EOF: return "unexpected EOF";
    case HU_ERROR_SYNTAX_ERROR: return "syntax error";
    case HU_ERROR_START_END_MISMATCH: return "start/end mismatch";
    default: return "!!unknown!!";
    }
}


void huInitVector(huVector * vector, int elementSize)
{
    vector->buffer = NULL;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = 0;
}


void huDestroyVector(huVector const * vector)
{
    // you bet your sweet bippy I'm casting away the const
    if (vector->buffer != NULL)
        { free((void *) vector->buffer); }
}


void huResetVector(huVector * vector)
{
    huDestroyVector(vector);
    huInitVector(vector, vector->elementSize);
}


int huGetVectorSize(huVector const * vector)
{
    return vector->numElements;
}


void * huGetVectorElement(huVector const * vector, int idx)
{
    return vector->buffer + vector->elementSize * idx;
}


void * huGrowVector(huVector * vector, int numElements)
{
    if (vector->numElements == 0)
    {
        // round up to a group of 16 elements
        int cap = numElements;
        if (cap % 16 != 0)
            { cap = ((numElements / 16) + 1) * 16; }

        vector->vectorCapacity = cap;
        vector->numElements = numElements;
        vector->buffer = malloc(cap * vector->elementSize);

        return vector->buffer;
    }
    else
    {
        vector->numElements += numElements;
        int cap = vector->vectorCapacity;
        while (vector->numElements > cap)
            { cap *= 2; }

        if (cap > vector->vectorCapacity)
        {
            vector->vectorCapacity = cap;
            vector->buffer = realloc(vector->buffer, cap * vector->elementSize);
            if (vector->buffer == NULL)
                { return NULL; }
        }

        return vector->buffer + (vector->numElements - numElements) * vector->elementSize;
    }
}


void huDestroyStringView(huStringView const * string)
{
    // here too, ya hoser
    if (string->str != NULL)
        { free((void *) string->str); }
}


huToken humon_nullToken = 
{
    .tokenKind = HU_TOKENKIND_NULL,
    .value = {
        .str = "",
        .size = 0
    },
    .line = 0,
    .col = 0,
    .endLine = 0,
    .endCol = 0
};

huNode humon_nullNode =
{
    .trove = & humon_nullTrove,
    .nodeIdx = -1,
    .kind = HU_NODEKIND_NULL,
    .firstToken = & humon_nullToken,
    .keyToken = & humon_nullToken,
    .firstValueToken = & humon_nullToken,
    .lastValueToken = & humon_nullToken,
    .lastToken = & humon_nullToken,
    .childIdx = 0,
    .parentNodeIdx = -1,
    .childNodeIdxs = (huVector) {
        .buffer = NULL,
        .elementSize = sizeof(int),
        .numElements = 0,
        .vectorCapacity = 0
    },
    .childDictKeys = (huVector) {
        .buffer = NULL,
        .elementSize = sizeof(huDictEntry),
        .numElements = 0,
        .vectorCapacity = 0
    },
    .annotations = (huVector) {
        .buffer = NULL,
        .elementSize = sizeof(huAnnotation),
        .numElements = 0,
        .vectorCapacity = 0
    },
    .comments = (huVector) {
        .buffer = NULL,
        .elementSize = sizeof(huComment),
        .numElements = 0,
        .vectorCapacity = 0
    }
};

huTrove humon_nullTrove = 
{
    .dataStringSize = 0,
    .dataString = "",
    .tokens = (huVector) {
        .buffer = & humon_nullToken,
        .elementSize = 0,
        .numElements = 0,
        .vectorCapacity = 0
    },
    .nodes = (huVector) {
        .buffer = & humon_nullNode,
        .elementSize = 0,
        .numElements = 0,
        .vectorCapacity = 0
    },
    .errors = (huVector) {
        .buffer = NULL,
        .elementSize = 0,
        .numElements = 0,
        .vectorCapacity = 0
    },
    .inputTabSize = 4,
    .outputTabSize = 4,
    .annotations = (huVector) {
        .buffer = NULL,
        .elementSize = 0,
        .numElements = 0,
        .vectorCapacity = 0
    },
    .comments = (huVector) {
        .buffer = NULL,
        .elementSize = 0,
        .numElements = 0,
        .vectorCapacity = 0
    }
};
