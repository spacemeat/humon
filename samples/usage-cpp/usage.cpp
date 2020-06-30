#include <humon.hpp>
#include <iostream>

using namespace std;
using namespace std::literals;

template<int NumRanks>
struct Version
{
    Version(std::string_view verString)
    {
    }

    Version()
    {
    }

    Version(std::initializer_list<int> args)
    {

    }

    std::array<int, NumRanks> ranks;

    bool operator<(Version const & rhs)
    {
        return true;
    }
};


using V3 = Version<3>;

template <>
struct hu::val<V3>
{
    static inline V3 extract(std::string_view valStr)
    {
        return V3(valStr);
    }
};



int main()
{
    if (auto trove = hu::Trove::fromFile("samples/sampleFiles/materials.hu"sv);
        trove)
    {
        auto extentsNode = trove.nodeByAddress("/assets/brick-diffuse/importData/extents"sv);
        tuple xyExtents = { extentsNode / 0 / hu::val<int>{}, 
                            extentsNode / 1 / hu::val<int>{} };
        std::cout << "Extents: (" << get<0>(xyExtents) << ", " << get<1>(xyExtents) << ")\n";
    }


    if (auto trove = hu::Trove::fromFile("samples/sampleFiles/hudo.hu"sv);
        trove)
    {
        if (trove.annotation("app") != "hudo"sv)
            { throw runtime_error("File is not a hudo file."); }

        auto versionString = trove.annotation("hudo-version");
        auto version = Version<3> { versionString };
        if (version < Version<3> { 0, 1, 0 }) { /*...*/ }
        else if (version < Version<3> { 0, 2, 0 }) { /*...*/ }
        // else ...
    }

    {
        auto trove = hu::Trove::fromString("{foo: [100, 200]}"sv);
        auto troveFromFile = hu::Trove::fromFile("data/foo.hu"sv);

        auto rootNode = trove.root();
        // or
        rootNode = trove.nodeByAddress("/");
        // or
        rootNode = trove.node(0);

        auto node = rootNode / "foo" / 0;
        // or
        node = trove.root() / "foo" / 0;
        // or
        node = rootNode.relative("foo/0");
        // or
        node = trove.nodeByAddress("/foo/0");

        // has a child with key 'foo'
        bool hasFoo = rootNode % "foo";
        // has a child with index 1 (so at least two children)
        bool hasFoosStuff = hasFoo && rootNode / "foo" % 1;

        hasFoo = rootNode % "foo";
        auto fooNode = rootNode / "foo";
        hasFoosStuff = rootNode / "foo" % 1;

        fooNode = rootNode / "foo";
        hasFoosStuff = fooNode ? fooNode % 1 : hu::Node(nullptr);

        hasFoosStuff = rootNode / "foo" % 1;

        auto foosStuff = rootNode / "foo" / 1;
        if (foosStuff) {  }

        string address = node.address();

        auto requiredNode = trove / 0 / 0 / "required";
        cout << "Address: " << requiredNode.address() << "\n";

        auto requiredAddress = requiredNode.relative("../.. / 1 / 0");

        string_view valStr = node.value();
        // or
        int valInt = node / hu::val<int>{};

        auto gccVersion = trove / "dependencies" / "gcc" / hu::val<V3>{};

        auto const literalVersion = "9.2.1";
        auto version = hu::val<V3>::extract(literalVersion);

        node = trove.nodeByAddress("/definitions/player/maxHp/type");

        int numAnnos = node.numAnnotations();
        for (int i = 0; i < numAnnos; ++i)
        {
            auto [k, v] = node.annotation(i);
            if (k == "numBits"sv) { }
            else if (k == "numBytes"sv) { }
        }
        // or (slower; hu::Node::allAnnotations() allocates a std::vector):
        for (auto [k, v] : node.allAnnotations())
        {
            if (k == "numBits"sv) { }
            else if (k == "numBytes"sv) { }
        }

        bool hasNumBits = node.hasAnnotation("numBits"sv);
        auto annoValue = node.annotation("numBits"sv);
        bool is32Bit = node.numAnnotationsWithValue("32"sv) > 0;
        auto annoKey = node.annotationWithValue("32"sv, 0);

        auto all32BitTypes = trove.findNodesWithAnnotationKeyValue("numBits"sv, "32"sv);
        for (auto & node : all32BitTypes) { node; }

        auto tokStr = trove.toPrettyString();

        // Output the exact token stream used to build the trove. Fast.
        tokStr = trove.toPreservedString();
        // Output the trove with minimal whitespace for most efficient storage/transmission. 
        // The parameter directs Humon to strip comments from the stream.

        tokStr = trove.toMinimalString({}, false);
        
        // Minimal whitespace, with old style HTML linebreaks.
        tokStr = trove.toMinimalString({}, true, "<br />");

        hu::ColorTable colorTable = hu::getAnsiColorTable();

        // You can specify minimal whitespace and still use a color table for the tokens--see below.
        tokStr = trove.toMinimalString(colorTable, false, "\n");

        // Pretty. Use an indentation of 4 spaces to format nested depths.
        tokStr = trove.toPrettyString(4, colorTable, false, "\n");
    }

    return 0;
}
