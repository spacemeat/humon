#include <string.h>
#include <string_view>
#include "humon.h"
#include "ansiColors.h"
#include <CppUTest/TestHarness.h>

#include "testData.h"

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
    CHECK_TEXT(pn != NULL, "root's parent is not NULL");
    POINTERS_EQUAL_TEXT(HU_NODEKIND_NULL, pn->kind, "kind of root's parent");
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
    CHECK_TEXT(pn != NULL, "root's parent is not NULL");
    POINTERS_EQUAL_TEXT(HU_NODEKIND_NULL, pn->kind, "kind of root's parent");
}

TEST(huGetParentNode, pathological)
{
    auto pn = huGetParentNode(NULL);
    CHECK_TEXT(pn != NULL, "NULL's parent is not NULL");
    POINTERS_EQUAL_TEXT(HU_NODEKIND_NULL, pn->kind, "kind of NULL's parent");

    pn = huGetParentNode(& humon_nullNode);
    CHECK_TEXT(pn != NULL, "nullNode's parent is not NULL");
    POINTERS_EQUAL_TEXT(HU_NODEKIND_NULL, pn->kind, "kind nullNode's parent");
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
    LONGS_EQUAL_TEXT(0, huGetNumChildren(& humon_nullNode), "nullNode.nch == 0");
}


TEST_GROUP(huGetChildNodeByIndex)
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

TEST(huGetChildNodeByIndex, lists)
{
    POINTERS_EQUAL_TEXT(l.a, huGetChildNodeByIndex(l.root, 0), "root.ch0 == a");
    POINTERS_EQUAL_TEXT(l.bp, huGetChildNodeByIndex(l.root, 1), "root.ch1 == bp");
    POINTERS_EQUAL_TEXT(l.b, huGetChildNodeByIndex(l.bp, 0), "bp.ch0 == b");
    POINTERS_EQUAL_TEXT(l.cpp, huGetChildNodeByIndex(l.root, 2), "root.ch1 == cpp");
    POINTERS_EQUAL_TEXT(l.cp, huGetChildNodeByIndex(l.cpp, 0), "cpp.ch0 == cp");
    POINTERS_EQUAL_TEXT(l.c, huGetChildNodeByIndex(l.cp, 0), "cp.ch0 == c");
}

TEST(huGetChildNodeByIndex, dicts)
{
    POINTERS_EQUAL_TEXT(d.a, huGetChildNodeByIndex(d.root, 0), "root.ch0 == a");
    POINTERS_EQUAL_TEXT(d.bp, huGetChildNodeByIndex(d.root, 1), "root.ch1 == bp");
    POINTERS_EQUAL_TEXT(d.b, huGetChildNodeByIndex(d.bp, 0), "bp.ch0 == b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetChildNodeByIndex(d.root, 2), "root.ch1 == cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetChildNodeByIndex(d.cpp, 0), "cpp.ch0 == cp");
    POINTERS_EQUAL_TEXT(d.c, huGetChildNodeByIndex(d.cp, 0), "cp.ch0 == c");
}

TEST(huGetChildNodeByIndex, pathological)
{
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByIndex(d.root, -1), "root.ch-1 == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByIndex(d.root, 3), "root.ch3 == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByIndex(NULL, 0), "NULL.ch0 == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByIndex(& humon_nullNode, 0), "null.ch0 == null");
}


TEST_GROUP(huGetChildNodeByKey)
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

TEST(huGetChildNodeByKey, dicts)
{
    POINTERS_EQUAL_TEXT(d.a, huGetChildNodeByKeyZ(d.root, "ak"), "root.chak == a");
    POINTERS_EQUAL_TEXT(d.bp, huGetChildNodeByKeyZ(d.root, "bk"), "root.chbk == bp");
    POINTERS_EQUAL_TEXT(d.b, huGetChildNodeByKeyZ(d.bp, "bk"), "bp.chbk == b");
    POINTERS_EQUAL_TEXT(d.cpp, huGetChildNodeByKeyZ(d.root, "ck"), "root.chck == cpp");
    POINTERS_EQUAL_TEXT(d.cp, huGetChildNodeByKeyZ(d.cpp, "ck"), "cpp.chck == cp");
    POINTERS_EQUAL_TEXT(d.c, huGetChildNodeByKeyZ(d.cp, "ck"), "cp.chck == c");
}

TEST(huGetChildNodeByKey, pathological)
{
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByKeyZ(d.root, "foo"), "root.chfoo == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByKeyZ(l.root, "foo"), "listroot.chfoo == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByKeyZ(NULL, "foo"), "NULL.chfoo == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByKeyZ(& humon_nullNode, "foo"), "null.chfoo == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByKeyZ(d.root, ""), "root.ch'' == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huGetChildNodeByKeyZ(l.root, ""), "listroot.ch'' == null");
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
    CHECK_EQUAL_TEXT(false, huHasKey(& humon_nullNode), "null.huHasKey == false");
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
    CHECK_EQUAL_TEXT(false, huHasValue(& humon_nullNode), "null.true == false");
}


TEST_GROUP(huNextSibling)
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

TEST(huNextSibling, lists)
{
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(l.root), "root.ns == null");
    POINTERS_EQUAL_TEXT(l.bp, huNextSibling(l.a), "a.ns == bp");
    POINTERS_EQUAL_TEXT(l.cpp, huNextSibling(l.bp), "bp.ns == cpp");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(l.cpp), "cpp.ns == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(l.cp), "cp.ns == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(l.c), "c.ns == null");
}

TEST(huNextSibling, dicts)
{
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(d.root), "root.ns == null");
    POINTERS_EQUAL_TEXT(d.bp, huNextSibling(d.a), "a.ns == bp");
    POINTERS_EQUAL_TEXT(d.cpp, huNextSibling(d.bp), "bp.ns == cpp");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(d.cpp), "cpp.ns == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(d.cp), "cp.ns == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(d.c), "c.ns == null");
}

TEST(huNextSibling, pathological)
{
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(NULL), "NULL.ns == null");
    POINTERS_EQUAL_TEXT(& humon_nullNode, huNextSibling(& humon_nullNode), "null.ns == null");
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
    LONGS_EQUAL_TEXT(0, huGetNumAnnotations(& humon_nullNode), "null.na == null");
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
    STRNCMP_EQUAL_TEXT("name", anno->key->value.str, anno->key->value.size, "a.anno 0 k=name");
    STRNCMP_EQUAL_TEXT("root", anno->value->value.str, anno->value->value.size, "a.anno 0 v=root");
    anno = huGetAnnotation(l.root, 1);
    CHECK_TEXT(anno != NULL, "root.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("otherName", anno->key->value.str, anno->key->value.size, "a.anno 1 k=otherName");
    STRNCMP_EQUAL_TEXT("root", anno->value->value.str, anno->value->value.size, "a.anno 1 k=root");

    anno = huGetAnnotation(l.a, 0);
    CHECK_TEXT(anno != NULL, "a.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("a", anno->key->value.str, anno->key->value.size, "a.anno 0 k=a");
    STRNCMP_EQUAL_TEXT("a", anno->value->value.str, anno->value->value.size, "a.anno 0 v=a");
    anno = huGetAnnotation(l.a, 1);
    CHECK_TEXT(anno != NULL, "a.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "a.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->value.str, anno->value->value.size, "a.anno 1 k=value");

    anno = huGetAnnotation(l.bp, 0);
    CHECK_TEXT(anno != NULL, "bp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->value.str, anno->key->value.size, "bp.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("bp", anno->value->value.str, anno->value->value.size, "bp.anno 0 v=bp");
    anno = huGetAnnotation(l.bp, 1);
    CHECK_TEXT(anno != NULL, "bp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "bp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("list", anno->value->value.str, anno->value->value.size, "bp.anno 1 k=list");

    anno = huGetAnnotation(l.b, 0);
    CHECK_TEXT(anno != NULL, "b.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->value.str, anno->key->value.size, "b.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("b", anno->value->value.str, anno->value->value.size, "b.anno 0 v=b");
    anno = huGetAnnotation(l.b, 1);
    CHECK_TEXT(anno != NULL, "b.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "b.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->value.str, anno->value->value.size, "b.anno 1 k=value");

    anno = huGetAnnotation(l.cpp, 0);
    CHECK_TEXT(anno != NULL, "cpp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->value.str, anno->key->value.size, "cpp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cpp", anno->value->value.str, anno->value->value.size, "cpp.anno 0 v=cpp");
    anno = huGetAnnotation(l.cpp, 1);
    CHECK_TEXT(anno != NULL, "cpp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "cpp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("list", anno->value->value.str, anno->value->value.size, "cpp.anno 1 k=list");

    anno = huGetAnnotation(l.cp, 0);
    CHECK_TEXT(anno != NULL, "cp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->value.str, anno->key->value.size, "cp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cp", anno->value->value.str, anno->value->value.size, "cp.anno 0 v=cp");
    anno = huGetAnnotation(l.cp, 1);
    CHECK_TEXT(anno != NULL, "cp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "cp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("list", anno->value->value.str, anno->value->value.size, "cp.anno 1 k=list");

    anno = huGetAnnotation(l.c, 0);
    CHECK_TEXT(anno != NULL, "c.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->value.str, anno->key->value.size, "c.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("c", anno->value->value.str, anno->value->value.size, "c.anno 0 v=c");
    anno = huGetAnnotation(l.c, 1);
    CHECK_TEXT(anno != NULL, "c.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "c.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->value.str, anno->value->value.size, "c.anno 1 k=value");
}

TEST(huGetAnnotation, dicts)
{
    huAnnotation const * anno = huGetAnnotation(d.root, 0);
    CHECK_TEXT(anno != NULL, "root.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("name", anno->key->value.str, anno->key->value.size, "a.anno 0 k=name");
    STRNCMP_EQUAL_TEXT("root", anno->value->value.str, anno->value->value.size, "a.anno 0 v=root");
    anno = huGetAnnotation(d.root, 1);
    CHECK_TEXT(anno != NULL, "root.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("otherName", anno->key->value.str, anno->key->value.size, "a.anno 1 k=otherName");
    STRNCMP_EQUAL_TEXT("root", anno->value->value.str, anno->value->value.size, "a.anno 1 k=root");

    anno = huGetAnnotation(d.a, 0);
    CHECK_TEXT(anno != NULL, "a.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("a", anno->key->value.str, anno->key->value.size, "a.anno 0 k=a");
    STRNCMP_EQUAL_TEXT("a", anno->value->value.str, anno->value->value.size, "a.anno 0 v=a");
    anno = huGetAnnotation(d.a, 1);
    CHECK_TEXT(anno != NULL, "a.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "a.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->value.str, anno->value->value.size, "a.anno 1 k=value");

    anno = huGetAnnotation(d.bp, 0);
    CHECK_TEXT(anno != NULL, "bp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->value.str, anno->key->value.size, "bp.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("bp", anno->value->value.str, anno->value->value.size, "bp.anno 0 v=bp");
    anno = huGetAnnotation(d.bp, 1);
    CHECK_TEXT(anno != NULL, "bp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "bp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", anno->value->value.str, anno->value->value.size, "bp.anno 1 k=dict");

    anno = huGetAnnotation(d.b, 0);
    CHECK_TEXT(anno != NULL, "b.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("b", anno->key->value.str, anno->key->value.size, "b.anno 0 k=b");
    STRNCMP_EQUAL_TEXT("b", anno->value->value.str, anno->value->value.size, "b.anno 0 v=b");
    anno = huGetAnnotation(d.b, 1);
    CHECK_TEXT(anno != NULL, "b.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "b.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->value.str, anno->value->value.size, "b.anno 1 k=value");

    anno = huGetAnnotation(d.cpp, 0);
    CHECK_TEXT(anno != NULL, "cpp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->value.str, anno->key->value.size, "cpp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cpp", anno->value->value.str, anno->value->value.size, "cpp.anno 0 v=cpp");
    anno = huGetAnnotation(d.cpp, 1);
    CHECK_TEXT(anno != NULL, "cpp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "cpp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", anno->value->value.str, anno->value->value.size, "cpp.anno 1 k=dict");

    anno = huGetAnnotation(d.cp, 0);
    CHECK_TEXT(anno != NULL, "cp.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->value.str, anno->key->value.size, "cp.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("cp", anno->value->value.str, anno->value->value.size, "cp.anno 0 v=cp");
    anno = huGetAnnotation(d.cp, 1);
    CHECK_TEXT(anno != NULL, "cp.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "cp.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("dict", anno->value->value.str, anno->value->value.size, "cp.anno 1 k=dict");

    anno = huGetAnnotation(d.c, 0);
    CHECK_TEXT(anno != NULL, "c.anno 0 != NULL");
    STRNCMP_EQUAL_TEXT("c", anno->key->value.str, anno->key->value.size, "c.anno 0 k=c");
    STRNCMP_EQUAL_TEXT("c", anno->value->value.str, anno->value->value.size, "c.anno 0 v=c");
    anno = huGetAnnotation(d.c, 1);
    CHECK_TEXT(anno != NULL, "c.anno 1 != NULL");
    STRNCMP_EQUAL_TEXT("type", anno->key->value.str, anno->key->value.size, "c.anno 1 k=type");
    STRNCMP_EQUAL_TEXT("value", anno->value->value.str, anno->value->value.size, "c.anno 1 k=value");
}

TEST(huGetAnnotation, pathological)
{
    POINTERS_EQUAL_TEXT(NULL, huGetAnnotation(NULL, 0), "NULL.anno 0 == NULL");
    POINTERS_EQUAL_TEXT(NULL, huGetAnnotation(& humon_nullNode, 0), "null.anno 0 == NULL");
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
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(& humon_nullNode, ""), "null.hawk '' == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(& humon_nullNode, "foo"), "null.hawk foo == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.root, ""), "root.hawk '' == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(l.root, NULL), "root.hawk NULL == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.root, ""), "root.hawk '' == false");
    CHECK_EQUAL_TEXT(false, huHasAnnotationWithKeyZ(d.root, NULL), "root.hawk NULL == false");
}


TEST_GROUP(huGetAnnotationByKey)
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

TEST(huGetAnnotationByKey, lists)
{
    auto anno = huGetAnnotationByKeyZ(l.root, "name")->value;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno name size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.str, anno.size, "root.anno name == root");
    anno = huGetAnnotationByKeyZ(l.root, "otherName")->value;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno otherName size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.str, anno.size, "root.anno otherName == root");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.root, "foo"), "root.hawk foo == false");

    anno = huGetAnnotationByKeyZ(l.a, "a")->value;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.str, anno.size, "a.anno a == a");
    anno = huGetAnnotationByKeyZ(l.a, "type")->value;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "a.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.str, anno.size, "a.anno type == value");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.a, "foo"), "a.anno foo == null");

    anno = huGetAnnotationByKeyZ(l.bp, "b")->value;
    LONGS_EQUAL_TEXT(strlen("bp"), anno.size, "bp.anno b size = sz bp");
    STRNCMP_EQUAL_TEXT("bp", anno.str, anno.size, "bp.anno a == bp");
    anno = huGetAnnotationByKeyZ(l.bp, "type")->value;
    LONGS_EQUAL_TEXT(strlen("list"), anno.size, "bp.anno type size = sz list");
    STRNCMP_EQUAL_TEXT("list", anno.str, anno.size, "bp.anno type == list");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.bp, "foo"), "bp.anno foo == null");

    anno = huGetAnnotationByKeyZ(l.b, "b")->value;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.str, anno.size, "b.anno b == b");
    anno = huGetAnnotationByKeyZ(l.b, "type")->value;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "b.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.str, anno.size, "b.anno type == value");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.b, "foo"), "b.anno foo == null");

    anno = huGetAnnotationByKeyZ(l.cpp, "c")->value;
    LONGS_EQUAL_TEXT(strlen("cpp"), anno.size, "cpp.anno b size = sz cpp");
    STRNCMP_EQUAL_TEXT("cpp", anno.str, anno.size, "cpp.anno b == cpp");
    anno = huGetAnnotationByKeyZ(l.cpp, "type")->value;
    LONGS_EQUAL_TEXT(strlen("list"), anno.size, "cpp.anno type size = sz list");
    STRNCMP_EQUAL_TEXT("list", anno.str, anno.size, "cpp.anno type == list");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.cpp, "foo"), "cpp.anno foo == null");

    anno = huGetAnnotationByKeyZ(l.cp, "c")->value;
    LONGS_EQUAL_TEXT(strlen("cp"), anno.size, "cp.anno c size = sz cp");
    STRNCMP_EQUAL_TEXT("cp", anno.str, anno.size, "cp.anno c == cp");
    anno = huGetAnnotationByKeyZ(l.cp, "type")->value;
    LONGS_EQUAL_TEXT(strlen("list"), anno.size, "cp.anno type size = sz list");
    STRNCMP_EQUAL_TEXT("list", anno.str, anno.size, "cp.anno type == list");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.cp, "foo"), "cp.anno foo == null");

    anno = huGetAnnotationByKeyZ(l.c, "c")->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "c.anno c == c");
    anno = huGetAnnotationByKeyZ(l.c, "type")->value;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "c.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.str, anno.size, "c.anno type == value");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.c, "foo"), "c.anno foo == null");
}

TEST(huGetAnnotationByKey, dicts)
{
    auto anno = huGetAnnotationByKeyZ(d.root, "name")->value;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno name size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.str, anno.size, "root.anno name == root");
    anno = huGetAnnotationByKeyZ(d.root, "otherName")->value;
    LONGS_EQUAL_TEXT(strlen("root"), anno.size, "root.anno otherName size = sz root");
    STRNCMP_EQUAL_TEXT("root", anno.str, anno.size, "root.anno otherName == root");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(d.root, "foo"), "root.hawk foo == false");

    anno = huGetAnnotationByKeyZ(d.a, "a")->value;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.str, anno.size, "a.anno a == a");
    anno = huGetAnnotationByKeyZ(d.a, "type")->value;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "a.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.str, anno.size, "a.anno type == value");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(d.a, "foo"), "a.anno foo == null");

    anno = huGetAnnotationByKeyZ(d.bp, "b")->value;
    LONGS_EQUAL_TEXT(strlen("bp"), anno.size, "bp.anno b size = sz bp");
    STRNCMP_EQUAL_TEXT("bp", anno.str, anno.size, "bp.anno b == bp");
    anno = huGetAnnotationByKeyZ(d.bp, "type")->value;
    LONGS_EQUAL_TEXT(strlen("dict"), anno.size, "bp.anno type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", anno.str, anno.size, "bp.anno type == dict");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(d.bp, "foo"), "bp.anno foo == null");

    anno = huGetAnnotationByKeyZ(d.b, "b")->value;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.str, anno.size, "b.anno b == b");
    anno = huGetAnnotationByKeyZ(d.b, "type")->value;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "b.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.str, anno.size, "b.anno type == value");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(d.b, "foo"), "b.anno foo == null");

    anno = huGetAnnotationByKeyZ(d.cpp, "c")->value;
    LONGS_EQUAL_TEXT(strlen("cpp"), anno.size, "cpp.anno b size = sz cpp");
    STRNCMP_EQUAL_TEXT("cpp", anno.str, anno.size, "cpp.anno b == cpp");
    anno = huGetAnnotationByKeyZ(d.cpp, "type")->value;
    LONGS_EQUAL_TEXT(strlen("dict"), anno.size, "cpp.anno type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", anno.str, anno.size, "cpp.anno type == dict");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(d.cpp, "foo"), "cpp.anno foo == null");

    anno = huGetAnnotationByKeyZ(d.cp, "c")->value;
    LONGS_EQUAL_TEXT(strlen("cp"), anno.size, "cp.anno c size = sz cp");
    STRNCMP_EQUAL_TEXT("cp", anno.str, anno.size, "cp.anno c == cp");
    anno = huGetAnnotationByKeyZ(d.cp, "type")->value;
    LONGS_EQUAL_TEXT(strlen("dict"), anno.size, "cp.anno type size = sz dict");
    STRNCMP_EQUAL_TEXT("dict", anno.str, anno.size, "cp.anno type == dict");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(d.cp, "foo"), "cp.anno foo == null");

    anno = huGetAnnotationByKeyZ(d.c, "c")->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "c.anno c == c");
    anno = huGetAnnotationByKeyZ(d.c, "type")->value;
    LONGS_EQUAL_TEXT(strlen("value"), anno.size, "c.anno type size = sz value");
    STRNCMP_EQUAL_TEXT("value", anno.str, anno.size, "c.anno type == value");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(d.c, "foo"), "c.anno foo == null");
}

TEST(huGetAnnotationByKey, pathological)
{
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(NULL, ""), "NULL.anno '' == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(NULL, "foo"), "NULL.anno foo == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(& humon_nullNode, ""), "null.anno '' == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(& humon_nullNode, "foo"), "null.anno foo == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.root, ""), "root.anno '' == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByKeyZ(l.root, NULL), "root.anno NULL == null");
}


TEST_GROUP(huGetNumAnnotationsByValue)
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

TEST(huGetNumAnnotationsByValue, lists)
{
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsByValueZ(l.root, "foo"), "root.gnabv foo == 0");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotationsByValueZ(l.root, "root"), "root.gnabv root == 2");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.a, "a"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.a, "value"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.bp, "bp"), "bp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.bp, "list"), "bp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.b, "b"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.b, "value"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.cpp, "cpp"), "cpp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.cpp, "list"), "cpp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.cp, "cp"), "cp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.cp, "list"), "cp.gnabv list == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.c, "c"), "c.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(l.c, "value"), "c.gnabv value == 1");
}

TEST(huGetNumAnnotationsByValue, dicts)
{
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsByValueZ(d.root, "foo"), "root.gnabv foo == 0");
    LONGS_EQUAL_TEXT(2, huGetNumAnnotationsByValueZ(d.root, "root"), "root.gnabv root == 2");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.a, "a"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.a, "value"), "a.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.bp, "bp"), "bp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.bp, "dict"), "bp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.b, "b"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.b, "value"), "b.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.cpp, "cpp"), "cpp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.cpp, "dict"), "cpp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.cp, "cp"), "cp.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.cp, "dict"), "cp.gnabv dict == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.c, "c"), "c.gnabv value == 1");
    LONGS_EQUAL_TEXT(1, huGetNumAnnotationsByValueZ(d.c, "value"), "c.gnabv value == 1");
}

TEST(huGetNumAnnotationsByValue, pathological)
{
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsByValueZ(NULL, "foo"), "NULL.gnabv foo == 0");
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsByValueZ(& humon_nullNode, "foo"), "null.gnabv foo == 0");
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsByValueZ(l.root, NULL), "NULL.gnabv NULL == 0");
    LONGS_EQUAL_TEXT(0, huGetNumAnnotationsByValueZ(l.root, ""), "NULL.gnabv '' == 0");
}


TEST_GROUP(huGetAnnotationByValue)
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

TEST(huGetAnnotationByValue, lists)
{
    auto anno = huGetAnnotationByValueZ(l.root, "root", 0)->value;
    LONGS_EQUAL_TEXT(strlen("name"), anno.size, "root.anno v0 name size = sz root");
    STRNCMP_EQUAL_TEXT("name", anno.str, anno.size, "root.anno v0 name == root");
    anno = huGetAnnotationByValueZ(l.root, "root", 1)->value;
    LONGS_EQUAL_TEXT(strlen("otherName"), anno.size, "root.anno v1 otherName size = sz root");
    STRNCMP_EQUAL_TEXT("otherName", anno.str, anno.size, "root.anno v1 otherName == root");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.root, "foo", 0), "root.hawk foo == false");

    anno = huGetAnnotationByValueZ(l.a, "a", 0)->value;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno v0 a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.str, anno.size, "a.anno v0 a == a");
    anno = huGetAnnotationByValueZ(l.a, "value", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "a.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "a.anno v0 value == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.a, "foo", 0), "a.anno foo == null");

    anno = huGetAnnotationByValueZ(l.bp, "bp", 0)->value;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "bp.anno v0 bp size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.str, anno.size, "bp.anno v0 bp == b");
    anno = huGetAnnotationByValueZ(l.bp, "list", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "bp.anno v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "bp.anno v0 list == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.bp, "foo", 0), "bp.anno foo == null");

    anno = huGetAnnotationByValueZ(l.b, "b", 0)->value;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno v0 b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.str, anno.size, "b.anno v0 b == b");
    anno = huGetAnnotationByValueZ(l.b, "value", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "b.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "b.anno v0 value == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.b, "foo", 0), "b.anno foo == null");

    anno = huGetAnnotationByValueZ(l.cpp, "cpp", 0)->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cpp.anno v0 cpp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "cpp.anno v0 cpp == c");
    anno = huGetAnnotationByValueZ(l.cpp, "list", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cpp.anno v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "cpp.anno v0 list == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.cpp, "foo", 0), "cpp.anno foo == null");

    anno = huGetAnnotationByValueZ(l.cp, "cp", 0)->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cp.anno v0 cp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "cp.anno v0 cp == c");
    anno = huGetAnnotationByValueZ(l.cp, "list", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cp.anno v0 list size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "cp.anno v0 list == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.cp, "foo", 0), "cp.anno foo == null");

    anno = huGetAnnotationByValueZ(l.c, "c", 0)->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno v0 c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "c.anno v0 c == c");
    anno = huGetAnnotationByValueZ(l.c, "value", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "c.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "c.anno v0 value == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.c, "foo", 0), "c.anno foo == null");
}

TEST(huGetAnnotationByValue, dicts)
{
    auto anno = huGetAnnotationByValueZ(d.root, "root", 0)->value;
    LONGS_EQUAL_TEXT(strlen("name"), anno.size, "root.anno v0 name size = sz root");
    STRNCMP_EQUAL_TEXT("name", anno.str, anno.size, "root.anno v0 name == root");
    anno = huGetAnnotationByValueZ(d.root, "root", 1)->value;
    LONGS_EQUAL_TEXT(strlen("otherName"), anno.size, "root.anno v1 otherName size = sz root");
    STRNCMP_EQUAL_TEXT("otherName", anno.str, anno.size, "root.anno v1 otherName == root");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(d.root, "foo", 0), "root.hawk foo == false");

    anno = huGetAnnotationByValueZ(d.a, "a", 0)->value;
    LONGS_EQUAL_TEXT(strlen("a"), anno.size, "a.anno v0 a size = sz a");
    STRNCMP_EQUAL_TEXT("a", anno.str, anno.size, "a.anno v0 a == a");
    anno = huGetAnnotationByValueZ(d.a, "value", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "a.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "a.anno v0 value == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(d.a, "foo", 0), "a.anno foo == null");

    anno = huGetAnnotationByValueZ(d.bp, "bp", 0)->value;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "bp.anno v0 bp size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.str, anno.size, "bp.anno v0 bp == b");
    anno = huGetAnnotationByValueZ(d.bp, "dict", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "bp.anno v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "bp.anno v0 dict == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(d.bp, "foo", 0), "bp.anno foo == null");

    anno = huGetAnnotationByValueZ(d.b, "b", 0)->value;
    LONGS_EQUAL_TEXT(strlen("b"), anno.size, "b.anno v0 b size = sz b");
    STRNCMP_EQUAL_TEXT("b", anno.str, anno.size, "b.anno v0 b == b");
    anno = huGetAnnotationByValueZ(d.b, "value", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "b.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "b.anno v0 value == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(d.b, "foo", 0), "b.anno foo == null");

    anno = huGetAnnotationByValueZ(d.cpp, "cpp", 0)->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cpp.anno v0 cpp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "cpp.anno v0 cpp == c");
    anno = huGetAnnotationByValueZ(d.cpp, "dict", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cpp.anno v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "cpp.anno v0 dict == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(d.cpp, "foo", 0), "cpp.anno foo == null");

    anno = huGetAnnotationByValueZ(d.cp, "cp", 0)->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "cp.anno v0 cp size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "cp.anno v0 cp == c");
    anno = huGetAnnotationByValueZ(d.cp, "dict", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "cp.anno v0 dict size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "cp.anno v0 dict == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(d.cp, "foo", 0), "cp.anno foo == null");

    anno = huGetAnnotationByValueZ(d.c, "c", 0)->value;
    LONGS_EQUAL_TEXT(strlen("c"), anno.size, "c.anno v0 c size = sz c");
    STRNCMP_EQUAL_TEXT("c", anno.str, anno.size, "c.anno v0 c == c");
    anno = huGetAnnotationByValueZ(d.c, "value", 0)->value;
    LONGS_EQUAL_TEXT(strlen("type"), anno.size, "c.anno v0 value size = sz type");
    STRNCMP_EQUAL_TEXT("type", anno.str, anno.size, "c.anno v0 value == type");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(d.c, "foo", 0), "c.anno foo == null");
}

TEST(huGetAnnotationByValue, pathological)
{
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(NULL, "", 0), "NULL.anno v0 '' == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(NULL, "foo", 0), "NULL.anno v0 foo == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(& humon_nullNode, "", 0), "null.anno v0 '' == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(& humon_nullNode, "foo", 0), "null.anno v0 foo == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.root, "", 0), "root.anno v0 '' == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.root, NULL, 0), "root.anno v0 NULL == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.root, "root", -1), "root.anno v-1 root == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetAnnotationByValueZ(l.root, "root", 3), "root.anno v3 root == null");
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
    LONGS_EQUAL_TEXT(0, huGetNumComments(& humon_nullNode), "null.gnc == 0");
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
    auto comm = huGetComment(l.a, 0)->value;
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.comm 0 == exp");
    comm = huGetComment(l.a, 1)->value;
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.comm 1 == exp");

    comm = huGetComment(l.bp, 0)->value;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.comm 0 == exp");
    comm = huGetComment(l.bp, 1)->value;
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.comm 1 == exp");

    comm = huGetComment(l.b, 0)->value;
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "b.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "b.comm 0 == exp");

    comm = huGetComment(l.cpp, 0)->value;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.comm 0 == exp");
    comm = huGetComment(l.cpp, 1)->value;
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.comm 1 == exp");

    comm = huGetComment(l.cp, 0)->value;
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cp.comm 0 == exp");

    comm = huGetComment(l.c, 0)->value;
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "c.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "c.comm 0 == exp");
}

TEST(huGetComment, dicts)
{
    auto comm = huGetComment(d.a, 0)->value;
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.comm 0 == exp");
    comm = huGetComment(d.a, 1)->value;
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.comm 1 == exp");

    comm = huGetComment(d.bp, 0)->value;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.comm 0 == exp");
    comm = huGetComment(d.bp, 1)->value;
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.comm 1 == exp");

    comm = huGetComment(d.b, 0)->value;
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "b.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "b.comm 0 == exp");

    comm = huGetComment(d.cpp, 0)->value;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.comm 0 == exp");
    comm = huGetComment(d.cpp, 1)->value;
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.comm 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.comm 1 == exp");

    comm = huGetComment(d.cp, 0)->value;
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cp.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cp.comm 0 == exp");

    comm = huGetComment(d.c, 0)->value;
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "c.comm 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "c.comm 0 == exp");
}

TEST(huGetComment, pathological)
{
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(NULL, 0), "NULL.comm 0 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(& humon_nullNode, 0), "null.comm 0 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.root, 0), "root.comm 0 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.root, -1), "root.comm -1 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.a, 2), "root.comm 2 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.a, -1), "root.comm -1 == null");
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
    huVector comms = huGetCommentsContainingZ(l.a, "aaa");
    LONGS_EQUAL_TEXT(2, comms.numElements, "a.gcc aaa size = 2");
    auto comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.gcc aaa 0 == exp");
    comm = (*((huToken **) huGetVectorElement(& comms, 1)))->value;
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.gcc aaa 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.gcc aaa 1 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.a, "right here");
    LONGS_EQUAL_TEXT(1, comms.numElements, "a.gcc right_here size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.gcc aaa 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.bp, "bp");
    LONGS_EQUAL_TEXT(2, comms.numElements, "bp.gcc bp size = 2");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.gcc bp 0 == exp");
    comm = (*((huToken **) huGetVectorElement(& comms, 1)))->value;
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.gcc bp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.gcc bp 1 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.bp, "right here");
    LONGS_EQUAL_TEXT(1, comms.numElements, "bp.gcc right_here size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.gcc bp 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.b, "bbb");
    LONGS_EQUAL_TEXT(1, comms.numElements, "b.gcc bbb size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "b.gcc bbb 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "b.gcc bbb 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.cpp, "cpp");
    LONGS_EQUAL_TEXT(2, comms.numElements, "cpp.gcc cpp size = 2");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.gcc cpp 0 == exp");
    comm = (*((huToken **) huGetVectorElement(& comms, 1)))->value;
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.gcc cpp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.gcc cpp 1 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.cpp, "right here");
    LONGS_EQUAL_TEXT(1, comms.numElements, "cpp.gcc right_here size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.gcc cpp 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.cp, "cp");
    LONGS_EQUAL_TEXT(1, comms.numElements, "cp.gcc cp size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cp.gcc cp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cp.gcc cp 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(l.c, "ccc");
    LONGS_EQUAL_TEXT(1, comms.numElements, "c.gcc ccc size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "c.gcc ccc 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cp.gcc ccc 0 == exp");
    huDestroyVector(& comms);
}

TEST(huGetCommentsContaining, dicts)
{
    huVector comms = huGetCommentsContainingZ(d.a, "aaa");
    LONGS_EQUAL_TEXT(2, comms.numElements, "a.gcc aaa size = 2");
    auto comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    auto exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.gcc aaa 0 == exp");
    comm = (*((huToken **) huGetVectorElement(& comms, 1)))->value;
    exp = "// aaaa";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.gcc aaa 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.gcc aaa 1 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.a, "right here");
    LONGS_EQUAL_TEXT(1, comms.numElements, "a.gcc right_here size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a aaaa right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "a.gcc aaa 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "a.gcc aaa 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.bp, "bp");
    LONGS_EQUAL_TEXT(2, comms.numElements, "bp.gcc bp size = 2");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.gcc bp 0 == exp");
    comm = (*((huToken **) huGetVectorElement(& comms, 1)))->value;
    exp = "// bp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.gcc bp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.gcc bp 1 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.bp, "right here");
    LONGS_EQUAL_TEXT(1, comms.numElements, "bp.gcc right_here size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a bp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "bp.gcc bp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "bp.gcc bp 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.b, "bbb");
    LONGS_EQUAL_TEXT(1, comms.numElements, "b.gcc bbb size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// bbbb";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "b.gcc bbb 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "b.gcc bbb 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.cpp, "cpp");
    LONGS_EQUAL_TEXT(2, comms.numElements, "cpp.gcc cpp size = 2");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.gcc cpp 0 == exp");
    comm = (*((huToken **) huGetVectorElement(& comms, 1)))->value;
    exp = "// cpp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.gcc cpp 1 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.gcc cpp 1 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.cpp, "right here");
    LONGS_EQUAL_TEXT(1, comms.numElements, "cpp.gcc right_here size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// This is a cpp right here.";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cpp.gcc cpp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cpp.gcc cpp 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.cp, "cp");
    LONGS_EQUAL_TEXT(1, comms.numElements, "cp.gcc cp size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// cp";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "cp.gcc cp 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cp.gcc cp 0 == exp");
    huDestroyVector(& comms);

    comms = huGetCommentsContainingZ(d.c, "ccc");
    LONGS_EQUAL_TEXT(1, comms.numElements, "c.gcc ccc size = 1");
    comm = (*((huToken **) huGetVectorElement(& comms, 0)))->value;
    exp = "// cccc";
    LONGS_EQUAL_TEXT(strlen(exp), comm.size, "c.gcc ccc 0 size = sz exp");
    STRNCMP_EQUAL_TEXT(exp, comm.str, comm.size, "cp.gcc ccc 0 == exp");
    huDestroyVector(& comms);
}

TEST(huGetCommentsContaining, pathological)
{
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(NULL, 0), "NULL.comm 0 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(& humon_nullNode, 0), "null.comm 0 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.root, 0), "root.comm 0 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.root, -1), "root.comm -1 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.a, 2), "a.comm 2 == null");
    POINTERS_EQUAL_TEXT(& humon_nullToken, huGetComment(l.a, -1), "a.comm -1 == null");
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
    int err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.a, huGetNodeByRelativeAddressZ(l.root, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.root, "0/..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.a, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.b, huGetNodeByRelativeAddressZ(l.bp, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.bp, huGetNodeByRelativeAddressZ(l.bp, "0/..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.bp, "0/../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.cp, huGetNodeByRelativeAddressZ(l.cpp, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.cpp, "0/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.cpp, huGetNodeByRelativeAddressZ(l.cpp, "0/0/../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.cp, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.cpp, huGetNodeByRelativeAddressZ(l.cp, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.cp, huGetNodeByRelativeAddressZ(l.c, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.cpp, huGetNodeByRelativeAddressZ(l.c, "../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.c, "../../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.b, huGetNodeByRelativeAddressZ(l.a, "../1/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.a, "../2/0/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.a, huGetNodeByRelativeAddressZ(l.b, "../../0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.b, "../../2/0/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.a, huGetNodeByRelativeAddressZ(l.c, "../../../0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.b, huGetNodeByRelativeAddressZ(l.c, "../../../1/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.c, huGetNodeByRelativeAddressZ(l.b, " .. / .. / 2 / 0 / 0 ", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);

    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, "..", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.a, "0", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, "3", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, "0/0", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, "1/1", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, "1/0/0", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
}

TEST(huGetNodeByRelativeAddress, dicts)
{
    int err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.root, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.root, "0/..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.a, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.bp, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.bp, huGetNodeByRelativeAddressZ(d.bp, "0/..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.bp, "0/../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.cpp, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cpp, "0/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cpp, "0/0/../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cp, "0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cp, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.c, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.c, "../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.c, "../../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.a, "../1/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.a, "../2/0/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.b, "../../0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, "../../2/0/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.c, "../../../0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.c, "../../../1/0", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, " .. / .. / 2 / 0 / 0 ", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;

    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.root, "ak", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.root, "ak/..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.a, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.bp, "bk", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.bp, huGetNodeByRelativeAddressZ(d.bp, "bk/..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.bp, "bk/../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.cpp, "ck", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cpp, "ck/ck", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cpp, "ck/ck/../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.cp, "ck", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.cp, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cp, huGetNodeByRelativeAddressZ(d.c, "..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.cpp, huGetNodeByRelativeAddressZ(d.c, "../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.c, "../../..", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.a, "../bk/bk", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.a, "../ck/ck/ck", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.b, "../../ak", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, "../../ck/ck/ck", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.a, huGetNodeByRelativeAddressZ(d.c, "../../../ak", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.b, huGetNodeByRelativeAddressZ(d.c, "../../../bk/bk", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, " .. / .. / ck / ck / ck ", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, "../../`ck`/'ck'/\"ck\"", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.c, huGetNodeByRelativeAddressZ(d.b, " .. / .. / `ck` / 'ck' / \"ck\" ", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);

    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(d.root, "..", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(d.a, "0", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(d.root, "3", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(d.root, "0/0", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(d.root, "1/1", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(d.root, "1/0/0", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
}

TEST(huGetNodeByRelativeAddress, pathological)
{
    int err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(NULL, "0", & err));
    LONGS_EQUAL(HU_ERROR_ILLEGAL, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(& humon_nullNode, "0", & err));
    LONGS_EQUAL(HU_ERROR_ILLEGAL, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, "-1", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.a, "-1", & err));
    LONGS_EQUAL(HU_ERROR_NOTFOUND, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, NULL, & err));
    LONGS_EQUAL(HU_ERROR_ILLEGAL, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(l.root, huGetNodeByRelativeAddressZ(l.root, "", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(d.root, huGetNodeByRelativeAddressZ(d.root, "", & err));
    LONGS_EQUAL(HU_ERROR_NO_ERROR, err);
    err = HU_ERROR_NO_ERROR;
    POINTERS_EQUAL(& humon_nullNode, huGetNodeByRelativeAddressZ(l.root, "/", & err));
    LONGS_EQUAL(HU_ERROR_SYNTAX_ERROR, err);
}


TEST_GROUP(huGetNodeAddress)
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

TEST(huGetNodeAddress, lists)
{
    huStringView sv = huGetNodeAddress(l.root);
    auto exp = "/";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(l.a);
    exp = "/0";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(l.bp);
    exp = "/1";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(l.b);
    exp = "/1/0";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(l.cpp);
    exp = "/2";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(l.cp);
    exp = "/2/0";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(l.c);
    exp = "/2/0/0";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);
}


TEST(huGetNodeAddress, dicts)
{
    huStringView sv = huGetNodeAddress(d.root);
    auto exp = "/";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(d.a);
    exp = "/ak";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(d.bp);
    exp = "/bk";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(d.b);
    exp = "/bk/bk";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(d.cpp);
    exp = "/ck";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(d.cp);
    exp = "/ck/ck";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(d.c);
    exp = "/ck/ck/ck";
    LONGS_EQUAL(strlen(exp), sv.size);
    STRNCMP_EQUAL(exp, sv.str, sv.size);
    huDestroyStringView(& sv);
}

TEST(huGetNodeAddress, pathological)
{
    huStringView sv = huGetNodeAddress(NULL);
    POINTERS_EQUAL(NULL, sv.str);
    huDestroyStringView(& sv);

    sv = huGetNodeAddress(& humon_nullNode);
    POINTERS_EQUAL(NULL, sv.str);
    huDestroyStringView(& sv);
}
