#include <string.h>
#include "humon.hpp"
#include "ansiColors.h"
#include <string_view>
#include <CppUTest/TestHarness.h>

using namespace std::literals;

TEST_GROUP(utf8)
{
    hu::Trove trove;

    void setup()
    {
        trove = hu::Trove::fromFile("src/test/testFiles/utf8.hu");
    }

    void teardown()
    {
    }
};

TEST(utf8, lengthOfIt)
{
    auto root = trove.getRootNode();
    CHECK_TEXT(root.getKind() == hu::NodeKind::dict, "root kind");
    CHECK_TEXT(root.getNumChildren() == 2, "root numChildren");
    auto child = root.getChild("otherKey");
    CHECK_TEXT(child.getKind() == hu::NodeKind::value, "value kind");
    CHECK_TEXT(child.getValue().size() == 9, "load3");
}
