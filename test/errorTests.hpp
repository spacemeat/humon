#define HUMON_SUPPRESS_NOEXCEPT

#include <sstream>
#include <string.h>
#include <string_view>
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include <iostream>
#include "humon/humon.hpp"
#include "ztest/ztest.hpp"
#include "testDataCpp.h"

using namespace std::literals;

TEST_GROUP(errors)
{
    htd_errors e;

    void setup()
    {
        e.setup();
    }

    void teardown()
    {
        e.teardown();
    }
};


TEST(errors, sugar)
{
    LONGS_EQUAL_TEXT(1, e.trove1.numErrors(), "t1 num errors");
    LONGS_EQUAL_TEXT(1, e.trove2.numErrors(), "t2 num errors");
    LONGS_EQUAL_TEXT(1, e.trove3.numErrors(), "t3 num errors");
    LONGS_EQUAL_TEXT(1, e.trove4.numErrors(), "t4 num errors");
}

