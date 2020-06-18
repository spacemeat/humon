#include <sstream>
#include <string.h>
#include <string_view>
#include <unistd.h>
#include "humon.hpp"
#include "ansiColors.h"
#include <CppUTest/TestHarness.h>

#include "testDataCpp.h"

using namespace std::literals;

SimpleString StringFrom(const hu::Trove & other)
{
    std::stringstream ss;
    ss << "{type: trove tokens:" << other.getNumTokens() << " nodes:" << other.getNumNodes() << "}";
    return SimpleString(ss.str().data());
}

SimpleString StringFrom(const hu::Node & other)
{
    std::stringstream ss;
    ss << "{type: node address:" << other.getAddress() << "}";
    return SimpleString(ss.str().data());
}

TEST_GROUP(makers)
{
    std::string_view humon;

    void setup()
    {
        humon = R"([woo])"sv;
    }

    void teardown()
    {
    }
};

TEST(makers, fromString)
{
    hu::Trove trove = hu::Trove::fromString(humon, 2);
    auto root = trove.getRootNode();
    CHECK_TEXT(root.getKind() == hu::NodeKind::list, "load0");
    CHECK_TEXT(root.getNumChildren() == 1, "load1");
    auto child = root.getChild(0);
    CHECK_TEXT(child.getKind() == hu::NodeKind::value, "load2");
    CHECK_TEXT(child.getValue() == "woo", "load3");
}

// TODO: tests for fromFile and fromStream

TEST_GROUP(cppSugar)
{
    htd_cppValues t;
    htd_cppGeneralMix m;

    void setup()
    {
        t.setup();
        m.setup();
    }

    void teardown()
    {
        t.teardown();
        m.teardown();
    }
};

TEST(cppSugar, sugar)
{
    auto tn = m.a; auto ttn = m.trove / 0;
    CHECK_EQUAL_TEXT(tn, ttn, "/a");
    tn = m.b; ttn = m.trove / 1 / 0;
    CHECK_EQUAL_TEXT(tn, ttn, "/b");
    tn = m.c; ttn = m.trove / 2 / 0 / 0;
    CHECK_EQUAL_TEXT(tn, ttn, "/c");

    auto nint = t.trove / 0 / hu::value<int>{};
    LONGS_EQUAL_TEXT(213, nint, "int");
    auto nfloat = t.trove / 1 / hu::value<float>{};
    CHECK_EQUAL_TEXT(25.25f, nfloat, "float");
    auto ndouble = t.trove / 2 / hu::value<double>{};
    DOUBLES_EQUAL_TEXT(25.25, ndouble, 0.0, "double");
    auto nstring = t.trove / 3 / hu::value<std::string>{};
    STRNCMP_EQUAL_TEXT("foo", nstring.data(), 3, "string");
    auto nbool = t.trove / 4 / hu::value<bool>{};
    LONGS_EQUAL_TEXT(true, nbool, "bool");

    CHECK_EQUAL_TEXT(t.root, t.trove.getNode("/"), "gn /");
    auto valsv = t.root.getValue();
    LONGS_EQUAL_TEXT(t.ts.size() - 1, valsv.size(), "compound value size");
    STRNCMP_EQUAL_TEXT(t.ts.data(), valsv.data(), valsv.size(), "compound value");

    
}
