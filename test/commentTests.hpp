#define HUMON_SUPPRESS_NOEXCEPT

#include <sstream>
#include <iostream>
#include <string.h>
#include <string_view>
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include "humon/humon.hpp"
#include "ztest/ztest.hpp"
#include "testDataCpp.h"

TEST_GROUP(comments)
{
    htd_comments t;

    void setup()
    {
        t.setup();
    }

    void teardown()
    {
        t.teardown();
    }
};

TEST(comments, associations)
{
    auto comments = t.root.allComments();
    LONGS_EQUAL_TEXT(39, comments.size(), "root has 39 comments");
    comments = t.root.commentsContaining("-");
    LONGS_EQUAL_TEXT(39, comments.size(), "root has 39 - comments");
    comments = t.root.commentsContaining("- 1");
    LONGS_EQUAL_TEXT(11, comments.size(), "root has 11 -1 comments");

    comments = t.k0.allComments();
    LONGS_EQUAL_TEXT(40, comments.size(), "/k0 has 40 comments");
    comments = t.k0.commentsContaining("-");
    LONGS_EQUAL_TEXT(40, comments.size(), "/k0 has 40 - comments");
    comments = t.k0.commentsContaining("- 1");
    LONGS_EQUAL_TEXT(11, comments.size(), "/k0 has 11 -1 comments");

    comments = t.k1.allComments();
    LONGS_EQUAL_TEXT(42, comments.size(), "/k1 has 42 comments");
    comments = t.k1.commentsContaining("-");
    LONGS_EQUAL_TEXT(42, comments.size(), "/k1 has 42 - comments");
    comments = t.k1.commentsContaining("- 1");
    LONGS_EQUAL_TEXT(11, comments.size(), "/k1 has 11 -1 comments");

    comments = t.k10.allComments();
    LONGS_EQUAL_TEXT(33, comments.size(), "/k1/0 has 33 comments");
    comments = t.k10.commentsContaining("-");
    LONGS_EQUAL_TEXT(33, comments.size(), "/k10 has 33 - comments");
    comments = t.k10.commentsContaining("- 1");
    LONGS_EQUAL_TEXT(11, comments.size(), "/k10 has 11 -1 comments");
}


TEST_GROUP(divergentComments)
{
    std::string_view humon = R"(
{
    foo: [
        a // 
        b //
        c
    ]
}
)"sv;

    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(divergentComments, emptyComment)
{
    std::optional<hu::Trove> trove;

    hu::DeserializeResult desRes = hu::Trove::fromString(humon.data());
    auto pt = std::get_if<hu::Trove>(& desRes);
    CHECK_TEXT(pt != NULL, "Bad humon!");
    trove = std::move(*pt);
    LONGS_EQUAL_TEXT(3, (trove->root() / "foo").numChildren(), "Invalid number of children.");
    CHECK_TEXT(trove->root()/"foo"/0%hu::val<std::string_view>{} == "a", "a failed");
    CHECK_TEXT(trove->root()/"foo"/1%hu::val<std::string_view>{} == "b", "a failed");
    CHECK_TEXT(trove->root()/"foo"/2%hu::val<std::string_view>{} == "c", "a failed");
}
