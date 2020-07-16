#include <string.h>
#include <string_view>
#include <unistd.h>
#include "humon.h"
#include <CppUTest/TestHarness.h>
#include "testData.h"

// ------------------------------ NODE API TESTS

TEST_GROUP(huGetParentNode)
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

TEST(huGetParentNode, lists)
{
    auto pn = huGetParentNode(l.a);
    POINTERS_EQUAL_TEXT(l.root, pn, "root == ap");

    pn = huGetParentNode(l.b);
    POINTERS_EQUAL_TEXT(l.bp, pn, "l.bp == l.b.parent");
    pn = huGetParentNode(pn);
    POINTERS_EQUAL_TEXT(l.root, pn, "root == l.bp.parent");

    pn = huGetParentNode(l.c);
    POINTERS_EQUAL_TEXT(l.cp, pn, "l.cp == l.c.parent");
    pn = huGetParentNode(pn);
    POINTERS_EQUAL_TEXT(l.cpp, pn, "l.cp == l.cp.parent");
    pn = huGetParentNode(pn);
    POINTERS_EQUAL_TEXT(l.root, pn, "root == l.cpp.parent");
    
    pn = huGetParentNode(l.root);
    POINTERS_EQUAL_TEXT(NULL, pn, "root's parent is not NULL");
}

TEST(huGetParentNode, dicts)
{
    auto pn = huGetParentNode(d.a);
    POINTERS_EQUAL_TEXT(d.root, pn, "root == ap");

    pn = huGetParentNode(d.b);
    POINTERS_EQUAL_TEXT(d.bp, pn, "d.bp == d.b.parent");
    pn = huGetParentNode(pn);
    POINTERS_EQUAL_TEXT(d.root, pn, "root == d.bp.parent");

    pn = huGetParentNode(d.c);
    POINTERS_EQUAL_TEXT(d.cp, pn, "d.cp == d.c.parent");
    pn = huGetParentNode(pn);
    POINTERS_EQUAL_TEXT(d.cpp, pn, "d.cp == d.cp.parent");
    pn = huGetParentNode(pn);
    POINTERS_EQUAL_TEXT(d.root, pn, "root == d.cpp.parent");
    
    pn = huGetParentNode(d.root);
    POINTERS_EQUAL_TEXT(NULL, pn, "root's parent is not NULL");
}

TEST(huGetParentNode, pathological)
{
    auto pn = huGetParentNode(NULL);
    POINTERS_EQUAL_TEXT(NULL, pn, "NULL's parent is not NULL");

    pn = huGetParentNode(hu_nullNode);
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
    LONGS_EQUAL_TEXT(0, huGetNumChildren(hu_nullNode), "nullNode.nch == 0");
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
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByIndex(d.root, -1), "root.ch-1 == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByIndex(d.root, 3), "root.ch3 == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByIndex(NULL, 0), "NULL.ch0 == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByIndex(hu_nullNode, 0), "null.ch0 == null");
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
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByKeyZ(d.root, "foo"), "root.chfoo == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByKeyZ(l.root, "foo"), "listroot.chfoo == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByKeyZ(NULL, "foo"), "NULL.chfoo == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByKeyZ(hu_nullNode, "foo"), "null.chfoo == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByKeyZ(d.root, ""), "root.ch'' == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetChildByKeyZ(l.root, ""), "listroot.ch'' == null");
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
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetFirstChild(NULL), "NULL gfc == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetFirstChild(hu_nullNode), "null gfc == null");
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
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(l.root), "root.ns == null");
    POINTERS_EQUAL_TEXT(l.bp, huGetNextSibling(l.a), "a.ns == bp");
    POINTERS_EQUAL_TEXT(l.cpp, huGetNextSibling(l.bp), "bp.ns == cpp");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(l.cpp), "cpp.ns == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(l.cp), "cp.ns == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(l.c), "c.ns == null");
}

TEST(huGetNextSibling, dicts)
{
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(d.root), "root.ns == null");
    POINTERS_EQUAL_TEXT(d.bp, huGetNextSibling(d.a), "a.ns == bp");
    POINTERS_EQUAL_TEXT(d.cpp, huGetNextSibling(d.bp), "bp.ns == cpp");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(d.cpp), "cpp.ns == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(d.cp), "cp.ns == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(d.c), "c.ns == null");
}

TEST(huGetNextSibling, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(NULL), "NULL.ns == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNextSibling(hu_nullNode), "null.ns == null");
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
    CHECK_EQUAL_TEXT(false, huHasKey(hu_nullNode), "null.huHasKey == false");
}


TEST_GROUP(huHasValue)
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

TEST(huHasValue, lists)
{
    CHECK_EQUAL_TEXT(true, huHasValue(l.root), "root.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(l.a), "a.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(l.bp), "bp.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(l.cpp), "cpp.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(l.cp), "cp.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(l.c), "c.true == true");
}

TEST(huHasValue, dicts)
{
    CHECK_EQUAL_TEXT(true, huHasValue(d.root), "root.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(d.a), "a.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(d.bp), "bp.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(d.cpp), "cpp.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(d.cp), "cp.true == true");
    CHECK_EQUAL_TEXT(true, huHasValue(d.c), "c.true == true");
}

TEST(huHasValue, pathological)
{
    CHECK_EQUAL_TEXT(false, huHasValue(NULL), "NULL.true == false");
    CHECK_EQUAL_TEXT(false, huHasValue(hu_nullNode), "null.true == false");
}


TEST_GROUP(huGetNestedValue)
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

TEST(huGetNestedValue, lists)
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

    auto sA = huGetNestedValue(l.a);
    auto sBp = huGetNestedValue(l.bp);
    auto sCpp = huGetNestedValue(l.cpp);

    LONGS_EQUAL_TEXT(strA.size(), sA.size, "a sz");
    STRNCMP_EQUAL_TEXT(strA.data(), sA.ptr, strA.size(), "a str");
    LONGS_EQUAL_TEXT(strBp.size(), sBp.size, "bp sz");
    STRNCMP_EQUAL_TEXT(strBp.data(), sBp.ptr, strBp.size(), "bp str");
    LONGS_EQUAL_TEXT(strCpp.size(), sCpp.size, "cpp sz");
    STRNCMP_EQUAL_TEXT(strCpp.data(), sCpp.ptr, strCpp.size(), "cpp str");
}

TEST(huGetNestedValue, dicts)
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

    auto sA = huGetNestedValue(d.a);
    auto sBp = huGetNestedValue(d.bp);
    auto sCpp = huGetNestedValue(d.cpp);

    LONGS_EQUAL_TEXT(strA.size(), sA.size, "a sz");
    STRNCMP_EQUAL_TEXT(strA.data(), sA.ptr, strA.size(), "a str");
    LONGS_EQUAL_TEXT(strBp.size(), sBp.size, "bp sz");
    STRNCMP_EQUAL_TEXT(strBp.data(), sBp.ptr, strBp.size(), "bp str");
    LONGS_EQUAL_TEXT(strCpp.size(), sCpp.size, "cpp sz");
    STRNCMP_EQUAL_TEXT(strCpp.data(), sCpp.ptr, strCpp.size(), "cpp str");
}

TEST(huGetNestedValue, stringy)
{
    auto strA = R"("aaa": bbb)"sv;
    auto strB = R"(ccc: "ddd")"sv;
    auto strC = R"("eee": "fff")"sv;
    auto strD = R"(ggg: hhh)"sv;

    auto sA = huGetNestedValue(s.aaa);
    auto sB = huGetNestedValue(s.ccc);
    auto sC = huGetNestedValue(s.eee);
    auto sD = huGetNestedValue(s.ggg);

    LONGS_EQUAL_TEXT(strA.size(), sA.size, "aaa sz");
    STRNCMP_EQUAL_TEXT(strA.data(), sA.ptr, strA.size(), "aaa str");
    LONGS_EQUAL_TEXT(strB.size(), sB.size, "ccc sz");
    STRNCMP_EQUAL_TEXT(strB.data(), sB.ptr, strB.size(), "ccc str");
    LONGS_EQUAL_TEXT(strC.size(), sC.size, "eee sz");
    STRNCMP_EQUAL_TEXT(strC.data(), sC.ptr, strC.size(), "eee str");
    LONGS_EQUAL_TEXT(strD.size(), sD.size, "ggg sz");
    STRNCMP_EQUAL_TEXT(strD.data(), sD.ptr, strD.size(), "ggg str");
}

TEST(huGetNestedValue, pathological)
{
    auto str = huGetNestedValue(NULL);
    LONGS_EQUAL_TEXT(0, str.size, "NULL -> 0 sz");
    POINTERS_EQUAL_TEXT(NULL, str.ptr, "NULL -> 0 sz");

    str = huGetNestedValue(hu_nullNode);
    LONGS_EQUAL_TEXT(0, str.size, "NULL -> 0 sz");
    POINTERS_EQUAL_TEXT(NULL, str.ptr, "NULL -> 0 sz");
}


TEST_GROUP(huGetNumAnnotations)
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

TEST(huGetNumAnnotations, lists)
{
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(l.root),  "root.na == 0");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(l.a),     "a.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(l.bp),    "bp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(l.b),     "b.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(l.cpp),   "cpp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(l.cp),    "cp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(l.c),     "c.na == 2");
}

TEST(huGetNumAnnotations, dicts)
{
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(d.root),  "root.na == 0");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(d.a),     "a.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(d.bp),    "bp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(d.b),     "b.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(d.cpp),   "cpp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(d.cp),    "cp.na == 2");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotations(d.c),     "c.na == 2");
}

TEST(huGetNumAnnotations, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumAnnotations(NULL), "NULL.na == null");
    LONGS_EQUAL_TEXT(0, huGetNumAnnotations(hu_nullNode), "null.na == null");
}


TEST_GROUP(huGetAnnotation)
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

TEST(huGetAnnotation, lists)
{
    huAnnotation const * anno = huGetAnnotation(l.root, 0);
    CHECK_TEXT(anno != NULL, "root.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("name", anno->key->str.ptr, anno->key->str.size, "a.anno 0 k=name");
    STRNCMP_EQUAL_TEXT("root", anno->value->str.ptr, anno->value->str.size, "a.anno 0 v=root");
    anno = huGetAnnotation(l.root, 1);
    CHECK_TEXT(anno != NULL, "root.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("otherName", anno->key->str.ptr, anno->key->str.size, "a.anno 1 k=otherName");
    STRNCMP_EQUAL_TEXT("root", anno->value->str.ptr, anno->value->str.size, "a.anno 1 k=root");

    anno = huGetAnnotation(l.a, 0);
    CHECK_TEXT(anno != NULL, "a.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, anno->key->str.size, "a.anno 0 k=a");
    STRNCMP_EQUAL_TEXT("a", anno->value->str.ptr, anno->value->str.size, "a.anno 0 v=a");
    anno = huGetAnnotation(l.a, 1);
    CHECK_TEXT(anno != NULL, "a.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "a.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->str.ptr, anno->value->str.size, "a.anno 1 k=value");

    anno = huGetAnnotation(l.bp, 0);
    CHECK_TEXT(anno != NULL, "bp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->str.ptr, anno->key->str.size, "bp.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("bp", anno->value->str.ptr, anno->value->str.size, "bp.anno 0 v=bp");
    anno = huGetAnnotation(l.bp, 1);
    CHECK_TEXT(anno != NULL, "bp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "bp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("list", anno->value->str.ptr, anno->value->str.size, "bp.anno 1 k=list");

    anno = huGetAnnotation(l.b, 0);
    CHECK_TEXT(anno != NULL, "b.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->str.ptr, anno->key->str.size, "b.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, anno->value->str.size, "b.anno 0 v=b");
    anno = huGetAnnotation(l.b, 1);
    CHECK_TEXT(anno != NULL, "b.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "b.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->str.ptr, anno->value->str.size, "b.anno 1 k=value");

    anno = huGetAnnotation(l.cpp, 0);
    CHECK_TEXT(anno != NULL, "cpp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, anno->key->str.size, "cpp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cpp", anno->value->str.ptr, anno->value->str.size, "cpp.anno 0 v=cpp");
    anno = huGetAnnotation(l.cpp, 1);
    CHECK_TEXT(anno != NULL, "cpp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "cpp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("list", anno->value->str.ptr, anno->value->str.size, "cpp.anno 1 k=list");

    anno = huGetAnnotation(l.cp, 0);
    CHECK_TEXT(anno != NULL, "cp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, anno->key->str.size, "cp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cp", anno->value->str.ptr, anno->value->str.size, "cp.anno 0 v=cp");
    anno = huGetAnnotation(l.cp, 1);
    CHECK_TEXT(anno != NULL, "cp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "cp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("list", anno->value->str.ptr, anno->value->str.size, "cp.anno 1 k=list");

    anno = huGetAnnotation(l.c, 0);
    CHECK_TEXT(anno != NULL, "c.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, anno->key->str.size, "c.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("c", anno->value->str.ptr, anno->value->str.size, "c.anno 0 v=c");
    anno = huGetAnnotation(l.c, 1);
    CHECK_TEXT(anno != NULL, "c.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "c.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->str.ptr, anno->value->str.size, "c.anno 1 k=value");
}

TEST(huGetAnnotation, dicts)
{
    huAnnotation const * anno = huGetAnnotation(d.root, 0);
    CHECK_TEXT(anno != NULL, "root.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("name", anno->key->str.ptr, anno->key->str.size, "a.anno 0 k=name");
    STRNCMP_EQUAL_TEXT("root", anno->value->str.ptr, anno->value->str.size, "a.anno 0 v=root");
    anno = huGetAnnotation(d.root, 1);
    CHECK_TEXT(anno != NULL, "root.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("otherName", anno->key->str.ptr, anno->key->str.size, "a.anno 1 k=otherName");
    STRNCMP_EQUAL_TEXT("root", anno->value->str.ptr, anno->value->str.size, "a.anno 1 k=root");

    anno = huGetAnnotation(d.a, 0);
    CHECK_TEXT(anno != NULL, "a.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, anno->key->str.size, "a.anno 0 k=a");
    STRNCMP_EQUAL_TEXT("a", anno->value->str.ptr, anno->value->str.size, "a.anno 0 v=a");
    anno = huGetAnnotation(d.a, 1);
    CHECK_TEXT(anno != NULL, "a.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "a.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->str.ptr, anno->value->str.size, "a.anno 1 k=value");

    anno = huGetAnnotation(d.bp, 0);
    CHECK_TEXT(anno != NULL, "bp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->str.ptr, anno->key->str.size, "bp.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("bp", anno->value->str.ptr, anno->value->str.size, "bp.anno 0 v=bp");
    anno = huGetAnnotation(d.bp, 1);
    CHECK_TEXT(anno != NULL, "bp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "bp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", anno->value->str.ptr, anno->value->str.size, "bp.anno 1 k=dict");

    anno = huGetAnnotation(d.b, 0);
    CHECK_TEXT(anno != NULL, "b.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->str.ptr, anno->key->str.size, "b.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, anno->value->str.size, "b.anno 0 v=b");
    anno = huGetAnnotation(d.b, 1);
    CHECK_TEXT(anno != NULL, "b.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "b.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->str.ptr, anno->value->str.size, "b.anno 1 k=value");

    anno = huGetAnnotation(d.cpp, 0);
    CHECK_TEXT(anno != NULL, "cpp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, anno->key->str.size, "cpp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cpp", anno->value->str.ptr, anno->value->str.size, "cpp.anno 0 v=cpp");
    anno = huGetAnnotation(d.cpp, 1);
    CHECK_TEXT(anno != NULL, "cpp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "cpp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", anno->value->str.ptr, anno->value->str.size, "cpp.anno 1 k=dict");

    anno = huGetAnnotation(d.cp, 0);
    CHECK_TEXT(anno != NULL, "cp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, anno->key->str.size, "cp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cp", anno->value->str.ptr, anno->value->str.size, "cp.anno 0 v=cp");
    anno = huGetAnnotation(d.cp, 1);
    CHECK_TEXT(anno != NULL, "cp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "cp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", anno->value->str.ptr, anno->value->str.size, "cp.anno 1 k=dict");

    anno = huGetAnnotation(d.c, 0);
    CHECK_TEXT(anno != NULL, "c.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, anno->key->str.size, "c.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("c", anno->value->str.ptr, anno->value->str.size, "c.anno 0 v=c");
    anno = huGetAnnotation(d.c, 1);
    CHECK_TEXT(anno != NULL, "c.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->str.ptr, anno->key->str.size, "c.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->str.ptr, anno->value->str.size, "c.anno 1 k=value");
}

TEST(huGetAnnotation, pathological)
{
    POINTERS_EQUAL_TEXT(NULL, huGetAnnotation(NULL, 0), "NULL.anno 0 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetAnnotation(hu_nullNode, 0), "null.anno 0 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetAnnotation(l.a, 3), "a.anno 3 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetAnnotation(l.a, -1), "a.anno -1 == NULL");
}


TEST_GROUP(huHasAnnotationWithKey)
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

TEST(huHasAnnotationWithKey, lists)
{
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.root, "name"), "root.hawk name == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.root, "otherName"), "root.hawk otherName == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.root, "foo"), "root.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.a, "a"), "a.hawk a == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.a, "type"), "a.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.a, "foo"), "a.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.bp, "b"), "bp.hawk b == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.bp, "type"), "bp.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.bp, "foo"), "bp.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.b, "b"), "b.hawk b == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.b, "type"), "b.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.b, "foo"), "b.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.cpp, "c"), "cpp.hawk c == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.cpp, "type"), "cpp.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.cpp, "foo"), "cpp.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.cp, "c"), "cp.hawk c == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.cp, "type"), "cp.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.cp, "foo"), "cp.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.c, "c"), "c.hawk c == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(l.c, "type"), "c.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.c, "foo"), "c.hawk foo == false");
}

TEST(huHasAnnotationWithKey, dicts)
{
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.root, "name"), "root.hawk name == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.root, "otherName"), "root.hawk otherName == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.root, "foo"), "root.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.a, "a"), "a.hawk a == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.a, "type"), "a.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.a, "foo"), "a.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.bp, "b"), "bp.hawk b == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.bp, "type"), "bp.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.bp, "foo"), "bp.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.b, "b"), "b.hawk b == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.b, "type"), "b.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.b, "foo"), "b.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.cpp, "c"), "cpp.hawk c == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.cpp, "type"), "cpp.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.cpp, "foo"), "cpp.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.cp, "c"), "cp.hawk c == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.cp, "type"), "cp.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.cp, "foo"), "cp.hawk foo == false");

    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.c, "c"), "c.hawk c == true");
    CHECK_EQUAL_TEXT(true, huHasAnnotationWithKeyZ(d.c, "type"), "c.hawk type == true");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.c, "foo"), "c.hawk foo == false");
}

TEST(huHasAnnotationWithKey, pathological)
{
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(NULL, ""), "NULL.hawk '' == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(NULL, "foo"), "NULL.hawk foo == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(hu_nullNode, ""), "null.hawk '' == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(hu_nullNode, "foo"), "null.hawk foo == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.root, ""), "root.hawk '' == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.root, NULL), "root.hawk NULL == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.root, ""), "root.hawk '' == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.root, NULL), "root.hawk NULL == false");
}


TEST_GROUP(huGetAnnotationWithKey)
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

TEST(huGetAnnotationWithKey, lists)
{
    CHECK(huGetAnnotationWithKeyZ(l.root, "name") != NULL);

    auto anno = huGetAnnotationWithKeyZ(l.root, "name")->str;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno name size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.ptr, anno.size, "root.anno name == root");
    anno = huGetAnnotationWithKeyZ(l.root, "otherName")->str;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno otherName size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.ptr, anno.size, "root.anno otherName == root");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.root, "foo"), "root.hawk foo == false");

    anno = huGetAnnotationWithKeyZ(l.a, "a")->str;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.ptr, anno.size, "a.anno a == a");
    anno = huGetAnnotationWithKeyZ(l.a, "type")->str;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "a.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.ptr, anno.size, "a.anno type == value");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.a, "foo"), "a.anno foo == null");

    anno = huGetAnnotationWithKeyZ(l.bp, "b")->str;
    LONGS_EQUAL_TEXT(strlen("bp"), anno.size, "bp.anno b size = sz bp");
    STRNCMP_EQUAL_TEXT("bp", anno.ptr, anno.size, "bp.anno a == bp");
    anno = huGetAnnotationWithKeyZ(l.bp, "type")->str;
    LONGS_EQUAL_TEXT(strlen("list"), anno.size, "bp.anno type size = sz list");
    STRNCMP_EQUAL_TEXT("list", anno.ptr, anno.size, "bp.anno type == list");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.bp, "foo"), "bp.anno foo == null");

    anno = huGetAnnotationWithKeyZ(l.b, "b")->str;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.ptr, anno.size, "b.anno b == b");
    anno = huGetAnnotationWithKeyZ(l.b, "type")->str;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "b.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.ptr, anno.size, "b.anno type == value");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.b, "foo"), "b.anno foo == null");

    anno = huGetAnnotationWithKeyZ(l.cpp, "c")->str;
    LONGS_EQUAL_TEXT(strlen("cpp"), anno.size, "cpp.anno b size = sz cpp");
    STRNCMP_EQUAL_TEXT("cpp", anno.ptr, anno.size, "cpp.anno b == cpp");
    anno = huGetAnnotationWithKeyZ(l.cpp, "type")->str;
    LONGS_EQUAL_TEXT(strlen("list"), anno.size, "cpp.anno type size = sz list");
    STRNCMP_EQUAL_TEXT("list", anno.ptr, anno.size, "cpp.anno type == list");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.cpp, "foo"), "cpp.anno foo == null");

    anno = huGetAnnotationWithKeyZ(l.cp, "c")->str;
    LONGS_EQUAL_TEXT(strlen("cp"), anno.size, "cp.anno c size = sz cp");
    STRNCMP_EQUAL_TEXT("cp", anno.ptr, anno.size, "cp.anno c == cp");
    anno = huGetAnnotationWithKeyZ(l.cp, "type")->str;
    LONGS_EQUAL_TEXT(strlen("list"), anno.size, "cp.anno type size = sz list");
    STRNCMP_EQUAL_TEXT("list", anno.ptr, anno.size, "cp.anno type == list");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.cp, "foo"), "cp.anno foo == null");

    anno = huGetAnnotationWithKeyZ(l.c, "c")->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "c.anno c == c");
    anno = huGetAnnotationWithKeyZ(l.c, "type")->str;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "c.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.ptr, anno.size, "c.anno type == value");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.c, "foo"), "c.anno foo == null");
}

TEST(huGetAnnotationWithKey, dicts)
{
    CHECK(huGetAnnotationWithKeyZ(d.root, "name") != NULL);

    auto anno = huGetAnnotationWithKeyZ(d.root, "name")->str;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno name size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.ptr, anno.size, "root.anno name == root");
    anno = huGetAnnotationWithKeyZ(d.root, "otherName")->str;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno otherName size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.ptr, anno.size, "root.anno otherName == root");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(d.root, "foo"), "root.hawk foo == false");

    anno = huGetAnnotationWithKeyZ(d.a, "a")->str;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.ptr, anno.size, "a.anno a == a");
    anno = huGetAnnotationWithKeyZ(d.a, "type")->str;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "a.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.ptr, anno.size, "a.anno type == value");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(d.a, "foo"), "a.anno foo == null");

    anno = huGetAnnotationWithKeyZ(d.bp, "b")->str;
    LONGS_EQUAL_TEXT(strlen("bp"), anno.size, "bp.anno b size = sz bp");
    STRNCMP_EQUAL_TEXT("bp", anno.ptr, anno.size, "bp.anno b == bp");
    anno = huGetAnnotationWithKeyZ(d.bp, "type")->str;
    LONGS_EQUAL_TEXT(strlen("dict"), anno.size, "bp.anno type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", anno.ptr, anno.size, "bp.anno type == dict");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(d.bp, "foo"), "bp.anno foo == null");

    anno = huGetAnnotationWithKeyZ(d.b, "b")->str;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.ptr, anno.size, "b.anno b == b");
    anno = huGetAnnotationWithKeyZ(d.b, "type")->str;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "b.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.ptr, anno.size, "b.anno type == value");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(d.b, "foo"), "b.anno foo == null");

    anno = huGetAnnotationWithKeyZ(d.cpp, "c")->str;
    LONGS_EQUAL_TEXT(strlen("cpp"), anno.size, "cpp.anno b size = sz cpp");
    STRNCMP_EQUAL_TEXT("cpp", anno.ptr, anno.size, "cpp.anno b == cpp");
    anno = huGetAnnotationWithKeyZ(d.cpp, "type")->str;
    LONGS_EQUAL_TEXT(strlen("dict"), anno.size, "cpp.anno type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", anno.ptr, anno.size, "cpp.anno type == dict");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(d.cpp, "foo"), "cpp.anno foo == null");

    anno = huGetAnnotationWithKeyZ(d.cp, "c")->str;
    LONGS_EQUAL_TEXT(strlen("cp"), anno.size, "cp.anno c size = sz cp");
    STRNCMP_EQUAL_TEXT("cp", anno.ptr, anno.size, "cp.anno c == cp");
    anno = huGetAnnotationWithKeyZ(d.cp, "type")->str;
    LONGS_EQUAL_TEXT(strlen("dict"), anno.size, "cp.anno type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", anno.ptr, anno.size, "cp.anno type == dict");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(d.cp, "foo"), "cp.anno foo == null");

    anno = huGetAnnotationWithKeyZ(d.c, "c")->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "c.anno c == c");
    anno = huGetAnnotationWithKeyZ(d.c, "type")->str;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "c.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.ptr, anno.size, "c.anno type == value");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(d.c, "foo"), "c.anno foo == null");
}

TEST(huGetAnnotationWithKey, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(NULL, ""), "NULL.anno '' == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(NULL, "foo"), "NULL.anno foo == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(hu_nullNode, ""), "null.anno '' == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(hu_nullNode, "foo"), "null.anno foo == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.root, ""), "root.anno '' == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithKeyZ(l.root, NULL), "root.anno NULL == null");
}


TEST_GROUP(huGetNumAnnotationsWithValue)
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

TEST(huGetNumAnnotationsWithValue, lists)
{
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsWithValueZ(l.root, "foo"), "root.gnabv foo == 0");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotationsWithValueZ(l.root, "root"), "root.gnabv root == 2");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.a, "a"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.a, "value"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.bp, "bp"), "bp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.bp, "list"), "bp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.b, "b"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.b, "value"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.cpp, "cpp"), "cpp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.cpp, "list"), "cpp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.cp, "cp"), "cp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.cp, "list"), "cp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.c, "c"), "c.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(l.c, "value"), "c.gnabv value == 1");
}

TEST(huGetNumAnnotationsWithValue, dicts)
{
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsWithValueZ(d.root, "foo"), "root.gnabv foo == 0");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotationsWithValueZ(d.root, "root"), "root.gnabv root == 2");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.a, "a"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.a, "value"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.bp, "bp"), "bp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.bp, "dict"), "bp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.b, "b"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.b, "value"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.cpp, "cpp"), "cpp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.cpp, "dict"), "cpp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.cp, "cp"), "cp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.cp, "dict"), "cp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.c, "c"), "c.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsWithValueZ(d.c, "value"), "c.gnabv value == 1");
}

TEST(huGetNumAnnotationsWithValue, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsWithValueZ(NULL, "foo"), "NULL.gnabv foo == 0");
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsWithValueZ(hu_nullNode, "foo"), "null.gnabv foo == 0");
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsWithValueZ(l.root, NULL), "NULL.gnabv NULL == 0");
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsWithValueZ(l.root, ""), "NULL.gnabv '' == 0");
}


TEST_GROUP(huGetAnnotationWithValue)
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

TEST(huGetAnnotationWithValue, lists)
{
    CHECK(huGetAnnotationWithValueZ(l.root, "root", 0) != NULL);

    auto anno = huGetAnnotationWithValueZ(l.root, "root", 0)->str;
    LONGS_EQUAL_TEXT(strlen("name"), anno.size, "root.anno v0 name size = sz root");
    STRNCMP_EQUAL_TEXT("name", anno.ptr, anno.size, "root.anno v0 name == root");
    anno = huGetAnnotationWithValueZ(l.root, "root", 1)->str;
    LONGS_EQUAL_TEXT(strlen("otherName"), anno.size, "root.anno v1 otherName size = sz root");
    STRNCMP_EQUAL_TEXT("otherName", anno.ptr, anno.size, "root.anno v1 otherName == root");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.root, "foo", 0), "root.hawk foo == false");

    anno = huGetAnnotationWithValueZ(l.a, "a", 0)->str;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno v0 a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.ptr, anno.size, "a.anno v0 a == a");
    anno = huGetAnnotationWithValueZ(l.a, "value", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "a.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "a.anno v0 value == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.a, "foo", 0), "a.anno foo == null");

    anno = huGetAnnotationWithValueZ(l.bp, "bp", 0)->str;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "bp.anno v0 bp size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.ptr, anno.size, "bp.anno v0 bp == b");
    anno = huGetAnnotationWithValueZ(l.bp, "list", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "bp.anno v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "bp.anno v0 list == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.bp, "foo", 0), "bp.anno foo == null");

    anno = huGetAnnotationWithValueZ(l.b, "b", 0)->str;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno v0 b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.ptr, anno.size, "b.anno v0 b == b");
    anno = huGetAnnotationWithValueZ(l.b, "value", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "b.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "b.anno v0 value == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.b, "foo", 0), "b.anno foo == null");

    anno = huGetAnnotationWithValueZ(l.cpp, "cpp", 0)->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cpp.anno v0 cpp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "cpp.anno v0 cpp == c");
    anno = huGetAnnotationWithValueZ(l.cpp, "list", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cpp.anno v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "cpp.anno v0 list == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.cpp, "foo", 0), "cpp.anno foo == null");

    anno = huGetAnnotationWithValueZ(l.cp, "cp", 0)->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cp.anno v0 cp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "cp.anno v0 cp == c");
    anno = huGetAnnotationWithValueZ(l.cp, "list", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cp.anno v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "cp.anno v0 list == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.cp, "foo", 0), "cp.anno foo == null");

    anno = huGetAnnotationWithValueZ(l.c, "c", 0)->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno v0 c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "c.anno v0 c == c");
    anno = huGetAnnotationWithValueZ(l.c, "value", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "c.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "c.anno v0 value == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.c, "foo", 0), "c.anno foo == null");
}

TEST(huGetAnnotationWithValue, dicts)
{
    CHECK(huGetAnnotationWithValueZ(d.root, "root", 0) != NULL);

    auto anno = huGetAnnotationWithValueZ(d.root, "root", 0)->str;
    LONGS_EQUAL_TEXT(strlen("name"), anno.size, "root.anno v0 name size = sz root");
    STRNCMP_EQUAL_TEXT("name", anno.ptr, anno.size, "root.anno v0 name == root");
    anno = huGetAnnotationWithValueZ(d.root, "root", 1)->str;
    LONGS_EQUAL_TEXT(strlen("otherName"), anno.size, "root.anno v1 otherName size = sz root");
    STRNCMP_EQUAL_TEXT("otherName", anno.ptr, anno.size, "root.anno v1 otherName == root");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(d.root, "foo", 0), "root.hawk foo == false");

    anno = huGetAnnotationWithValueZ(d.a, "a", 0)->str;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno v0 a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.ptr, anno.size, "a.anno v0 a == a");
    anno = huGetAnnotationWithValueZ(d.a, "value", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "a.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "a.anno v0 value == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(d.a, "foo", 0), "a.anno foo == null");

    anno = huGetAnnotationWithValueZ(d.bp, "bp", 0)->str;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "bp.anno v0 bp size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.ptr, anno.size, "bp.anno v0 bp == b");
    anno = huGetAnnotationWithValueZ(d.bp, "dict", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "bp.anno v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "bp.anno v0 dict == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(d.bp, "foo", 0), "bp.anno foo == null");

    anno = huGetAnnotationWithValueZ(d.b, "b", 0)->str;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno v0 b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.ptr, anno.size, "b.anno v0 b == b");
    anno = huGetAnnotationWithValueZ(d.b, "value", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "b.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "b.anno v0 value == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(d.b, "foo", 0), "b.anno foo == null");

    anno = huGetAnnotationWithValueZ(d.cpp, "cpp", 0)->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cpp.anno v0 cpp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "cpp.anno v0 cpp == c");
    anno = huGetAnnotationWithValueZ(d.cpp, "dict", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cpp.anno v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "cpp.anno v0 dict == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(d.cpp, "foo", 0), "cpp.anno foo == null");

    anno = huGetAnnotationWithValueZ(d.cp, "cp", 0)->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cp.anno v0 cp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "cp.anno v0 cp == c");
    anno = huGetAnnotationWithValueZ(d.cp, "dict", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cp.anno v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "cp.anno v0 dict == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(d.cp, "foo", 0), "cp.anno foo == null");

    anno = huGetAnnotationWithValueZ(d.c, "c", 0)->str;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno v0 c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.ptr, anno.size, "c.anno v0 c == c");
    anno = huGetAnnotationWithValueZ(d.c, "value", 0)->str;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "c.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.ptr, anno.size, "c.anno v0 value == type");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(d.c, "foo", 0), "c.anno foo == null");
}

TEST(huGetAnnotationWithValue, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(NULL, "", 0), "NULL.anno v0 '' == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(NULL, "foo", 0), "NULL.anno v0 foo == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(hu_nullNode, "", 0), "null.anno v0 '' == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(hu_nullNode, "foo", 0), "null.anno v0 foo == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.root, "", 0), "root.anno v0 '' == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.root, NULL, 0), "root.anno v0 NULL == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.root, "root", -1), "root.anno v-1 root == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetAnnotationWithValueZ(l.root, "root", 3), "root.anno v3 root == null");
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
    LONGS_EQUAL_TEXT(0, huGetNumComments(hu_nullNode), "null.gnc == 0");
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
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetComment(NULL, 0), "NULL.comm 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetComment(hu_nullNode, 0), "null.comm 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetComment(l.root, 0), "root.comm 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetComment(l.root, -1), "root.comm -1 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetComment(l.a, 2), "root.comm 2 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetComment(l.a, -1), "root.comm -1 == null");
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
    huToken const * comm = huGetCommentsContainingZ(l.a, "aaa", NULL);
    CHECK(comm != hu_nullToken);
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(l.a, "aaa", comm);
    CHECK(comm != hu_nullToken);
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 1 == exp");
    comm = huGetCommentsContainingZ(l.a, "aaa", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc aaa 2 = null");
    
    comm = huGetCommentsContainingZ(l.a, "right here", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(l.a, "right here", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc right here 1 = null");

    comm = huGetCommentsContainingZ(l.bp, "bp", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(l.bp, "bp", comm);
    CHECK(comm != hu_nullToken);
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 1 == exp");
    comm = huGetCommentsContainingZ(l.bp, "bp", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc bp 2 = null");

    comm = huGetCommentsContainingZ(l.bp, "right here", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(l.bp, "right here", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc bp 1 = null");

    comm = huGetCommentsContainingZ(l.b, "bbb", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "b.gcc bbb 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "b.gcc bbb 0 == exp");
    comm = huGetCommentsContainingZ(l.b, "bbb", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc b 1 = null");

    comm = huGetCommentsContainingZ(l.cpp, "cpp", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(l.cpp, "cpp", comm);
    CHECK(comm != hu_nullToken);
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 1 == exp");
    comm = huGetCommentsContainingZ(l.cpp, "cpp", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc cpp 2 = null");

    comm = huGetCommentsContainingZ(l.cpp, "right here", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(l.cpp, "right here", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "cpp.gcc right here 1 = null");

    comm = huGetCommentsContainingZ(l.cp, "cp", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cp.gcc cp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc cp 0 == exp");
    comm = huGetCommentsContainingZ(l.cp, "cp", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "cp.gcc cp 1 = null");

    comm = huGetCommentsContainingZ(l.c, "ccc", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "c.gcc ccc 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc ccc 0 == exp");
    comm = huGetCommentsContainingZ(l.c, "ccc", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "c.gcc ccc 1 = null");
}

TEST(huGetCommentsContaining, dicts)
{
    huToken const * comm = huGetCommentsContainingZ(d.a, "aaa", NULL);
    CHECK(comm != hu_nullToken);
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(d.a, "aaa", comm);
    CHECK(comm != hu_nullToken);
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 1 == exp");
    comm = huGetCommentsContainingZ(d.a, "aaa", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc aaa 2 = null");
    
    comm = huGetCommentsContainingZ(d.a, "right here", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "a.gcc aaa 0 == exp");
    comm = huGetCommentsContainingZ(d.a, "right here", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc right here 1 = null");

    comm = huGetCommentsContainingZ(d.bp, "bp", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(d.bp, "bp", comm);
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 1 == exp");
    comm = huGetCommentsContainingZ(d.bp, "bp", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc bp 2 = null");

    comm = huGetCommentsContainingZ(d.bp, "right here", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "bp.gcc bp 0 == exp");
    comm = huGetCommentsContainingZ(d.bp, "right here", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc bp 1 = null");

    comm = huGetCommentsContainingZ(d.b, "bbb", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "b.gcc bbb 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "b.gcc bbb 0 == exp");
    comm = huGetCommentsContainingZ(d.b, "bbb", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc b 1 = null");

    comm = huGetCommentsContainingZ(d.cpp, "cpp", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(d.cpp, "cpp", comm);
    CHECK(comm != hu_nullToken);
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 1 == exp");
    comm = huGetCommentsContainingZ(d.cpp, "cpp", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "a.gcc cpp 2 = null");

    comm = huGetCommentsContainingZ(d.cpp, "right here", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cpp.gcc cpp 0 == exp");
    comm = huGetCommentsContainingZ(d.cpp, "right here", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "cpp.gcc right here 1 = null");

    comm = huGetCommentsContainingZ(d.cp, "cp", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "cp.gcc cp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc cp 0 == exp");
    comm = huGetCommentsContainingZ(d.cp, "cp", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "cp.gcc cp 1 = null");

    comm = huGetCommentsContainingZ(d.c, "ccc", NULL);
    CHECK(comm != hu_nullToken);
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm->str.size, "c.gcc ccc 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm->str.ptr, comm->str.size, "cp.gcc ccc 0 == exp");
    comm = huGetCommentsContainingZ(d.c, "ccc", comm);
    POINTERS_EQUAL_TEXT(hu_nullToken, comm, "c.gcc ccc 1 = null");
}

TEST(huGetCommentsContaining, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetCommentsContainingZ(NULL, "aaa", NULL), "NULL.gcc aaa == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetCommentsContainingZ(hu_nullNode, "aaa", NULL), "null.gcc aaa == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetCommentsContainingZ(l.root, "aaa", NULL), "root.comm aaa == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetCommentsContainingZ(l.root, NULL, NULL), "a.comm NULL == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetCommentsContainingZ(l.a, NULL, NULL), "a.comm NULL == null");
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

    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, ".."));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.a, "0"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, "3"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, "0/0"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, "1/1"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, "1/0/0"));
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

    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(d.root, ".."));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(d.a, "0"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(d.root, "3"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(d.root, "0/0"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(d.root, "1/1"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(d.root, "1/0/0"));
}

TEST(huGetNodeByRelativeAddress, pathological)
{
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(NULL, "0"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(hu_nullNode, "0"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, "-1"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.a, "-1"));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, NULL));
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.root, ""));
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.root, ""));
    POINTERS_EQUAL(hu_nullNode, huGetNodeByRelativeAddressZ(l.root, "/"));
}


TEST_GROUP(huGetNodeAddress)
{
    htd_listOfLists l;
    htd_dictOfDicts d;
    htd_withFunkyAddresses a;

    void setup()
    {
        l.setup();
        d.setup();
        a.setup();
    }

    void teardown()
    {
        d.teardown();
        l.teardown();
        a.teardown();
    }
};

TEST(huGetNodeAddress, lists)
{
    int addressLen = 0;
    huGetNodeAddress(l.root, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(l.root, s, & addressLen);
    auto exp = "/";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(l.a, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(l.a, s, & addressLen);
    exp = "/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(l.bp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(l.bp, s, & addressLen);
    exp = "/1";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(l.b, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(l.b, s, & addressLen);
    exp = "/1/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(l.cpp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(l.cpp, s, & addressLen);
    exp = "/2";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(l.cp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(l.cp, s, & addressLen);
    exp = "/2/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(l.c, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(l.c, s, & addressLen);
    exp = "/2/0/0";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;
}


TEST(huGetNodeAddress, dicts)
{
    int addressLen = 0;
    huGetNodeAddress(d.root, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(d.root, s, & addressLen);
    auto exp = "/";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(d.a, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(d.a, s, & addressLen);
    exp = "/ak";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(d.bp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(d.bp, s, & addressLen);
    exp = "/bk";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(d.b, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(d.b, s, & addressLen);
    exp = "/bk/bk";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(d.cpp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(d.cpp, s, & addressLen);
    exp = "/ck";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(d.cp, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(d.cp, s, & addressLen);
    exp = "/ck/ck";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(d.c, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(d.c, s, & addressLen);
    exp = "/ck/ck/ck";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    delete [] s;
}

TEST(huGetNodeAddress, overflow)
{
    int addressLen = 0;
    huGetNodeAddress(d.c, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    addressLen = 3;
    s[addressLen] = '!';
    huGetNodeAddress(d.c, s, & addressLen);
    char const * exp = "/ck!";             // contains "/ck/ck/ck"
    LONGS_EQUAL(3, addressLen);
    STRNCMP_EQUAL(exp, s, 4);
    delete [] s;
}

TEST(huGetNodeAddress, funky)
{
    int addressLen = 0;
    huGetNodeAddress(a.a, NULL, & addressLen);
    char * s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.a, s, & addressLen);
    auto exp = "/\"/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.a, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.b, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.b, s, & addressLen);
    exp = "/\"a/b\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.b, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.c, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.c, s, & addressLen);
    exp = "/\"/b\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.c, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.d, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.d, s, & addressLen);
    exp = "/\"a/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.d, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.e, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.e, s, & addressLen);
    exp = "/\"/\\\"foo\\\"\"";      //        /"/\"foo\"
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.e, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.f, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.f, s, & addressLen);
    exp = "/\"/\\\"foo'bar'\\\"\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.f, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.g, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.g, s, & addressLen);
    exp = "/\"/\\\"foo'bar`baz`'\\\"\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.g, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.h, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.h, s, & addressLen);
    exp = "/\"/'foo`bar\\\"baz\\\"`'\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.h, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.i, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.i, s, & addressLen);
    exp = "/\"/`foo\\\"bar'baz'\\\"`\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.i, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.j, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.j, s, & addressLen);
    exp = "/\"a\\\"foo\\\"/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.j, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.k, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.k, s, & addressLen);
    exp = "/\"a\\\"foo'bar'\\\"/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.k, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.l, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.l, s, & addressLen);
    exp = "/\"a\\\"foo'bar`baz`'\\\"/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.l, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.m, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.m, s, & addressLen);
    exp = "/\"a'foo`bar\\\"baz\\\"`'/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.m, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.n, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.n, s, & addressLen);
    exp = "/\"a`foo\\\"bar'baz'\\\"`/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.n, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.o, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.o, s, & addressLen);
    exp = "/\"0\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.o, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.p, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.p, s, & addressLen);
    exp = "/01m";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.p, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.q, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.q, s, & addressLen);
    exp = "/\"/\\\"0123456789012345678901234567890123456789\\\"\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.q, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;

    addressLen = 0;
    huGetNodeAddress(a.r, NULL, & addressLen);
    s = new char[addressLen + 1];
    s[addressLen] = '\0';
    huGetNodeAddress(a.r, s, & addressLen);
    exp = "/\"a\\\"0123456789012345678901234567890123456789\\\"/\"";
    LONGS_EQUAL(strlen(exp), addressLen);
    STRNCMP_EQUAL(exp, s, addressLen);
    POINTERS_EQUAL(a.r, huGetNodeByFullAddressN(a.trove, s, addressLen));
    delete [] s;
}

TEST(huGetNodeAddress, pathological)
{
    int len = 256;
    char str[256] = {0};
    huGetNodeAddress(NULL, NULL, & len);
    LONGS_EQUAL_TEXT(0, str[0], "NULL");

    huGetNodeAddress(NULL, str, & len);
    LONGS_EQUAL_TEXT(0, str[0], "NULL");

    huGetNodeAddress(hu_nullNode, NULL, & len);
    LONGS_EQUAL_TEXT(0, str[0], "null");

    huGetNodeAddress(hu_nullNode, str, & len);
    LONGS_EQUAL_TEXT(0, str[0], "null");
}


// ------------------------------ TROVE API TESTS

TEST_GROUP(huMakeTroveFromString)
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
TEST(huMakeTroveFromString, pathological)
{
    huTrove const * trove = hu_nullTrove;
    int error = HU_ERROR_NOERROR;
    huLoadParams params;
    huInitLoadParams(& params, HU_ENCODING_UTF8, 4, true);

    error = huMakeTroveFromStringZ(NULL, "", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);

    error = huMakeTroveFromStringZ(& trove, NULL, & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromString NULL == NULL");

    error = huMakeTroveFromStringZ(& trove, "", & params);
    LONGS_EQUAL(HU_ERROR_NOERROR, error);
    CHECK_TEXT(hu_nullTrove != trove, "fromString '' != NULL");
    huDestroyTrove(trove);

    params.encoding = -1;
    error = huMakeTroveFromStringZ(& trove, "[]", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "encoding=-1 == NULL");

    params.encoding = HU_ENCODING_UNKNOWN + 1;
    error = huMakeTroveFromStringZ(& trove, "[]", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "encoding=unk+1 == NULL");

    params.encoding = HU_ENCODING_UTF8;
    params.tabSize = -1;
    error = huMakeTroveFromStringZ(& trove, "[]", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "tabs=-1 == NULL");
}


TEST_GROUP(huMakeTroveFromFile)
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
TEST(huMakeTroveFromFile, pathological)
{
    huTrove const * trove = hu_nullTrove;
    int error = HU_ERROR_NOERROR;
    huLoadParams params;
    huInitLoadParams(& params, HU_ENCODING_UTF8, 4, true);

    error = huMakeTroveFromFileZ(NULL, "", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);

    error = huMakeTroveFromFileZ(& trove, NULL, & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromFile NULL == NULL");

    error = huMakeTroveFromFileZ(& trove, "", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromFile '' == NULL");

    error = huMakeTroveFromFileZ(& trove, "..", & params);
    LONGS_EQUAL(HU_ERROR_BADFILE, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromFile .. == NULL");

    error = huMakeTroveFromFileZ(& trove, "/", & params);
    LONGS_EQUAL(HU_ERROR_BADFILE, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromFile / == NULL");

    params.encoding = -1;
    error = huMakeTroveFromFileZ(& trove, "src/test/testFiles/utf8.hu", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromFile encoding=-1 == NULL");

    params.encoding = HU_ENCODING_UNKNOWN + 1;
    error = huMakeTroveFromFileZ(& trove, "src/test/testFiles/utf8.hu", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromFile encoding=unk+1 == NULL");

    params.encoding = HU_ENCODING_UTF8;
    params.tabSize = -1;
    error = huMakeTroveFromFileZ(& trove, "src/test/testFiles/utf8.hu", & params);
    LONGS_EQUAL(HU_ERROR_BADPARAMETER, error);
    POINTERS_EQUAL_TEXT(hu_nullTrove, trove, "fromFile tabSize=-1 == NULL");
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
    huDestroyTrove(hu_nullTrove);
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
    LONGS_EQUAL_TEXT(0, huGetNumTokens(hu_nullTrove), "null numTokens == 0");
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
    CHECK(huGetToken(l.trove, 90) != hu_nullToken);
    LONGS_EQUAL_TEXT(HU_TOKENKIND_EOF, huGetToken(l.trove, 90)->kind, "l tok last == eof");
    CHECK(huGetToken(d.trove, 102) != hu_nullToken);
    LONGS_EQUAL_TEXT(HU_TOKENKIND_EOF, huGetToken(d.trove, 102)->kind, "l tok last == eof");
}

TEST(huGetToken, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetToken(NULL, 0), "NULL tok 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetToken(hu_nullTrove, 0), "null tok 0 == null");
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
    LONGS_EQUAL_TEXT(0, huGetNumNodes(hu_nullTrove), "null numNodes == 0");
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
    CHECK(huGetRootNode(l.trove) != hu_nullNode);
    LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, huGetRootNode(l.trove)->kind, "l grn == list");
    LONGS_EQUAL_TEXT(-1, huGetRootNode(l.trove)->parentNodeIdx, "l grn == root");
    CHECK(huGetRootNode(d.trove) != hu_nullNode);
    LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, huGetRootNode(d.trove)->kind, "d grn == dict");
    LONGS_EQUAL_TEXT(-1, huGetRootNode(d.trove)->parentNodeIdx, "d grn == root");
}

TEST(huGetRootNode, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetRootNode(NULL), "NULL grn == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetRootNode(hu_nullTrove), "null grn == null");
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
    CHECK(huGetNode(l.trove, 0) != hu_nullNode);
    LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, huGetNode(l.trove, 0)->kind, "l gn 0 == list");
    LONGS_EQUAL_TEXT(-1, huGetNode(l.trove, 0)->parentNodeIdx, "l gn 0 == root");
    CHECK(huGetNode(d.trove, 0) != hu_nullNode);
    LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, huGetNode(d.trove, 0)->kind, "d gn 0 == dict");
    LONGS_EQUAL_TEXT(-1, huGetNode(d.trove, 0)->parentNodeIdx, "d gn 0 == root");

    CHECK(huGetNode(l.trove, 1) != hu_nullNode);
    LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, huGetNode(l.trove, 1)->kind, "l gn 1 == list");
    LONGS_EQUAL_TEXT(0, huGetNode(l.trove, 1)->parentNodeIdx, "l gn 1 == child of root");
    CHECK(huGetNode(d.trove, 1) != hu_nullNode);
    LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, huGetNode(d.trove, 1)->kind, "d gn 1 == dict");
    LONGS_EQUAL_TEXT(0, huGetNode(d.trove, 1)->parentNodeIdx, "d gn 1 == child of root");
}

TEST(huGetNode, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNode(NULL, 0), "NULL gn 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNode(hu_nullTrove, 0), "null gn 0 == null");
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
    LONGS_EQUAL_TEXT(0, huGetNumErrors(hu_nullTrove), "null numErrors == 0");
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
    POINTERS_EQUAL_TEXT(NULL, huGetError(hu_nullTrove, 0), "null numErrors == 0");
    POINTERS_EQUAL_TEXT(NULL, huGetError(e.trove, -1), "e ge 0 == syntax error");
}



TEST_GROUP(huGetNumTroveAnnotations)
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

TEST(huGetNumTroveAnnotations, normal)
{
    LONGS_EQUAL_TEXT(3, huGetNumTroveAnnotations(l.trove), "l gnta == 3");
    LONGS_EQUAL_TEXT(3, huGetNumTroveAnnotations(d.trove), "d gnta == 3");
}

TEST(huGetNumTroveAnnotations, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumTroveAnnotations(NULL), "NULL gnta == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTroveAnnotations(hu_nullTrove), "null gnta == 0");
}


TEST_GROUP(huGetTroveAnnotation)
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

TEST(huGetTroveAnnotation, normal)
{
    auto anno = huGetTroveAnnotation(l.trove, 0);
    CHECK_TEXT(anno != NULL, "l anno 0 not NULL");
    LONGS_EQUAL_TEXT(2, anno->key->str.size, "l anno 0 k sz == 2");
    STRNCMP_EQUAL_TEXT("tx", anno->key->str.ptr, anno->key->str.size, "l anno 0 k == tx");
    LONGS_EQUAL_TEXT(2, anno->value->str.size, "l anno 0 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->value->str.ptr, anno->value->str.size, "l anno 0 v == ta");

    anno = huGetTroveAnnotation(l.trove, 1);
    CHECK_TEXT(anno != NULL, "l anno 1 not NULL");
    LONGS_EQUAL_TEXT(2, anno->key->str.size, "l anno 1 k sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->key->str.ptr, anno->key->str.size, "l anno 1 k == ta");
    LONGS_EQUAL_TEXT(2, anno->value->str.size, "l anno 1 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->value->str.ptr, anno->value->str.size, "l anno 1 v == ta");

    anno = huGetTroveAnnotation(l.trove, 2);
    CHECK_TEXT(anno != NULL, "l anno 2 not NULL");
    LONGS_EQUAL_TEXT(2, anno->key->str.size, "l anno 2 k sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->key->str.ptr, anno->key->str.size, "l anno 2 k == tb");
    LONGS_EQUAL_TEXT(2, anno->value->str.size, "l anno 2 v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->value->str.ptr, anno->value->str.size, "l anno 2 v == tb");

    anno = huGetTroveAnnotation(d.trove, 0);
    CHECK_TEXT(anno != NULL, "d anno 0 not NULL");
    LONGS_EQUAL_TEXT(2, anno->key->str.size, "d anno 0 k sz == 2");
    STRNCMP_EQUAL_TEXT("tx", anno->key->str.ptr, anno->key->str.size, "d anno 0 k == tx");
    LONGS_EQUAL_TEXT(2, anno->value->str.size, "d anno 0 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->value->str.ptr, anno->value->str.size, "d anno 0 v == ta");

    anno = huGetTroveAnnotation(d.trove, 1);
    CHECK_TEXT(anno != NULL, "d anno 1 not NULL");
    LONGS_EQUAL_TEXT(2, anno->key->str.size, "d anno 1 k sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->key->str.ptr, anno->key->str.size, "d anno 1 k == ta");
    LONGS_EQUAL_TEXT(2, anno->value->str.size, "d anno 1 v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->value->str.ptr, anno->value->str.size, "d anno 1 v == ta");

    anno = huGetTroveAnnotation(d.trove, 2);
    CHECK_TEXT(anno != NULL, "d anno 2 not NULL");
    LONGS_EQUAL_TEXT(2, anno->key->str.size, "d anno 2 k sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->key->str.ptr, anno->key->str.size, "d anno 2 k == tb");
    LONGS_EQUAL_TEXT(2, anno->value->str.size, "d anno 2 v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->value->str.ptr, anno->value->str.size, "d anno 2 v == tb");
}

TEST(huGetTroveAnnotation, pathological)
{
    POINTERS_EQUAL_TEXT(NULL, huGetTroveAnnotation(NULL, 0), "NULL anno 0 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetTroveAnnotation(hu_nullTrove, 0), "NULL anno 0 == NULL");
}


TEST_GROUP(huTroveHasAnnotationWithKey)
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

TEST(huTroveHasAnnotationWithKey, normal)
{
    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(l.trove, "tx"), "l thawk tx");
    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(l.trove, "ta"), "l thawk ta");
    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(l.trove, "tb"), "l thawk tb");
    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(l.trove, "foo") == false, "l thawk foo == false");

    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(d.trove, "tx"), "d thawk tx");
    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(d.trove, "ta"), "d thawk ta");
    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(d.trove, "tb"), "d thawk tb");
    CHECK_TEXT(huTroveHasAnnotationWithKeyZ(d.trove, "foo") == false, "d thawk foo == false");
}

TEST(huTroveHasAnnotationWithKey, pathological)
{
    CHECK_TEXT(false == huTroveHasAnnotationWithKeyZ(NULL, 0), "NULL thawk 0 == false");
    CHECK_TEXT(false == huTroveHasAnnotationWithKeyZ(hu_nullTrove, 0), "NULL thawk 0 == false");
}


TEST_GROUP(huGetTroveAnnotationWithKey)
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

TEST(huGetTroveAnnotationWithKey, normal)
{
    auto anno = huGetTroveAnnotationWithKeyZ(l.trove, "tx");
    CHECK_TEXT(anno != hu_nullToken, "l anno tx not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno tx v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->str.ptr, anno->str.size, "l anno tx v == ta");

    anno = huGetTroveAnnotationWithKeyZ(l.trove, "ta");
    CHECK_TEXT(anno != hu_nullToken, "l anno ta not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno ta v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->str.ptr, anno->str.size, "l anno ta v == ta");

    anno = huGetTroveAnnotationWithKeyZ(l.trove, "tb");
    CHECK_TEXT(anno != hu_nullToken, "l anno tb not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno tb v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->str.ptr, anno->str.size, "l anno tb v == tb");

    anno = huGetTroveAnnotationWithKeyZ(d.trove, "tx");
    CHECK_TEXT(anno != hu_nullToken, "l anno tx not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno tx v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->str.ptr, anno->str.size, "l anno tx v == ta");

    anno = huGetTroveAnnotationWithKeyZ(d.trove, "ta");
    CHECK_TEXT(anno != hu_nullToken, "l anno ta not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno ta v sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->str.ptr, anno->str.size, "l anno ta v == ta");

    anno = huGetTroveAnnotationWithKeyZ(d.trove, "tb");
    CHECK_TEXT(anno != hu_nullToken, "l anno tb not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno tb v sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->str.ptr, anno->str.size, "l anno tb v == tb");
}

TEST(huGetTroveAnnotationWithKey, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(NULL, "tx"), "NULL anno tx == NULL");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(hu_nullTrove, "tx"), "null anno tx == NULL");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(l.trove, "tq"), "l anno tq == NULL");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(l.trove, ""), "l anno '' == NULL");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(l.trove, NULL), "l anno NULL == NULL");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(d.trove, "tq"), "d anno tq == NULL");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(d.trove, ""), "d anno '' == NULL");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithKeyZ(d.trove, NULL), "d anno NULL == NULL");
}


TEST_GROUP(huGetNumTroveAnnotationsWithValue)
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

TEST(huGetNumTroveAnnotationsWithValue, normal)
{
    LONGS_EQUAL_TEXT(2, huGetNumTroveAnnotationsWithValueZ(l.trove, "ta"), "l gntabv ta == 2");
    LONGS_EQUAL_TEXT(1, huGetNumTroveAnnotationsWithValueZ(l.trove, "tb"), "l gntabv tb == 2");
    LONGS_EQUAL_TEXT(0, huGetNumTroveAnnotationsWithValueZ(l.trove, "tc"), "l gntabv tb == 0");
    LONGS_EQUAL_TEXT(2, huGetNumTroveAnnotationsWithValueZ(d.trove, "ta"), "d gntabv ta == 2");
    LONGS_EQUAL_TEXT(1, huGetNumTroveAnnotationsWithValueZ(d.trove, "tb"), "d gntabv tb == 2");
    LONGS_EQUAL_TEXT(0, huGetNumTroveAnnotationsWithValueZ(d.trove, "tc"), "d gntabv tb == 0");
}

TEST(huGetNumTroveAnnotationsWithValue, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumTroveAnnotationsWithValueZ(NULL, "ta"), "NULL gntabv ta == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTroveAnnotationsWithValueZ(hu_nullTrove, "ta"), "null gntabv ta == 0");
    LONGS_EQUAL_TEXT(0, huGetNumTroveAnnotationsWithValueZ(l.trove, NULL), "l gntabv NULL == 0");
}


TEST_GROUP(huGetTroveAnnotationWithValue)
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

TEST(huGetTroveAnnotationWithValue, normal)
{
    auto anno = huGetTroveAnnotationWithValueZ(l.trove, "ta", 0);
    CHECK_TEXT(anno != hu_nullToken, "l anno v ta 0 not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno v ta 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tx", anno->str.ptr, anno->str.size, "l anno v ta 0 == tx");

    anno = huGetTroveAnnotationWithValueZ(l.trove, "ta", 1);
    CHECK_TEXT(anno != hu_nullToken, "l anno v ta 1 not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno v ta 1 sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->str.ptr, anno->str.size, "l anno v ta 1 == ta");

    anno = huGetTroveAnnotationWithValueZ(l.trove, "tb", 0);
    CHECK_TEXT(anno != hu_nullToken, "l anno v tb 0 not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno v tb 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->str.ptr, anno->str.size, "l anno v tb 0 == tb");

    anno = huGetTroveAnnotationWithValueZ(d.trove, "ta", 0);
    CHECK_TEXT(anno != hu_nullToken, "l anno v ta 0 not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno v ta 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tx", anno->str.ptr, anno->str.size, "l anno v ta 0 == tx");

    anno = huGetTroveAnnotationWithValueZ(d.trove, "ta", 1);
    CHECK_TEXT(anno != hu_nullToken, "l anno v ta 1 not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno v ta 1 sz == 2");
    STRNCMP_EQUAL_TEXT("ta", anno->str.ptr, anno->str.size, "l anno v ta 1 == ta");

    anno = huGetTroveAnnotationWithValueZ(d.trove, "tb", 0);
    CHECK_TEXT(anno != hu_nullToken, "l anno v tb 0 not NULL");
    LONGS_EQUAL_TEXT(2, anno->str.size, "l anno v tb 0 sz == 2");
    STRNCMP_EQUAL_TEXT("tb", anno->str.ptr, anno->str.size, "l anno v tb 0 == tb");
}

TEST(huGetTroveAnnotationWithValue, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithValueZ(NULL, "ta", 0), "NULL anno v ta 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithValueZ(hu_nullTrove, "ta", 0), "null anno v ta 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithValueZ(l.trove, NULL, 0), "l anno v NULL 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithValueZ(l.trove, "ta", 3), "l anno v ta 3 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveAnnotationWithValueZ(l.trove, "ta", -1), "l anno v ta -1 == null");
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
    LONGS_EQUAL_TEXT(0, huGetNumTroveComments(hu_nullTrove), "null gntc == 0");
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

    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveComment(l.trove, 2), "l comm 2 == null");
}

TEST(huGetTroveComment, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveComment(NULL, 0), "NULL comm 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveComment(hu_nullTrove, 0), "null comm 0 == null");
    POINTERS_EQUAL_TEXT(hu_nullToken, huGetTroveComment(l.trove, -1), "l comm -1 == null");
}


TEST_GROUP(huGetNodeByFullAddress)
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

TEST(huGetNodeByFullAddress, lists)
{
    POINTERS_EQUAL_TEXT(l.root, huGetNodeByFullAddressZ(l.trove, "/"), "l gnbfa '/' == root");
    POINTERS_EQUAL_TEXT(l.a, huGetNodeByFullAddressZ(l.trove, "/0"), "l gnbfa '/0' == a");
    POINTERS_EQUAL_TEXT(l.bp, huGetNodeByFullAddressZ(l.trove, "/1"), "l gnbfa '/1' == bp");
    POINTERS_EQUAL_TEXT(l.b, huGetNodeByFullAddressZ(l.trove, "/1/0"), "l gnbfa '/1/0'== b");
    POINTERS_EQUAL_TEXT(l.cpp, huGetNodeByFullAddressZ(l.trove, "/2"), "l gnbfa '/2'== cpp");
    POINTERS_EQUAL_TEXT(l.cp, huGetNodeByFullAddressZ(l.trove, "/2/0"), "l gnbfa '/2/0'== cp");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByFullAddressZ(l.trove, "/2/0/0"), "l gnbfa '/2/0/0'== c");

    POINTERS_EQUAL_TEXT(l.c, huGetNodeByFullAddressZ(l.trove, "/0/../2/0/0"), "l gnbfa '/0/../2/0/0'== c");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByFullAddressZ(l.trove, "/0/../1/0/../../2/0/0"), "l gnbfa '/0/../1/0/../../2/0/0'== c");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByFullAddressZ(l.trove, " / 0 / .. / 2 / 0 / 0 "), "l gnbfa ' / 0 / .. / 2 / 0 / 0 '== c");
    POINTERS_EQUAL_TEXT(l.c, huGetNodeByFullAddressZ(l.trove, " / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 "), "l gnbfa ' / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 '== c");
}

TEST(huGetNodeByFullAddress, dicts)
{
    POINTERS_EQUAL_TEXT(d.root, huGetNodeByFullAddressZ(d.trove, "/"), "d gnbfa '/' == a");
    POINTERS_EQUAL_TEXT(d.a, huGetNodeByFullAddressZ(d.trove, "/0"), "d gnbfa '/0'== a");
    POINTERS_EQUAL_TEXT(d.bp, huGetNodeByFullAddressZ(d.trove, "/1"), "d gnbfa '/1'== bp");
    POINTERS_EQUAL_TEXT(d.b, huGetNodeByFullAddressZ(d.trove, "/1/0"), "d gnbfa '/1/0'== b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetNodeByFullAddressZ(d.trove, "/2"), "d gnbfa '/2'== cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetNodeByFullAddressZ(d.trove, "/2/0"), "d gnbfa '/2/0'== cp");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, "/2/0/0"), "d gnbfa '/2/0/0'== c");

    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, "/0/../2/0/0"), "d gnbfa '/0/../2/0/0'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, "/0/../1/0/../../2/0/0"), "d gnbfa '/0/../1/0/../../2/0/0'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, " / 0 / .. / 2 / 0 / 0 "), "d gnbfa ' / 0 / .. / 2 / 0 / 0 '== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, " / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 "), "d gnbfa ' / 0 / .. / 1 / 0 / .. / .. / 2 / 0 / 0 '== c");

    POINTERS_EQUAL_TEXT(d.root, huGetNodeByFullAddressZ(d.trove, "/"), "d gnbfa '/' == a");
    POINTERS_EQUAL_TEXT(d.a, huGetNodeByFullAddressZ(d.trove, "/ak"), "d gnbfa '/ak'== a");
    POINTERS_EQUAL_TEXT(d.bp, huGetNodeByFullAddressZ(d.trove, "/bk"), "d gnbfa '/bk'== bp");
    POINTERS_EQUAL_TEXT(d.b, huGetNodeByFullAddressZ(d.trove, "/bk/bk"), "d gnbfa '/bk/bk'== b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetNodeByFullAddressZ(d.trove, "/ck"), "d gnbfa '/ck'== cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetNodeByFullAddressZ(d.trove, "/ck/ck"), "d gnbfa '/ck/ck'== cp");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, "/ck/ck/ck"), "d gnbfa '/ck/ck/ck'== c");

    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, "/ak/../ck/ck/ck"), "d gnbfa '/ak/../ck/ck/ck'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, "/ak/../bk/bk/../../ck/ck/ck"), "d gnbfa '/ak/../bk/bk/../../ck/ck/ck'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, " / ak / .. / ck / ck / ck "), "d gnbfa ' / ak / .. / ck / ck / ck '== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, " / ak / .. / bk / bk / .. / .. / ck / ck / ck "), "d gnbfa ' / ak / .. / bk / bk / .. / .. / ck / ck / ck '== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, "/ak/../`ck`/'ck'/\"ck\""), "d gnbfa '/ak/../`ck`/'ck'/\"ck\"'== c");
    POINTERS_EQUAL_TEXT(d.c, huGetNodeByFullAddressZ(d.trove, " / ak / .. / `ck` / 'ck' / \"ck\" "), "d gnbfa ' / ak / .. / `ck` / 'ck' / \"ck\" '== c");
}

TEST(huGetNodeByFullAddress, pathological)
{
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNodeByFullAddressZ(NULL, "/"), "NULL gnbfa '/' == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNodeByFullAddressZ(hu_nullTrove, "/"), "null gnbfa '/' == c");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNodeByFullAddressZ(l.trove, "/.."), "l gnbfa '/..' == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNodeByFullAddressZ(l.trove, ".."), "l gnbfa '..' == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNodeByFullAddressZ(l.trove, "0"), "l gnbfa '0' == null");
    POINTERS_EQUAL_TEXT(hu_nullNode, huGetNodeByFullAddressZ(l.trove, "//"), "l gnbfa '//' == null");
}


TEST_GROUP(huFindNodesWithAnnotationKey)
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

TEST(huFindNodesWithAnnotationKey, normal)
{
    huNode const * node = huFindNodesWithAnnotationKeyZ(l.trove, "a", NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbak a 0 == a");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "a", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak a 1 == null");

    node = huFindNodesWithAnnotationKeyZ(l.trove, "b", NULL);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbak b 0 == bp");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "b", node);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbak b 1 == b");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "b", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak b 2 == null");

    node = huFindNodesWithAnnotationKeyZ(l.trove, "c", NULL);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbak c 0 == cpp");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "c", node);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbak c 1 == cp");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "c", node);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbak c 2 == c");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "c", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak c 3 == null");

    node = huFindNodesWithAnnotationKeyZ(l.trove, "type", NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbak type 0 == a");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "type", node);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbak type 1 == bp");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "type", node);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbak type 2 == b");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "type", node);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbak type 3 == cpp");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "type", node);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbak type 4 == cp");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "type", node);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbak type 5 == c");
    node = huFindNodesWithAnnotationKeyZ(l.trove, "type", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak type 6 == null");

    node = huFindNodesWithAnnotationKeyZ(l.trove, "foo", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak foo 0 == null");

    node = huFindNodesWithAnnotationKeyZ(d.trove, "a", NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbak a 0 == a");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "a", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbak a 1 == null");

    node = huFindNodesWithAnnotationKeyZ(d.trove, "b", NULL);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbak b 0 == bp");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "b", node);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbak b 1 == b");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "b", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbak b 2 == null");

    node = huFindNodesWithAnnotationKeyZ(d.trove, "c", NULL);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbak c 0 == cpp");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "c", node);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbak c 1 == cp");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "c", node);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbak c 2 == c");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "c", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbak c 3 == null");

    node = huFindNodesWithAnnotationKeyZ(d.trove, "type", NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbak type 0 == a");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "type", node);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbak type 1 == bp");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "type", node);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbak type 2 == b");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "type", node);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbak type 3 == cpp");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "type", node);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbak type 4 == cp");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "type", node);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbak type 5 == c");
    node = huFindNodesWithAnnotationKeyZ(d.trove, "type", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbak type 6 == null");

    node = huFindNodesWithAnnotationKeyZ(d.trove, "foo", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbak foo 0 == null");
}

TEST(huFindNodesWithAnnotationKey, pathological)
{
    huNode const * node = huFindNodesWithAnnotationKeyZ(NULL, "type", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "NULL fnbak type == null");

    node = huFindNodesWithAnnotationKeyZ(hu_nullTrove, "type", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "null fnbak foo 0 == null");

    node = huFindNodesWithAnnotationKeyZ(l.trove, NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak NULL == null");

    node = huFindNodesWithAnnotationKeyZ(l.trove, "", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak '' 0 == null");

    node = huFindNodesWithAnnotationKeyZ(d.trove, NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbak NULL == null");

    node = huFindNodesWithAnnotationKeyZ(d.trove, "", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbak '' 0 == null");
}


TEST_GROUP(huFindNodesWithAnnotationValue)
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

TEST(huFindNodesWithAnnotationValue, normal)
{
    huNode const * node = huFindNodesWithAnnotationValueZ(l.trove, "a", NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbav a 0 == a");
    node = huFindNodesWithAnnotationValueZ(l.trove, "a", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav a 1 == null");

    node = huFindNodesWithAnnotationValueZ(l.trove, "b", NULL);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbav b 0 == b");
    node = huFindNodesWithAnnotationValueZ(l.trove, "b", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav b 1 == null");

    node = huFindNodesWithAnnotationValueZ(l.trove, "c", NULL);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbav c 0 == c");
    node = huFindNodesWithAnnotationValueZ(l.trove, "c", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav c 1 == null");

    node = huFindNodesWithAnnotationValueZ(l.trove, "value", NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbav value 0 == a");
    node = huFindNodesWithAnnotationValueZ(l.trove, "value", node);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbav value 1 == b");
    node = huFindNodesWithAnnotationValueZ(l.trove, "value", node);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbav value 2 == c");
    node = huFindNodesWithAnnotationValueZ(l.trove, "value", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav value 3 == null");

    node = huFindNodesWithAnnotationValueZ(l.trove, "list", NULL);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbav list 0 == bp");
    node = huFindNodesWithAnnotationValueZ(l.trove, "list", node);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbav list 1 == cpp");
    node = huFindNodesWithAnnotationValueZ(l.trove, "list", node);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbav list 2 == cp");
    node = huFindNodesWithAnnotationValueZ(l.trove, "list", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav list 3 == null");

    node = huFindNodesWithAnnotationValueZ(l.trove, "foo", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav foo 0 == null");

    node = huFindNodesWithAnnotationValueZ(d.trove, "a", NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbav a 0 == a");
    node = huFindNodesWithAnnotationValueZ(d.trove, "a", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbav a 1 == null");

    node = huFindNodesWithAnnotationValueZ(d.trove, "b", NULL);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbav b 0 == b");
    node = huFindNodesWithAnnotationValueZ(d.trove, "b", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbav b 1 == null");

    node = huFindNodesWithAnnotationValueZ(d.trove, "c", NULL);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbav c 0 == c");
    node = huFindNodesWithAnnotationValueZ(d.trove, "c", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbav c 1 == null");

    node = huFindNodesWithAnnotationValueZ(d.trove, "value", NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbav value 0 == a");
    node = huFindNodesWithAnnotationValueZ(d.trove, "value", node);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbav value 1 == b");
    node = huFindNodesWithAnnotationValueZ(d.trove, "value", node);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbav value 2 == c");
    node = huFindNodesWithAnnotationValueZ(d.trove, "value", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbav value 3 == null");

    node = huFindNodesWithAnnotationValueZ(d.trove, "dict", NULL);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbav dict 0 == bp");
    node = huFindNodesWithAnnotationValueZ(d.trove, "dict", node);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbav dict 1 == cpp");
    node = huFindNodesWithAnnotationValueZ(d.trove, "dict", node);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbav dict 2 == cp");
    node = huFindNodesWithAnnotationValueZ(d.trove, "dict", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbav dict 3 == null");

    node = huFindNodesWithAnnotationValueZ(d.trove, "foo", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbav foo 0 == null");
}

TEST(huFindNodesWithAnnotationValue, pathological)
{
    huNode const * node = huFindNodesWithAnnotationValueZ(NULL, "type", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "NULL fnbav type == 0");

    node = huFindNodesWithAnnotationValueZ(hu_nullTrove, "type", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "null fnbav foo == 0");

    node = huFindNodesWithAnnotationValueZ(l.trove, NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav NULL == 0");

    node = huFindNodesWithAnnotationValueZ(l.trove, "", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav '' == 0");

    node = huFindNodesWithAnnotationValueZ(d.trove, NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav NULL == 0");

    node = huFindNodesWithAnnotationValueZ(d.trove, "", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbav '' == 0");
}


TEST_GROUP(huFindNodesWithAnnotationKeyValue)
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

TEST(huFindNodesWithAnnotationKeyValue, normal)
{
    huNode const * node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "a", "a", NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbakv a 0 == a");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "a", "a", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv a 1 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "b", "bp", NULL);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbakv b 0 == bp");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "b", "bp", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv b 1 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "c", "c", NULL);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbakv c 0 == c");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "c", "c", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv c 1 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "value", NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbakv value 0 == a");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "value", node);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbakv value 1 == b");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "value", node);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbakv value 2 == c");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "value", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv value 3 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "list", NULL);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbakv list 0 == bp");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "list", node);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbakv list 1 == cpp");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "list", node);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbakv list 2 == cp");
    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "type", "list", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv list 3 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "foo", "foo", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv foo 0 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "foo", "bar", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv foo 0 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "a", "foo", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv foo 0 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "a", "a", NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbakv a 0 == a");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "a", "a", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbakv a 1 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "b", "bp", NULL);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbakv b 0 == bp");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "b", "bp", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbakv b 1 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "c", "c", NULL);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbakv c 0 == c");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "c", "c", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbakv c 1 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "value", NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbakv value 0 == a");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "value", node);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbakv value 1 == b");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "value", node);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbakv value 2 == c");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "value", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbakv value 3 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "dict", NULL);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbakv dict 0 == bp");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "dict", node);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbakv dict 1 == cpp");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "dict", node);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbakv dict 2 == cp");
    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "type", "dict", node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbakv dict 3 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "foo", "bar", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbakv foo 0 == null");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "a", "foo", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbakv foo 0 == null");
}

TEST(huFindNodesWithAnnotationKeyValue, pathological)
{
    huNode const * node = huFindNodesWithAnnotationKeyValueZZ(NULL, "type", "value", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "NULL fnbakv type == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(hu_nullTrove, "type", "value", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "null fnbakv foo == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, NULL, "a", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv NULL == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "a", NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv NULL == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "", "a", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv '' == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(l.trove, "a", "", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv '' == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, NULL, "a", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv NULL == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "a", NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv NULL == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "", "a", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv '' == 0");

    node = huFindNodesWithAnnotationKeyValueZZ(d.trove, "a", "", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbakv '' == 0");
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
    auto exp = "This is a "sv;
    huNode const * node = NULL;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbcc This is a 0 == a");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbcc This is a 1 == bp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbcc This is a 2 == cpp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbcc This is a 3 == null");

    exp = "aaaa"sv;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbcc aaaa 0 == a");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbcc aaaa 1 == null");

    exp = "cp"sv;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbcc cp 0 == cpp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbcc cp 1 == cp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbcc cp 2 == null");

    exp = ""sv;
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(l.a, node, "l fnbcc '' 0 == a");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.bp, node, "l fnbcc '' 1 == bp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.b, node, "l fnbcc '' 2 == b");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.cpp, node, "l fnbcc '' 3 == cpp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.cp, node, "l fnbcc '' 4 == cp");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(l.c, node, "l fnbcc '' 5 == c");
    node = huFindNodesByCommentContainingN(l.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbcc '' 6 == null");

    exp = "This is a "sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbcc This is a 0 == a");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbcc This is a 1 == bp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbcc This is a 2 == cpp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbcc This is a 3 == null");

    exp = "aaaa"sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbcc aaaa 0 == a");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbcc aaaa 1 == null");

    exp = "cp"sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbcc cp 1 == cpp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbcc cp 0 == cp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbcc cp 2 == null");

    exp = ""sv;
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), NULL);
    POINTERS_EQUAL_TEXT(d.a, node, "d fnbcc '' 0 == a");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.bp, node, "d fnbcc '' 1 == bp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.b, node, "d fnbcc '' 2 == b");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.cpp, node, "d fnbcc '' 3 == cpp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.cp, node, "d fnbcc '' 4 == cp");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(d.c, node, "d fnbcc '' 5 == c");
    node = huFindNodesByCommentContainingN(d.trove, exp.data(), exp.size(), node);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbcc '' 6 == null");
}

TEST(huFindNodesByCommentContaining, pathological)
{
    huNode const * node = huFindNodesByCommentContainingZ(NULL, "This", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "NULL fnbcc This == 0");

    node = huFindNodesByCommentContainingZ(hu_nullTrove, "This", NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "null fnbcc This == 0");

    node = huFindNodesByCommentContainingZ(l.trove, NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "l fnbcc NULL == 0");

    node = huFindNodesByCommentContainingZ(d.trove, NULL, NULL);
    POINTERS_EQUAL_TEXT(hu_nullNode, node, "d fnbcc NULL == 0");
}


TEST_GROUP(huTroveToString)
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

    std::string makeFileName(std::string_view path, int outputFormat, bool useColors, bool printComments, bool printBom)
    {
        std::string format = ".pp";
        if (outputFormat == HU_OUTPUTFORMAT_XERO)
            { format = ".px"; }
        else if (outputFormat == HU_OUTPUTFORMAT_MINIMAL)
            { format = ".pm"; }

        std::string consPath = std::string {path} + 
            format +
            (printComments ? ".my" : ".mn") +
            (useColors ? ".ca" : ".cn") +
            (printBom ? ".by" : ".bn") + 
            ".hu";

        return consPath;        
    }

    std::string getFile(std::string_view path, int outputFormat, bool useColors, bool printComments, bool printBom)
    {
        std::string consPath = makeFileName(path, outputFormat, useColors, printComments, printBom);
        std::string str;

        FILE * fp = fopen(consPath.data(), "rb");
        fseek(fp, 0L, SEEK_END);
        str.resize(ftell(fp));
        rewind(fp);
        if (fread((void*)str.c_str(), 1, str.size(), fp) != str.size())
            { str = "<could not read test file>"; }
        fclose(fp);

        return str;
    }

    std::string troveToString(std::string_view srcFile, int format, bool useColors, bool printComments, bool printBom)
    {
        huTrove const * tc = nullptr;
        int error = huMakeTroveFromFileN(& tc, srcFile.data(), srcFile.size(), NULL);
        if (error != HU_ERROR_NOERROR)
            { return "<could not make trove>"; }

        int toStrLen = 0;
        huStoreParams storeParams;
        huStringView colors[HU_COLORCODE_NUMCOLORKINDS];
        if (useColors)
            { huFillAnsiColorTable(colors); }
        huInitStoreParamsZ(& storeParams, format, 4, useColors, colors, printComments, "\n", printBom);

        error = huTroveToString(tc, NULL, & toStrLen, & storeParams);
        if (error != HU_ERROR_NOERROR)
            { return "<could not troveToString for length>"; }

        std::string str;
        str.resize(toStrLen);
        error = huTroveToString(tc, str.data(), & toStrLen, & storeParams);
        if (error != HU_ERROR_NOERROR)
            { return "<could not troveToString for content>"; }

        huDestroyTrove(tc);

        return str;
    }
};

std::string testFiles[] = {
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


TEST(huTroveToString, correctness)
{
    for (auto testFile : testFiles)
    {
        for (int outputFormat = 0; outputFormat < 3; ++outputFormat)
        {
            for (bool useColors = false; ! useColors; useColors = !useColors)
            {
                for (bool printComments = false; ! printComments; printComments = !printComments)
                {
                    for (bool printBom = false; ! printBom; printBom = !printBom)
                    {
                        //if (testFile == testFiles[7])
                        //    { bool debugBreak = true; }

                        auto file = getFile(testFile, 
                            outputFormat, useColors, printComments, printBom);
                        auto ttos = troveToString(testFile,
                            outputFormat, useColors, printComments, printBom);
                        std::string consPath = makeFileName(testFile, outputFormat, useColors, printComments, printBom);
                        LONGS_EQUAL_TEXT(file.size(), ttos.size(), consPath.data());
                        MEMCMP_EQUAL_TEXT(file.data(), ttos.data(), file.size(), consPath.data());
                    }
                }
            }
        }
    }

}

// Just doing patho tests -- this is heavily function-tested elsewhere.
TEST(huTroveToString, pathological)
{
    int error = HU_ERROR_NOERROR;
    int strLen = 1024;
    huStoreParams params;
    huInitStoreParamsZ(& params, HU_OUTPUTFORMAT_XERO, 4, false, NULL, true, "\n", false);

    error = huTroveToString(NULL, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "NULL->str sz == 0");

    strLen = 1024;
    error = huTroveToString(hu_nullTrove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    error = huTroveToString(l.trove, NULL, NULL, & params);

    strLen = 1024;
    huInitStoreParamsZ(& params, 3, 4, false, NULL, true, "\n", false);
    error = huTroveToString(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitStoreParamsZ(& params, -1, 4, false, NULL, true, "\n", false);
    error = huTroveToString(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitStoreParamsZ(& params, HU_OUTPUTFORMAT_XERO, -1, false, NULL, true, "\n", false);
    error = huTroveToString(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitStoreParamsN(& params, HU_OUTPUTFORMAT_XERO, -1, false, NULL, true, NULL, 1, false);
    error = huTroveToString(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitStoreParamsN(& params, HU_OUTPUTFORMAT_XERO, -1, false, NULL, true, "\n", 0, false);
    error = huTroveToString(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");

    strLen = 1024;
    huInitStoreParamsN(& params, HU_OUTPUTFORMAT_XERO, -1, false, NULL, true, "\n", -1, false);
    error = huTroveToString(l.trove, NULL, & strLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->str sz == 0");
}


TEST_GROUP(huTroveToFile)
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

// Just doing patho tests -- this is heavily function-tested elsewhere.
TEST(huTroveToFile, pathological)
{
    auto validFile = "src/test/testFiles/testout.hu";

    // Remove this file if it exists.
    int acc = access(validFile, F_OK);
    if (acc != -1)
        { remove(validFile); }

    huStoreParams params;
    huInitStoreParamsZ(& params, HU_OUTPUTFORMAT_XERO, 4, false, NULL, false, "\n", false);

    int error = HU_ERROR_NOERROR;
    int fileLen = 0;

    error = huTroveToFileZ(NULL, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "NULL->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    error = huTroveToFileZ(hu_nullTrove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huTroveToFileZ(hu_nullTrove, NULL, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huTroveToFileZ(hu_nullTrove, "", & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huTroveToFileZ(hu_nullTrove, "..", & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    error = huTroveToFileZ(hu_nullTrove, "/", & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");

    huInitStoreParamsZ(& params, 3, 4, false, NULL, false, "\n", false);
    error = huTroveToFileZ(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitStoreParamsZ(& params, -1, 4, false, NULL, false, "\n", false);
    error = huTroveToFileZ(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitStoreParamsZ(& params, HU_OUTPUTFORMAT_XERO, -1, false, NULL, false, "\n", false);
    error = huTroveToFileZ(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitStoreParamsN(& params, HU_OUTPUTFORMAT_XERO, 4, false, NULL, false, NULL, 1, false);
    error = huTroveToFileZ(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitStoreParamsN(& params, HU_OUTPUTFORMAT_XERO, 4, true, NULL, false, "\n", 1, false);
    error = huTroveToFileZ(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitStoreParamsN(& params, HU_OUTPUTFORMAT_PRETTY, 4, false, NULL, false, "\n", 0, false);
    error = huTroveToFileZ(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }

    huInitStoreParamsN(& params, HU_OUTPUTFORMAT_XERO, 4, false, NULL, false, "\n", -1, false);
    error = huTroveToFileZ(l.trove, validFile, & fileLen, & params);
    LONGS_EQUAL_TEXT(HU_ERROR_BADPARAMETER, error, "null->file sz == 0");
    acc = access(validFile, F_OK);
    LONGS_EQUAL_TEXT(-1, acc, "file does not exist");
    if (acc != -1)
        { remove(validFile); }
}

