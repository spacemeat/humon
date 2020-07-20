#define HUMON_SUPPRESS_NOEXCEPT

#include <sstream>
#include <iostream>
#include <string.h>
#include <string_view>
#include <unistd.h>
#include "humon.hpp"
#include <CppUTest/TestHarness.h>
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
