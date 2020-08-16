#include <string.h>
#include <string_view>
#ifdef _MSC_VER
#else
#include <unistd.h>
#endif
#include "../src/humon.internal.h"
#include "utest.hpp"
#include "testData.h"


TEST_GROUP(huVectorTests)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(huVectorTests, initVectorForCounting)
{
    huVector v;
    initVectorForCounting(&v);

    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(0, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    huIndexSize_t g = 1;
    auto p = growVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(NULL, p);
    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(1, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    g = 1;
    p = growVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(NULL, p);
    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(2, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    g = 1000;
    p = growVector(&v, &g);
    LONGS_EQUAL(1000, g);
    POINTERS_EQUAL(NULL, p);
    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(1002, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);
}

TEST(huVectorTests, initVectorPreallocated)
{
    huVector v;
    huIndexSize_t const numElements = 100;
    char buf[numElements];
    initVectorPreallocated(&v, buf, 1, numElements);

    POINTERS_EQUAL(buf, v.buffer);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(0, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    huIndexSize_t g = 1;  
    auto p = growVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(v.buffer, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(1, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    g = 1;  
    p = growVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL((char*) v.buffer + 1, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    g = 2000;
    p = growVector(&v, &g);
    LONGS_EQUAL(98, g);
    POINTERS_EQUAL((char*) v.buffer + 2, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(100, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    g = 1;
    p = growVector(&v, &g);
    LONGS_EQUAL(0, g);
    POINTERS_EQUAL(NULL, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(100, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);
}

TEST(huVectorTests, initGrowableVector)
{
    huVector v;
    initGrowableVector(&v, 1);

    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(0, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    huIndexSize_t g = 1;  
    auto p = growVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(v.buffer, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(1, v.numElements);
    LONGS_EQUAL(16, v.vectorCapacity);

    g = 1;  
    p = growVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL((char*) v.buffer + 1, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2, v.numElements);
    LONGS_EQUAL(16, v.vectorCapacity);

    g = 2004;
    p = growVector(&v, &g);
    LONGS_EQUAL(2004, g);
    POINTERS_EQUAL((char*) v.buffer + 2, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2006, v.numElements);
    LONGS_EQUAL(2048, v.vectorCapacity);

    g = 1;
    p = growVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL((char*) v.buffer + 2006, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2007, v.numElements);
    LONGS_EQUAL(2048, v.vectorCapacity);

    destroyVector(&v);
}
