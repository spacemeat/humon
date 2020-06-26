#pragma once

#include <string_view>
#include "humon.hpp"

using namespace std::literals;


struct htd_cppGeneralMix
{
    std::string_view ts = 
R"( @{tx:ta, ta:ta, tb:tb}
{                   @name:root  @otherName:root
    // This is a aaaa right here.
    ak:a            @a:a        @type:value     // aaaa
    // This is a bp right here.
    bk:[
           b        @b:b        @type:value     // bbbb
    ]               @b:bp       @type:list      // bp
    // This is a cpp right here.
    ck:{
        ck:[
            c       @c:c        @type:value     // cccc
        ]           @c:cp       @type:list      // cp
    }               @c:cpp      @type:dict      // cpp
}
// This is a trove comment.
// This is also a trove comment.
)"sv;

    hu::Trove trove;
    hu::Node root;
    hu::Node a;
    hu::Node bp;
    hu::Node b;
    hu::Node cpp;
    hu::Node cp;
    hu::Node c;

    void setup()
    {
        trove = hu::Trove::fromString(ts);
        root = trove.rootNode();
        a = root.child(0);
        bp = root.child(1);
        cpp = root.child(2);
        b = bp.child(0);
        cp = cpp.child(0);
        c = cp.child(0);
    }

    void teardown()
    {
    }
};


struct htd_cppValues
{
    std::string_view ts = 
R"({
    int:213
    float:25.25
    double:25.25
    string:foo
    bool:true
}
)"sv;

    hu::Trove trove;
    hu::Node root;
    hu::Node nint;
    hu::Node nfloat;
    hu::Node ndouble;
    hu::Node nstring;
    hu::Node nbool;

    void setup()
    {
        trove = hu::Trove::fromString(ts);
        root = trove.rootNode();
        nint = root.child(0);
        nfloat = root.child(1);
        ndouble = root.child(2);
        nstring = root.child(3);
        nbool = root.child(4);
    }

    void teardown()
    {
    }
};

struct htd_comments
{
    std::string_view file = "../testFiles/comments.hu";

    hu::Trove trove;
    hu::Node root;
    hu::Node k0;
    hu::Node k1;
    hu::Node k10;

    void setup()
    {
        trove = hu::Trove::fromFile(file);
        root = trove.rootNode();
        k0 = root / "k0";
        k1 = root / "k1";
        k10 = k1 / 0;
    }

    void teardown()
    {
    }
};

