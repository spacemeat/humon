#pragma once

#include <string_view>
#include "humon/humon.h"

using namespace std::literals;

struct htd_listOfLists
{
    std::string_view ts = 
R"(// This is a trove comment.
@{tx:ta, ta:ta, tb:tb}
[               @name:root  @otherName:root
    // This is a aaaa right here.
    a           @a:a        @type:value     // aaaa
    // This is a bp right here.
    [
        b       @b:b        @type:value     // bbbb
    ]           @b:bp       @type:list      // bp
    // This is a cpp right here.
    [
        [
            c   @c:c        @type:value     // cccc
        ]       @c:cp       @type:list      // cp
    ]           @c:cpp      @type:list      // cpp
]
// This is also a trove comment.
)"sv;

    int error = HU_ERROR_NOERROR;
    huTrove const * trove = NULL;
    huNode const * root;
    huNode const * a;
    huNode const * bp;
    huNode const * b;
    huNode const * cpp;
    huNode const * cp;
    huNode const * c;

    void setup()
    {
        error = huDeserializeTroveN(& trove, ts.data(), (int)ts.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
        root = huGetRootNode(trove);
        a = huGetChildByIndex(root, 0);
        bp = huGetChildByIndex(root, 1);
        cpp = huGetChildByIndex(root, 2);
        b = huGetChildByIndex(bp, 0);
        cp = huGetChildByIndex(cpp, 0);
        c = huGetChildByIndex(cp, 0);
    }

    void teardown()
    {
        huDestroyTrove(trove);
    }
};

struct htd_dictOfDicts
{
    std::string_view ts = 
R"(// This is a trove comment.
@{tx:ta, ta:ta, tb:tb}
{                   @name:root  @otherName:root
    // This is a aaaa right here.
    ak:a            @a:a        @type:value     // aaaa
    // This is a bp right here.
    bk:{
        bk:b        @b:b        @type:value     // bbbb
    }               @b:bp       @type:dict      // bp
    // This is a cpp right here.
    ck:{
        ck:{
            ck:c    @c:c        @type:value     // cccc
        }           @c:cp       @type:dict      // cp
    }               @c:cpp      @type:dict      // cpp
}
// This is a trove comment.
)"sv;

    int error = HU_ERROR_NOERROR;
    huTrove const * trove = NULL;
    huNode const * root;
    huNode const * a;
    huNode const * bp;
    huNode const * b;
    huNode const * cpp;
    huNode const * cp;
    huNode const * c;

    void setup()
    {
        error = huDeserializeTroveN(& trove, ts.data(), (int) ts.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
        root = huGetRootNode(trove);
        a = huGetChildByIndex(root, 0);
        bp = huGetChildByIndex(root, 1);
        cpp = huGetChildByIndex(root, 2);
        b = huGetChildByIndex(bp, 0);
        cp = huGetChildByIndex(cpp, 0);
        c = huGetChildByIndex(cp, 0);
    }

    void teardown()
    {
        huDestroyTrove(trove);
    }
};


struct htd_erroneous
{
    std::string_view ts = 
R"(// This is a trove comment.
@{tx:ta, ta:ta, tb:tb}
{                   @name:root  @otherName:root
    // This is a aaaa right here.
    ak:a            @a:a        @type:value     // aaaa
    // This is a bp right here.
    bk:{
        bk:b        @b:b        @type:value     // bbbb
    }               @b:bp       @type:dict      // bp
    // This is a cpp right here.
    ck:{
        ck {
            ck:c    @c:c        @type:value     // cccc
        }           @c:cp       @type:dict      // cp
    }               @c:cpp      @type:dict      // cpp
}
// This is also a trove comment.
)"sv;

    int error = HU_ERROR_NOERROR;
    huTrove const * trove = NULL;
    huNode const * root;
    huNode const * a;
    huNode const * bp;
    huNode const * b;
    huNode const * cpp;
    huNode const * cp;
    huNode const * c;

    void setup()
    {
        error = huDeserializeTroveN(& trove, ts.data(), (int) ts.size(), NULL, HU_ERRORRESPONSE_MUM);
        root = huGetRootNode(trove);
        a = huGetChildByIndex(root, 0);
        bp = huGetChildByIndex(root, 1);
        cpp = huGetChildByIndex(root, 2);
        b = huGetChildByIndex(bp, 0);
        cp = huGetChildByIndex(cpp, 0);
        c = huGetChildByIndex(cp, 0);
    }

    void teardown()
    {
        huDestroyTrove(trove);
    }
};


struct htd_withFunkyAddresses
{
    std::string_view ts = 
R"({
    /: a
    a/b: b
    /b: c
    a/: d
    /"foo": e
    /"foo'bar'": f
    /"foo'bar`baz`'": g
    /'foo`bar"baz"`': h
    /`foo"bar'baz'"`: i
    a"foo"/: j
    a"foo'bar'"/: k
    a"foo'bar`baz`'"/: l
    a'foo`bar"baz"`'/: m
    a`foo"bar'baz'"`/: n
    0: o
    01m: p
    /"0123456789012345678901234567890123456789": q
    a"0123456789012345678901234567890123456789"/: r
}
)"sv;

    int error = HU_ERROR_NOERROR;
    huTrove const * trove = NULL;
    huNode const * root;
    huNode const * a;
    huNode const * b;
    huNode const * c;
    huNode const * d;
    huNode const * e;
    huNode const * f;
    huNode const * g;
    huNode const * h;
    huNode const * i;
    huNode const * j;
    huNode const * k;
    huNode const * l;
    huNode const * m;
    huNode const * n;
    huNode const * o;
    huNode const * p;
    huNode const * q;
    huNode const * r;

    void setup()
    {
        error = huDeserializeTroveN(& trove, ts.data(), (int) ts.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
        root = huGetRootNode(trove);
        a = huGetChildByIndex(root, 0);
        b = huGetChildByIndex(root, 1);
        c = huGetChildByIndex(root, 2);
        d = huGetChildByIndex(root, 3);
        e = huGetChildByIndex(root, 4);
        f = huGetChildByIndex(root, 5);
        g = huGetChildByIndex(root, 6);
        h = huGetChildByIndex(root, 7);
        i = huGetChildByIndex(root, 8);
        j = huGetChildByIndex(root, 9);
        k = huGetChildByIndex(root, 10);
        l = huGetChildByIndex(root, 11);
        m = huGetChildByIndex(root, 12);
        n = huGetChildByIndex(root, 13);
        o = huGetChildByIndex(root, 14);
        p = huGetChildByIndex(root, 15);
        q = huGetChildByIndex(root, 16);
        r = huGetChildByIndex(root, 17);
    }

    void teardown()
    {
        huDestroyTrove(trove);
    }
};


struct htd_withSomeStrings
{
    std::string_view ts = 
R"({
    "aaa": bbb
    ccc: "ddd"
    "eee": "fff"
    ggg: hhh
}
)"sv;

    int error = HU_ERROR_NOERROR;
    huTrove const * trove = NULL;
    huNode const * root;
    huNode const * aaa;
    huNode const * ccc;
    huNode const * eee;
    huNode const * ggg;

    void setup()
    {
        error = huDeserializeTroveN(& trove, ts.data(), (int) ts.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
        root = huGetRootNode(trove);
        aaa = huGetChildByIndex(root, 0);
        ccc = huGetChildByIndex(root, 1);
        eee = huGetChildByIndex(root, 2);
        ggg = huGetChildByIndex(root, 3);
    }

    void teardown()
    {
        huDestroyTrove(trove);
    }
};
