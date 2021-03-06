#pragma once

#define HUMON_SUPPRESS_NOEXCEPT

#include <string_view>
#include "humon/humon.hpp"

using namespace std::literals;


struct htd_cppGeneralMix
{
    std::string_view ts =
R"(// This is a trove comment.
@{tx:ta, ta:ta, tb:tb}
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
        trove = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts)));
        root = trove.root();
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
        trove = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts)));
        root = trove.root();
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
    std::string_view file = "test/testFiles/comments.hu";

    hu::Trove trove;
    hu::Node root;
    hu::Node k0;
    hu::Node k1;
    hu::Node k10;

    void setup()
    {
        trove = std::move(std::get<hu::Trove>(hu::Trove::fromFile(file)));
        root = trove.root();
        k0 = root / "k0";
        k1 = root / "k1";
        k10 = k1 / 0;
    }

    void teardown()
    {
    }
};

struct htd_errors
{
    /*
    std::string_view ts0 =
R"({
    int:213
    float:25.25
    float:25.25     // duplictae key
    string:foo
    bool:true
}
)"sv;*/

    std::string_view ts1 =
R"({
    int:213
    float:25.25
    double:25.25
    string:foo
    bool:true       @ { dup: foo, dup: bar }
}
)"sv;

    std::string_view ts2 =
R"({
    int:213
    float:25.25
    double:25.25
    string:foo
    bool:true       @ dup: foo @ dup: bar
}
)"sv;

    std::string_view ts3 =
R"(@ {dup: foo dup: bar}
{
    int:213
    float:25.25
    double:25.25
    string:foo
    bool:true
}
)"sv;

    std::string_view ts4 =
R"({@ dup: foo @ dup: bar
    int:213
    float:25.25
    double:25.25
    string:foo
    bool:true
}
)"sv;

    hu::Trove trove0;
    hu::Trove trove1;
    hu::Trove trove2;
    hu::Trove trove3;
    hu::Trove trove4;

    void setup()
    {
        //trove0 = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts0, {}, hu::ErrorResponse::mum)));
        trove1 = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts1, {}, hu::ErrorResponse::mum)));
        trove2 = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts2, {}, hu::ErrorResponse::mum)));
        trove3 = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts3, {}, hu::ErrorResponse::mum)));
        trove4 = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts4, {}, hu::ErrorResponse::mum)));
    }

    void teardown()
    {
    }
};


struct htd_wakka
{
    std::string_view ts =
u8R"({
    \key: \val
    k\ey: v\al
    key\ : val\ // space after backslash
    k\"ey: v\"al
    \"key": \"val"
    ^^key\ key^^: ^^val\ val^^
    ^A^key\{key\}^A^: ^🤔^val\{val\}^🤔^
    ^
^key^
^\::val\:
    \🤔:\∑
}
)"sv;

    hu::Trove trove;
    hu::Node root;

    void setup()
    {
        trove = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts)));
        root = trove.root();
    }

    void teardown()
    {
    }
};


struct htd_changes
{
    std::string_view ts =
R"({
    @a:b /**/aaa: AAA//wtf
    bbb: [BBB @c:d BBB/**/ BBB]
    ccc: {CCCa: cccA, CCCb: cccB, CCCc: cccC}
}
)"sv;

    hu::Trove trove;
    hu::Node root;
    hu::Node aaa;
    hu::Node bbb;
    hu::Node ccc;

    void setup()
    {
        trove = std::move(std::get<hu::Trove>(hu::Trove::fromString(ts)));
        root = trove.root();
        aaa = root.child(0);
        bbb = root.child(1);
        ccc = root.child(2);
    }

    void teardown()
    {
    }
};

