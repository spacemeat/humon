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
    vector->allocator = NULL;
}


void initVectorPreallocated(huVector * vector, void * buffer, huSize_t elementSize, huSize_t numElements)
{
    vector->kind = HU_VECTORKIND_PREALLOCATED;
    vector->buffer = buffer;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = numElements;
    vector->allocator = NULL;
}


void initGrowableVector(huVector * vector, huSize_t elementSize, huAllocator const * allocator)
{
    vector->kind = HU_VECTORKIND_GROWABLE;
    vector->buffer = NULL;
    vector->elementSize = elementSize;
    vector->numElements = 0;
    vector->vectorCapacity = 0;
    vector->allocator = allocator;
}


void destroyVector(huVector const * vector)
{
    // you bet your sweet bippy I'm casting away the const
    if (vector->kind == HU_VECTORKIND_GROWABLE &&
        vector->buffer != NULL)
    {
        huVector * ncVector = (huVector *) vector;
        ourFree(ncVector->allocator, ncVector->buffer);
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
        initGrowableVector(vector, vector->elementSize, vector->allocator);
        break;
    }
}


huSize_t getVectorSize(huVector const * vector)
{
    return vector->numElements;
}


void * getVectorElement(huVector const * vector, huSize_t idx)
{
    return vector->buffer + vector->elementSize * idx;
}


void * growVector(huVector * vector, huSize_t * numElements)
{
    char * next = NULL;
    huSize_t numToAppend = * numElements;

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
            huSize_t cap = numToAppend;
            if (cap % 16 != 0)
                { cap = ((numToAppend / 16) + 1) * 16; }

            vector->numElements = numToAppend;
            vector->vectorCapacity = cap;

            vector->buffer = ourAlloc(vector->allocator, vector->vectorCapacity * vector->elementSize);
            next = vector->buffer;
        }
        else
        {
            huSize_t nextOffset = vector->numElements * vector->elementSize;

            // adjust capacity
            huSize_t cap = vector->vectorCapacity;
            while (vector->numElements + numToAppend > vector->vectorCapacity)
                { vector->vectorCapacity *= 2; }

            // if we grew, realloc
            if (vector->vectorCapacity > cap)
            {
                vector->buffer = ourRealloc(vector->allocator, vector->buffer,
                    vector->vectorCapacity * vector->elementSize);
            }

            vector->numElements += numToAppend;
            next = vector->buffer + nextOffset;
        }

        return next;

    default:
        return NULL;
    }
}


huSize_t appendToVector(huVector * vector, void const * data, huSize_t numElements)
{
    huSize_t maxAppend = numElements;
    void * dest = growVector(vector, & maxAppend);
    if (vector->kind != HU_VECTORKIND_COUNTING)
        { memcpy(dest, data, maxAppend * vector->elementSize); }

    return maxAppend;
}


void shrinkVector(huVector * vector, huSize_t numElements)
{
    if (numElements > vector->numElements)
        { numElements = vector->numElements; }

    vector->numElements -= numElements;
}
