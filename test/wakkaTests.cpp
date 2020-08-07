#define HUMON_SUPPRESS_NOEXCEPT

#include <sstream>
#include <string.h>
#include <string_view>
#include <iostream>
#ifdef _MSC_VER
#else
#include <unistd.h>
#endif
#include "humon.hpp"
#include <CppUTest/TestHarness.h>
#include "testDataCpp.h"

using namespace std::literals;


TEST_GROUP(wakka)
{
    htd_wakka w;

    void setup()
    {
        w.setup();
    }

    void teardown()
    {
        w.teardown();
    }
};

TEST(wakka, tokens)
{
    auto n = w.trove / 0;
    auto k = "\\key"sv;
    auto v = "\\val"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 1;
    k = "k\\ey"sv;
    v = "v\\al"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 2;
    k = "key\\ "sv;
    v = "val\\ "sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 3;
    k = "k\\\"ey"sv;
    v = "v\\\"al"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 4;
    k = "\\\"key"sv;
    v = "\\\"val"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 5;
    k = "key\\ key"sv;
    v = "val\\ val"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 6;
    k = "key\\{key\\}"sv;
    v = "val\\{val\\}"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 7;
    k = "key\\:"sv;
    v = "val\\:"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());

    n = w.trove / 8;
    k = u8"\\🤔"sv;
    v = u8"\\∑"sv;
    MEMCMP_EQUAL(k.data(), n.key().str().data(), k.size());
    MEMCMP_EQUAL(v.data(), n.value().str().data(), v.size());
}
