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
        trove = hu::Trove::fromFile("test/testFiles/utf8.hu");
    }

    void teardown()
    {
    }
};

TEST(utf8, lengthOfIt)
{
    auto root = trove.rootNode();
    LONGS_EQUAL_TEXT((int) hu::NodeKind::dict, (int) root.kind(), "root kind");
    LONGS_EQUAL_TEXT(2, root.numChildren(), "root numChildren");
    auto child = root.child("otherKey");
    LONGS_EQUAL_TEXT((int) hu::NodeKind::value, (int) child.kind(), "value kind");
    LONGS_EQUAL_TEXT(9, child.value().str().size(), "load3");
}
