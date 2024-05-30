#define HUMON_SUPPRESS_NOEXCEPT

#include <string_view>
#include <cstring>
#include "humon/humon.hpp"
#include "ztest/ztest.hpp"

using namespace std;
using namespace std::literals;

TEST_GROUP(utf8)
{
    hu::Trove trove;
    bool die = false;

    void setup()
    {
        die = false;
        try
        {
            trove = move(get<hu::Trove>(hu::Trove::fromFile("test/testFiles/utf8.hu", {hu::Encoding::utf8, false})));
        }
        catch (...)
        {
            die = true;
        }
    }

    void teardown()
    {
    }
};

TEST(utf8, lengthOfIt)
{
    CHECK_TEXT(die != true, "Didn't make the trove.");
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
    CHECK_TEXT(die != true, "Didn't make the trove.");
    auto root = trove.root();
    auto ch = root / u8"Я";
    STRNCMP_EQUAL("two", ch.value().str().data(), strlen("two"));
    ch = root / u8"⾀";
    STRNCMP_EQUAL("three", ch.value().str().data(), strlen("three"));
    ch = root / u8"𠜎";
    STRNCMP_EQUAL("four", ch.value().str().data(), strlen("four"));

    ch = root / "two";
    MEMCMP_EQUAL(u8"Я", ch.value().str().data(), 2);
    ch = root / "three";
    MEMCMP_EQUAL(u8"⾀", ch.value().str().data(), 2);
    ch = root / "four";
    MEMCMP_EQUAL(u8"𠜎", ch.value().str().data(), 2);

    auto [k, v] = root.metatag(0);
    LONGS_EQUAL(k.str().size(), 2);
    MEMCMP_EQUAL(u8"Я", k.str().data(), 2);
    STRNCMP_EQUAL("two", v.str().data(), strlen("two"));
    tie(k, v) = root.metatag(1);
    LONGS_EQUAL(k.str().size(), 3);
    MEMCMP_EQUAL(u8"⾀", k.str().data(), 3);
    STRNCMP_EQUAL("three", v.str().data(), strlen("three"));
    tie(k, v) = root.metatag(2);
    LONGS_EQUAL(k.str().size(), 4);
    MEMCMP_EQUAL(u8"𠜎", k.str().data(), 4);
    STRNCMP_EQUAL("four", v.str().data(), strlen("four"));

    tie(k, v) = root.metatag(3);
    STRNCMP_EQUAL("two", k.str().data(), strlen("two"));
    LONGS_EQUAL(v.str().size(), 2);
    MEMCMP_EQUAL(u8"Я", v.str().data(), 2);
    tie(k, v) = root.metatag(4);
    STRNCMP_EQUAL("three", k.str().data(), strlen("three"));
    LONGS_EQUAL(v.str().size(), 3);
    MEMCMP_EQUAL(u8"⾀", v.str().data(), 3);
    tie(k, v) = root.metatag(5);
    STRNCMP_EQUAL("four", k.str().data(), strlen("four"));
    LONGS_EQUAL(v.str().size(), 4);
    MEMCMP_EQUAL(u8"𠜎", v.str().data(), 4);
}
