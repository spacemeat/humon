#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


bool stringInString(char const * haystack, int haystackLen, char const * needle, int needleLen)
{
    // I'm unconcerned about O(m*n).
    for (int i = 0; i < haystackLen - needleLen + 1; ++i)
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
    case HU_TOKENKIND_NULL: return "null";
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
    case HU_ERROR_NOERROR: return "no error";
    case HU_ERROR_UNFINISHEDQUOTE: return "unfinished quote";
    case HU_ERROR_UNFINISHEDCSTYLECOMMENT: return "unfinished C-style comment";
    case HU_ERROR_UNEXPECTEDEOF: return "unexpected EOF";
    case HU_ERROR_TOOMANYROOTS: return "too many roots";
    case HU_ERROR_NONUNIQUEKEY: return "non-unique key";
    case HU_ERROR_SYNTAXERROR: return "syntax error";
    case HU_ERROR_NOTFOUND: return "not found";    
    case HU_ERROR_ILLEGAL: return "illegal operation";
    default: return "!!unknown!!";
    }
}


void huInitVector(huVector * vector, int elementSize)
{
    vector->buffer = NULL;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = 0;
    vector->canGrowCapacity = true;
}


void huInitVectorPreallocated(huVector * vector, int elementSize, void * buffer, int numElements, bool canStillGrow)
{
    vector->buffer = buffer;
    vector->elementSize = elementSize;
    vector->numElements = 0;

    int cap = numElements;
    vector->vectorCapacity = cap;
    vector->canGrowCapacity = canStillGrow;
}


void huDestroyVector(huVector const * vector)
{
    huVector * ncVector = (huVector *) vector;
    // you bet your sweet bippy I'm casting away the const
    if (ncVector->buffer != NULL)
    {
        free(ncVector->buffer);
        ncVector->buffer = NULL;
        ncVector->numElements = 0;
        ncVector->vectorCapacity = 0;
    }
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


int min(int a, int b) { if (a < b) { return a; } else { return b; } }
int max(int a, int b) { if (a >= b) { return a; } else { return b; } }


void * huGrowVector(huVector * vector, int * numElements)
{
    // get remaining capacity
    int maxAppend = * numElements;
    if (vector->canGrowCapacity == false)
        { maxAppend = min(maxAppend, vector->vectorCapacity - vector->numElements); }

    * numElements = 0;

    if (vector->numElements == 0)
    {
        vector->numElements = maxAppend;

        // round up to a group of 16 elements
        int cap = maxAppend;
        if (vector->canGrowCapacity)
        {
            if (cap % 16 != 0)
                { cap = ((maxAppend / 16) + 1) * 16; }
        }

        // capacity can be set even if numElements is 0.
        if (cap > vector->vectorCapacity)
        {
            if (vector->buffer)
                { free(vector->buffer); vector->buffer = NULL; }

            vector->vectorCapacity = cap;

            // If elementSize is 0, we're just counting, no malloc
            if (vector->elementSize > 0)
                { vector->buffer = malloc(cap * vector->elementSize); }
        }

        * numElements = maxAppend;
        return vector->buffer;
    }
    else
    {
        vector->numElements += maxAppend;
        int cap = vector->vectorCapacity;
        while (vector->numElements > cap)
            { cap *= 2; }

        if (cap > vector->vectorCapacity)
        {
            vector->vectorCapacity = cap;
            if (vector->elementSize > 0)
                { vector->buffer = realloc(vector->buffer, cap * vector->elementSize); }
            if (vector->buffer == NULL)
                { return NULL; }
        }

        * numElements = maxAppend;
        return vector->buffer + (vector->numElements - maxAppend) * vector->elementSize;
    }
}


int huAppendToVector(huVector * vector, void const * data, int numElements)
{
    int maxAppend = numElements;
    void * dest = huGrowVector(vector, & maxAppend);
    memcpy(dest, data, maxAppend * vector->elementSize);

    return maxAppend;
}


huToken const * hu_nullToken = NULL;
huNode const * hu_nullNode = NULL;
huTrove const * hu_nullTrove = NULL;
