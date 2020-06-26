#include <sstream>
#include <string.h>
#include <string_view>
#include <unistd.h>
#include "humon.hpp"
#include "ansiColors.h"
#include <CppUTest/TestHarness.h>

#include <iostream>

#include "testDataCpp.h"

using namespace std::literals;

SimpleString StringFrom(const hu::Trove & other)
{
    std::stringstream ss;
    ss << "{type: trove tokens:" << other.numTokens() << " nodes:" << other.numNodes() << "}";
    return SimpleString(ss.str().data());
}

SimpleString StringFrom(const hu::Node & other)
{
    std::stringstream ss;
    ss << "{type: node address:" << other.address() << "}";
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
    auto root = trove.rootNode();
    CHECK_TEXT(root.kind() == hu::NodeKind::list, "load0");
    CHECK_TEXT(root.numChildren() == 1, "load1");
    auto child = root.child(0);
    CHECK_TEXT(child.kind() == hu::NodeKind::value, "load2");
    CHECK_TEXT(child.value() == "woo"sv, "load3");
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


class TypeContainer
{
public:
    int type;

    operator int()
        { return type; }
};

template <>
struct hu::val<TypeContainer>
{
    inline TypeContainer extract(std::string_view valStr)
    {
        if (valStr == "25.25")
            { return {1337}; }
        else
            { return {0xbadf00d}; }
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

    auto nint = t.trove / 0 / hu::val<int>{};
    LONGS_EQUAL_TEXT(213, nint, "int");
    auto nfloat = t.trove / 1 / hu::val<float>{};
    CHECK_EQUAL_TEXT(25.25f, nfloat, "float");
    auto ndouble = t.trove / 2 / hu::val<double>{};
    DOUBLES_EQUAL_TEXT(25.25, ndouble, 0.0, "double");
    auto nstring = t.trove / 3 / hu::val<std::string>{};
    STRNCMP_EQUAL_TEXT("foo", nstring.data(), 3, "string");
    auto nbool = t.trove / 4 / hu::val<bool>{};
    LONGS_EQUAL_TEXT(true, nbool, "bool");

    CHECK_TEXT(m.bp % 0, "bp has child");
    CHECK_TEXT(false == m.b % 0, "b has no child");

    LONGS_EQUAL_TEXT(1337, t.trove / 2 / hu::val<TypeContainer>{}, "custom hu::value good");
    LONGS_EQUAL_TEXT(0xbadf00d, t.trove / 0 / hu::val<TypeContainer>{}, "custom hu::value no good");


}
