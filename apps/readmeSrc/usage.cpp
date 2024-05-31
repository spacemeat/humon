#include <iostream>
#include <sstream>
#include <iomanip>
#include <humon/humon.hpp>

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

//!!! val1
template <>
struct hu::val<V3>
{
    static V3 extract(hu::Node const & val)
    {
        return V3(val.value().str());
    }
};
//!!!

class YourMemoryManager
{
public:

    YourMemoryManager(ostringstream & out) : out(out) { }

    void * alloc(::size_t len) 
    { 
        out << "Alloc: #" << ++numAllocs << " - " << len << " bytes\n";
        return ::malloc(len);
    }
    void * realloc(void * alloc, ::size_t len)
    { 
        out << "Realloc: #" << ++numReallocs << " - " << len << " bytes\n";
        return ::realloc(alloc, len);
    }
    void free(void * alloc)
    { 
        out << "Free: #" << ++numFrees << "\n";
        return ::free(alloc);
    }

    ostringstream & out;
    int numAllocs = 0;
    int numReallocs = 0;
    int numFrees = 0;
};

int main(int argc, char ** argv)
{
    bool update = false;
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'u')
        { update = true; }

    ostringstream out;

    {
//!!! materials
        auto desRes = hu::Trove::fromFile("apps/readmeSrc/materials.hu"sv);
        if (auto trove = std::get_if<hu::Trove>(& desRes))
        {
            auto extentsNode = trove->nodeByAddress("/assets/brick-diffuse/importData/extents"sv);
            tuple xyExtents = { extentsNode / 0 % hu::val<int>{}, 
                                extentsNode / 1 % hu::val<int>{} };
            // ...
//!!!
            out << "Extents: (" << get<0>(xyExtents) << ", " << get<1>(xyExtents) << ")\n";
        }
    }

    {
//!!! materials2
        auto desRes = hu::Trove::fromFile("apps/readmeSrc/materials.hu"sv, 
            { hu::Encoding::utf8, false });     // UTF-8, and disable Unicode checks
        if (auto trove = std::get_if<hu::Trove>(& desRes))
        {
            // ...
//!!!
            out << std::get<string>(trove->toPrettyString());
        }
    }

    {
//!!! statics
        auto desResFromRam = hu::Trove::fromString("{foo: [100, 200]}"sv);
        auto DesResFromFile = hu::Trove::fromFile("apps/readmeSrc/materials.hu"sv);
//!!!
    }


    {
//!!! loadOptions
        YourMemoryManager memMan(out);
        //...
        auto alloc = hu::Allocator
        {
            & memMan,
            // These are fn pointers, so no captures allowed.
            [](void * memManager, ::size_t len )
                { return ((YourMemoryManager *) memManager)->alloc(len); },
            [](void * memManager, void * alloc, ::size_t len )
                { return ((YourMemoryManager *) memManager)->realloc(alloc, len); },
            [](void * memManager, void * alloc )
                { return ((YourMemoryManager *) memManager)->free(alloc); }
        };
        auto desResFromRam = hu::Trove::fromString("{foo: [100, 200]}"sv, 
            { hu::Encoding::unknown, true, 4, alloc});
//!!!
//!!! roots
        auto & trove = std::get<hu::Trove>(desResFromRam);

        auto rootNode = trove.root();           /*!!!eol*/out << "rootNode: " << rootNode.address() << "\n";
        // or
        rootNode = trove.nodeByAddress("/");    /*!!!eol*/out << "rootNode: " << rootNode.address() << "\n";
        // or
        rootNode = trove.nodeByIndex(0);        /*!!!eol*/out << "rootNode: " << rootNode.address() << "\n";
//!!!
//!!! drill
        auto node = rootNode / "foo" / 0;       /*!!!eol*/out << "node: " << node.address() << "\n";
        // or
        node = trove / "foo" / 0;               /*!!!eol*/out << "node: " << node.address() << "\n";
        // or
        node = trove / "foo" / 1 / hu::Parent{} / 0; /*!!!eol*/out << "node: " << node.address() << "\n";
        // or
        node = rootNode.nodeByAddress("foo/0"); /*!!!eol*/out << "node: " << node.address() << "\n";
        // or
        node = trove.nodeByAddress("/foo/0");   /*!!!eol*/out << "node: " << node.address() << "\n";
        // or
        node = rootNode.child("foo").child(0);  /*!!!eol*/out << "node: " << node.address() << "\n";
//!!!
//!!! child1
        // has a child with key 'foo'
        bool hasFoo = rootNode % "foo";                         /*!!!eol*/out << "hasFoo: " << hasFoo << "\n";
        // has a child with index 1 (so at least two children)
        bool hasFoosStuff = hasFoo && rootNode / "foo" % 1;     /*!!!eol*/out << "hasFoosStuff: " << hasFoosStuff << "\n";
//!!!
//!!! child2
        auto fooNode = rootNode / "foo";
        hasFoosStuff = fooNode ? fooNode % 1 : false;           /*!!!eol*/out << "hasFoosStuff: " << hasFoosStuff << "\n";
//!!!
//!!! child3
        fooNode = rootNode / "foo";
        hasFoosStuff = fooNode % 1;                             /*!!!eol*/out << "hasFoosStuff: " << hasFoosStuff << "\n";
//!!!
//!!! child4
        hasFoosStuff = rootNode / "foo" % 1;                    /*!!!eol*/out << "hasFoosStuff: " << hasFoosStuff << "\n";
//!!!
//!!! child5
        auto foosStuff = rootNode / "foo" / 1;                  /*!!!eol*/out << "foosStuff exists: " << foosStuff << "\n";
        if (foosStuff)
        { 
            // ... 
//!!!
            out << "foosStuff: " << foosStuff % hu::val<int>{} << "\n";
        }
//!!! child6
        string address = node.address();                        /*!!!eol*/out << "address: " << address << "\n";
//!!!
//!!! child7
        node = trove.nodeByAddress("/foo/0");
//!!!
    }

    {
//!!! addressWeird1
        auto src = R"(
{
    bufferSources: {
        'res/"game_assets"/meshes.hu': {
            required: true
            monitoredForChanges: true
        }
    } 
    pipelineSources: {
        `res/"game_assets"/materials.hu`: {
            required: false
            monitoredForChanges: true
        }
    }
    renderPlans: {
        res/"game_assets"/renderPlan-overland.hu: {
            required: true
            monitoredForChanges: true
        }
    }
}
)"sv;
//!!!
//!!! addressWeird2
        auto trove = get<hu::Trove>(hu::Trove::fromString(src));
        auto requiredNode = trove / 0 / 0 / "required";             /*!!!eol*/out << "required's address: " << requiredNode.address() << "\n";
//!!!
//!!! addressWeird3
        auto relativeNode = requiredNode.nodeByAddress("../../../2/0/required"); /*!!!eol*/out << "relative node address: " << relativeNode.address() << "\n";
//!!!
//!!! getNode
        // get the root node
        auto root = trove.root();
    
        // get its second child (index 1)
        auto node = root.child(1);                                  /*!!!eol*/out << "node: " << node.address() << "\n";
        
        // or, get its child by key
        node = root.child("pipelineSources"sv);                     /*!!!eol*/out << "node: " << node.address() << "\n";

        // cycle through each of root's child nodes
        auto childNode = trove.root().firstChild();                 
        do
        {                                                           /*!!!eol*/out << "childNode: " << childNode.address() << "\n";
            // ...do something with childNode
            childNode = childNode.nextSibling();
        }
        while (childNode);

        // call your mom
        node = node.parent();                                       /*!!!eol*/out << "node: " << node.address() << "\n";
//!!!
//!!! getValue
        node = trove / "bufferSources" / 0 / "monitoredForChanges";
        string_view valStr = node.value();       /* [1] */          /*!!!eol*/out << "valStr: " << valStr << "\n";            
        // or
        bool valBool = node % hu::val<bool>{};   /* [2] */          /*!!!eol*/out << "valBool: " << valBool << "\n";
//!!!
    }

    {
//!!! val2
        auto src = R"(
{
    dependencies: {
        gcc: 9.2.1
    }
}
)"sv;
//!!!

        auto trove = get<hu::Trove>(hu::Trove::fromString(src));
//!!! val3
        auto gccVersion = trove / "dependencies" / "gcc" % hu::val<V3>{}; /*!!!eol*/out << "gccVersion: " << gccVersion << "\n";
//!!!
    }

    {
//!!! metatag1
        auto src = R"(
{
    definitions: { 
        player: {
            maxHp: {
                type: int @{numBits: 32 numBytes: 4}
            }
            damage: {
                type: int @{numBits: 32 numBytes: 4}
            }
            // ...
        }
    }
}
)"sv;
//!!!
//!!! metatag2
        auto trove = move(get<hu::Trove>(hu::Trove::fromString(src)));

		//...

        auto node = trove.nodeByAddress("/definitions/player/maxHp/type");

        int numMetatags = node.numMetatags();               /*!!!eol*/out << "num metatags: " << numMetatags << "\n";
        for (int i = 0; i < numMetatags; ++i)
        {
            auto [k, v] = node.metatag(i);               /*!!!eol*/out << "k: " << k << "  v: " << v << "\n";
            if (k == "numBits"sv) { /*...*/ }
            else if (k == "numBytes"sv) { /*...*/ }
        }
        // or (slower; hu::Node::allMetatags() allocates a std::vector):
        for (auto [k, v] : node.allMetatags())
        {                                                   /*!!!eol*/out << "k: " << k << "  v: " << v << "\n";
            if (k == "numBits"sv) { /*...*/ }
            else if (k == "numBytes"sv) { /*...*/ }
        }

        int numBits = node.numMetatagsWithKey("numBits"sv);  // verify metatag by key
                                                            /*!!!eol*/out << "numBits: " << numBits << "\n";
        auto && metatagValue = node.metatagsWithKey("numBits"sv);      // get metatag by key
                                                            /*!!!eol*/out << "metatagValue: " << metatagValue[0] << "\n";
        // many metatags in a single node might have the same value; run through them all
        for (auto & metatagKey: node.metatagsWithValue("32"sv))
        {
            //...
//!!!                                                            
                                                            /*!!!eol*/out << "metatagKey: " << metatagKey << "\n";
        }
//!!! metatag3
        auto all32BitTypeNodes = trove.findNodesWithMetatagKeyValue("numBits"sv, "32"sv);
        for (auto & node : all32BitTypeNodes)
        {
            //...
//!!!                                                            
                                                            /*!!!eol*/out << "node: " << node.address() << "\n";
        }
//!!! print1
        hu::SerializeOptions opts {};
        auto tokStr = trove.toString(opts);
//!!!
//!!! print2
        // Output the exact token stream used to build the trove. Fast.
        tokStr = trove.toClonedString();

        // Output the trove with minimal whitespace for most efficient storage/transmission. 
        // The parameter directs Humon to strip comments from the stream.
        tokStr = trove.toMinimalString({}, false);
        
        // Minimal whitespace, with old-style HTML linebreaks.
        tokStr = trove.toMinimalString({}, true, "<br />");
//!!!
//!!! print3
        hu::ColorTable colorTable = hu::getAnsiColorTable();

        // You can specify minimal whitespace and still use a color table for the tokens--see below.
        tokStr = trove.toMinimalString(colorTable, false, "\n");
        if (auto str = get_if<std::string>(& tokStr))
            { out << * str << "\n"; }

        // Pretty. Use an indentation of 4 spaces to format nested depths.
        tokStr = trove.toPrettyString(4, false, colorTable, false, "\n");
        if (auto str = get_if<std::string>(& tokStr))
            { out << * str; }
//!!!
    }

    {
//!!! hudo
        auto desRes = hu::Trove::fromFile("apps/readmeSrc/hudo.hu"sv);
        if (auto trove = get_if<hu::Trove>(& desRes))
        {
			auto && metatags = trove->troveMetatagsWithKey("app");
            if (metatags.size() == 0 || metatags[0] != "hudo"sv)
                { throw runtime_error("File is not a hudo file."); }

			metatags = trove->troveMetatagsWithKey("hudo-version");
			if (metatags.size() == 0)
				{ throw runtime_error("File has no version metatag."); }
            auto versionString = metatags[0];
            auto version = V3 { versionString.str() };
            if      (version < V3 { 0, 1, 0 }) { out << "Using version 0.0.x\n"; /*...*/ }
            else if (version < V3 { 0, 2, 0 }) { out << "Using version 0.1.x\n"; /*...*/ }
            else { out << "Using latest version 0.2.x\n"; /*...*/ }
            // ...
//!!!
        }
    }

    if (update)
    {
        ofstream expStr("apps/readmeSrc/usage_cpp_out.txt");
        expStr << out.str();
        printf("Updated expectations.\n");
    }
    else
    {
        ofstream gotStr("apps/readmeSrc/usage_cpp_got.txt");
        gotStr << out.str();

        ifstream expStr("apps/readmeSrc/usage_cpp_out.txt");
		istringstream iss(out.str());
		auto done = false;
		int line = 1;
		bool is_different = false;
		ostringstream oss;
		while (! done)
		{
			string exp, got;
			std::getline(expStr, exp);
			std::getline(iss, got);
			if (exp == got)
			{
				oss << std::setw(3) << line << "     " << got.data() << "\n";
				//printf("%3d     %s\n", line, got.data());
			}
			else
			{
				oss << std::setw(3) << line << " :(  " << got.data() << "\n";
				//printf("%3d :(  %s\n", line, got.data());
				is_different = true;
			}

			line += 1;
			if (expStr.eof() || iss.eof())
			{
				done = true;
			}
		}

        if (is_different == false)
            { printf("Copasetic, my sisters and brothers.\n"); return 0; }
        else
			{ printf("Epic fail:\n%s", oss.str().data()); return 1; }
    }

    return 0;
}
