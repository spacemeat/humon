#include "humon2c.h"


void huInitVector(huVector_t * vector)
{
  vector->buffer = NULL;
  vector->size = 0;
  vector->capacity = 0;
}


void huDestroyVector(huVector_t * vector)
{
  if (vector->buffer != NULL)
    { free(vector->buffer); }
}


void huResetVector(huVector_t * vector)
{
  huDestroyVector(vector);
  huInitVector(vector);
}


void huGrowVector(huVector_t * vector, int numBytes)
{
  if (vector->buffer == NULL)
  {
    int cap = numBytes;
    if (cap % 64 != 0)
      { cap = ((numBytes / 64) + 1) * 64; }

    vector->capacity = cap;
    vector->size = numBytes;
    vector->buffer = malloc(cap);
  }
  else
  {
    int size = vector->size + numBytes;
    int cap = vector->capacity;
    while (size > vector->capacity)
      { cap *= 2; }

    vector->capacity = cap;
    vector->size = size;
    vector->buffer = realloc(vector->buffer, vector->capacity);
  }
}
