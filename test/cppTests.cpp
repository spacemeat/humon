#include <sstream>
#include <string.h>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include "humon.hpp"
#include <CppUTest/TestHarness.h>
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
    hu::Trove trove = std::move(std::get<hu::Trove>(hu::Trove::fromString(humon, {hu::Encoding::utf8, true, 2})));
    auto root = trove.root();
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
    static inline TypeContainer extract(std::string_view valStr)
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

    LONGS_EQUAL_TEXT(1337, hu::val<TypeContainer>::extract("25.25"), "static extract - good");
    LONGS_EQUAL_TEXT(0xbadf00d, hu::val<TypeContainer>::extract("yeehaw"), "static extract - fail");

    LONGS_EQUAL_TEXT(1337, t.trove / 1 / hu::Parent {} / 2 / hu::val<TypeContainer>{}, "custom hu::value good");

    auto spuriousNode = t.trove / "big" / "fat" / 0 / "sloppy" / "wet" / 1;
    CHECK_TEXT(spuriousNode.isNullish(), "wildly wrong path");    
}

TEST(cppSugar, annos)
{
    LONGS_EQUAL(3, m.trove.numAnnotations());
    LONGS_EQUAL(true, m.trove.hasAnnotation("tx"));
    LONGS_EQUAL(true, m.trove.hasAnnotation("ta"));
    LONGS_EQUAL(true, m.trove.hasAnnotation("tb"));
    LONGS_EQUAL(false, m.trove.hasAnnotation("foo"));
    LONGS_EQUAL(2, m.trove.numAnnotationsWithValue("ta"));
    LONGS_EQUAL(1, m.trove.numAnnotationsWithValue("tb"));

    auto tas = m.trove.annotationsWithValue("ta");
    LONGS_EQUAL(2, tas.size());
    LONGS_EQUAL(2, tas[0].str().size());
    MEMCMP_EQUAL("tx", tas[0].str().data(), 2);
    LONGS_EQUAL(2, tas[1].str().size());
    MEMCMP_EQUAL("ta", tas[1].str().data(), 2);
    
    tas = m.trove.annotationsWithValue("tb");
    LONGS_EQUAL(1, tas.size());
    LONGS_EQUAL(2, tas[0].str().size());
    MEMCMP_EQUAL("tb", tas[0].str().data(), 2);

    auto nodes = m.trove.findNodesWithAnnotationKey("a");
    LONGS_EQUAL(1, nodes.size());
    CHECK_EQUAL(m.a, nodes[0]);

    nodes = m.trove.findNodesWithAnnotationKey("b");
    LONGS_EQUAL(2, nodes.size());
    CHECK_EQUAL(m.bp, nodes[0]);
    CHECK_EQUAL(m.b, nodes[1]);

    nodes = m.trove.findNodesWithAnnotationKey("c");
    LONGS_EQUAL(3, nodes.size());
    CHECK_EQUAL(m.cpp, nodes[0]);
    CHECK_EQUAL(m.cp, nodes[1]);
    CHECK_EQUAL(m.c, nodes[2]);

    nodes = m.trove.findNodesWithAnnotationValue("a");
    LONGS_EQUAL(1, nodes.size());
    CHECK_EQUAL(m.a, nodes[0]);

    nodes = m.trove.findNodesWithAnnotationValue("b");
    LONGS_EQUAL(1, nodes.size());
    CHECK_EQUAL(m.b, nodes[0]);

    nodes = m.trove.findNodesWithAnnotationValue("c");
    LONGS_EQUAL(1, nodes.size());
    CHECK_EQUAL(m.c, nodes[0]);

    nodes = m.trove.findNodesWithAnnotationValue("value");
    LONGS_EQUAL(3, nodes.size());
    CHECK_EQUAL(m.a, nodes[0]);
    CHECK_EQUAL(m.b, nodes[1]);
    CHECK_EQUAL(m.c, nodes[2]);

    nodes = m.trove.findNodesWithAnnotationKeyValue("a", "a");
    LONGS_EQUAL(1, nodes.size());
    CHECK_EQUAL(m.a, nodes[0]);

    nodes = m.trove.findNodesWithAnnotationKeyValue("type", "value");
    LONGS_EQUAL(3, nodes.size());
    CHECK_EQUAL(m.a, nodes[0]);
    CHECK_EQUAL(m.b, nodes[1]);
    CHECK_EQUAL(m.c, nodes[2]);
}

TEST(cppSugar, comments)
{
    auto tcs = m.trove.allComments();
    LONGS_EQUAL(2, tcs.size());
    auto str = "// This is a trove comment."sv;
    LONGS_EQUAL(str.size(), std::get<0>(tcs[0]).str().size());
    MEMCMP_EQUAL(str.data(), std::get<0>(tcs[0]).str().data(), str.size());
    str = "// This is also a trove comment."sv;
    LONGS_EQUAL(str.size(), std::get<0>(tcs[1]).str().size());
    MEMCMP_EQUAL(str.data(), std::get<0>(tcs[1]).str().data(), str.size());

    auto nodes = m.trove.findNodesByCommentContaining("This is a ");
    LONGS_EQUAL(3, nodes.size());
    CHECK_EQUAL(m.a, nodes[0]);
    CHECK_EQUAL(m.bp, nodes[1]);
    CHECK_EQUAL(m.cpp, nodes[2]);
}
