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
        root = trove.getRootNode();
        a = root.getChild(0);
        bp = root.getChild(1);
        cpp = root.getChild(2);
        b = bp.getChild(0);
        cp = cpp.getChild(0);
        c = cp.getChild(0);
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
        root = trove.getRootNode();
        nint = root.getChild(0);
        nfloat = root.getChild(1);
        ndouble = root.getChild(2);
        nstring = root.getChild(3);
        nbool = root.getChild(4);
    }

    void teardown()
    {
    }
};

