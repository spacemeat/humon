#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


void initVectorForCounting(huVector * vector)
{
    vector->kind = HU_VECTORKIND_COUNTING;
    vector->buffer = NULL;
    vector->elementSize = 0;    // <- This is how we know we're counting only
    vector->numElements = 0;
    vector->vectorCapacity = 0;
}


void initVectorPreallocated(huVector * vector, void * buffer, huIndexSize_t elementSize, huIndexSize_t numElements)
{
    vector->kind = HU_VECTORKIND_PREALLOCATED;
    vector->buffer = buffer;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = numElements;
}


void initGrowableVector(huVector * vector, huIndexSize_t elementSize)
{
    vector->kind = HU_VECTORKIND_GROWABLE;
    vector->buffer = NULL;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = 0;
}


void destroyVector(huVector const * vector)
{
    // you bet your sweet bippy I'm casting away the const    
    if (vector->kind == HU_VECTORKIND_GROWABLE &&
        vector->buffer != NULL)
    {
        huVector * ncVector = (huVector *) vector;
        free(ncVector->buffer);
    }
}


void resetVector(huVector * vector)
{
    switch(vector->kind)
    {
    case HU_VECTORKIND_COUNTING:
        initVectorForCounting(vector);
        break;
    case HU_VECTORKIND_PREALLOCATED:
        initVectorPreallocated(vector, vector->buffer, vector->elementSize, vector->numElements);
        break;
    case HU_VECTORKIND_GROWABLE:
        destroyVector(vector);    // doesn't change .elementSize
        initGrowableVector(vector, vector->elementSize);
        break;
    }
}


huIndexSize_t getVectorSize(huVector const * vector)
{
    return vector->numElements;
}


void * getVectorElement(huVector const * vector, huIndexSize_t idx)
{
    return vector->buffer + vector->elementSize * idx;
}


void * growVector(huVector * vector, huIndexSize_t * numElements)
{
    char * next = NULL;
    huIndexSize_t numToAppend = * numElements;

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
            huIndexSize_t cap = numToAppend;
            if (cap % 16 != 0)
                { cap = ((numToAppend / 16) + 1) * 16; }

            vector->numElements = numToAppend;
            vector->vectorCapacity = cap;

            vector->buffer = malloc(vector->vectorCapacity * vector->elementSize);
            next = vector->buffer;
        }
        else
        {
            huIndexSize_t nextOffset = vector->numElements * vector->elementSize;

            // adjust capacity
            huIndexSize_t cap = vector->vectorCapacity;
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


huIndexSize_t appendToVector(huVector * vector, void const * data, huIndexSize_t numElements)
{
    huIndexSize_t maxAppend = numElements;
    void * dest = growVector(vector, & maxAppend);
    if (vector->kind != HU_VECTORKIND_COUNTING)
        { memcpy(dest, data, maxAppend * vector->elementSize); }

    return maxAppend;
}
