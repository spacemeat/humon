#include <string.h>
#include <string_view>
#include <unistd.h>
#include "../src/humon.internal.h"
#include <CppUTest/TestHarness.h>
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

TEST(huVectorTests, huInitVectorForCounting)
{
    huVector v;
    huInitVectorForCounting(&v);

    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(0, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    int g = 1;
    auto p = huGrowVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(NULL, p);
    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(1, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    g = 1;
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(NULL, p);
    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(2, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    g = 1000;
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(1000, g);
    POINTERS_EQUAL(NULL, p);
    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(0, v.elementSize);
    LONGS_EQUAL(1002, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);
}

TEST(huVectorTests, huInitVectorPreallocated)
{
    huVector v;
    int numElements = 100;
    char buf[numElements];
    huInitVectorPreallocated(&v, buf, 1, numElements);

    POINTERS_EQUAL(buf, v.buffer);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(0, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    int g = 1;  
    auto p = huGrowVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(v.buffer, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(1, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    g = 1;  
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL((char*) v.buffer + 1, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    g = 2000;
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(98, g);
    POINTERS_EQUAL((char*) v.buffer + 2, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(100, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);

    g = 1;
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(0, g);
    POINTERS_EQUAL(NULL, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(100, v.numElements);
    LONGS_EQUAL(100, v.vectorCapacity);
}

TEST(huVectorTests, huInitGrowableVector)
{
    huVector v;
    huInitGrowableVector(&v, 1);

    POINTERS_EQUAL(NULL, v.buffer);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(0, v.numElements);
    LONGS_EQUAL(0, v.vectorCapacity);

    int g = 1;  
    auto p = huGrowVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL(v.buffer, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(1, v.numElements);
    LONGS_EQUAL(16, v.vectorCapacity);

    g = 1;  
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL((char*) v.buffer + 1, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2, v.numElements);
    LONGS_EQUAL(16, v.vectorCapacity);

    g = 2004;
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(2004, g);
    POINTERS_EQUAL((char*) v.buffer + 2, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2006, v.numElements);
    LONGS_EQUAL(2048, v.vectorCapacity);

    g = 1;
    p = huGrowVector(&v, &g);
    LONGS_EQUAL(1, g);
    POINTERS_EQUAL((char*) v.buffer + 2006, p);
    LONGS_EQUAL(1, v.elementSize);
    LONGS_EQUAL(2007, v.numElements);
    LONGS_EQUAL(2048, v.vectorCapacity);

    huDestroyVector(&v);
}
