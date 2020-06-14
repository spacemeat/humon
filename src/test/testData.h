#pragma once

#include <string_view>
#include "humon.h"

using namespace std::literals;

struct htd_listOfLists
{
    std::string_view ts = 
R"( @{tx:ta, ta:ta, tb:tb}
[               @name:root  @otherName:root
    // This is a aaaa right here.
    a           @a:a        @type:value     // aaaa
    // This is a bbbb right here.
    [
        b       @b:b        @type:value     // bbbb
    ]           @b:bp       @type:list      // bp
    // This is a cccc right here.
    [
        [
            c   @c:c        @type:value     // cccc
        ]       @c:cp       @type:list      // cp
    ]           @c:cpp      @type:list      // cpp
]
// This is a trove comment.
// This is also a trove comment.
)"sv;

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
        trove = huMakeTroveFromStringN(ts.data(), ts.size(), 4);
        root = huGetRootNode(trove);
        a = huGetChildNodeByIndex(root, 0);
        bp = huGetChildNodeByIndex(root, 1);
        cpp = huGetChildNodeByIndex(root, 2);
        b = huGetChildNodeByIndex(bp, 0);
        cp = huGetChildNodeByIndex(cpp, 0);
        c = huGetChildNodeByIndex(cp, 0);
    }

    void teardown()
    {
        huDestroyTrove(trove);
    }
};

struct htd_dictOfDicts
{
    std::string_view ts = 
R"( @{tx:ta, ta:ta, tb:tb}
{                   @name:root  @otherName:root
    // This is a aaaa right here.
    ak:a            @a:a        @type:value     // aaaa
    // This is a bbbb right here.
    bk:{
        bk:b        @b:b        @type:value     // bbbb
    }               @b:bp       @type:dict      // bp
    // This is a cccc right here.
    ck:{
        ck:{
            ck:c    @c:c        @type:value     // cccc
        }           @c:cp       @type:dict      // cp
    }               @c:cpp      @type:dict      // cpp
}
// This is a trove comment.
// This is also a trove comment.
)"sv;

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
        trove = huMakeTroveFromStringN(ts.data(), ts.size(), 4);
        root = huGetRootNode(trove);
        a = huGetChildNodeByIndex(root, 0);
        bp = huGetChildNodeByIndex(root, 1);
        cpp = huGetChildNodeByIndex(root, 2);
        b = huGetChildNodeByIndex(bp, 0);
        cp = huGetChildNodeByIndex(cpp, 0);
        c = huGetChildNodeByIndex(cp, 0);
    }

    void teardown()
    {
        huDestroyTrove(trove);
    }
};


