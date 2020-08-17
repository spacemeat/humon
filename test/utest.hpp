#pragma once

// utest - header-only unit testing framework for C++. CPPUtest isn't doing it for me.

#include <map>
#include <string>
#include <functional>
#include <memory>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <string_view>

using namespace std::literals;


std::string toString(bool val);
std::string toString(long val);
std::string toString(long long val);
std::string toString(double val);
std::string toString(char const * val, long len);
std::string toString(void * val);


struct TestGroup
{
public:
    TestGroup() { }

    void setGroupAndTest(std::string_view sourceFile, std::string_view groupName, std::string_view testName)
    {
        this->sourceFile = sourceFile;
        this->groupName = groupName;
        this->testName = testName;
    }

    bool testPassed() { return passed; }
    bool testFailed() { return ! passed; }
    std::string checkFailure() { return checkThatFailed; }

    virtual void setup() {};
    virtual void teardown() {};

    template <typename T>
    void fail(T const & expected, T const & got, std::string_view checkThatFailed, 
        std::string_view file, int line)
    {
        this->passed = false;
        this->expected = toString(expected);
        this->got = toString(got);
        this->checkThatFailed = checkThatFailed;
        this->file = file;
        this->line = line;
    }

    void fail(std::string_view checkThatFailed, std::string_view file, int line)
    {
        this->passed = false;
        this->expected = "";
        this->got = "";
        this->checkThatFailed = checkThatFailed;
        this->file = file;
        this->line = line;
    }

    void fail(char const * expected, char const * got, long long len, std::string_view checkThatFailed, 
        std::string_view, int line)
    {
        this->passed = false;
        this->expected = toString(expected, (long) len);
        this->got = toString(got, (long) len);
        this->checkThatFailed = checkThatFailed;
        this->file = file;
        this->line = line;
    }

    int checksRun = 0;
    bool passed = true;
    std::string sourceFile;
    std::string groupName;
    std::string testName;
    std::string expected;
    std::string got;
    std::string checkThatFailed;
    std::string file;
    int line = 0;
};


std::map<std::string, std::map<std::string, std::vector<std::string>>> getAllTests();
TestGroup runTest(std::string_view file, std::string_view groupName, std::string_view testName);


#define TEST_GROUP(groupName) \
struct TestGroup_##groupName : public TestGroup


#define TEST(groupName, testName) \
struct Test_##groupName##_##testName : public TestGroup_##groupName \
{ \
    virtual void runTest(); \
}; \
void Test_##groupName##_##testName::runTest()


#define LONGS_EQUAL_TEXT(utest_lhs, utest_rhs, utest_text) \
LONGS_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_text,  __FILE__, __LINE__)

#define LONGS_EQUAL(utest_lhs, utest_rhs) \
LONGS_EQUAL_LOCATION(utest_lhs, utest_rhs, "",  __FILE__, __LINE__)

#define LONGS_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_text, utest_file, utest_line)\
{checksRun += 1; long long utest_l = (utest_lhs); long long utest_r = (utest_rhs); \
if (utest_l != utest_r) { fail(utest_l, utest_r, utest_text, utest_file, utest_line); return; }}


#define DOUBLES_EQUAL_TEXT(utest_lhs, utest_rhs, utest_epsilon, utest_text) \
DOUBLES_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_epsilon, utest_text, __FILE__, __LINE__)

#define DOUBLES_EQUAL(utest_lhs, utest_rhs, utest_epsilon) \
DOUBLES_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_epsilon, "", __FILE__, __LINE__)

#define DOUBLES_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_epsilon, utest_text, utest_file, utest_line) \
{checksRun += 1; double utest_l = (utest_lhs); double utest_r = (utest_rhs); \
if (std::fabs(utest_l - utest_r) < (utest_epsilon)) { fail(utest_l, utest_r, utest_text, utest_file, utest_line); return; }}


#define CHECK_EQUAL_TEXT(utest_lhs, utest_rhs, utest_text) \
CHECK_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_text, __FILE__, __LINE__)

#define CHECK_EQUAL(utest_lhs, utest_rhs) \
CHECK_EQUAL_LOCATION(utest_lhs, utest_rhs, "", __FILE__, __LINE__)

#define CHECK_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_text, utest_file, utest_line) \
{checksRun += 1; if ((utest_lhs) != (utest_rhs)) { fail(utest_text, utest_file, utest_line); return; }}


#define CHECK_TEXT(utest_what, utest_text) \
CHECK_LOCATION(utest_what, utest_text, __FILE__, __LINE__)

#define CHECK(utest_what) \
CHECK_LOCATION(utest_what, "", __FILE__, __LINE__)

#define CHECK_LOCATION(utest_what, utest_text, utest_file, utest_line) \
{checksRun += 1; if (! (utest_what)) { fail(utest_text, utest_file, utest_line); return; }}


#define CHECK_FALSE_TEXT(utest_what, utest_text) \
CHECK_FALSE_LOCATION(utest_what, utest_text, __FILE__, __LINE__)

#define CHECK_FALSE(utest_what) \
CHECK_FALSE_LOCATION(utest_what, "", __FILE__, __LINE__)

#define CHECK_FALSE_LOCATION(utest_what, utest_text, utest_file, utest_line) \
{checksRun += 1; if (utest_what) { fail(utest_text, utest_file, utest_line); return; }}


#define STRNCMP_EQUAL_TEXT(utest_lhs, utest_rhs, len, utest_text) \
STRNCMP_EQUAL_LOCATION(utest_lhs, utest_rhs, len, utest_text, __FILE__, __LINE__)

#define STRNCMP_EQUAL(utest_lhs, utest_rhs, len) \
STRNCMP_EQUAL_LOCATION(utest_lhs, utest_rhs, len, "", __FILE__, __LINE__)

#define STRNCMP_EQUAL_LOCATION(utest_lhs, utest_rhs, len, utest_text, utest_file, utest_line) \
{checksRun += 1; if (strncmp((utest_lhs), (utest_rhs), (long)(len)) != 0) { fail(utest_lhs, utest_rhs, len, utest_text, utest_file, utest_line); return; }}


#define MEMCMP_EQUAL_TEXT(utest_lhs, utest_rhs, len, utest_text) \
MEMCMP_EQUAL_LOCATION(utest_lhs, utest_rhs, len, utest_text, __FILE__, __LINE__)

#define MEMCMP_EQUAL(utest_lhs, utest_rhs, len) \
MEMCMP_EQUAL_LOCATION(utest_lhs, utest_rhs, len, "", __FILE__, __LINE__)

#define MEMCMP_EQUAL_LOCATION(utest_lhs, utest_rhs, len, utest_text, utest_file, utest_line) \
{checksRun += 1; if (memcmp((utest_lhs), (utest_rhs), (long)(len)) != 0) { fail(utest_text, utest_file, utest_line); return; }}


#define POINTERS_EQUAL_TEXT(utest_lhs, utest_rhs, utest_text) \
POINTERS_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_text, __FILE__, __LINE__)

#define POINTERS_EQUAL(utest_lhs, utest_rhs) \
POINTERS_EQUAL_LOCATION(utest_lhs, utest_rhs, "", __FILE__, __LINE__)

#define POINTERS_EQUAL_LOCATION(utest_lhs, utest_rhs, utest_text, utest_file, utest_line) \
{checksRun += 1; if ((utest_lhs) != (utest_rhs)) { fail((void *)(utest_lhs), (void *)(utest_rhs), utest_text, utest_file, utest_line); return; }}

