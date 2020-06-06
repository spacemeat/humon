#include <string.h>
#include "humon.hpp"
#include "ansiColors.h"
#include <string_view>
#include <CppUTest/TestHarness.h>

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
    hu::Trove trove = hu::Trove::fromString(humon, 2, 2);
    auto root = trove.getRootNode();
    CHECK_TEXT(root.getKind() == hu::NodeKind::list, "load0");
    CHECK_TEXT(root.getNumChildren() == 1, "load1");
    auto child = root.getChild(0);
    CHECK_TEXT(child.getKind() == hu::NodeKind::value, "load2");
    CHECK_TEXT(child.getValue() == "woo", "load3");
}

// TODO: tests for fromFile and fromStream

TEST_GROUP(api)
{
    std::string_view humon = R"([woo {boo: [2]}])"sv;
    hu::Trove trove;

    void setup()
    {
        trove = hu::Trove::fromString(humon, 2, 2);
    }

    void teardown()
    {
    }
};

TEST(api, sugar)
{
    auto root = trove.getRootNode();
    CHECK_TEXT(root.getKind() == hu::NodeKind::list, "root kind");
    LONGS_EQUAL_TEXT(2, root.getNumChildren(), "root numChildren");
    auto node = root.getChild(1);
    CHECK_TEXT(node.getKind() == hu::NodeKind::dict, "root child kind");
    LONGS_EQUAL_TEXT(1, node.getNumChildren(), "root child numChildren");
    node = node.getChild("boo");
    CHECK_TEXT(node.getKind() == hu::NodeKind::list, "root child child kind");
    LONGS_EQUAL_TEXT(1, node.getNumChildren(), "root child child numChildren");
    node = node.getChild(0);
    CHECK_TEXT(node.getKind() == hu::NodeKind::value, "root child child child kind");
    STRNCMP_EQUAL_TEXT("2", node.getValue().data(), 1, "root child child child value");
    LONGS_EQUAL_TEXT(2, trove / 1 / "boo" / 0 / hu::value<int>{}, "root child child child value value")
}
