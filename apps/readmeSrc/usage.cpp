#include <humon.hpp>
#include <iostream>

using namespace std;
using namespace std::literals;

template<int NumComponents>
struct Version
{
    Version(std::string_view verString = ""sv)
    {
        char const * cur = verString.data();
        int comp = 0;
        int compIdx = 0;
        while (static_cast<size_t>(cur - verString.data()) < verString.size())
        {
            if (*cur >= '0' && *cur <= '9')
            {
                comp *= 10;
                comp += *cur - '0';
            }
            else if (*cur == '.')
            {
                if (compIdx < NumComponents)
                {
                    components[compIdx] = comp;
                    comp = 0;
                    compIdx += 1;
                }
            }
            else
                { comp = 0; break; }
            cur += 1;
        }
        if (compIdx < NumComponents)
            { components[compIdx] = comp; }

        for (int i = compIdx + 1; i < NumComponents; ++i)
        {
            components[i] = 0;
        }
    }

    Version(std::initializer_list<int> init)
    {
        int i = 0;
        for (auto comp : init)
        {
            if (i < NumComponents)   { components[i] = comp; }
            else                { break; }
            i += 1;
        }

        for (int i = init.size(); i < NumComponents; ++i)
        {
            components[i] = 0;
        }
    }

    bool operator < (Version const & rhs) const
    {
        for (int i = 0; i < NumComponents; ++i)
        {
            if ((*this)[i] < rhs[i])
                { return true; }
            else if ((*this)[i] > rhs[i])
                { return false; }
        }

        return false;
    }

    int operator [] (int compIdx) const { return components[compIdx]; }

    friend std::ostream & operator << (std::ostream & out, Version const & v)
    {
        for (int i = 0; i < NumComponents - 1; ++i)
            { out << v[i] << "."; }
        out << v[NumComponents - 1];

        return out;
    }

private:
    std::array<int, NumComponents> components;
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
    {
        auto desRes = hu::Trove::fromFile("apps/readmeSrc/materials.hu"sv);
        if (auto trove = std::get_if<hu::Trove>(& desRes))
        {
            auto extentsNode = trove->nodeByAddress("/assets/brick-diffuse/importData/extents"sv);
            tuple xyExtents = { extentsNode / 0 / hu::val<int>{}, 
                                extentsNode / 1 / hu::val<int>{} };
            std::cout << "Extents: (" << get<0>(xyExtents) << ", " << get<1>(xyExtents) << ")\n";
        }
    }

    {
        auto desRes = hu::Trove::fromFile("apps/readmeSrc/materials.hu"sv, { hu::Encoding::utf8});
        if (auto trove = std::get_if<hu::Trove>(& desRes))
        {
            std::cout << std::get<string>(trove->toPrettyString());
        }
    }

    {
        auto desResFromRam = hu::Trove::fromString("{foo: [100, 200]}"sv);
        auto DesResFromFile = hu::Trove::fromFile("data/foo.hu"sv);

        auto & trove = std::get<hu::Trove>(desResFromRam);

        auto rootNode = trove.root();           cout << "rootNode: " << rootNode.address() << "\n";
        // or
        rootNode = trove.nodeByAddress("/");    cout << "rootNode: " << rootNode.address() << "\n";
        // or
        rootNode = trove.nodeByIndex(0);        cout << "rootNode: " << rootNode.address() << "\n";

        auto node = rootNode / "foo" / 0;       cout << "node: " << node.address() << "\n";
        // or
        node = trove / "foo" / 0;               cout << "node: " << node.address() << "\n";
        // or
        node = trove / "foo" / 1 / hu::Parent{} / 0;
                                                cout << "node: " << node.address() << "\n";
        // or
        node = rootNode.nodeByAddress("foo/0"); cout << "node: " << node.address() << "\n";
        // or
        node = trove.nodeByAddress("/foo/0");   cout << "node: " << node.address() << "\n";
        // or
        node = rootNode.child("foo").child(0);  cout << "node: " << node.address() << "\n";


        // has a child with key 'foo'
        bool hasFoo = rootNode % "foo";                         cout << "hasFoo: " << hasFoo << "\n";
        // has a child with index 1 (so at least two children)
        bool hasFoosStuff = hasFoo && rootNode / "foo" % 1;     cout << "hasFoosStuff: " << hasFoosStuff << "\n";

        hasFoo = rootNode % "foo";                              cout << "hasFoo: " << hasFoo << "\n";
        auto fooNode = rootNode / "foo";
        hasFoosStuff = rootNode / "foo" % 1;                    cout << "hasFoosStuff: " << hasFoosStuff << "\n";

        fooNode = rootNode / "foo";
        hasFoosStuff = fooNode ? fooNode % 1 : false;           cout << "hasFoosStuff: " << hasFoosStuff << "\n";

        hasFoosStuff = rootNode / "foo" % 1;                    cout << "hasFoosStuff: " << hasFoosStuff << "\n";

        auto foosStuff = rootNode / "foo" / 1;                  cout << "foosStuff exists: " << foosStuff << "\n";
        if (foosStuff) { cout << "foosStuff: " << foosStuff / hu::val<int>{} << "\n"; }

        string address = node.address();                        cout << "address: " << address << "\n";
    }

    {
        auto src = R"(
{
    bufferSources: {
        res/"game\ assets"/meshes.hu: {
            required: true
            monitoredForChanges: true
        }
    } 
    pipelineSources: {
        res/"game\ assets"/materials.hu: {
            required: false
            monitoredForChanges: true
        }
    }
    renderPlans: {
        res/"game\ assets"/renderPlan-overland.hu: {
            required: true
            monitoredForChanges: true
        }
    }
}
)"sv;

        auto trove = move(get<hu::Trove>(hu::Trove::fromString(src)));
        auto requiredNode = trove / 0 / 0 / "required";             cout << "required's address: " << requiredNode.address() << "\n";
        
        auto relativeNode = requiredNode.nodeByAddress("../.. / .. /1 / 0"); cout << "relative node address: " << relativeNode.address() << "\n";

        auto node = trove.root();                                   cout << "node: " << node.address() << "\n";
        node = node.child(1);                                       cout << "node: " << node.address() << "\n";
        node = node.child(R"(res/"game\ assets"/materials.hu)"sv);  cout << "node: " << node.address() << "\n";

        auto childNode = trove.root().firstChild();                 
        do
        {                                                           cout << "childNode: " << childNode.address() << "\n";
            childNode = childNode.nextSibling();
        }
        while (childNode);

        node = node.parent();                                       cout << "node: " << node.address() << "\n";

        node = trove / "bufferSources" / 0 / "monitoredForChanges";
        string_view valStr = node.value();                          cout << "valStr: " << valStr << "\n";
        // or
        int valBool = node / hu::val<bool>{};                       cout << "valBool: " << valBool << "\n";
    }

    {
        auto src = R"(
{
    dependencies: {
        gcc: 9.2.1
    }
}
)"sv;

        auto trove = move(get<hu::Trove>(hu::Trove::fromString(src)));
        auto gccVersion = trove / "dependencies" / "gcc" / hu::val<V3>{};
                                                                    cout << "gccVersion: " << gccVersion << "\n";
        auto const literalVersion = "9.2.1";                        
        auto toolVersion = hu::val<V3>::extract(literalVersion);    cout << "toolVersion: " << toolVersion << "\n";
    }

    {
        auto src = R"(
{
    definitions: { 
        player: {
            maxHp: {
                type: int @{numBits: 32 numBytes: 4}
            }
            damage: {
                type: int  @{numBits: 32 numBytes: 4}
            }
            // ...
        }
    }
}
)"sv;

        auto trove = move(get<hu::Trove>(hu::Trove::fromString(src)));
        auto node = trove.nodeByAddress("/definitions/player/maxHp/type");

        int numAnnos = node.numAnnotations();               cout << "num annos: " << numAnnos << "\n";
        for (int i = 0; i < numAnnos; ++i)
        {
            auto [k, v] = node.annotation(i);               cout << "k: " << k << "  v: " << v << "\n";
            if (k == "numBits"sv) { /*...*/ }
            else if (k == "numBytes"sv) { /*...*/ }
        }
        // or (slower; hu::Node::allAnnotations() allocates a std::vector):
        for (auto [k, v] : node.allAnnotations())
        {                                                   cout << "k: " << k << "  v: " << v << "\n";
            if (k == "numBits"sv) { /*...*/ }
            else if (k == "numBytes"sv) { /*...*/ }
        }

        bool hasNumBits = node.hasAnnotation("numBits"sv);  // verify annotation by key
                                                            cout << "hasNumBits: " << hasNumBits << "\n";
        auto annoValue = node.annotation("numBits"sv);      // get annotation by key
                                                            cout << "annoValue: " << annoValue << "\n";
        // many annotations in a single node might have the same value; run through them all
        for (auto & annoKey: node.annotationsWithValue("32"sv))
        {
            //...
                                                            cout << "annoKey: " << annoKey << "\n";
        }

        auto all32BitTypeNodes = trove.findNodesWithAnnotationKeyValue("numBits"sv, "32"sv);
        for (auto & node : all32BitTypeNodes)
        {
            //...
                                                            cout << "node: " << node.address() << "\n";
        }

        auto tokStr = trove.toPrettyString();

        // Output the exact token stream used to build the trove. Fast.
        tokStr = trove.toClonedString();

        // Output the trove with minimal whitespace for most efficient storage/transmission. 
        // The parameter directs Humon to strip comments from the stream.
        tokStr = trove.toMinimalString({}, false);
        
        // Minimal whitespace, with old style HTML linebreaks.
        tokStr = trove.toMinimalString({}, true, "<br />");

        hu::ColorTable colorTable = hu::getAnsiColorTable();

        // You can specify minimal whitespace and still use a color table for the tokens--see below.
        tokStr = trove.toMinimalString(colorTable, false, "\n");
        if (auto str = std::get_if<std::string>(& tokStr))
            { cout << * str; }

        // Pretty. Use an indentation of 4 spaces to format nested depths.
        tokStr = trove.toPrettyString(4, false, colorTable, false, "\n");
        if (auto str = std::get_if<std::string>(& tokStr))
            { cout << * str; }
    }

    {
        auto desRes = hu::Trove::fromFile("samples/sampleFiles/hudo.hu"sv);
        if (auto trove = std::get_if<hu::Trove>(& desRes))
        {
            if (trove->troveAnnotation("app") != "hudo"sv)
                { throw runtime_error("File is not a hudo file."); }

            auto versionString = trove->troveAnnotation("hudo-version");
            auto version = V3 { versionString.str() };
            if      (version < V3 { 0, 1, 0 }) { std::cout << "Using version 0.0.x\n"; /*...*/ }
            else if (version < V3 { 0, 2, 0 }) { std::cout << "Using version 0.1.x\n"; /*...*/ }
            else { std::cout << "Using latest version 0.2.x\n"; /*...*/ }
            // ...
        }
    }

    return 0;
}
