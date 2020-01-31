#include <stdlib.h>
#include "humon.internal.h"


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



void huInitVector(huVector_t * vector, int elementSize)
{
  vector->buffer = NULL;
  vector->elementSize = elementSize;
  vector->numElements = 0;
  vector->elementCapacity = 0;
}


void huDestroyVector(huVector_t * vector)
{
  if (vector->buffer != NULL)
    { free(vector->buffer); }
}


void huResetVector(huVector_t * vector)
{
  huDestroyVector(vector);
  huInitVector(vector, vector->elementSize);
}


int huGetVectorSize(huVector_t * vector)
{
  return vector->numElements;
}


void * huGetElement(huVector_t * vector, int idx)
{
  return vector->buffer + vector->elementSize * idx;
}


void * huGrowVector(huVector_t * vector, int numElements)
{
  if (vector->numElements == 0)
  {
    // round up to a group of 16 elements
    int cap = numElements;
    if (cap % 16 != 0)
      { cap = ((numElements / 16) + 1) * 16; }

    vector->elementCapacity = cap;
    vector->numElements = numElements;
    vector->buffer = malloc(cap * vector->elementSize);

    return vector->buffer;
  }
  else
  {
    vector->numElements += numElements;
    int cap = vector->elementCapacity;
    while (vector->numElements > cap)
      { cap *= 2; }

    if (cap > vector->elementCapacity)
    {
      vector->elementCapacity = cap;
      vector->buffer = realloc(vector->buffer, cap * vector->elementSize);
    }

    return vector->buffer + (vector->numElements - numElements) * vector->elementSize;
  }
}
