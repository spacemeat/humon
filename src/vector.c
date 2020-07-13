#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


void huInitVectorForCounting(huVector * vector)
{
    vector->kind = HU_VECTORKIND_COUNTING;
    vector->buffer = NULL;
    vector->elementSize = 0;    // <- This is how we know we're counting only
    vector->numElements = 0;
    vector->vectorCapacity = 0;
}


void huInitVectorPreallocated(huVector * vector, void * buffer, int elementSize, int numElements)
{
    vector->kind = HU_VECTORKIND_PREALLOCATED;
    vector->buffer = buffer;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = numElements;
}


void huInitGrowableVector(huVector * vector, int elementSize)
{
    vector->kind = HU_VECTORKIND_GROWABLE;
    vector->buffer = NULL;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = 0;
}


void huDestroyVector(huVector const * vector)
{
    // you bet your sweet bippy I'm casting away the const    
    if (vector->kind == HU_VECTORKIND_GROWABLE &&
        vector->buffer != NULL)
    {
        huVector * ncVector = (huVector *) vector;
        free(ncVector->buffer);
    }
}


void huResetVector(huVector * vector)
{
    switch(vector->kind)
    {
    case HU_VECTORKIND_COUNTING:
        huInitVectorForCounting(vector);
        break;
    case HU_VECTORKIND_PREALLOCATED:
        huInitVectorPreallocated(vector, vector->buffer, vector->elementSize, vector->numElements);
        break;
    case HU_VECTORKIND_GROWABLE:
        huDestroyVector(vector);    // doesn't change .elementSize
        huInitGrowableVector(vector, vector->elementSize);
        break;
    }
}


int huGetVectorSize(huVector const * vector)
{
    return vector->numElements;
}


void * huGetVectorElement(huVector const * vector, int idx)
{
    return vector->buffer + vector->elementSize * idx;
}


void * huGrowVector(huVector * vector, int * numElements)
{
    char * next = NULL;
    int numToAppend = * numElements;

    switch(vector->kind)
    {
    case HU_VECTORKIND_COUNTING:
        vector->numElements += * numElements;
        return NULL;

    case HU_VECTORKIND_PREALLOCATED:
        next = vector->buffer + vector->numElements * vector->elementSize;
        numToAppend = min(* numElements, vector->vectorCapacity - vector->numElements);
        * numElements = numToAppend;
        vector->numElements += numToAppend;
        return numToAppend > 0 ? next : NULL;

    case HU_VECTORKIND_GROWABLE:
        if (vector->buffer == NULL)
        {
            // round up capacity to nearest multiple of 16
            int cap = numToAppend;
            if (cap % 16 != 0)
                { cap = ((numToAppend / 16) + 1) * 16; }

            vector->numElements = numToAppend;
            vector->vectorCapacity = cap;

            vector->buffer = malloc(vector->vectorCapacity * vector->elementSize);
            next = vector->buffer;
        }
        else
        {
            int nextOffset = vector->numElements * vector->elementSize;

            // adjust capacity
            int cap = vector->vectorCapacity;
            while (vector->numElements + numToAppend > vector->vectorCapacity)
                { vector->vectorCapacity *= 2; }

            // if we grew, realloc
            if (vector->vectorCapacity > cap)
                { vector->buffer = realloc(vector->buffer, vector->vectorCapacity * vector->elementSize); }

            vector->numElements += numToAppend;
            next = vector->buffer + nextOffset;
        }

        return next;
    
    default:
        return NULL;
    }
}


int huAppendToVector(huVector * vector, void const * data, int numElements)
{
    int maxAppend = numElements;
    void * dest = huGrowVector(vector, & maxAppend);
    if (vector->kind != HU_VECTORKIND_COUNTING)
        { memcpy(dest, data, maxAppend * vector->elementSize); }

    return maxAppend;
}
