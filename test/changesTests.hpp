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

TEST_GROUP(changes)
{
    htd_changes t;

    void setup()
    {
        t.setup();
    }

    void teardown()
    {
        t.teardown();
    }
};

TEST(changes, replaceValue)
{
    hu::ChangeSet changes(t.trove);
    changes.replaceNode(t.aaa, "aaa: 'Hello, world!'");

    auto des = hu::Trove::fromChanges(changes);
    CHECK_TEXT(std::holds_alternative<hu::Trove>(des), "replace made invalid humon");

    auto & nt = std::get<hu::Trove>(des);

    auto pt = "Hello, world!"sv;
    auto tt = nt.root() / "aaa" % hu::val<std::string> {};
    STRNCMP_EQUAL_TEXT(pt.data(), tt.data(), pt.size(), "aaa");
}

TEST(changes, replaceList)
{
    hu::ChangeSet changes(t.trove);
    changes.replaceNode(t.bbb, "bbb: 'Hello, world!'");

    auto des = hu::Trove::fromChanges(changes);
    CHECK_TEXT(std::holds_alternative<hu::Trove>(des), "replace made invalid humon");

    auto & nt = std::get<hu::Trove>(des);

    auto pt = "Hello, world!"sv;
    auto tt = nt.root() / "bbb" % hu::val<std::string> {};
    STRNCMP_EQUAL_TEXT(pt.data(), tt.data(), pt.size(), "bbb");
}

TEST(changes, replaceDict)
{
    hu::ChangeSet changes(t.trove);
    changes.replaceNode(t.ccc, "ccc: 'Hello, world!'");

    auto des = hu::Trove::fromChanges(changes);
    CHECK_TEXT(std::holds_alternative<hu::Trove>(des), "replace made invalid humon");

    auto & nt = std::get<hu::Trove>(des);

    auto pt = "Hello, world!"sv;
    auto tt = nt.root() / "ccc" % hu::val<std::string> {};
    STRNCMP_EQUAL_TEXT(pt.data(), tt.data(), pt.size(), "ccc");
}

TEST(changes, appendList)
{
    hu::ChangeSet changes(t.trove);
    changes.append(t.bbb, "'Hello, world!'");

    auto des = hu::Trove::fromChanges(changes);
    CHECK_TEXT(std::holds_alternative<hu::Trove>(des), "replace made invalid humon");

    auto & nt = std::get<hu::Trove>(des);

    auto pt = "Hello, world!"sv;
    auto tt = nt.root() / "bbb" / 3 % hu::val<std::string> {};
    STRNCMP_EQUAL_TEXT(pt.data(), tt.data(), pt.size(), "bbb");
}

TEST(changes, appendDict)
{
    hu::ChangeSet changes(t.trove);
    changes.append(t.ccc, "CCCd: 'Hello, world!'");

    auto des = hu::Trove::fromChanges(changes);
    CHECK_TEXT(std::holds_alternative<hu::Trove>(des), "replace made invalid humon");

    auto & nt = std::get<hu::Trove>(des);

    auto pt = "Hello, world!"sv;
    auto tt = nt.root() / "ccc" / "CCCd" % hu::val<std::string> {};
    STRNCMP_EQUAL_TEXT(pt.data(), tt.data(), pt.size(), "ccc");
}

TEST(changes, insertList)
{
    hu::ChangeSet changes(t.trove);
    changes.insertAtIndex(t.bbb, 2, "'Hello, world!'");

    auto des = hu::Trove::fromChanges(changes);
    CHECK_TEXT(std::holds_alternative<hu::Trove>(des), "replace made invalid humon");

    auto & nt = std::get<hu::Trove>(des);

    auto pt = "Hello, world!"sv;
    auto tt = nt.root() / "bbb" / 2 % hu::val<std::string> {};
    STRNCMP_EQUAL_TEXT(pt.data(), tt.data(), pt.size(), "bbb");
}

TEST(changes, insertDict)
{
    hu::ChangeSet changes(t.trove);
    changes.insertAtIndex(t.ccc, 0, "CCC0: 'Hello, world!'");

    auto des = hu::Trove::fromChanges(changes);
    CHECK_TEXT(std::holds_alternative<hu::Trove>(des), "replace made invalid humon");

    auto & nt = std::get<hu::Trove>(des);

    auto pt = "Hello, world!"sv;
    auto tt = nt.root() / "ccc" / 0 % hu::val<std::string> {};
    STRNCMP_EQUAL_TEXT(pt.data(), tt.data(), pt.size(), "ccc");
}
