#define HUMON_SUPPRESS_NOEXCEPT

#include <sstream>
#include <string.h>
#include <string_view>
#include <iostream>
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include "humon/humon.hpp"
#include "ztest/ztest.hpp"
#include "testDataCpp.h"

using namespace std::literals;


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

TEST(makers, fromStringFast)
{
    hu::Trove trove = std::move(std::get<hu::Trove>(hu::Trove::fromString(humon, {hu::Encoding::utf8, false, 2})));
    auto root = trove.root();
    CHECK_TEXT(root.kind() == hu::NodeKind::list, "load0");
    CHECK_TEXT(root.numChildren() == 1, "load1");
    auto child = root.child(0);
    CHECK_TEXT(child.kind() == hu::NodeKind::value, "load2");
    CHECK_TEXT(child.value() == "woo"sv, "load3");
}

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
    static inline TypeContainer extract(hu::Node const & val)
    {
        auto const & valStr = val.value().str();
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

    auto nint = t.trove / 0 % hu::val<int>{};
    LONGS_EQUAL_TEXT(213, nint, "int");
    auto nfloat = t.trove / 1 % hu::val<float>{};
    CHECK_EQUAL_TEXT(25.25f, nfloat, "float");
    auto ndouble = t.trove / 2 % hu::val<double>{};
    DOUBLES_EQUAL_TEXT(25.25, ndouble, 0.0, "double");
    auto nstring = t.trove / 3 % hu::val<std::string>{};
    STRNCMP_EQUAL_TEXT("foo", nstring.data(), 3, "string");
    auto nbool = t.trove / 4 % hu::val<bool>{};
    LONGS_EQUAL_TEXT(true, nbool, "bool");

    CHECK_TEXT(m.bp % 0, "bp has child");
    CHECK_TEXT(false == m.b % 0, "b has no child");

    LONGS_EQUAL_TEXT(1337, t.trove / 2 % hu::val<TypeContainer>{}, "custom hu::value good");
    LONGS_EQUAL_TEXT(0xbadf00d, t.trove / 0 % hu::val<TypeContainer>{}, "custom hu::value no good");

    LONGS_EQUAL_TEXT(1337, t.trove / 1 / hu::Parent {} / 2 % hu::val<TypeContainer>{}, "custom hu::value good");

    auto spuriousNode = t.trove / "big" / "fat" / 0 / "sloppy" / "wet" / 1;
    CHECK_TEXT(spuriousNode.isNullish(), "wildly wrong path");    
}

TEST(cppSugar, annos)
{
    LONGS_EQUAL(3, m.trove.numTroveAnnotations());
    LONGS_EQUAL(1, m.trove.numTroveAnnotationsWithKey("tx"));
    LONGS_EQUAL(1, m.trove.numTroveAnnotationsWithKey("ta"));
    LONGS_EQUAL(1, m.trove.numTroveAnnotationsWithKey("tb"));
    LONGS_EQUAL(0, m.trove.numTroveAnnotationsWithKey("foo"));
    LONGS_EQUAL(2, m.trove.numTroveAnnotationsWithValue("ta"));
    LONGS_EQUAL(1, m.trove.numTroveAnnotationsWithValue("tb"));

    auto tas = m.trove.troveAnnotationsWithValue("ta");
    LONGS_EQUAL(2, tas.size());
    LONGS_EQUAL(2, tas[0].str().size());
    MEMCMP_EQUAL("tx", tas[0].str().data(), 2);
    LONGS_EQUAL(2, tas[1].str().size());
    MEMCMP_EQUAL("ta", tas[1].str().data(), 2);
    
    tas = m.trove.troveAnnotationsWithValue("tb");
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
    auto tcs = m.trove.allTroveComments();
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
