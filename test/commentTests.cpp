#include <sstream>
#include <string.h>
#include <string_view>
#include <unistd.h>
#include "humon.hpp"
#include "ansiColors.h"
#include <CppUTest/TestHarness.h>

#include <iostream>

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

    comments = t.k0.allComments();
    LONGS_EQUAL_TEXT(40, comments.size(), "/k0 has 40 comments");

    comments = t.k1.allComments();
    LONGS_EQUAL_TEXT(42, comments.size(), "/k1 has 42 comments");

    comments = t.k10.allComments();
    LONGS_EQUAL_TEXT(33, comments.size(), "/k1/0 has 33 comments");
}
