#include <string_view>
#include <cstring>
#include "humon.hpp"
#include <CppUTest/TestHarness.h>

using namespace std;
using namespace std::literals;

TEST_GROUP(utf8)
{
    hu::Trove trove;

    void setup()
    {
        trove = move(get<hu::Trove>(hu::Trove::fromFile("test/testFiles/utf8.hu")));
    }

    void teardown()
    {
    }
};

TEST(utf8, lengthOfIt)
{
    auto root = trove.root();
    LONGS_EQUAL_TEXT((int) hu::NodeKind::dict, (int) root.kind(), "root kind");
    LONGS_EQUAL_TEXT(10, root.numChildren(), "root numChildren");
    auto child = root.child("otherKey");
    LONGS_EQUAL_TEXT((int) hu::NodeKind::value, (int) child.kind(), "value kind");
    LONGS_EQUAL_TEXT(9, child.value().str().size(), "load3");
    child = root.child("someOtherText");
    LONGS_EQUAL_TEXT((int) hu::NodeKind::value, (int) child.kind(), "value kind");
    LONGS_EQUAL_TEXT("foo"sv.size(), child.value().str().size(), "foo size");
    STRNCMP_EQUAL_TEXT("foo", child.value().str().data(), 3, "foo");
}

TEST(utf8, keys)
{
    auto root = trove.root();
    auto ch = root / "Я";
    STRNCMP_EQUAL("two", ch.value().str().data(), strlen("two"));
    ch = root / "⾀";
    STRNCMP_EQUAL("three", ch.value().str().data(), strlen("three"));
    ch = root / "𠜎";
    STRNCMP_EQUAL("four", ch.value().str().data(), strlen("four"));

    ch = root / "two";
    MEMCMP_EQUAL("Я", ch.value().str().data(), 2);
    ch = root / "three";
    MEMCMP_EQUAL("⾀", ch.value().str().data(), 2);
    ch = root / "four";
    MEMCMP_EQUAL("𠜎", ch.value().str().data(), 2);

    auto [k, v] = root.annotation(0);
    MEMCMP_EQUAL("Я", k.str().data(), 2);
    STRNCMP_EQUAL("two", v.str().data(), strlen("two"));
    tie(k, v) = root.annotation(1);
    MEMCMP_EQUAL("⾀", k.str().data(), 3);
    STRNCMP_EQUAL("three", v.str().data(), strlen("three"));
    tie(k, v) = root.annotation(2);
    MEMCMP_EQUAL("𠜎", k.str().data(), 4);
    STRNCMP_EQUAL("four", v.str().data(), strlen("four"));

    tie(k, v) = root.annotation(3);
    STRNCMP_EQUAL("two", k.str().data(), strlen("two"));
    MEMCMP_EQUAL("Я", v.str().data(), 2);
    tie(k, v) = root.annotation(4);
    STRNCMP_EQUAL("three", k.str().data(), strlen("three"));
    MEMCMP_EQUAL("⾀", v.str().data(), 3);
    tie(k, v) = root.annotation(5);
    STRNCMP_EQUAL("four", k.str().data(), strlen("four"));
    MEMCMP_EQUAL("𠜎", v.str().data(), 4);
}
