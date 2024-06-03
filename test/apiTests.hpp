#include <string.h>
#include <string_view>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "humon/humon.h"
#include "../src/humon.internal.h"
#include "ztest/ztest.hpp"
#include "testData.h"


#define isSignedType(t) (((t)(-1)) < ((t) 0))

#define uMaxOfType(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                        (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define sMaxOfType(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                        (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define maxOfType(t) ((unsigned long long) (isSignedType(t) ? sMaxOfType(t) : uMaxOfType(t)))


// ------------------------------ NODE API TESTS

TEST_GROUP(huGetParent)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetParent, lists)
{
    auto pn = huGetParent(l.a);
    POINTERS_EQUAL_TEXT(l.root, pn, "root == ap");

    pn = huGetParent(l.b);
    POINTERS_EQUAL_TEXT(l.bp, pn, "l.bp == l.b.parent");
    pn = huGetParent(pn);
    POINTERS_EQUAL_TEXT(l.root, pn, "root == l.bp.parent");

    pn = huGetParent(l.c);
    POINTERS_EQUAL_TEXT(l.cp, pn, "l.cp == l.c.parent");
    pn = huGetParent(pn);
    POINTERS_EQUAL_TEXT(l.cpp, pn, "l.cp == l.cp.parent");
    pn = huGetParent(pn);
    POINTERS_EQUAL_TEXT(l.root, pn, "root == l.cpp.parent");

    pn = huGetParent(l.root);
    POINTERS_EQUAL_TEXT(NULL, pn, "root's parent is not NULL");
}

TEST(huGetParent, dicts)
{
    auto pn = huGetParent(d.a);
    POINTERS_EQUAL_TEXT(d.root, pn, "root == ap");

    pn = huGetParent(d.b);
    POINTERS_EQUAL_TEXT(d.bp, pn, "d.bp == d.b.parent");
    pn = huGetParent(pn);
    POINTERS_EQUAL_TEXT(d.root, pn, "root == d.bp.parent");

    pn = huGetParent(d.c);
    POINTERS_EQUAL_TEXT(d.cp, pn, "d.cp == d.c.parent");
    pn = huGetParent(pn);
    POINTERS_EQUAL_TEXT(d.cpp, pn, "d.cp == d.cp.parent");
    pn = huGetParent(pn);
    POINTERS_EQUAL_TEXT(d.root, pn, "root == d.cpp.parent");

    pn = huGetParent(d.root);
    POINTERS_EQUAL_TEXT(NULL, pn, "root's parent is not NULL");
}

TEST(huGetParent, pathological)
{
    auto pn = huGetParent(NULL);
    POINTERS_EQUAL_TEXT(NULL, pn, "NULL's parent is not NULL");

    pn = huGetParent(HU_NULLNODE);
    POINTERS_EQUAL_TEXT(NULL, pn, "nullNode's parent is not NULL");
}


TEST_GROUP(huGetNumChildren)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumChildren, lists)
{
    LONGS_EQUAL_TEXT(3, huGetNumChildren(l.root), "root.nch == 3");
    LONGS_EQUAL_TEXT(0, huGetNumChildren(l.a), "a.nch == 0");
    LONGS_EQUAL_TEXT(1, huGetNumChildren(l.bp), "bp.nch == 1");
    LONGS_EQUAL_TEXT(0, huGetNumChildren(l.b), "b.nch == 0");
    LONGS_EQUAL_TEXT(1, huGetNumChildren(l.cpp), "cpp.nch == 1");
    LONGS_EQUAL_TEXT(1, huGetNumChildren(l.cp), "cp.nch == 1");
    LONGS_EQUAL_TEXT(0, huGetNumChildren(l.c), "c.nch == 0");
}

TEST(huGetNumChildren, dicts)
{
    LONGS_EQUAL_TEXT(3, huGetNumChildren(d.root), "root.nch == 3");
    LONGS_EQUAL_TEXT(0, huGetNumChildren(d.a), "a.nch == 0");
    LONGS_EQUAL_TEXT(1, huGetNumChildren(d.bp), "bp.nch == 1");
    LONGS_EQUAL_TEXT(0, huGetNumChildren(d.b), "b.nch == 0");
    LONGS_EQUAL_TEXT(1, huGetNumChildren(d.cpp), "cpp.nch == 1");
    LONGS_EQUAL_TEXT(1, huGetNumChildren(d.cp), "cp.nch == 1");
    LONGS_EQUAL_TEXT(0, huGetNumChildren(d.c), "c.nch == 0");
}

TEST(huGetNumChildren, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumChildren(NULL), "NULL.nch == 0");
    LONGS_EQUAL_TEXT(0, huGetNumChildren(HU_NULLNODE), "nullNode.nch == 0");
}


TEST_GROUP(huGetChildByIndex)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetChildByIndex, lists)
{
    POINTERS_EQUAL_TEXT(l.a, huGetChildByIndex(l.root, 0), "root.ch0 == a");
    POINTERS_EQUAL_TEXT(l.bp, huGetChildByIndex(l.root, 1), "root.ch1 == bp");
    POINTERS_EQUAL_TEXT(l.b, huGetChildByIndex(l.bp, 0), "bp.ch0 == b");
    POINTERS_EQUAL_TEXT(l.cpp, huGetChildByIndex(l.root, 2), "root.ch1 == cpp");
    POINTERS_EQUAL_TEXT(l.cp, huGetChildByIndex(l.cpp, 0), "cpp.ch0 == cp");
    POINTERS_EQUAL_TEXT(l.c, huGetChildByIndex(l.cp, 0), "cp.ch0 == c");
}

TEST(huGetChildByIndex, dicts)
{
    POINTERS_EQUAL_TEXT(d.a, huGetChildByIndex(d.root, 0), "root.ch0 == a");
    POINTERS_EQUAL_TEXT(d.bp, huGetChildByIndex(d.root, 1), "root.ch1 == bp");
    POINTERS_EQUAL_TEXT(d.b, huGetChildByIndex(d.bp, 0), "bp.ch0 == b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetChildByIndex(d.root, 2), "root.ch1 == cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetChildByIndex(d.cpp, 0), "cpp.ch0 == cp");
    POINTERS_EQUAL_TEXT(d.c, huGetChildByIndex(d.cp, 0), "cp.ch0 == c");
}

TEST(huGetChildByIndex, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByIndex(d.root, -1), "root.ch-1 == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByIndex(d.root, 3), "root.ch3 == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByIndex(NULL, 0), "NULL.ch0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByIndex(HU_NULLNODE, 0), "null.ch0 == null");
}


TEST_GROUP(huGetChildByKey)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetChildByKey, dicts)
{
    POINTERS_EQUAL_TEXT(d.a, huGetChildByKeyZ(d.root, "ak"), "root.chak == a");
    POINTERS_EQUAL_TEXT(d.bp, huGetChildByKeyZ(d.root, "bk"), "root.chbk == bp");
    POINTERS_EQUAL_TEXT(d.b, huGetChildByKeyZ(d.bp, "bk"), "bp.chbk == b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetChildByKeyZ(d.root, "ck"), "root.chck == cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetChildByKeyZ(d.cpp, "ck"), "cpp.chck == cp");
    POINTERS_EQUAL_TEXT(d.c, huGetChildByKeyZ(d.cp, "ck"), "cp.chck == c");
}

TEST(huGetChildByKey, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByKeyZ(d.root, "foo"), "root.chfoo == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByKeyZ(l.root, "foo"), "listroot.chfoo == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByKeyZ(NULL, "foo"), "NULL.chfoo == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByKeyZ(HU_NULLNODE, "foo"), "null.chfoo == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByKeyZ(d.root, ""), "root.ch'' == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetChildByKeyZ(l.root, ""), "listroot.ch'' == null");
}


TEST_GROUP(huGetSharedKeyIndex)
{
	htd_sharedKeys t;

	void setup()
	{
		t.setup();
	}

	void teardown()
	{
		t.teardown();
	}
};

TEST(huGetSharedKeyIndex, dicts)
{
	huSize_t numChildren = huGetNumChildren(t.root);
	for (huSize_t i = 0; i < numChildren; ++i)
	{
		huNode const * ch = huGetChildByIndex(t.root, i);
		huNode const * l = huGetChildByIndex(ch, 1);
		huSize_t ski = huGetSharedKeyIndex(ch);
		huToken const * lvt = huGetValue(l);
		huSize_t lv = atoi(std::string(std::string_view(lvt->str.ptr, lvt->str.size)).data());
		LONGS_EQUAL(lv, ski);
	}
}


TEST(huGetSharedKeyIndex, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetFirstChild(NULL), "NULL ski == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetFirstChild(HU_NULLNODE), "null ski == null");
}


TEST_GROUP(huGetFirstChild)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetFirstChild, lists)
{
    POINTERS_EQUAL_TEXT(l.a, huGetFirstChild(l.root), "root gfc == a");
    POINTERS_EQUAL_TEXT(l.b, huGetFirstChild(l.bp), "bp gfc == b");
    POINTERS_EQUAL_TEXT(l.cp, huGetFirstChild(l.cpp), "cpp gfc == cp");
    POINTERS_EQUAL_TEXT(l.c, huGetFirstChild(l.cp), "cp gfc == c");
}

TEST(huGetFirstChild, dicts)
{
    POINTERS_EQUAL_TEXT(d.a, huGetFirstChild(d.root), "root gfc == a");
    POINTERS_EQUAL_TEXT(d.b, huGetFirstChild(d.bp), "bp gfc == b");
    POINTERS_EQUAL_TEXT(d.cp, huGetFirstChild(d.cpp), "cpp gfc == cp");
    POINTERS_EQUAL_TEXT(d.c, huGetFirstChild(d.cp), "cp gfc == c");
}

TEST(huGetFirstChild, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetFirstChild(NULL), "NULL gfc == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetFirstChild(HU_NULLNODE), "null gfc == null");
}


TEST_GROUP(huGetNextSibling)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNextSibling, lists)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(l.root), "root.ns == null");
    POINTERS_EQUAL_TEXT(l.bp, huGetNextSibling(l.a), "a.ns == bp");
    POINTERS_EQUAL_TEXT(l.cpp, huGetNextSibling(l.bp), "bp.ns == cpp");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(l.cpp), "cpp.ns == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(l.cp), "cp.ns == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(l.c), "c.ns == null");
}

TEST(huGetNextSibling, dicts)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(d.root), "root.ns == null");
    POINTERS_EQUAL_TEXT(d.bp, huGetNextSibling(d.a), "a.ns == bp");
    POINTERS_EQUAL_TEXT(d.cpp, huGetNextSibling(d.bp), "bp.ns == cpp");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(d.cpp), "cpp.ns == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(d.cp), "cp.ns == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(d.c), "c.ns == null");
}

TEST(huGetNextSibling, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(NULL), "NULL.ns == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNextSibling(HU_NULLNODE), "null.ns == null");
}


TEST_GROUP(huHasKey)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huHasKey, lists)
{
    CHECK_EQUAL_TEXT(false, huHasKey(l.root), "root.huHasKey == false");
    CHECK_EQUAL_TEXT(false, huHasKey(l.a), "a.huHasKey == false");
    CHECK_EQUAL_TEXT(false, huHasKey(l.bp), "bp.huHasKey == false");
    CHECK_EQUAL_TEXT(false, huHasKey(l.cpp), "cpp.huHasKey == false");
    CHECK_EQUAL_TEXT(false, huHasKey(l.cp), "cp.huHasKey == false");
    CHECK_EQUAL_TEXT(false, huHasKey(l.c), "c.huHasKey == false");
}

TEST(huHasKey, dicts)
{
    CHECK_EQUAL_TEXT(false, huHasKey(d.root), "root.huHasKey == false");
    CHECK_EQUAL_TEXT(true, huHasKey(d.a), "a.huHasKey == true");
    CHECK_EQUAL_TEXT(true, huHasKey(d.bp), "bp.huHasKey == true");
    CHECK_EQUAL_TEXT(true, huHasKey(d.cpp), "cpp.huHasKey == true");
    CHECK_EQUAL_TEXT(true, huHasKey(d.cp), "cp.huHasKey == true");
    CHECK_EQUAL_TEXT(true, huHasKey(d.c), "c.huHasKey == true");
}

TEST(huHasKey, pathological)
{
    CHECK_EQUAL_TEXT(false, huHasKey(NULL), "NULL.huHasKey == false");
    CHECK_EQUAL_TEXT(false, huHasKey(HU_NULLNODE), "null.huHasKey == false");
}


TEST_GROUP(huGetSourceText)
{
    htd_listOfLists l;
    htd_dictOfDicts d;
    htd_withSomeStrings s;

    void setup()
    {
        l.setup();
        d.setup();
        s.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
        s.teardown();
    }
};

TEST(huGetSourceText, lists)
{
    auto strA = R"(// This is a aaaa right here.
    a           @a:a        @type:value     // aaaa)"sv;

    auto strBp = R"(// This is a bp right here.
    [
        b       @b:b        @type:value     // bbbb
    ]           @b:bp       @type:list      // bp)"sv;

    auto strCpp = R"(// This is a cpp right here.
    [
        [
            c   @c:c        @type:value     // cccc
        ]       @c:cp       @type:list      // cp
    ]           @c:cpp      @type:list      // cpp)"sv;

    auto sA = huGetSourceText(l.a);
    auto sBp = huGetSourceText(l.bp);
    auto sCpp = huGetSourceText(l.cpp);

    LONGS_EQUAL_TEXT(strA.size(), sA.size, "a sz");
    STRNCMP_EQUAL_TEXT(strA.data(), sA.ptr, strA.size(), "a str");
    LONGS_EQUAL_TEXT(strBp.size(), sBp.size, "bp sz");
    STRNCMP_EQUAL_TEXT(strBp.data(), sBp.ptr, strBp.size(), "bp str");
    LONGS_EQUAL_TEXT(strCpp.size(), sCpp.size, "cpp sz");
    STRNCMP_EQUAL_TEXT(strCpp.data(), sCpp.ptr, strCpp.size(), "cpp str");
}

TEST(huGetSourceText, dicts)
{
    auto strA = R"(// This is a aaaa right here.
    ak:a            @a:a        @type:value     // aaaa)"sv;

    auto strBp = R"(// This is a bp right here.
    bk:{
        bk:b        @b:b        @type:value     // bbbb
    }               @b:bp       @type:dict      // bp)"sv;

    auto strCpp = R"(// This is a cpp right here.
    ck:{
        ck:{
            ck:c    @c:c        @type:value     // cccc
        }           @c:cp       @type:dict      // cp
    }               @c:cpp      @type:dict      // cpp)"sv;

    auto sA = huGetSourceText(d.a);
    auto sBp = huGetSourceText(d.bp);
    auto sCpp = huGetSourceText(d.cpp);

    LONGS_EQUAL_TEXT(strA.size(), sA.size, "a sz");
    STRNCMP_EQUAL_TEXT(strA.data(), sA.ptr, strA.size(), "a str");
    LONGS_EQUAL_TEXT(strBp.size(), sBp.size, "bp sz");
    STRNCMP_EQUAL_TEXT(strBp.data(), sBp.ptr, strBp.size(), "bp str");
    LONGS_EQUAL_TEXT(strCpp.size(), sCpp.size, "cpp sz");
    STRNCMP_EQUAL_TEXT(strCpp.data(), sCpp.ptr, strCpp.size(), "cpp str");
}

TEST(huGetSourceText, stringy)
{
    auto strA = R"("aaa": bbb)"sv;
    auto strB = R"(ccc: "ddd")"sv;
    auto strC = R"("eee": "fff")"sv;
    auto strD = R"(ggg: hhh)"sv;

    auto sA = huGetSourceText(s.aaa);
    auto sB = huGetSourceText(s.ccc);
    auto sC = huGetSourceText(s.eee);
    auto sD = huGetSourceText(s.ggg);

    LONGS_EQUAL_TEXT(strA.size(), sA.size, "aaa sz");
    STRNCMP_EQUAL_TEXT(strA.data(), sA.ptr, strA.size(), "aaa str");
    LONGS_EQUAL_TEXT(strB.size(), sB.size, "ccc sz");
    STRNCMP_EQUAL_TEXT(strB.data(), sB.ptr, strB.size(), "ccc str");
    LONGS_EQUAL_TEXT(strC.size(), sC.size, "eee sz");
    STRNCMP_EQUAL_TEXT(strC.data(), sC.ptr, strC.size(), "eee str");
    LONGS_EQUAL_TEXT(strD.size(), sD.size, "ggg sz");
    STRNCMP_EQUAL_TEXT(strD.data(), sD.ptr, strD.size(), "ggg str");
}

TEST(huGetSourceText, pathological)
{
    auto str = huGetSourceText(NULL);
    LONGS_EQUAL_TEXT(0, str.size, "NULL -> 0 sz");
    POINTERS_EQUAL_TEXT(NULL, str.ptr, "NULL -> 0 sz");

    str = huGetSourceText(HU_NULLNODE);
    LONGS_EQUAL_TEXT(0, str.size, "NULL -> 0 sz");
    POINTERS_EQUAL_TEXT(NULL, str.ptr, "NULL -> 0 sz");
}


TEST_GROUP(huGetNumMetatags)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumMetatags, lists)
{
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(l.root),  "root.na == 0");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(l.a),     "a.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(l.bp),    "bp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(l.b),     "b.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(l.cpp),   "cpp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(l.cp),    "cp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(l.c),     "c.na == 2");
}

TEST(huGetNumMetatags, dicts)
{
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(d.root),  "root.na == 0");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(d.a),     "a.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(d.bp),    "bp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(d.b),     "b.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(d.cpp),   "cpp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(d.cp),    "cp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumMetatags(d.c),     "c.na == 2");
}

TEST(huGetNumMetatags, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumMetatags(NULL), "NULL.na == null");
    LONGS_EQUAL_TEXT(0, huGetNumMetatags(HU_NULLNODE), "null.na == null");
}


TEST_GROUP(huGetMetatag)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetMetatag, lists)
{
    huMetatag const * metatag = huGetMetatag(l.root, 0);
    CHECK_TEXT(metatag != NULL, "root.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("name", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 0 k=name");
    STRNCMP_EQUAL_TEXT("root", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 0 v=root");
    metatag = huGetMetatag(l.root, 1);
    CHECK_TEXT(metatag != NULL, "root.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("otherName", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 1 k=otherName");
    STRNCMP_EQUAL_TEXT("root", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 1 k=root");

    metatag = huGetMetatag(l.a, 0);
    CHECK_TEXT(metatag != NULL, "a.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("a", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 0 k=a");
    STRNCMP_EQUAL_TEXT("a", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 0 v=a");
    metatag = huGetMetatag(l.a, 1);
    CHECK_TEXT(metatag != NULL, "a.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("value", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 1 k=value");

    metatag = huGetMetatag(l.bp, 0);
    CHECK_TEXT(metatag != NULL, "bp.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", metatag->key->str.ptr, metatag->key->str.size, "bp.metatag 0 k=b");
    STRNCMP_EQUAL_TEXT("bp", metatag->value->str.ptr, metatag->value->str.size, "bp.metatag 0 v=bp");
    metatag = huGetMetatag(l.bp, 1);
    CHECK_TEXT(metatag != NULL, "bp.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "bp.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("list", metatag->value->str.ptr, metatag->value->str.size, "bp.metatag 1 k=list");

    metatag = huGetMetatag(l.b, 0);
    CHECK_TEXT(metatag != NULL, "b.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", metatag->key->str.ptr, metatag->key->str.size, "b.metatag 0 k=b");
    STRNCMP_EQUAL_TEXT("b", metatag->value->str.ptr, metatag->value->str.size, "b.metatag 0 v=b");
    metatag = huGetMetatag(l.b, 1);
    CHECK_TEXT(metatag != NULL, "b.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "b.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("value", metatag->value->str.ptr, metatag->value->str.size, "b.metatag 1 k=value");

    metatag = huGetMetatag(l.cpp, 0);
    CHECK_TEXT(metatag != NULL, "cpp.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", metatag->key->str.ptr, metatag->key->str.size, "cpp.metatag 0 k=c");
    STRNCMP_EQUAL_TEXT("cpp", metatag->value->str.ptr, metatag->value->str.size, "cpp.metatag 0 v=cpp");
    metatag = huGetMetatag(l.cpp, 1);
    CHECK_TEXT(metatag != NULL, "cpp.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "cpp.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("list", metatag->value->str.ptr, metatag->value->str.size, "cpp.metatag 1 k=list");

    metatag = huGetMetatag(l.cp, 0);
    CHECK_TEXT(metatag != NULL, "cp.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", metatag->key->str.ptr, metatag->key->str.size, "cp.metatag 0 k=c");
    STRNCMP_EQUAL_TEXT("cp", metatag->value->str.ptr, metatag->value->str.size, "cp.metatag 0 v=cp");
    metatag = huGetMetatag(l.cp, 1);
    CHECK_TEXT(metatag != NULL, "cp.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "cp.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("list", metatag->value->str.ptr, metatag->value->str.size, "cp.metatag 1 k=list");

    metatag = huGetMetatag(l.c, 0);
    CHECK_TEXT(metatag != NULL, "c.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", metatag->key->str.ptr, metatag->key->str.size, "c.metatag 0 k=c");
    STRNCMP_EQUAL_TEXT("c", metatag->value->str.ptr, metatag->value->str.size, "c.metatag 0 v=c");
    metatag = huGetMetatag(l.c, 1);
    CHECK_TEXT(metatag != NULL, "c.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "c.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("value", metatag->value->str.ptr, metatag->value->str.size, "c.metatag 1 k=value");
}

TEST(huGetMetatag, dicts)
{
    huMetatag const * metatag = huGetMetatag(d.root, 0);
    CHECK_TEXT(metatag != NULL, "root.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("name", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 0 k=name");
    STRNCMP_EQUAL_TEXT("root", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 0 v=root");
    metatag = huGetMetatag(d.root, 1);
    CHECK_TEXT(metatag != NULL, "root.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("otherName", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 1 k=otherName");
    STRNCMP_EQUAL_TEXT("root", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 1 k=root");

    metatag = huGetMetatag(d.a, 0);
    CHECK_TEXT(metatag != NULL, "a.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("a", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 0 k=a");
    STRNCMP_EQUAL_TEXT("a", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 0 v=a");
    metatag = huGetMetatag(d.a, 1);
    CHECK_TEXT(metatag != NULL, "a.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "a.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("value", metatag->value->str.ptr, metatag->value->str.size, "a.metatag 1 k=value");

    metatag = huGetMetatag(d.bp, 0);
    CHECK_TEXT(metatag != NULL, "bp.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", metatag->key->str.ptr, metatag->key->str.size, "bp.metatag 0 k=b");
    STRNCMP_EQUAL_TEXT("bp", metatag->value->str.ptr, metatag->value->str.size, "bp.metatag 0 v=bp");
    metatag = huGetMetatag(d.bp, 1);
    CHECK_TEXT(metatag != NULL, "bp.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "bp.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", metatag->value->str.ptr, metatag->value->str.size, "bp.metatag 1 k=dict");

    metatag = huGetMetatag(d.b, 0);
    CHECK_TEXT(metatag != NULL, "b.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", metatag->key->str.ptr, metatag->key->str.size, "b.metatag 0 k=b");
    STRNCMP_EQUAL_TEXT("b", metatag->value->str.ptr, metatag->value->str.size, "b.metatag 0 v=b");
    metatag = huGetMetatag(d.b, 1);
    CHECK_TEXT(metatag != NULL, "b.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "b.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("value", metatag->value->str.ptr, metatag->value->str.size, "b.metatag 1 k=value");

    metatag = huGetMetatag(d.cpp, 0);
    CHECK_TEXT(metatag != NULL, "cpp.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", metatag->key->str.ptr, metatag->key->str.size, "cpp.metatag 0 k=c");
    STRNCMP_EQUAL_TEXT("cpp", metatag->value->str.ptr, metatag->value->str.size, "cpp.metatag 0 v=cpp");
    metatag = huGetMetatag(d.cpp, 1);
    CHECK_TEXT(metatag != NULL, "cpp.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "cpp.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", metatag->value->str.ptr, metatag->value->str.size, "cpp.metatag 1 k=dict");

    metatag = huGetMetatag(d.cp, 0);
    CHECK_TEXT(metatag != NULL, "cp.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", metatag->key->str.ptr, metatag->key->str.size, "cp.metatag 0 k=c");
    STRNCMP_EQUAL_TEXT("cp", metatag->value->str.ptr, metatag->value->str.size, "cp.metatag 0 v=cp");
    metatag = huGetMetatag(d.cp, 1);
    CHECK_TEXT(metatag != NULL, "cp.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "cp.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", metatag->value->str.ptr, metatag->value->str.size, "cp.metatag 1 k=dict");

    metatag = huGetMetatag(d.c, 0);
    CHECK_TEXT(metatag != NULL, "c.metatag 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", metatag->key->str.ptr, metatag->key->str.size, "c.metatag 0 k=c");
    STRNCMP_EQUAL_TEXT("c", metatag->value->str.ptr, metatag->value->str.size, "c.metatag 0 v=c");
    metatag = huGetMetatag(d.c, 1);
    CHECK_TEXT(metatag != NULL, "c.metatag 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", metatag->key->str.ptr, metatag->key->str.size, "c.metatag 1 k=type");
    STRNCMP_EQUAL_TEXT("value", metatag->value->str.ptr, metatag->value->str.size, "c.metatag 1 k=value");
}

TEST(huGetMetatag, pathological)
{
    POINTERS_EQUAL_TEXT(NULL, huGetMetatag(NULL, 0), "NULL.metatag 0 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetMetatag(HU_NULLNODE, 0), "null.metatag 0 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetMetatag(l.a, 3), "a.metatag 3 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetMetatag(l.a, -1), "a.metatag -1 == NULL");
}


TEST_GROUP(huHasMetatagWithKey)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST_GROUP(huGetMetatagWithKey)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetMetatagWithKey, lists)
{
	huSize_t cursor = 0;
    CHECK(huGetMetatagWithKeyZ(l.root, "name", & cursor) != NULL);

	cursor = 0;
    auto metatag = huGetMetatagWithKeyZ(l.root, "name", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("root"), metatag.size, "root.metatag name size = sz root");
    STRNCMP_EQUAL_TEXT("root", metatag.ptr, metatag.size, "root.metatag name == root");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.root, "otherName", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("root"), metatag.size, "root.metatag otherName size = sz root");
    STRNCMP_EQUAL_TEXT("root", metatag.ptr, metatag.size, "root.metatag otherName == root");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(l.root, "foo", & cursor), "root.hawk foo == false");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.a, "a", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("a"), metatag.size, "a.metatag a size = sz a");
    STRNCMP_EQUAL_TEXT("a", metatag.ptr, metatag.size, "a.metatag a == a");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.a, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("value"), metatag.size, "a.metatag type size = sz value");
    STRNCMP_EQUAL_TEXT("value", metatag.ptr, metatag.size, "a.metatag type == value");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(l.a, "foo", & cursor), "a.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.bp, "b", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("bp"), metatag.size, "bp.metatag b size = sz bp");
    STRNCMP_EQUAL_TEXT("bp", metatag.ptr, metatag.size, "bp.metatag a == bp");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.bp, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("list"), metatag.size, "bp.metatag type size = sz list");
    STRNCMP_EQUAL_TEXT("list", metatag.ptr, metatag.size, "bp.metatag type == list");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(l.bp, "foo", & cursor), "bp.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.b, "b", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("b"), metatag.size, "b.metatag b size = sz b");
    STRNCMP_EQUAL_TEXT("b", metatag.ptr, metatag.size, "b.metatag b == b");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.b, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("value"), metatag.size, "b.metatag type size = sz value");
    STRNCMP_EQUAL_TEXT("value", metatag.ptr, metatag.size, "b.metatag type == value");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(l.b, "foo", & cursor), "b.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.cpp, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("cpp"), metatag.size, "cpp.metatag b size = sz cpp");
    STRNCMP_EQUAL_TEXT("cpp", metatag.ptr, metatag.size, "cpp.metatag b == cpp");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.cpp, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("list"), metatag.size, "cpp.metatag type size = sz list");
    STRNCMP_EQUAL_TEXT("list", metatag.ptr, metatag.size, "cpp.metatag type == list");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(l.cpp, "foo", & cursor), "cpp.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.cp, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("cp"), metatag.size, "cp.metatag c size = sz cp");
    STRNCMP_EQUAL_TEXT("cp", metatag.ptr, metatag.size, "cp.metatag c == cp");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.cp, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("list"), metatag.size, "cp.metatag type size = sz list");
    STRNCMP_EQUAL_TEXT("list", metatag.ptr, metatag.size, "cp.metatag type == list");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(l.cp, "foo", & cursor), "cp.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.c, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "c.metatag c size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "c.metatag c == c");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(l.c, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("value"), metatag.size, "c.metatag type size = sz value");
    STRNCMP_EQUAL_TEXT("value", metatag.ptr, metatag.size, "c.metatag type == value");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(l.c, "foo", & cursor), "c.metatag foo == null");
}

TEST(huGetMetatagWithKey, dicts)
{
	huSize_t cursor = 0;
    CHECK(huGetMetatagWithKeyZ(d.root, "name", & cursor) != NULL);

	cursor = 0;
    auto metatag = huGetMetatagWithKeyZ(d.root, "name", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("root"), metatag.size, "root.metatag name size = sz root");
    STRNCMP_EQUAL_TEXT("root", metatag.ptr, metatag.size, "root.metatag name == root");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.root, "otherName", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("root"), metatag.size, "root.metatag otherName size = sz root");
    STRNCMP_EQUAL_TEXT("root", metatag.ptr, metatag.size, "root.metatag otherName == root");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(d.root, "foo", & cursor), "root.hawk foo == false");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.a, "a", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("a"), metatag.size, "a.metatag a size = sz a");
    STRNCMP_EQUAL_TEXT("a", metatag.ptr, metatag.size, "a.metatag a == a");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.a, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("value"), metatag.size, "a.metatag type size = sz value");
    STRNCMP_EQUAL_TEXT("value", metatag.ptr, metatag.size, "a.metatag type == value");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(d.a, "foo", & cursor), "a.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.bp, "b", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("bp"), metatag.size, "bp.metatag b size = sz bp");
    STRNCMP_EQUAL_TEXT("bp", metatag.ptr, metatag.size, "bp.metatag b == bp");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.bp, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("dict"), metatag.size, "bp.metatag type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", metatag.ptr, metatag.size, "bp.metatag type == dict");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(d.bp, "foo", & cursor), "bp.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.b, "b", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("b"), metatag.size, "b.metatag b size = sz b");
    STRNCMP_EQUAL_TEXT("b", metatag.ptr, metatag.size, "b.metatag b == b");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.b, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("value"), metatag.size, "b.metatag type size = sz value");
    STRNCMP_EQUAL_TEXT("value", metatag.ptr, metatag.size, "b.metatag type == value");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(d.b, "foo", & cursor), "b.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.cpp, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("cpp"), metatag.size, "cpp.metatag b size = sz cpp");
    STRNCMP_EQUAL_TEXT("cpp", metatag.ptr, metatag.size, "cpp.metatag b == cpp");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.cpp, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("dict"), metatag.size, "cpp.metatag type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", metatag.ptr, metatag.size, "cpp.metatag type == dict");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(d.cpp, "foo", & cursor), "cpp.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.cp, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("cp"), metatag.size, "cp.metatag c size = sz cp");
    STRNCMP_EQUAL_TEXT("cp", metatag.ptr, metatag.size, "cp.metatag c == cp");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.cp, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("dict"), metatag.size, "cp.metatag type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", metatag.ptr, metatag.size, "cp.metatag type == dict");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(d.cp, "foo", & cursor), "cp.metatag foo == null");

	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.c, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "c.metatag c size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "c.metatag c == c");
	cursor = 0;
    metatag = huGetMetatagWithKeyZ(d.c, "type", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("value"), metatag.size, "c.metatag type size = sz value");
    STRNCMP_EQUAL_TEXT("value", metatag.ptr, metatag.size, "c.metatag type == value");
	cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(d.c, "foo", & cursor), "c.metatag foo == null");
}

TEST(huGetMetatagWithKey, pathological)
{
	huSize_t zero = 0;

	auto lroot = l.root;

	const huNode * nodes[] =     {lroot, NULL, HU_NULLNODE};
	const char * keys[] =        {"foo", NULL, ""};
	huSize_t * cursors[] =       {& zero, NULL};

	for (long unsigned c = 0; c < sizeof(cursors) / sizeof(cursors[0]); ++c)
	{
		huSize_t * pc = cursors[c];
		huSize_t pcv = 0;
		if (pc != NULL)
			{ pcv = * pc; }
		for (long unsigned k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
		{
			const char * pk = keys[k];
			for (long unsigned n = 0; n < sizeof(nodes) / sizeof(nodes[0]); ++n)
			{
				const huNode * pn = nodes[n];
				if (c != 0 || k != 0 || n != 0)
				{
					std::ostringstream oss;
					oss << "c: " << c << "; k: " << k << "; n: " << n;
					POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithKeyZ(pn, pk, pc), oss.str());
					if (pc != NULL)
						{ * pc = pcv; }
				}
			}
		}
	}
}


TEST_GROUP(huGetNumMetatagsWithValue)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumMetatagsWithValue, lists)
{
    LONGS_EQUAL_TEXT(0, huGetNumMetatagsWithValueZ(l.root, "foo"), "root.gnabv foo == 0");
    LONGS_EQUAL_TEXT(2, huGetNumMetatagsWithValueZ(l.root, "root"), "root.gnabv root == 2");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.a, "a"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.a, "value"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.bp, "bp"), "bp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.bp, "list"), "bp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.b, "b"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.b, "value"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.cpp, "cpp"), "cpp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.cpp, "list"), "cpp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.cp, "cp"), "cp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.cp, "list"), "cp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.c, "c"), "c.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(l.c, "value"), "c.gnabv value == 1");
}

TEST(huGetNumMetatagsWithValue, dicts)
{
    LONGS_EQUAL_TEXT(0, huGetNumMetatagsWithValueZ(d.root, "foo"), "root.gnabv foo == 0");
    LONGS_EQUAL_TEXT(2, huGetNumMetatagsWithValueZ(d.root, "root"), "root.gnabv root == 2");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.a, "a"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.a, "value"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.bp, "bp"), "bp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.bp, "dict"), "bp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.b, "b"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.b, "value"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.cpp, "cpp"), "cpp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.cpp, "dict"), "cpp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.cp, "cp"), "cp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.cp, "dict"), "cp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.c, "c"), "c.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumMetatagsWithValueZ(d.c, "value"), "c.gnabv value == 1");
}

TEST(huGetNumMetatagsWithValue, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumMetatagsWithValueZ(NULL, "foo"), "NULL.gnabv foo == 0");
    LONGS_EQUAL_TEXT(0, huGetNumMetatagsWithValueZ(HU_NULLNODE, "foo"), "null.gnabv foo == 0");
    LONGS_EQUAL_TEXT(0, huGetNumMetatagsWithValueZ(l.root, NULL), "NULL.gnabv NULL == 0");
    LONGS_EQUAL_TEXT(0, huGetNumMetatagsWithValueZ(l.root, ""), "NULL.gnabv '' == 0");
}


TEST_GROUP(huGetMetatagWithValue)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetMetatagWithValue, lists)
{
    huSize_t cursor = 0;
    auto metatag = huGetMetatagWithValueZ(l.root, "root", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("name"), metatag.size, "root.metatag v0 name size = sz root");
    STRNCMP_EQUAL_TEXT("name", metatag.ptr, metatag.size, "root.metatag v0 name == root");
    metatag = huGetMetatagWithValueZ(l.root, "root", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("otherName"), metatag.size, "root.metatag v1 otherName size = sz root");
    STRNCMP_EQUAL_TEXT("otherName", metatag.ptr, metatag.size, "root.metatag v1 otherName == root");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.root, "foo", & cursor), "root.hawk foo == false");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(l.a, "a", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("a"), metatag.size, "a.metatag v0 a size = sz a");
    STRNCMP_EQUAL_TEXT("a", metatag.ptr, metatag.size, "a.metatag v0 a == a");
    metatag = huGetMetatagWithValueZ(l.a, "value", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "a.metatag v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "a.metatag v0 value == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.a, "foo", & cursor), "a.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(l.bp, "bp", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("b"), metatag.size, "bp.metatag v0 bp size = sz b");
    STRNCMP_EQUAL_TEXT("b", metatag.ptr, metatag.size, "bp.metatag v0 bp == b");
    metatag = huGetMetatagWithValueZ(l.bp, "list", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "bp.metatag v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "bp.metatag v0 list == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.bp, "foo", & cursor), "bp.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(l.b, "b", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("b"), metatag.size, "b.metatag v0 b size = sz b");
    STRNCMP_EQUAL_TEXT("b", metatag.ptr, metatag.size, "b.metatag v0 b == b");
    metatag = huGetMetatagWithValueZ(l.b, "value", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "b.metatag v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "b.metatag v0 value == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.b, "foo", & cursor), "b.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(l.cpp, "cpp", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "cpp.metatag v0 cpp size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "cpp.metatag v0 cpp == c");
    metatag = huGetMetatagWithValueZ(l.cpp, "list", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "cpp.metatag v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "cpp.metatag v0 list == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.cpp, "foo", & cursor), "cpp.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(l.cp, "cp", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "cp.metatag v0 cp size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "cp.metatag v0 cp == c");
    metatag = huGetMetatagWithValueZ(l.cp, "list", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "cp.metatag v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "cp.metatag v0 list == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.cp, "foo", & cursor), "cp.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(l.c, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "c.metatag v0 c size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "c.metatag v0 c == c");
    metatag = huGetMetatagWithValueZ(l.c, "value", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "c.metatag v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "c.metatag v0 value == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.c, "foo", & cursor), "c.metatag foo == null");
}

TEST(huGetMetatagWithValue, dicts)
{
    huSize_t cursor = 0;
    auto metatag = huGetMetatagWithValueZ(d.root, "root", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("name"), metatag.size, "root.metatag v0 name size = sz root");
    STRNCMP_EQUAL_TEXT("name", metatag.ptr, metatag.size, "root.metatag v0 name == root");
    metatag = huGetMetatagWithValueZ(d.root, "root", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("otherName"), metatag.size, "root.metatag v1 otherName size = sz root");
    STRNCMP_EQUAL_TEXT("otherName", metatag.ptr, metatag.size, "root.metatag v1 otherName == root");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(d.root, "foo", & cursor), "root.hawk foo == false");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(d.a, "a", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("a"), metatag.size, "a.metatag v0 a size = sz a");
    STRNCMP_EQUAL_TEXT("a", metatag.ptr, metatag.size, "a.metatag v0 a == a");
    metatag = huGetMetatagWithValueZ(d.a, "value", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "a.metatag v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "a.metatag v0 value == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(d.a, "foo", & cursor), "a.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(d.bp, "bp", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("b"), metatag.size, "bp.metatag v0 bp size = sz b");
    STRNCMP_EQUAL_TEXT("b", metatag.ptr, metatag.size, "bp.metatag v0 bp == b");
    metatag = huGetMetatagWithValueZ(d.bp, "dict", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "bp.metatag v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "bp.metatag v0 dict == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(d.bp, "foo", & cursor), "bp.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(d.b, "b", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("b"), metatag.size, "b.metatag v0 b size = sz b");
    STRNCMP_EQUAL_TEXT("b", metatag.ptr, metatag.size, "b.metatag v0 b == b");
    metatag = huGetMetatagWithValueZ(d.b, "value", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "b.metatag v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "b.metatag v0 value == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(d.b, "foo", & cursor), "b.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(d.cpp, "cpp", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "cpp.metatag v0 cpp size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "cpp.metatag v0 cpp == c");
    metatag = huGetMetatagWithValueZ(d.cpp, "dict", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "cpp.metatag v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "cpp.metatag v0 dict == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(d.cpp, "foo", & cursor), "cpp.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(d.cp, "cp", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "cp.metatag v0 cp size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "cp.metatag v0 cp == c");
    metatag = huGetMetatagWithValueZ(d.cp, "dict", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "cp.metatag v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "cp.metatag v0 dict == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(d.cp, "foo", & cursor), "cp.metatag foo == null");

    cursor = 0;
    metatag = huGetMetatagWithValueZ(d.c, "c", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("c"), metatag.size, "c.metatag v0 c size = sz c");
    STRNCMP_EQUAL_TEXT("c", metatag.ptr, metatag.size, "c.metatag v0 c == c");
    metatag = huGetMetatagWithValueZ(d.c, "value", & cursor)->str;
    LONGS_EQUAL_TEXT(strlen("type"), metatag.size, "c.metatag v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", metatag.ptr, metatag.size, "c.metatag v0 value == type");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(d.c, "foo", & cursor), "c.metatag foo == null");
}

TEST(huGetMetatagWithValue, pathological)
{
    huSize_t cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(NULL, "", & cursor), "NULL.metatag v0 '' == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(NULL, "foo", & cursor), "NULL.metatag v0 foo == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(HU_NULLNODE, "", & cursor), "null.metatag v0 '' == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(HU_NULLNODE, "foo", & cursor), "null.metatag v0 foo == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.root, "", & cursor), "root.metatag v0 '' == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.root, NULL, & cursor), "root.metatag v0 NULL == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.root, "root", NULL), "root.metatag v-1 root == null");
    cursor = -1;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.root, "root", & cursor), "root.metatag v-1 root == null");
    cursor = 3;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetMetatagWithValueZ(l.root, "root", & cursor), "root.metatag v3 root == null");
}


TEST_GROUP(huGetNumComments)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumComments, lists)
{
    LONGS_EQUAL_TEXT(0, huGetNumComments(l.root), "root.gnc == 0");
    LONGS_EQUAL_TEXT(2, huGetNumComments(l.a), "a.gnc == 2");
    LONGS_EQUAL_TEXT(2, huGetNumComments(l.bp), "bp.gnc == 2");
    LONGS_EQUAL_TEXT(1, huGetNumComments(l.b), "b.gnc == 1");
    LONGS_EQUAL_TEXT(2, huGetNumComments(l.cpp), "cpp.gnc == 2");
    LONGS_EQUAL_TEXT(1, huGetNumComments(l.cp), "cp.gnc == 1");
    LONGS_EQUAL_TEXT(1, huGetNumComments(l.c), "c.gnc == 1");
}

TEST(huGetNumComments, dicts)
{
    LONGS_EQUAL_TEXT(0, huGetNumComments(d.root), "root.gnc == 0");
    LONGS_EQUAL_TEXT(2, huGetNumComments(d.a), "a.gnc == 2");
    LONGS_EQUAL_TEXT(2, huGetNumComments(d.bp), "bp.gnc == 2");
    LONGS_EQUAL_TEXT(1, huGetNumComments(d.b), "b.gnc == 1");
    LONGS_EQUAL_TEXT(2, huGetNumComments(d.cpp), "cpp.gnc == 2");
    LONGS_EQUAL_TEXT(1, huGetNumComments(d.cp), "cp.gnc == 1");
    LONGS_EQUAL_TEXT(1, huGetNumComments(d.c), "c.gnc == 1");
}

TEST(huGetNumComments, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumComments(NULL), "NULL.gnc == 0");
    LONGS_EQUAL_TEXT(0, huGetNumComments(HU_NULLNODE), "null.gnc == 0");
}


TEST_GROUP(huGetComment)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetComment, lists)
{
    CHECK(huGetComment(d.a, 0) != NULL);
    auto comm = huGetComment(l.a, 0)->str;
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "a.comm 0 == exp");
    CHECK(huGetComment(l.a, 0) != NULL);
    comm = huGetComment(l.a, 1)->str;
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "a.comm 1 == exp");

    comm = huGetComment(l.bp, 0)->str;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "bp.comm 0 == exp");
    comm = huGetComment(l.bp, 1)->str;
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "bp.comm 1 == exp");

    comm = huGetComment(l.b, 0)->str;
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "b.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "b.comm 0 == exp");

    comm = huGetComment(l.cpp, 0)->str;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "cpp.comm 0 == exp");
    comm = huGetComment(l.cpp, 1)->str;
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "cpp.comm 1 == exp");

    comm = huGetComment(l.cp, 0)->str;
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "cp.comm 0 == exp");

    comm = huGetComment(l.c, 0)->str;
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "c.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "c.comm 0 == exp");
}

TEST(huGetComment, dicts)
{
    CHECK(huGetComment(d.a, 0) != NULL);
    auto comm = huGetComment(d.a, 0)->str;
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "a.comm 0 == exp");
    CHECK(huGetComment(d.a, 1) != NULL);
    comm = huGetComment(d.a, 1)->str;
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "a.comm 1 == exp");

    comm = huGetComment(d.bp, 0)->str;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "bp.comm 0 == exp");
    comm = huGetComment(d.bp, 1)->str;
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "bp.comm 1 == exp");

    comm = huGetComment(d.b, 0)->str;
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "b.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "b.comm 0 == exp");

    comm = huGetComment(d.cpp, 0)->str;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "cpp.comm 0 == exp");
    comm = huGetComment(d.cpp, 1)->str;
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "cpp.comm 1 == exp");

    comm = huGetComment(d.cp, 0)->str;
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "cp.comm 0 == exp");

    comm = huGetComment(d.c, 0)->str;
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "c.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.ptr, comm.size, "c.comm 0 == exp");
}

TEST(huGetComment, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetComment(NULL, 0), "NULL.comm 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetComment(HU_NULLNODE, 0), "null.comm 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetComment(l.root, 0), "root.comm 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetComment(l.root, -1), "root.comm -1 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetComment(l.a, 2), "root.comm 2 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetComment(l.a, -1), "root.comm -1 == null");
}


TEST_GROUP(huHasCommentsContaining)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huHasCommentsContaining, lists)
{
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.a, "aaa"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.a, "right here"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.bp, "bp"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.bp, "right here"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.b, "bbb"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.cpp, "cpp"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.cpp, "right here"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.cp, "cp"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(l.cp, "c"));
}

TEST(huHasCommentsContaining, dicts)
{
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.a, "aaa"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.a, "right here"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.bp, "bp"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.bp, "right here"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.b, "bbb"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.cpp, "cpp"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.cpp, "right here"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.cp, "cp"));
    LONGS_EQUAL(1, huHasCommentsContainingZ(d.cp, "c"));
}

TEST(huHasCommentsContaining, pathological)
{
    LONGS_EQUAL(0, huHasCommentsContainingN(NULL, "aaa", 3));
    LONGS_EQUAL(0, huHasCommentsContainingN(d.a, NULL, 3));
    LONGS_EQUAL(0, huHasCommentsContainingN(d.a, "aaa", -1));
}


TEST_GROUP(huGetNumCommentsContaining)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumCommentsContaining, lists)
{
    LONGS_EQUAL(2, huGetNumCommentsContainingZ(l.a, "aaa"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(l.a, "right here"));
    LONGS_EQUAL(2, huGetNumCommentsContainingZ(l.bp, "bp"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(l.bp, "right here"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(l.b, "bbb"));
    LONGS_EQUAL(2, huGetNumCommentsContainingZ(l.cpp, "cpp"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(l.cpp, "right here"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(l.cp, "cp"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(l.c, "ccc"));
}

TEST(huGetNumCommentsContaining, dicts)
{
    LONGS_EQUAL(2, huGetNumCommentsContainingZ(d.a, "aaa"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(d.a, "right here"));
    LONGS_EQUAL(2, huGetNumCommentsContainingZ(d.bp, "bp"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(d.bp, "right here"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(d.b, "bbb"));
    LONGS_EQUAL(2, huGetNumCommentsContainingZ(d.cpp, "cpp"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(d.cpp, "right here"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(d.cp, "cp"));
    LONGS_EQUAL(1, huGetNumCommentsContainingZ(d.c, "ccc"));
}

TEST(huGetNumCommentsContaining, pathological)
{
    LONGS_EQUAL(0, huGetNumCommentsContainingN(NULL, "aaa", 3));
    LONGS_EQUAL(0, huGetNumCommentsContainingN(d.a, NULL, 3));
    LONGS_EQUAL(0, huGetNumCommentsContainingN(d.a, "aaa", -1));
}


TEST_GROUP(huGetCommentsContaining)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetCommentsContaining, lists)
{
    huSize_t cursor = 0;
    huToken const * comm = huGetCommentsContainingZ(l.a, "aaa", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(l.a, "aaa", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 1 == exp");
    comm = huGetCommentsContainingZ(l.a, "aaa", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc aaa 2 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.a, "right here", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(l.a, "right here", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc right here 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.bp, "bp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(l.bp, "bp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 1 == exp");
    comm = huGetCommentsContainingZ(l.bp, "bp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc bp 2 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.bp, "right here", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(l.bp, "right here", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc bp 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.b, "bbb", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "b.gcc bbb 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "b.gcc bbb 0 == exp");
    comm = huGetCommentsContainingZ(l.b, "bbb", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc b 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.cpp, "cpp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(l.cpp, "cpp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 1 == exp");
    comm = huGetCommentsContainingZ(l.cpp, "cpp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc cpp 2 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.cpp, "right here", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(l.cpp, "right here", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "cpp.gcc right here 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.cp, "cp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cp.gcc cp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc cp 0 == exp");
    comm = huGetCommentsContainingZ(l.cp, "cp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "cp.gcc cp 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(l.c, "ccc", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "c.gcc ccc 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc ccc 0 == exp");
    comm = huGetCommentsContainingZ(l.c, "ccc", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "c.gcc ccc 1 = null");
}

TEST(huGetCommentsContaining, dicts)
{
    huSize_t cursor = 0;
    huToken const * comm = huGetCommentsContainingZ(d.a, "aaa", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(d.a, "aaa", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 1 == exp");
    comm = huGetCommentsContainingZ(d.a, "aaa", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc aaa 2 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.a, "right here", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(d.a, "right here", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc right here 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.bp, "bp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(d.bp, "bp", & cursor);
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 1 == exp");
    comm = huGetCommentsContainingZ(d.bp, "bp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc bp 2 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.bp, "right here", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(d.bp, "right here", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc bp 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.b, "bbb", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "b.gcc bbb 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "b.gcc bbb 0 == exp");
    comm = huGetCommentsContainingZ(d.b, "bbb", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc b 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.cpp, "cpp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(d.cpp, "cpp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 1 == exp");
    comm = huGetCommentsContainingZ(d.cpp, "cpp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "a.gcc cpp 2 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.cpp, "right here", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(d.cpp, "right here", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "cpp.gcc right here 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.cp, "cp", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cp.gcc cp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc cp 0 == exp");
    comm = huGetCommentsContainingZ(d.cp, "cp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "cp.gcc cp 1 = null");

    cursor = 0;
    comm = huGetCommentsContainingZ(d.c, "ccc", & cursor);
    CHECK(comm != HU_NULLTOKEN);
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "c.gcc ccc 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc ccc 0 == exp");
    comm = huGetCommentsContainingZ(d.c, "ccc", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, comm, "c.gcc ccc 1 = null");
}

TEST(huGetCommentsContaining, pathological)
{
    huSize_t cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetCommentsContainingZ(NULL, "aaa", & cursor), "NULL.gcc aaa == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetCommentsContainingZ(HU_NULLNODE, "aaa", & cursor), "null.gcc aaa == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetCommentsContainingZ(l.root, "aaa", & cursor), "root.comm aaa == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetCommentsContainingZ(l.root, NULL, & cursor), "a.comm NULL == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetCommentsContainingZ(l.a, NULL, & cursor), "a.comm NULL == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetCommentsContainingZ(l.a, NULL, NULL), "a.comm NULL == null");
    cursor = -1;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetCommentsContainingZ(l.a, NULL, & cursor), "a.comm NULL == null");
}


TEST_GROUP(huGetNodeByRelativeAddress)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNodeByRelativeAddress, lists)
{
    POINTERS_EQUAL(l.a, huGetNodeByRelativeAddressZ(l.root, "0"));
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.root, "0/.."));
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.a, ".."));
    POINTERS_EQUAL(l.b, huGetNodeByRelativeAddressZ(l.bp, "0"));
    POINTERS_EQUAL(l.bp, huGetNodeByRelativeAddressZ(l.bp, "0/.."));
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.bp, "0/../.."));
    POINTERS_EQUAL(l.cp, huGetNodeByRelativeAddressZ(l.cpp, "0"));
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.cpp, "0/0"));
    POINTERS_EQUAL(l.cpp, huGetNodeByRelativeAddressZ(l.cpp, "0/0/../.."));
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.cp, "0"));
    POINTERS_EQUAL(l.cpp, huGetNodeByRelativeAddressZ(l.cp, ".."));
    POINTERS_EQUAL(l.cp, huGetNodeByRelativeAddressZ(l.c, ".."));
    POINTERS_EQUAL(l.cpp, huGetNodeByRelativeAddressZ(l.c, "../.."));
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.c, "../../.."));
    POINTERS_EQUAL(l.b, huGetNodeByRelativeAddressZ(l.a, "../1/0"));
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.a, "../2/0/0"));
    POINTERS_EQUAL(l.a, huGetNodeByRelativeAddressZ(l.b, "../../0"));
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.b, "../../2/0/0"));
    POINTERS_EQUAL(l.a, huGetNodeByRelativeAddressZ(l.c, "../../../0"));
    POINTERS_EQUAL(l.b, huGetNodeByRelativeAddressZ(l.c, "../../../1/0"));
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.b, " .. / .. / 2 / 0 / 0 "));

    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, ".."));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.a, "0"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, "3"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, "0/0"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, "1/1"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, "1/0/0"));
}

TEST(huGetNodeByRelativeAddress, dicts)
{
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.root, "0"));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.root, "0/.."));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.a, ".."));
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.bp, "0"));
    POINTERS_EQUAL(d.bp, huGetNodeByRelativeAddressZ(d.bp, "0/.."));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.bp, "0/../.."));
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.cpp, "0"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cpp, "0/0"));
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cpp, "0/0/../.."));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cp, "0"));
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cp, ".."));
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.c, ".."));
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.c, "../.."));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.c, "../../.."));
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.a, "../1/0"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.a, "../2/0/0"));
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.b, "../../0"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, "../../2/0/0"));
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.c, "../../../0"));
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.c, "../../../1/0"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, " .. / .. / 2 / 0 / 0 "));

    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.root, "ak"));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.root, "ak/.."));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.a, ".."));
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.bp, "bk"));
    POINTERS_EQUAL(d.bp, huGetNodeByRelativeAddressZ(d.bp, "bk/.."));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.bp, "bk/../.."));
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.cpp, "ck"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cpp, "ck/ck"));
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cpp, "ck/ck/../.."));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cp, "ck"));
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cp, ".."));
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.c, ".."));
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.c, "../.."));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.c, "../../.."));
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.a, "../bk/bk"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.a, "../ck/ck/ck"));
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.b, "../../ak"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, "../../ck/ck/ck"));
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.c, "../../../ak"));
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.c, "../../../bk/bk"));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, " .. / .. / ck / ck / ck "));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, "../../`ck`/'ck'/\"ck\""));
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, " .. / .. / `ck` / 'ck' / \"ck\" "));

    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(d.root, ".."));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(d.a, "0"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(d.root, "3"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(d.root, "0/0"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(d.root, "1/1"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(d.root, "1/0/0"));
}

TEST(huGetNodeByRelativeAddress, pathological)
{
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(NULL, "0"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(HU_NULLNODE, "0"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, "-1"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.a, "-1"));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, NULL));
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.root, ""));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.root, ""));
    POINTERS_EQUAL(HU_NULLNODE, huGetNodeByRelativeAddressZ(l.root, "/"));
}


TEST_GROUP(huGetAddress)
{
    htd_listOfLists l;
    htd_dictOfDicts d;
    htd_withFunkyAddresses a;
	htd_sharedKeys t;

    void setup()
    {
        l.setup();
        d.setup();
        a.setup();
		t.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
        a.teardown();
		t.teardown();
    }
};

TEST(huGetAddress, lists)
{
    huSize_t addressLen = 0;
    huGetAddress(l.root, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(l.root, s, & addressLen);
    auto exp = "/";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(l.a, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(l.a, s, & addressLen);
    exp = "/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(l.bp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(l.bp, s, & addressLen);
    exp = "/1";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(l.b, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(l.b, s, & addressLen);
    exp = "/1/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(l.cpp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(l.cpp, s, & addressLen);
    exp = "/2";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(l.cp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(l.cp, s, & addressLen);
    exp = "/2/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(l.c, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(l.c, s, & addressLen);
    exp = "/2/0/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;
}


TEST(huGetAddress, dicts)
{
    huSize_t addressLen = 0;
    huGetAddress(d.root, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(d.root, s, & addressLen);
    auto exp = "/";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(d.a, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(d.a, s, & addressLen);
    exp = "/ak";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(d.bp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(d.bp, s, & addressLen);
    exp = "/bk";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(d.b, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(d.b, s, & addressLen);
    exp = "/bk/bk";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(d.cpp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(d.cpp, s, & addressLen);
    exp = "/ck";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(d.cp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(d.cp, s, & addressLen);
    exp = "/ck/ck";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetAddress(d.c, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(d.c, s, & addressLen);
    exp = "/ck/ck/ck";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;
}

TEST(huGetAddress, overflow)
{
    huSize_t addressLen = 0;
    huGetAddress(d.c, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    addressLen = 3;
    s[addressLen] = '!';
    huGetAddress(d.c, s, & addressLen);
    char const * exp = "/ck!";             // contains "/ck/ck/ck"
    LONGS_EQUAL(3, addressLen);
    STRNCMP_EQUAL(exp, s, 4);
    delete [] s;
}

TEST(huGetAddress, funky)
{
    huSize_t addressLen = 0;
    huGetAddress(a.a, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.a, s, & addressLen);
    auto exp = "/^^/^^";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.a, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.b, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.b, s, & addressLen);
    exp = "/^^a/b^^";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.b, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.c, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.c, s, & addressLen);
    exp = "/^^/b^^";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.c, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.d, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.d, s, & addressLen);
    exp = "/^^a/^^";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.d, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.e, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.e, s, & addressLen);
    exp = R"(/^^/"foo"^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.e, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.f, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.f, s, & addressLen);
    exp = R"(/^^/"foo'bar'"^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.f, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.g, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.g, s, & addressLen);
    exp = R"(/^^/"foo'bar`baz`'"^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.g, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.h, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.h, s, & addressLen);
    exp = R"(/^^/'foo`bar"baz"`'^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.h, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.i, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.i, s, & addressLen);
    exp = R"(/^^/`foo"bar'baz'"`^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.i, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.j, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.j, s, & addressLen);
    exp = R"(/^^a"foo"/^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.j, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.k, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.k, s, & addressLen);
    exp = R"(/^^a"foo'bar'"/^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.k, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.l, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.l, s, & addressLen);
    exp = R"(/^^a"foo'bar`baz`'"/^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.l, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.m, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.m, s, & addressLen);
    exp = R"(/^^a'foo`bar"baz"`'/^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.m, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.n, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.n, s, & addressLen);
    exp = R"(/^^a`foo"bar'baz'"`/^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.n, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.o, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.o, s, & addressLen);
    exp = R"(/'0')";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.o, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.p, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.p, s, & addressLen);
    exp = "/01m";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.p, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.q, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.q, s, & addressLen);
    exp = R"(/^^/"0123456789012345678901234567890123456789"^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.q, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetAddress(a.r, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetAddress(a.r, s, & addressLen);
    exp = R"(/^^a"0123456789012345678901234567890123456789"/^^)";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.r, huGetNodeByAddressN(a.trove, s, addressLen));
    delete [] s;
}

TEST(huGetAddress, sharedKeyIndex)
{
	huSize_t numChildren = huGetNumChildren(t.root);
	for (huSize_t i = 0; i < numChildren; ++i)
	{
		huNode const * ch = huGetChildByIndex(t.root, i);
		huToken const * a = huGetValue(huGetChildByIndex(ch, 0));
		auto astr = std::string("/") + std::string(std::string_view(a->str.ptr, a->str.size));
		huSize_t alen = 0;
		huGetAddress(ch, NULL, & alen);
		char * s = new char[alen + 1];
		s[alen] = '\0';
		huGetAddress(ch, s, & alen);
		STRNCMP_EQUAL_TEXT(astr.data(), s, alen, std::string(s));
	}
}

TEST(huGetAddress, pathological)
{
    huSize_t len = 256;
    char str[256] = {0};
    huGetAddress(NULL, NULL, & len);
    LONGS_EQUAL_TEXT(0, str[0], "NULL");

    huGetAddress(NULL, str, & len);
    LONGS_EQUAL_TEXT(0, str[0], "NULL");

    huGetAddress(HU_NULLNODE, NULL, & len);
    LONGS_EQUAL_TEXT(0, str[0], "null");

    huGetAddress(HU_NULLNODE, str, & len);
    LONGS_EQUAL_TEXT(0, str[0], "null");
}


// ------------------------------ TROVE API TESTS

TEST_GROUP(huDeserializeTrove)
{
};

// Patho cases only for this method. Used extensively elsewhere.
TEST(huDeserializeTrove, pathological)
{
    huTrove * trove = HU_NULLTROVE;
    int error = HU_ERROR_NOERROR;
    huDeserializeOptions params;
    huInitDeserializeOptions(& params, HU_ENCODING_UTF8, true, 4, NULL, HU_BUFFERMANAGEMENT_COPYANDOWN);

    error = huDeserializeTroveZ(NULL, "", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);

    trove = (huTrove *) 4;
    error = huDeserializeTroveZ(& trove, NULL, & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromString NULL == NULL");

    trove = (huTrove *) 4;
    error = huDeserializeTroveZ(& trove, "", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_NOERROR, error);
    CHECK_TEXT(HU_NULLTROVE != trove, "fromString '' != NULL");
    huDestroyTrove(trove);

    trove = (huTrove *) 4;
    params.encoding = (huEncoding) -1;
    error = huDeserializeTroveZ(& trove, "[]", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "encoding=-1 == NULL");

    trove = (huTrove *) 4;
    params.encoding = (huEncoding)(HU_ENCODING_UNKNOWN + 1);
    error = huDeserializeTroveZ(& trove, "[]", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "encoding=unk+1 == NULL");

    if (isSignedType(huCol_t))
    {
        trove = (huTrove *) 4;
        params.encoding = HU_ENCODING_UTF8;
        params.tabSize = -1;
        error = huDeserializeTroveZ(& trove, "[]", & params, HU_ERRORRESPONSE_MUM);
        LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
        POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "tabs=-1 == NULL");
    }

    trove = (huTrove *) 4;
    params.tabSize = 4;
    error = huDeserializeTroveZ(& trove, "[]", & params, (huErrorResponse) -1);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "errorResponse=-1 == NULL");

    trove = (huTrove *) 4;
    error = huDeserializeTroveZ(& trove, "[]", & params, HU_ERRORRESPONSE_NUMRESPONSES);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "errorResponse=big == NULL");
}


TEST_GROUP(huDeserializeTroveFromFile)
{
    int numAllocs = 0;
    int numReallocs = 0;
    int numFrees = 0;
};

void * myalloc(void * allocator, size_t len)
{
    auto allocatorator = (TestGroup_huDeserializeTroveFromFile *) allocator;
    allocatorator->numAllocs += 1;
    return malloc(len);
}

void * myrealloc(void * allocator, void * alloc, size_t len)
{
    auto allocatorator = (TestGroup_huDeserializeTroveFromFile *) allocator;
    allocatorator->numReallocs += 1;
    return realloc(alloc, len);
}

void myfree(void * allocator, void * alloc)
{
    auto allocatorator = (TestGroup_huDeserializeTroveFromFile *) allocator;
    allocatorator->numFrees += 1;
    return free(alloc);
}

TEST(huDeserializeTroveFromFile, malloc)
{
    huTrove * trove = HU_NULLTROVE;
    int error = HU_ERROR_NOERROR;
    huDeserializeOptions params;
    huAllocator alloc;
    alloc.manager = this;
    alloc.memAlloc = myalloc;
    alloc.memRealloc = myrealloc;
    alloc.memFree = myfree;
    huInitDeserializeOptions(& params, HU_ENCODING_UTF8, true, 4, & alloc, HU_BUFFERMANAGEMENT_COPYANDOWN);

    error = huDeserializeTroveFromFile(& trove, "test/testFiles/utf8.hu", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_NOERROR, error);
    huDestroyTrove(trove);

    LONGS_EQUAL(12, numAllocs);
    LONGS_EQUAL(2, numReallocs);
    LONGS_EQUAL(12, numFrees);
}

TEST(huDeserializeTroveFromFile, pathological)
{
    huTrove * trove = HU_NULLTROVE;
    int error = HU_ERROR_NOERROR;
    huDeserializeOptions params;
    huInitDeserializeOptions(& params, HU_ENCODING_UTF8, true, 4, NULL, HU_BUFFERMANAGEMENT_COPYANDOWN);

    error = huDeserializeTroveFromFile(NULL, "", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile NULL == NULL");

    trove = (huTrove *) 4;
    error = huDeserializeTroveFromFile(& trove, NULL, & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile NULL == NULL");

    trove = (huTrove *) 4;
    error = huDeserializeTroveFromFile(& trove, "", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADFILE, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile '' == NULL");

    trove = (huTrove *) 4;
    error = huDeserializeTroveFromFile(& trove, "..", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADFILE, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile .. == NULL");

    trove = (huTrove *) 4;
    error = huDeserializeTroveFromFile(& trove, "/", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADFILE, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile / == NULL");

    trove = (huTrove *) 4;
    params.encoding = (huEncoding) -1;
    error = huDeserializeTroveFromFile(& trove, "test/testFiles/utf8.hu", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile encoding=-1 == NULL");

    trove = (huTrove *) 4;
    params.encoding = (huEncoding)(HU_ENCODING_UNKNOWN + 1);
    error = huDeserializeTroveFromFile(& trove, "test/testFiles/utf8.hu", & params, HU_ERRORRESPONSE_MUM);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile encoding=unk+1 == NULL");

    if (isSignedType(huCol_t))
    {
        trove = (huTrove *) 4;
        params.encoding = HU_ENCODING_UTF8;
        params.tabSize = -1;
        error = huDeserializeTroveFromFile(& trove, "test/testFiles/utf8.hu", & params, HU_ERRORRESPONSE_MUM);
        LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
        POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile tabSize=-1 == NULL");
    }

    trove = (huTrove *) 4;
    params.tabSize = 4;
    error = huDeserializeTroveFromFile(& trove, "test/testFiles/utf8.hu", & params, (huErrorResponse) -1);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile errorResponse=-1 == NULL");

    trove = (huTrove *) 4;
    error = huDeserializeTroveFromFile(& trove, "test/testFiles/utf8.hu", & params, HU_ERRORRESPONSE_NUMRESPONSES);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(HU_NULLTROVE, trove, "fromFile errorResponse=high == NULL");
}


TEST_GROUP(huDestroyTrove)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

// Patho cases only for this method. Used extensively elsewhere.
TEST(huDestroyTrove, pathological)
{
    // These guys just fail if they throw.
    huDestroyTrove(NULL);
    huDestroyTrove(HU_NULLTROVE);
}


TEST_GROUP(huGetNumTokens)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumTokens, normal)
{
    LONGS_EQUAL_TEXT(91, huGetNumTokens(l.trove), "l numTokens == 90");
    LONGS_EQUAL_TEXT(103, huGetNumTokens(d.trove), "d numTokens == 102");
}

TEST(huGetNumTokens, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumTokens(NULL), "NULL numTokens == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTokens(HU_NULLTROVE), "null numTokens == 0");
}


TEST_GROUP(huGetToken)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetToken, normal)
{
    CHECK(huGetToken(l.trove, 90) != HU_NULLTOKEN);
    LONGS_EQUAL_TEXT(HU_TOKENKIND_EOF, huGetToken(l.trove, 90)->kind, "l tok last == eof");
    CHECK(huGetToken(d.trove, 102) != HU_NULLTOKEN);
    LONGS_EQUAL_TEXT(HU_TOKENKIND_EOF, huGetToken(d.trove, 102)->kind, "l tok last == eof");
}

TEST(huGetToken, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetToken(NULL, 0), "NULL tok 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetToken(HU_NULLTROVE, 0), "null tok 0 == null");
}


TEST_GROUP(huGetNumNodes)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumNodes, normal)
{
    LONGS_EQUAL_TEXT(7, huGetNumNodes(l.trove), "l numNodes == 7");
    LONGS_EQUAL_TEXT(7, huGetNumNodes(d.trove), "d numNodes == 7");
}

TEST(huGetNumNodes, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumNodes(NULL), "NULL numNodes == 0");
    LONGS_EQUAL_TEXT(0, huGetNumNodes(HU_NULLTROVE), "null numNodes == 0");
}


TEST_GROUP(huGetRootNode)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetRootNode, normal)
{
    CHECK(huGetRootNode(l.trove) != HU_NULLNODE);
    LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, huGetRootNode(l.trove)->kind, "l grn == list");
    LONGS_EQUAL_TEXT(-1, huGetRootNode(l.trove)->parentNodeIdx, "l grn == root");
    CHECK(huGetRootNode(d.trove) != HU_NULLNODE);
    LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, huGetRootNode(d.trove)->kind, "d grn == dict");
    LONGS_EQUAL_TEXT(-1, huGetRootNode(d.trove)->parentNodeIdx, "d grn == root");
}

TEST(huGetRootNode, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetRootNode(NULL), "NULL grn == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetRootNode(HU_NULLTROVE), "null grn == null");
}


TEST_GROUP(huGetNode)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNode, normal)
{
    CHECK(huGetNodeByIndex(l.trove, 0) != HU_NULLNODE);
    LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, huGetNodeByIndex(l.trove, 0)->kind, "l gn 0 == list");
    LONGS_EQUAL_TEXT(-1, huGetNodeByIndex(l.trove, 0)->parentNodeIdx, "l gn 0 == root");
    CHECK(huGetNodeByIndex(d.trove, 0) != HU_NULLNODE);
    LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, huGetNodeByIndex(d.trove, 0)->kind, "d gn 0 == dict");
    LONGS_EQUAL_TEXT(-1, huGetNodeByIndex(d.trove, 0)->parentNodeIdx, "d gn 0 == root");

    CHECK(huGetNodeByIndex(l.trove, 1) != HU_NULLNODE);
    LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, huGetNodeByIndex(l.trove, 1)->kind, "l gn 1 == list");
    LONGS_EQUAL_TEXT(0, huGetNodeByIndex(l.trove, 1)->parentNodeIdx, "l gn 1 == child of root");
    CHECK(huGetNodeByIndex(d.trove, 1) != HU_NULLNODE);
    LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, huGetNodeByIndex(d.trove, 1)->kind, "d gn 1 == dict");
    LONGS_EQUAL_TEXT(0, huGetNodeByIndex(d.trove, 1)->parentNodeIdx, "d gn 1 == child of root");
}

TEST(huGetNode, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByIndex(NULL, 0), "NULL gn 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByIndex(HU_NULLTROVE, 0), "null gn 0 == null");
}


TEST_GROUP(huGetNumErrors)
{
    htd_listOfLists l;
    htd_dictOfDicts d;
    htd_erroneous e;

    void setup()
    {
        l.setup();
        d.setup();
        e.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
        e.teardown();
    }
};

TEST(huGetNumErrors, normal)
{
    LONGS_EQUAL_TEXT(0, huGetNumErrors(l.trove), "l numErrors == 0");
    LONGS_EQUAL_TEXT(0, huGetNumErrors(d.trove), "d numErrors == 0");
    LONGS_EQUAL_TEXT(3, huGetNumErrors(e.trove), "e numErrors == 0");
}

TEST(huGetNumErrors, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumErrors(NULL), "NULL numErrors == 0");
    LONGS_EQUAL_TEXT(0, huGetNumErrors(HU_NULLTROVE), "null numErrors == 0");
}


TEST_GROUP(huGetError)
{
    htd_listOfLists l;
    htd_dictOfDicts d;
    htd_erroneous e;

    void setup()
    {
        l.setup();
        d.setup();
        e.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
        e.teardown();
    }
};

TEST(huGetError, normal)
{
    POINTERS_EQUAL_TEXT(NULL, huGetError(l.trove, 0), "l ge 0 == 0");
    POINTERS_EQUAL_TEXT(NULL, huGetError(d.trove, 0), "d ge 0 == 0");

    CHECK(huGetNumErrors(e.trove) > 0);
    LONGS_EQUAL_TEXT(HU_ERROR_SYNTAXERROR, huGetError(e.trove, 0)->errorCode, "e ge 0 == syntax error");
    LONGS_EQUAL_TEXT(12, huGetError(e.trove, 0)->token->line, "e ge 0 line == 11");
    LONGS_EQUAL_TEXT(12, huGetError(e.trove, 0)->token->col, "e ge 0 col == 12");

    LONGS_EQUAL_TEXT(12, huGetError(e.trove, 0)->line, "e ge 0 line == 11");
    LONGS_EQUAL_TEXT(12, huGetError(e.trove, 0)->col, "e ge 0 col == 12");
}

TEST(huGetError, pathological)
{
    POINTERS_EQUAL_TEXT(NULL, huGetError(NULL, 0), "NULL numErrors == 0");
    POINTERS_EQUAL_TEXT(NULL, huGetError(HU_NULLTROVE, 0), "null numErrors == 0");
    POINTERS_EQUAL_TEXT(NULL, huGetError(e.trove, -1), "e ge 0 == syntax error");
}



TEST_GROUP(huGetNumTroveMetatags)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumTroveMetatags, normal)
{
    LONGS_EQUAL_TEXT(3, huGetNumTroveMetatags(l.trove), "l gnta == 3");
    LONGS_EQUAL_TEXT(3, huGetNumTroveMetatags(d.trove), "d gnta == 3");
}

TEST(huGetNumTroveMetatags, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumTroveMetatags(NULL), "NULL gnta == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTroveMetatags(HU_NULLTROVE), "null gnta == 0");
}


TEST_GROUP(huGetTroveMetatag)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetTroveMetatag, normal)
{
    auto metatag = huGetTroveMetatag(l.trove, 0);
    CHECK_TEXT(metatag != NULL, "l metatag 0 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->key->str.size, "l metatag 0 k sz == 2");
    STRNCMP_EQUAL_TEXT("tx", metatag->key->str.ptr, metatag->key->str.size, "l metatag 0 k == tx");
    LONGS_EQUAL_TEXT(2, metatag->value->str.size, "l metatag 0 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->value->str.ptr, metatag->value->str.size, "l metatag 0 v == ta");

    metatag = huGetTroveMetatag(l.trove, 1);
    CHECK_TEXT(metatag != NULL, "l metatag 1 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->key->str.size, "l metatag 1 k sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->key->str.ptr, metatag->key->str.size, "l metatag 1 k == ta");
    LONGS_EQUAL_TEXT(2, metatag->value->str.size, "l metatag 1 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->value->str.ptr, metatag->value->str.size, "l metatag 1 v == ta");

    metatag = huGetTroveMetatag(l.trove, 2);
    CHECK_TEXT(metatag != NULL, "l metatag 2 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->key->str.size, "l metatag 2 k sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->key->str.ptr, metatag->key->str.size, "l metatag 2 k == tb");
    LONGS_EQUAL_TEXT(2, metatag->value->str.size, "l metatag 2 v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->value->str.ptr, metatag->value->str.size, "l metatag 2 v == tb");

    metatag = huGetTroveMetatag(d.trove, 0);
    CHECK_TEXT(metatag != NULL, "d metatag 0 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->key->str.size, "d metatag 0 k sz == 2");
    STRNCMP_EQUAL_TEXT("tx", metatag->key->str.ptr, metatag->key->str.size, "d metatag 0 k == tx");
    LONGS_EQUAL_TEXT(2, metatag->value->str.size, "d metatag 0 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->value->str.ptr, metatag->value->str.size, "d metatag 0 v == ta");

    metatag = huGetTroveMetatag(d.trove, 1);
    CHECK_TEXT(metatag != NULL, "d metatag 1 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->key->str.size, "d metatag 1 k sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->key->str.ptr, metatag->key->str.size, "d metatag 1 k == ta");
    LONGS_EQUAL_TEXT(2, metatag->value->str.size, "d metatag 1 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->value->str.ptr, metatag->value->str.size, "d metatag 1 v == ta");

    metatag = huGetTroveMetatag(d.trove, 2);
    CHECK_TEXT(metatag != NULL, "d metatag 2 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->key->str.size, "d metatag 2 k sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->key->str.ptr, metatag->key->str.size, "d metatag 2 k == tb");
    LONGS_EQUAL_TEXT(2, metatag->value->str.size, "d metatag 2 v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->value->str.ptr, metatag->value->str.size, "d metatag 2 v == tb");
}

TEST(huGetTroveMetatag, pathological)
{
    POINTERS_EQUAL_TEXT(NULL, huGetTroveMetatag(NULL, 0), "NULL metatag 0 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetTroveMetatag(HU_NULLTROVE, 0), "NULL metatag 0 == NULL");
}


TEST_GROUP(huGetTroveMetatagWithKey)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetTroveMetatagWithKey, normal)
{
	huSize_t cursor = 0;
    auto metatag = huGetTroveMetatagWithKeyZ(l.trove, "tx", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag tx not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag tx v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->str.ptr, metatag->str.size, "l metatag tx v == ta");

	cursor = 0;
    metatag = huGetTroveMetatagWithKeyZ(l.trove, "ta", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag ta not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag ta v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->str.ptr, metatag->str.size, "l metatag ta v == ta");

	cursor = 0;
    metatag = huGetTroveMetatagWithKeyZ(l.trove, "tb", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag tb not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag tb v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->str.ptr, metatag->str.size, "l metatag tb v == tb");

	cursor = 0;
    metatag = huGetTroveMetatagWithKeyZ(d.trove, "tx", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag tx not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag tx v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->str.ptr, metatag->str.size, "l metatag tx v == ta");

	cursor = 0;
    metatag = huGetTroveMetatagWithKeyZ(d.trove, "ta", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag ta not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag ta v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->str.ptr, metatag->str.size, "l metatag ta v == ta");

	cursor = 0;
    metatag = huGetTroveMetatagWithKeyZ(d.trove, "tb", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag tb not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag tb v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->str.ptr, metatag->str.size, "l metatag tb v == tb");
}

TEST(huGetTroveMetatagWithKey, pathological)
{
	huSize_t zero = 0;

	huTrove * troves[] =   {l.trove, d.trove, NULL, HU_NULLNODE};
	const char * keys[] =        {"foo", NULL, ""};
	huSize_t * cursors[] =       {& zero, NULL};

	for (long unsigned c = 0; c < sizeof(cursors) / sizeof(cursors[0]); ++c)
	{
		huSize_t * pc = cursors[c];
		huSize_t pcv = 0;
		if (pc != NULL)
			{ pcv = * pc; }
		for (long unsigned k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k)
		{
			const char * pk = keys[k];
			for (long unsigned t = 0; t < sizeof(troves) / sizeof(troves[0]); ++t)
			{
				huTrove * pt = troves[t];
				if (c != 0 || k != 0 || (t != 0 && t != 1))
				{
					POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveMetatagWithKeyZ(pt, pk, pc), "");
					if (pc != NULL)
						{ * pc = pcv; }
				}
			}
		}
	}
}


TEST_GROUP(huGetNumTroveMetatagsWithValue)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumTroveMetatagsWithValue, normal)
{
    LONGS_EQUAL_TEXT(2, huGetNumTroveMetatagsWithValueZ(l.trove, "ta"), "l gntabv ta == 2");
    LONGS_EQUAL_TEXT(1, huGetNumTroveMetatagsWithValueZ(l.trove, "tb"), "l gntabv tb == 2");
    LONGS_EQUAL_TEXT(0, huGetNumTroveMetatagsWithValueZ(l.trove, "tc"), "l gntabv tb == 0");
    LONGS_EQUAL_TEXT(2, huGetNumTroveMetatagsWithValueZ(d.trove, "ta"), "d gntabv ta == 2");
    LONGS_EQUAL_TEXT(1, huGetNumTroveMetatagsWithValueZ(d.trove, "tb"), "d gntabv tb == 2");
    LONGS_EQUAL_TEXT(0, huGetNumTroveMetatagsWithValueZ(d.trove, "tc"), "d gntabv tb == 0");
}

TEST(huGetNumTroveMetatagsWithValue, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumTroveMetatagsWithValueZ(NULL, "ta"), "NULL gntabv ta == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTroveMetatagsWithValueZ(HU_NULLTROVE, "ta"), "null gntabv ta == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTroveMetatagsWithValueZ(l.trove, NULL), "l gntabv NULL == 0");
}


TEST_GROUP(huGetTroveMetatagWithValue)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetTroveMetatagWithValue, normal)
{
    huSize_t cursor = 0;
    auto metatag = huGetTroveMetatagWithValueZ(l.trove, "ta", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag v ta 0 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag v ta 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tx", metatag->str.ptr, metatag->str.size, "l metatag v ta 0 == tx");
    metatag = huGetTroveMetatagWithValueZ(l.trove, "ta", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag v ta 1 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag v ta 1 sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->str.ptr, metatag->str.size, "l metatag v ta 1 == ta");

    cursor = 0;
    metatag = huGetTroveMetatagWithValueZ(l.trove, "tb", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag v tb 0 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag v tb 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->str.ptr, metatag->str.size, "l metatag v tb 0 == tb");

    cursor = 0;
    metatag = huGetTroveMetatagWithValueZ(d.trove, "ta", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag v ta 0 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag v ta 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tx", metatag->str.ptr, metatag->str.size, "l metatag v ta 0 == tx");
    metatag = huGetTroveMetatagWithValueZ(d.trove, "ta", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag v ta 1 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag v ta 1 sz == 2");
    STRNCMP_EQUAL_TEXT("ta", metatag->str.ptr, metatag->str.size, "l metatag v ta 1 == ta");

    cursor = 0;
    metatag = huGetTroveMetatagWithValueZ(d.trove, "tb", & cursor);
    CHECK_TEXT(metatag != HU_NULLTOKEN, "l metatag v tb 0 not NULL");
    LONGS_EQUAL_TEXT(2, metatag->str.size, "l metatag v tb 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tb", metatag->str.ptr, metatag->str.size, "l metatag v tb 0 == tb");
}

TEST(huGetTroveMetatagWithValue, pathological)
{
    huSize_t cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveMetatagWithValueZ(NULL, "ta", & cursor), "NULL metatag v ta 0 == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveMetatagWithValueZ(HU_NULLTROVE, "ta", & cursor), "null metatag v ta 0 == null");
    cursor = 0;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveMetatagWithValueZ(l.trove, NULL, & cursor), "l metatag v NULL 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveMetatagWithValueZ(l.trove, "ta", NULL), "l metatag v ta 3 == null");
    cursor = 3;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveMetatagWithValueZ(l.trove, "ta", & cursor), "l metatag v ta 3 == null");
    cursor = -1;
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveMetatagWithValueZ(l.trove, "ta", & cursor), "l metatag v ta -1 == null");
}


TEST_GROUP(huGetNumTroveComments)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetNumTroveComments, normal)
{
    LONGS_EQUAL_TEXT(2, huGetNumTroveComments(l.trove), "l gntc == 2");
    LONGS_EQUAL_TEXT(2, huGetNumTroveComments(d.trove), "l gntc == 2");
}

TEST(huGetNumTroveComments, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumTroveComments(NULL), "NULL gntc == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTroveComments(HU_NULLTROVE), "null gntc == 0");
}


TEST_GROUP(huGetTroveComment)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huGetTroveComment, normal)
{
    auto comm = huGetTroveComment(l.trove, 0);
    auto exp = "// This is a trove comment."sv;
    CHECK_FALSE(comm == NULL);
    LONGS_EQUAL_TEXT(exp.size(), comm->str.size, "l comm 0 sz == exp sz");
    STRNCMP_EQUAL_TEXT(exp.data(), comm->str.ptr, exp.size(), "l comm 0 == exp");

    comm = huGetTroveComment(l.trove, 1);
    exp = "// This is also a trove comment."sv;
    CHECK_FALSE(comm == NULL);
    LONGS_EQUAL_TEXT(exp.size(), comm->str.size, "l comm 1 sz == exp sz");
    STRNCMP_EQUAL_TEXT(exp.data(), comm->str.ptr, exp.size(), "l comm 1 == exp");

    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveComment(l.trove, 2), "l comm 2 == null");
}

TEST(huGetTroveComment, pathological)
{
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveComment(NULL, 0), "NULL comm 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveComment(HU_NULLTROVE, 0), "null comm 0 == null");
    POINTERS_EQUAL_TEXT(HU_NULLTOKEN, huGetTroveComment(l.trove, -1), "l comm -1 == null");
}


TEST_GROUP(huGetNodeByAddress)
{
	htd_inane inane;
    htd_listOfLists l;
    htd_dictOfDicts d;
	htd_sharedKeys t;

    void setup()
    {
		inane.setup();
        l.setup();
        d.setup();
		t.setup();
    }

    void teardown()
    {
		inane.teardown();
        d.teardown();
        l.teardown();
		t.teardown();
    }
};

TEST(huGetNodeByAddress, lists)
{
    POINTERS_EQUAL_TEXT(l.root, huGetNodeByAddressZ(l.trove, "/"), "l gnbfa '/' == root");
    POINTERS_EQUAL_TEXT(l.a, huGetNodeByAddressZ(l.trove, "/0"), "l gnbfa '/0' == a");
    POINTERS_EQUAL_TEXT(l.bp, huGetNodeByAddressZ(l.trove, "/1"), "l gnbfa '/1' == bp");
    POINTERS_EQUAL_TEXT(l.b, huGetNodeByAddressZ(l.trove, "/1/0"), "l gnbfa '/1/0'== b");
    POINTERS_EQUAL_TEXT(l.cpp, huGetNodeByAddressZ(l.trove, "/2"), "l gnbfa '/2'== cpp");
    POINTERS_EQUAL_TEXT(l.cp, huGetNodeByAddressZ(l.trove, "/2/0"), "l gnbfa '/2/0'== cp");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByAddressZ(l.trove, "/2/0/0"), "l gnbfa '/2/0/0'== c");

    POINTERS_EQUAL_TEXT(l.c, huGetNodeByAddressZ(l.trove, "/0/../2/0/0"), "l gnbfa '/0/../2/0/0'== c");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByAddressZ(l.trove, "/0/../1/0/../../2/0/0"), "l gnbfa '/0/../1/0/../../2/0/0'== c");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByAddressZ(l.trove, " / 0 / .. / 2 / 0 / 0 "), "l gnbfa ' / 0 / .. / 2 / 0 / 0 '== c");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByAddressZ(l.trove, " / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 "), "l gnbfa ' / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 '== c");
}

TEST(huGetNodeByAddress, dicts)
{
    POINTERS_EQUAL_TEXT(d.root, huGetNodeByAddressZ(d.trove, "/"), "d gnbfa '/' == a");
    POINTERS_EQUAL_TEXT(d.a, huGetNodeByAddressZ(d.trove, "/0"), "d gnbfa '/0'== a");
    POINTERS_EQUAL_TEXT(d.bp, huGetNodeByAddressZ(d.trove, "/1"), "d gnbfa '/1'== bp");
    POINTERS_EQUAL_TEXT(d.b, huGetNodeByAddressZ(d.trove, "/1/0"), "d gnbfa '/1/0'== b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetNodeByAddressZ(d.trove, "/2"), "d gnbfa '/2'== cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetNodeByAddressZ(d.trove, "/2/0"), "d gnbfa '/2/0'== cp");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, "/2/0/0"), "d gnbfa '/2/0/0'== c");

    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, "/0/../2/0/0"), "d gnbfa '/0/../2/0/0'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, "/0/../1/0/../../2/0/0"), "d gnbfa '/0/../1/0/../../2/0/0'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, " / 0 / .. / 2 / 0 / 0 "), "d gnbfa ' / 0 / .. / 2 / 0 / 0 '== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, " / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 "), "d gnbfa ' / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 '== c");

    POINTERS_EQUAL_TEXT(d.root, huGetNodeByAddressZ(d.trove, "/"), "d gnbfa '/' == a");
    POINTERS_EQUAL_TEXT(d.a, huGetNodeByAddressZ(d.trove, "/ak"), "d gnbfa '/ak'== a");
    POINTERS_EQUAL_TEXT(d.bp, huGetNodeByAddressZ(d.trove, "/bk"), "d gnbfa '/bk'== bp");
    POINTERS_EQUAL_TEXT(d.b, huGetNodeByAddressZ(d.trove, "/bk/bk"), "d gnbfa '/bk/bk'== b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetNodeByAddressZ(d.trove, "/ck"), "d gnbfa '/ck'== cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetNodeByAddressZ(d.trove, "/ck/ck"), "d gnbfa '/ck/ck'== cp");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, "/ck/ck/ck"), "d gnbfa '/ck/ck/ck'== c");

    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, "/ak/../ck/ck/ck"), "d gnbfa '/ak/../ck/ck/ck'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, "/ak/../bk/bk/../../ck/ck/ck"), "d gnbfa '/ak/../bk/bk/../../ck/ck/ck'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, " / ak / .. / ck / ck / ck "), "d gnbfa ' / ak / .. / ck / ck / ck '== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, " / ak / .. / bk / bk / .. / .. / ck / ck / ck "), "d gnbfa ' / ak / .. / bk / bk / .. / .. / ck / ck / ck '== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, "/ak/../`ck`/'ck'/\"ck\""), "d gnbfa '/ak/../`ck`/'ck'/\"ck\"'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByAddressZ(d.trove, " / ak / .. / `ck` / 'ck' / \"ck\" "), "d gnbfa ' / ak / .. / `ck` / 'ck' / \"ck\" '== c");
}

TEST(huGetNodeByAddress, sharedKeyIndex)
{
	huSize_t numChildren = huGetNumChildren(t.root);
	for (huSize_t i = 0; i < numChildren; ++i)
	{
		huNode const * ch = huGetChildByIndex(t.root, i);
		huToken const * a = huGetValue(huGetChildByIndex(ch, 2));
		std::string full_address = std::string("/") + std::string(std::string_view(a->str.ptr, a->str.size));
		huNode const * an = huGetNodeByAddressN(t.trove, full_address.data(), full_address.size());
		POINTERS_EQUAL_TEXT(ch, an, full_address);
	}
}

TEST(huGetNodeByAddress, pathological)
{
	POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByAddressZ(inane.trove, "/"), "no root node");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByAddressZ(NULL, "/"), "NULL gnbfa '/' == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByAddressZ(HU_NULLTROVE, "/"), "null gnbfa '/' == c");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByAddressZ(l.trove, "/.."), "l gnbfa '/..' == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByAddressZ(l.trove, ".."), "l gnbfa '..' == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByAddressZ(l.trove, "0"), "l gnbfa '0' == null");
    POINTERS_EQUAL_TEXT(HU_NULLNODE, huGetNodeByAddressZ(l.trove, "//"), "l gnbfa '//' == null");
}


TEST_GROUP(huFindNodesWithMetatagKey)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huFindNodesWithMetatagKey, normal)
{
    huSize_t cursor = 0;
    huNode const * node = huFindNodesWithMetatagKeyZ(l.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbak a 0 == a");
    node = huFindNodesWithMetatagKeyZ(l.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak a 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(l.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbak b 0 == bp");
    node = huFindNodesWithMetatagKeyZ(l.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbak b 1 == b");
    node = huFindNodesWithMetatagKeyZ(l.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak b 2 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(l.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbak c 0 == cpp");
    node = huFindNodesWithMetatagKeyZ(l.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbak c 1 == cp");
    node = huFindNodesWithMetatagKeyZ(l.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbak c 2 == c");
    node = huFindNodesWithMetatagKeyZ(l.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak c 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(l.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbak type 0 == a");
    node = huFindNodesWithMetatagKeyZ(l.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbak type 1 == bp");
    node = huFindNodesWithMetatagKeyZ(l.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbak type 2 == b");
    node = huFindNodesWithMetatagKeyZ(l.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbak type 3 == cpp");
    node = huFindNodesWithMetatagKeyZ(l.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbak type 4 == cp");
    node = huFindNodesWithMetatagKeyZ(l.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbak type 5 == c");
    node = huFindNodesWithMetatagKeyZ(l.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak type 6 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(l.trove, "foo", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak foo 0 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(d.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbak a 0 == a");
    node = huFindNodesWithMetatagKeyZ(d.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbak a 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(d.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbak b 0 == bp");
    node = huFindNodesWithMetatagKeyZ(d.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbak b 1 == b");
    node = huFindNodesWithMetatagKeyZ(d.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbak b 2 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(d.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbak c 0 == cpp");
    node = huFindNodesWithMetatagKeyZ(d.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbak c 1 == cp");
    node = huFindNodesWithMetatagKeyZ(d.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbak c 2 == c");
    node = huFindNodesWithMetatagKeyZ(d.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbak c 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(d.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbak type 0 == a");
    node = huFindNodesWithMetatagKeyZ(d.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbak type 1 == bp");
    node = huFindNodesWithMetatagKeyZ(d.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbak type 2 == b");
    node = huFindNodesWithMetatagKeyZ(d.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbak type 3 == cpp");
    node = huFindNodesWithMetatagKeyZ(d.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbak type 4 == cp");
    node = huFindNodesWithMetatagKeyZ(d.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbak type 5 == c");
    node = huFindNodesWithMetatagKeyZ(d.trove, "type", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbak type 6 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(d.trove, "foo", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbak foo 0 == null");
}

TEST(huFindNodesWithMetatagKey, pathological)
{
    huSize_t cursor = 0;
    huNode const * node = huFindNodesWithMetatagKeyZ(NULL, "type", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "NULL fnbak type == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(HU_NULLTROVE, "type", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "null fnbak foo 0 == null");


    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(l.trove, NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak NULL == null");
    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(l.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak '' 0 == null");
    node = huFindNodesWithMetatagKeyZ(l.trove, "", NULL);
    cursor = -1;
    node = huFindNodesWithMetatagKeyZ(l.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak '' 0 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(d.trove, NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbak NULL == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyZ(d.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak '' 0 == null");
    node = huFindNodesWithMetatagKeyZ(d.trove, "", NULL);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak '' 0 == null");
    cursor = -1;
    node = huFindNodesWithMetatagKeyZ(d.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbak '' 0 == null");
}


TEST_GROUP(huFindNodesWithMetatagValue)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huFindNodesWithMetatagValue, normal)
{
    huSize_t cursor = 0;
    huNode const * node = huFindNodesWithMetatagValueZ(l.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbav a 0 == a");
    node = huFindNodesWithMetatagValueZ(l.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav a 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(l.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbav b 0 == b");
    node = huFindNodesWithMetatagValueZ(l.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav b 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(l.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbav c 0 == c");
    node = huFindNodesWithMetatagValueZ(l.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav c 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(l.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbav value 0 == a");
    node = huFindNodesWithMetatagValueZ(l.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbav value 1 == b");
    node = huFindNodesWithMetatagValueZ(l.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbav value 2 == c");
    node = huFindNodesWithMetatagValueZ(l.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav value 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(l.trove, "list", & cursor);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbav list 0 == bp");
    node = huFindNodesWithMetatagValueZ(l.trove, "list", & cursor);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbav list 1 == cpp");
    node = huFindNodesWithMetatagValueZ(l.trove, "list", & cursor);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbav list 2 == cp");
    node = huFindNodesWithMetatagValueZ(l.trove, "list", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav list 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(l.trove, "foo", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav foo 0 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbav a 0 == a");
    node = huFindNodesWithMetatagValueZ(d.trove, "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav a 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbav b 0 == b");
    node = huFindNodesWithMetatagValueZ(d.trove, "b", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav b 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbav c 0 == c");
    node = huFindNodesWithMetatagValueZ(d.trove, "c", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav c 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbav value 0 == a");
    node = huFindNodesWithMetatagValueZ(d.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbav value 1 == b");
    node = huFindNodesWithMetatagValueZ(d.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbav value 2 == c");
    node = huFindNodesWithMetatagValueZ(d.trove, "value", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav value 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, "dict", & cursor);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbav dict 0 == bp");
    node = huFindNodesWithMetatagValueZ(d.trove, "dict", & cursor);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbav dict 1 == cpp");
    node = huFindNodesWithMetatagValueZ(d.trove, "dict", & cursor);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbav dict 2 == cp");
    node = huFindNodesWithMetatagValueZ(d.trove, "dict", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav dict 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, "foo", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav foo 0 == null");
}

TEST(huFindNodesWithMetatagValue, pathological)
{
    huSize_t cursor = 0;
    huNode const * node = huFindNodesWithMetatagValueZ(NULL, "type", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "NULL fnbav type == 0");
    cursor = 0;
    node = huFindNodesWithMetatagValueZ(HU_NULLTROVE, "type", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "null fnbav foo == 0");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(l.trove, NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav NULL == 0");
    cursor = 0;
    node = huFindNodesWithMetatagValueZ(l.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav '' == 0");
    node = huFindNodesWithMetatagValueZ(l.trove, "", NULL);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav '' == 0");
    cursor = -1;
    node = huFindNodesWithMetatagValueZ(l.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbav '' == 0");

    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav NULL == 0");
    cursor = 0;
    node = huFindNodesWithMetatagValueZ(d.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav '' == 0");
    node = huFindNodesWithMetatagValueZ(d.trove, "", NULL);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav '' == 0");
    cursor = -1;
    node = huFindNodesWithMetatagValueZ(d.trove, "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbav '' == 0");
}


TEST_GROUP(huFindNodesWithMetatagKeyValue)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huFindNodesWithMetatagKeyValue, normal)
{
    huSize_t cursor = 0;
    huNode const * node = huFindNodesWithMetatagKeyValueZZ(l.trove, "a", "a", & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbakv a 0 == a");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "a", "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv a 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "b", "bp", & cursor);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbakv b 0 == bp");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "b", "bp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv b 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "c", "c", & cursor);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbakv c 0 == c");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "c", "c", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv c 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbakv value 0 == a");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbakv value 1 == b");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbakv value 2 == c");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv value 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "list", & cursor);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbakv list 0 == bp");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "list", & cursor);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbakv list 1 == cpp");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "list", & cursor);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbakv list 2 == cp");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "type", "list", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv list 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "foo", "foo", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv foo 0 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "foo", "bar", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv foo 0 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "a", "foo", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv foo 0 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "a", "a", & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbakv a 0 == a");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "a", "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv a 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "b", "bp", & cursor);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbakv b 0 == bp");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "b", "bp", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv b 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "c", "c", & cursor);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbakv c 0 == c");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "c", "c", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv c 1 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbakv value 0 == a");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbakv value 1 == b");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbakv value 2 == c");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv value 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "dict", & cursor);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbakv dict 0 == bp");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "dict", & cursor);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbakv dict 1 == cpp");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "dict", & cursor);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbakv dict 2 == cp");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "type", "dict", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv dict 3 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "foo", "bar", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv foo 0 == null");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "a", "foo", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv foo 0 == null");
}

TEST(huFindNodesWithMetatagKeyValue, pathological)
{
    huSize_t cursor = 0;
    huNode const * node = huFindNodesWithMetatagKeyValueZZ(NULL, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "NULL fnbakv type == 0");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(HU_NULLTROVE, "type", "value", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "null fnbakv foo == 0");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, NULL, "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv NULL == 0");
    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "a", NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv NULL == 0");
    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "", "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv '' == 0");
    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "a", "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv '' == 0");
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "a", "a", NULL);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv '' == 0");
    cursor = -1;
    node = huFindNodesWithMetatagKeyValueZZ(l.trove, "a", "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv '' == 0");

    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, NULL, "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv NULL == 0");
    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "a", NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv NULL == 0");
    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "", "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbakv '' == 0");
    cursor = 0;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "a", "", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv '' == 0");
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "a", "a", NULL);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv '' == 0");
    cursor = -1;
    node = huFindNodesWithMetatagKeyValueZZ(d.trove, "a", "a", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbakv '' == 0");
}


TEST_GROUP(huFindNodesByCommentContaining)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};

TEST(huFindNodesByCommentContaining, normal)
{
    huSize_t cursor = 0;
    auto exp = "This is a "sv;
    huNode const * node = NULL;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbcc This is a 0 == a");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbcc This is a 1 == bp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbcc This is a 2 == cpp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc This is a 3 == null");

    cursor = 0;
    exp = "aaaa"sv;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbcc aaaa 0 == a");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc aaaa 1 == null");

    cursor = 0;
    exp = "cp"sv;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbcc cp 0 == cpp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbcc cp 1 == cp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc cp 2 == null");

    cursor = 0;
    exp = ""sv;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbcc '' 0 == a");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbcc '' 1 == bp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbcc '' 2 == b");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbcc '' 3 == cpp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbcc '' 4 == cp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbcc '' 5 == c");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc '' 6 == null");

    cursor = 0;
    exp = "This is a "sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbcc This is a 0 == a");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbcc This is a 1 == bp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbcc This is a 2 == cpp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbcc This is a 3 == null");

    cursor = 0;
    exp = "aaaa"sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbcc aaaa 0 == a");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbcc aaaa 1 == null");

    cursor = 0;
    exp = "cp"sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbcc cp 1 == cpp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbcc cp 0 == cp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbcc cp 2 == null");

    cursor = 0;
    exp = ""sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbcc '' 0 == a");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbcc '' 1 == bp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbcc '' 2 == b");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbcc '' 3 == cpp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbcc '' 4 == cp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbcc '' 5 == c");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), (int) exp.size(), & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbcc '' 6 == null");
}

TEST(huFindNodesByCommentContaining, pathological)
{
    huSize_t cursor = 0;
    huNode const * node = huFindNodesByCommentContainingZ(NULL, "This", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "NULL fnbcc This == 0");

    cursor = 0;
    node = huFindNodesByCommentContainingZ(HU_NULLTROVE, "This", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "null fnbcc This == 0");

    cursor = 0;
    node = huFindNodesByCommentContainingZ(l.trove, NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc NULL == 0");
    node = huFindNodesByCommentContainingZ(l.trove, "This", NULL);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc NULL == 0");
    cursor = -1;
    node = huFindNodesByCommentContainingZ(l.trove, "This", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc NULL == 0");

    cursor = 0;
    node = huFindNodesByCommentContainingZ(d.trove, NULL, & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "d fnbcc NULL == 0");
    node = huFindNodesByCommentContainingZ(d.trove, "This", NULL);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc NULL == 0");
    cursor = -1;
    node = huFindNodesByCommentContainingZ(d.trove, "This", & cursor);
    POINTERS_EQUAL_TEXT(HU_NULLNODE, node, "l fnbcc NULL == 0");
}


static std::string makeFileName(std::string_view path, int WhitespaceFormat, bool useColors, bool printComments, bool printBom)
{
    std::string format = ".pp";
    if (WhitespaceFormat == HU_WHITESPACEFORMAT_CLONED)
        { format = ".pc"; }
    else if (WhitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL)
        { format = ".pm"; }

    std::string consPath = std::string {path} +
        format +
        (printComments ? ".my" : ".mn") +
        (useColors ? ".ca" : ".cn") +
        (printBom ? ".by" : ".bn") +
        ".hu";

    return consPath;
}



FILE * openHuFile(char const * path, char const * mode)
{
	FILE * fp = NULL;
#ifdef _WIN32
	errno_t err = fopen_s(&fp, path, mode);
#else
	fp = fopen(path, mode);
#endif
	return fp;
}

static std::tuple<std::string, huSize_t> getFile(std::string_view path)
{
    std::string str;
    huSize_t fileSize = 0;
    FILE * fp = openHuFile(path.data(), "rb");
    fseek(fp, 0L, SEEK_END);
    str.resize(fileSize = ftell(fp));
    rewind(fp);
    if (fread((void*)str.c_str(), 1, str.size(), fp) != str.size())
        { str = "<could not read test file>"; }
    fclose(fp);

    return { str, fileSize };
}

static std::tuple<std::string, huSize_t> getFile(std::string_view path, huWhitespaceFormat whitespaceFormat, bool useColors, bool printComments, bool printBom)
{
    std::string consPath = makeFileName(path, whitespaceFormat, useColors, printComments, printBom);
    return getFile(consPath);
}


std::string testFiles_SourceText[] = {
    "test/testFiles/comments.hu",
    "test/testFiles/commentsCstyle.hu",
    "test/testFiles/quothTheHumon.hu",
    "test/testFiles/utf8.hu",
};

TEST_GROUP(huGetTroveSourceText)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(huGetTroveSourceText, correctness)
{
    for (auto testFile : testFiles_SourceText)
    {
        auto [src, sz] = getFile(testFile);
        huTrove * trove;
        huErrorCode err = huDeserializeTroveFromFile(& trove, testFile.data(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
        LONGS_EQUAL(HU_ERROR_NOERROR, err);
        auto sv = huGetTroveSourceText(trove);
        LONGS_EQUAL(sz, sv.size);
        STRNCMP_EQUAL(src.data(), sv.ptr, sz);
    }
}


std::string testFiles_Serialize[] = {
    "test/testFiles/comments.hu",
    "test/testFiles/commentsCstyle.hu",
    "test/testFiles/quothTheHumon.hu",
    "test/testFiles/utf8.hu",
    "test/testFiles/utf8bom.hu",
    "test/testFiles/utf16be.hu",
    "test/testFiles/utf16bebom.hu",
    "test/testFiles/utf16le.hu",
    "test/testFiles/utf16lebom.hu",
    "test/testFiles/utf32be.hu",
    "test/testFiles/utf32bebom.hu",
    "test/testFiles/utf32le.hu",
    "test/testFiles/utf32lebom.hu"
};

TEST_GROUP(huSerializeTrove)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }

    std::string troveToString(std::string_view srcFile, huWhitespaceFormat format, bool useColors, bool printComments, bool printBom)
    {
        huTrove * tc = nullptr;
        huErrorCode error = huDeserializeTroveFromFile(& tc, srcFile.data(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
        if (error != HU_ERROR_NOERROR)
            { return "<could not make trove>"; }

        huSize_t toStrLen = 0;
        huSerializeOptions SerializeOptions;
        huStringView colors[HU_COLORCODE_NUMCOLORS];
        if (useColors)
            { huFillAnsiColorTable(colors); }
        huInitSerializeOptionsZ(& SerializeOptions, format, 4, false, useColors, colors, printComments, "\n", HU_ENCODING_UTF8, printBom);

        error = huSerializeTrove(tc, NULL, & toStrLen, & SerializeOptions);
        if (error != HU_ERROR_NOERROR)
            { return "<could not troveToString for length>"; }

        std::string str;
        str.resize(toStrLen);
        error = huSerializeTrove(tc, str.data(), & toStrLen, & SerializeOptions);
        if (error != HU_ERROR_NOERROR)
            { return "<could not troveToString for content>"; }

        huDestroyTrove(tc);

        return str;
    }
};

TEST(huSerializeTrove, correctness)
{
    for (auto testFile : testFiles_Serialize)
    {
        for (int whitespaceFormat = 0; whitespaceFormat < 3; ++whitespaceFormat)
        {
            for (bool useColors = false; ! useColors; useColors = !useColors)
            {
                for (bool printComments = false; ! printComments; printComments = !printComments)
                {
                    for (bool printBom = false; ! printBom; printBom = !printBom)
                    {
                        //if (testFile == testFiles[7])
                        //    { bool debugBreak = true; }

                        auto [file, sz] = getFile(testFile,
                            (huWhitespaceFormat) whitespaceFormat, useColors, printComments, printBom);
                        auto ttos = troveToString(testFile,
                            (huWhitespaceFormat) whitespaceFormat, useColors, printComments, printBom);
                        std::string consPath = makeFileName(testFile, (huWhitespaceFormat) whitespaceFormat, useColors, printComments, printBom);

                        LONGS_EQUAL_TEXT(file.size(), ttos.size(), consPath.data());
                        MEMCMP_EQUAL_TEXT(file.data(), ttos.data(), file.size(), consPath.data());
                    }
                }
            }
        }
    }
}

TEST(huSerializeTrove, pathological)
{
    huErrorCode error = HU_ERROR_NOERROR;
    huSize_t strLen = 1024;
    huSerializeOptions params;
    huInitSerializeOptionsZ(& params, HU_WHITESPACEFORMAT_CLONED, 4, false, false, NULL, true, "\n", HU_ENCODING_UTF8, false);

    error = huSerializeTrove(NULL, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "NULL->str sz == 0");

    strLen = 1024;
    error = huSerializeTrove(HU_NULLTROVE, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    error = huSerializeTrove(l.trove, NULL, NULL, & params);

    strLen = 1024;
    huInitSerializeOptionsZ(& params, HU_WHITESPACEFORMAT_CLONED, 4, false, false, NULL, true, "\n", HU_ENCODING_UNKNOWN, false);
    error = huSerializeTrove(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADENCODING, error, "null->str sz == 0");

    strLen = 1024;
    huInitSerializeOptionsZ(& params, (huWhitespaceFormat) 3, 4, false, false, NULL, true, "\n", HU_ENCODING_UTF8, false);
    error = huSerializeTrove(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitSerializeOptionsZ(& params, (huWhitespaceFormat) -1, 4, false, false, NULL, true, "\n", HU_ENCODING_UTF8, false);
    error = huSerializeTrove(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    if (isSignedType(huCol_t))
    {
        strLen = 1024;
        huInitSerializeOptionsZ(& params, HU_WHITESPACEFORMAT_CLONED, -1, false, false, NULL, true, "\n", HU_ENCODING_UTF8, false);
        error = huSerializeTrove(l.trove, NULL, & strLen, & params);
        LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");
    }

    strLen = 1024;
    huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_MINIMAL, 4, false, false, NULL, true, NULL, 1, HU_ENCODING_UTF8, false);
    error = huSerializeTrove(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_MINIMAL, 4, false, false, NULL, true, "\n", 0, HU_ENCODING_UTF8, false);
    error = huSerializeTrove(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_PRETTY, 4, false, false, NULL, true, NULL, 1, HU_ENCODING_UTF8, false);
    error = huSerializeTrove(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_PRETTY, 4, false, false, NULL, true, "\n", 0, HU_ENCODING_UTF8, false);
    error = huSerializeTrove(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    if (isSignedType(huCol_t))
    {
        strLen = 1024;
        huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_PRETTY, 4, false, false, NULL, true, "\n", -1, HU_ENCODING_UTF8, false);
        error = huSerializeTrove(l.trove, NULL, & strLen, & params);
        LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");
    }
}


TEST_GROUP(huSerializeTroveToFile)
{
    htd_listOfLists l;
    htd_dictOfDicts d;

    void setup()
    {
        l.setup();
        d.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
    }
};


int haveAccess(char const * path)
{
#ifdef _WIN32
	return _access(path, 06);
#else
	return access(path, R_OK | W_OK);
#endif
}

// Just doing patho tests -- this is heavily function-tested elsewhere.
TEST(huSerializeTroveToFile, pathological)
{
    auto validFile = "src/test/testFiles/testout.hu";

    // Remove this file if it exists.
	int acc = haveAccess(validFile);
    if (acc != -1)
        { remove(validFile); }

    huSerializeOptions params;
    huInitSerializeOptionsZ(& params, HU_WHITESPACEFORMAT_CLONED, 4, false, false, NULL, false, "\n", HU_ENCODING_UTF8, false);

    huErrorCode error = HU_ERROR_NOERROR;
    huSize_t fileLen = 0;

    error = huSerializeTroveToFile(NULL, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "NULL->file sz == 0");
    acc = haveAccess(validFile);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    error = huSerializeTroveToFile(HU_NULLTROVE, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huSerializeTroveToFile(HU_NULLTROVE, NULL, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huSerializeTroveToFile(HU_NULLTROVE, "", & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huSerializeTroveToFile(HU_NULLTROVE, "..", & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huSerializeTroveToFile(HU_NULLTROVE, "/", & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    huInitSerializeOptionsZ(& params, (huWhitespaceFormat) 3, 4, false, false, NULL, false, "\n", HU_ENCODING_UTF8, false);
    error = huSerializeTroveToFile(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = haveAccess(validFile);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitSerializeOptionsZ(& params, (huWhitespaceFormat) -1, 4, false, false, NULL, false, "\n", HU_ENCODING_UTF8, false);
    error = huSerializeTroveToFile(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = haveAccess(validFile);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    if (isSignedType(huCol_t))
    {
        huInitSerializeOptionsZ(& params, HU_WHITESPACEFORMAT_CLONED, -1, false, false, NULL, false, "\n", HU_ENCODING_UTF8, false);
        error = huSerializeTroveToFile(l.trove, validFile, & fileLen, & params);
        LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
        acc = haveAccess(validFile);
        LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
        if (acc != -1)
            { remove(validFile); }
    }

    huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_CLONED, 4, false, false, NULL, false, NULL, 1, HU_ENCODING_UTF8, false);
    error = huSerializeTroveToFile(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = haveAccess(validFile);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_CLONED, 4, false, true, NULL, false, "\n", 1, HU_ENCODING_UTF8, false);
    error = huSerializeTroveToFile(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = haveAccess(validFile);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_PRETTY, 4, false, false, NULL, false, "\n", 0, HU_ENCODING_UTF8, false);
    error = huSerializeTroveToFile(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = haveAccess(validFile);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    if (isSignedType(huSize_t))
    {
        huInitSerializeOptionsN(& params, HU_WHITESPACEFORMAT_CLONED, 4, false, false, NULL, false, "\n", -1, HU_ENCODING_UTF8, false);
        error = huSerializeTroveToFile(l.trove, validFile, & fileLen, & params);
        LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
        acc = haveAccess(validFile);
        LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
        if (acc != -1)
            { remove(validFile); }
    }
}

