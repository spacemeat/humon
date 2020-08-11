# Humon

*Human-Usable Machine Object Notation*

*Note: Certain language in this document assumes you are human.*

## Introduction

Humon may refer to any of three things:
1. A general text-based, structured language format similar to JSON,
1. some programming interfaces that understand it, like that described here, and
1. a reference implementation of said interface with bindings to several programming languages, like that defined by this repo.

Mainly, Humon takes some of the roles that JSON has been taking lately, but with an eye towards the human side of the experience. It is intended to be convenient to create and maintain data structures by hand.

I use it for design. It's a great mind-mapping tool. I use it to model database tables, serialized structure definitions, app configuration. Designing a complex structure in Humon is a fun and interactive task, even in a text editor. The absence of much syntax makes authorship feel fluid, and I can keep my mind on my creative work. With a useful API, loading and examining that data in software is a simple matter.

### Examples

JSON looks like this, as we all know:

    {
        "someDict": {
            "cow": {
                "says": "moo",
                "source": "mouth"
            }
            "cat": {
                "say": "meow, my dude",
                "source": "mouth"
            }
        }
        "someList": [
            "first", 1, true
        ]
        "someValue": "bug"
    }

It's not very friendly to composition by hand. Minimal Humon looks pretty close, but without some nits:

    {
        someDict: {
            cow: {
                says: moo
                source: mouth
            }
            cat: {
                says: "meow, my dude"
                source: mouth
            }
        }
        someList: [
            first 1 true
        ]
        someValue: bug
    }

No quotes except where necessary, and no commas required. (They're optional, and treated like whitespace). So that's kind of nice. Strict JSON is fussy about all that punctuation.

More useful Humon might look like this asset description for a material in a game:

    @ engineVersion: 0.1.0
    assets: {
        brick-diffuse: {
            src: [brick-diffuse.png]
            importData: {
                type: image
                extents: [1024 1024 1]
                numMipLevels: 0    // generate all the mips
                format: R8G8B8A8Unorm
            }
        }
    }

The `@` syntax is an *annotation*. We'll discuss that, but for now just think of it as a certain kind of metadata. Not all Humon token streams will use them.

Using the APIs is straightforward. To get the image's (x, y) extents above, we might invoke the following in C:

    #include <Humon.h>
    ...
        huTrove const * trove = NULL;
        int error = huDeserializeTroveFromFileZ(& trove, "samples/sampleFiles/materials.hu", NULL,
            HU_ERRORRESPONSE_STDERRANSICOLOR);
        if (error == HU_ERROR_NOERROR && huGetNumErrors(trove) == 0)
        {
            huNode const * extentsNode = huGetNodeByAddressZ(trove, "/assets/brick-diffuse/importData/extents");
            huNode const * valueNode = huGetChildByIndex(extentsNode, 0);
            huStringView const * sExt = valueNode ? & valueNode->valueToken->str : NULL;
            int extX = sExt ? strntol(sExt->ptr, sExt->size, NULL, 10) : 0;
            valueNode = huGetChildByIndex(extentsNode, 1);
            sExt = valueNode ? & valueNode->valueToken->str : NULL;
            int extY = sExt ? strntol(sExt->ptr, sExt->size, NULL, 10) : 0;
            printf("Extents: (%d, %d)\n", extX, extY);
            // ...

or in C++:

    #include <Humon.hpp>
    ...
        auto desRes = hu::Trove::fromFile("samples/sampleFiles/materials.hu"sv);
        if (auto trove = std::get_if<hu::Trove>(& desRes))
        {
            auto extentsNode = trove->nodeByAddress("/assets/brick-diffuse/importData/extents"sv);
            tuple xyExtents = { extentsNode / 0 / hu::val<int>{}, 
                                extentsNode / 1 / hu::val<int>{} };
            std::cout << "Extents: (" << get<0>(xyExtents) << ", " << get<1>(xyExtents) << ")\n";            ...

### Installation
If you're just using Humon files with some application you've installed, you don't need to do anything. However, the project contains a directory called `vscode/humon-lang` which contains a TextMate colorizer for VS Code. You can copy the `humon-lang` directory into your `~/.vscode/extensions` directory, and VS Code will make files with a ".hu" extension a bit more colorful.

Building Humon in Linux is easy enough, but you need to have Python3 installed. Starting from the Humon project directory, you can build the binaries with gcc:

    ~/src/humon$ ./build-linux.py

or, if you want to use clang:

    ~/src/humon$ ./build-linux.py clang

> Currently the clang build uses gcc's standard library. A better build experience is in the works, but in the meantime it's trivial to modify the `build-linux.py` script to use whatever library you have.

In Windows, you can open the root-level `humon.sln` file in Visual Studio 2017+, and build the targets you like.

For all builds, the binary artifacts are produced in `{humon directory}/build/bin`. Currently, you can just copy the built binaries and the headers from `{humon directory}/include/humon` for use in your projects.

The following are built in Linux:

* libhumon.a                    - optimized static library for Linux
* libhumon-d.a                  - debug static library for Linux
* libhumon.so.ver.si.on         - optimized shared library for Linux
* libhumon-d.so.ver.si.on       - debug shared library for Linux
* humon-test-d                  - test binary, debug version
* humon-test-r                  - test binary, optimized version
* hux                           - a command-line tool for translating and validating Humon
* readmeSrc-c                   - a small sample with example code from the README.md
* readmeSrc-cpp                 - a less small sample with example code from the README.md

These files are built in Windows:
* humon-win32.lib               - optimized static library for 32-bit Windows
* humon-win32-d.lib             - debug static library for 32-bit Windows
* humon-win32-shared.dll        - optimized DLL for 32-bit Windows
* humon-win32-shared.lib        - import library for the 32-bit optimized DLL
* humon-win32-shared-d.dll      - debug DLL for 32-bit Windows
* humon-win32-shared-d.lib      - import library for the 32-bit debug DLL
* humon-win32-shared-d.pdb      - PDB for the 32-bit debug DLL
* humon-win64.lib               - optimized static library for 64-bit Windows
* humon-win64-d.lib             - debug static library for 64-bit Windows
* humon-win64-shared.dll        - optimized DLL for 64-bit Windows
* humon-win64-shared.lib        - import library for the 64-bit optimized DLL
* humon-win64-shared-d.dll      - debug DLL for 64-bit Windows
* humon-win64-shared-d.lib      - import library for the 64-bit debug DLL
* humon-win64-shared-d.pdb      - PDB for the 64-bit debug DLL
* humon-test.Win32.Debug.exe    - test binary
* humon-test.Win32.Debug.pdb    - PDB for test binary
* hux.exe                       - a command-line tool for translating and validating Humon
* hux.pdb                       - PDB for hux.exe

The tests are built along with the libraries. Run them from the project root:

    ~/src/humon$ build/bin/humon-test-d

or

    PS C:\Users\you\src\humon> build\bin\humon-test.Win32.Debug.exe

or run the test project directly from Visual Studio.

### Humon version
Humon uses three values in its language/API versioning scheme: `major.minor.revision` For changes that do not affect the API, the revision is incremented. For changes that only add to the API but do not break builds or behaviors, the minor value is incremented. For breaking changes, the major value is incremented. The version will generally refer to the API version; the Humon format is considered stable. (Though of course, it's only Humon, and may not know of its own imperfections yet.)

## Design principles
We're going to keep referencing JSON, as it's the model that Humon takes after. But we want it to be better, so the obvious thing to do is just fix the problems with JSON. Once we commit to designing a new format / language, we're free to make the rules. Rules should come from guiding principles:

1. **We are serving humans with this language.**
Human authors and maintainers are one of the two primary customers. Reading, modifying, and writing Humon data in an editor should be as simple and hassle-free as possible. It must never come as a surprise what the language thinks you mean, or what is legal to state in the language. A minimal syntax promotes this principle. Humon token streams must be able to be large, storing lots of kinds of data, free to fulfill human needs tersely.
1. **We are serving machines with this language.**
Computing machines are the other primary customer. Software written against the language must run quick, and the language must promote simple interfaces and consistent operation. There must be adequate facilities for programmed machines to utilize the data exposed by Humon. Resource usage for grokking Humon objects must be reasonable.
1. **We are serving humans who are serving machines with this language.**
The APIs are the conjunction of human and machine. The machine has been taught how to use them. We can learn the same. API design must be convenient and unsurprising, consistent and unambiguous.

### Principles in the language
No more required quotes. Commas are optional. When you do quote keys or values, you can use any quote character `` ' " ` ``. Keys and values are just text data. Quoted strings can contain newlines. Annotations and comments can provide "human hooks" into the data without the need to muck with the structure design, and without polluting the data structure expected by an extant application. Humon files are *intended* to be authored and modified in text editing tools, by humans, and also be simple enough for a machine to generate as well.

### Principles in the APIs
Most of the interesting use cases for Humon involve a machine reading data authored by either a machine or a human. The performance characteristics of the implementation reflect this; loaded Humon troves are *immutable*. Searching and scanning through Humon data is quick, and references to internal data do not go bad for the life of the trove.

The base API is a C interface. Everything a human or machine needs to traverse the data hierarchically or by search is there and easy to use. Mainly it exists for other language bindings, but this makes Humon available to pure C code as well.

There is a C++ layer which wraps the C API, giving syntactic sugar and a more accessible interface. You'll almost certainly prefer it over the C API for C++ projects.

Language bindings forthcoming include Python, Node, Rust, and .NET.

## The language
I happen to love me some JSON. To my mind, it has the best syntax for describing structured data. It's incredibly expressive, incredibly simple, and open to just about any practical use. There are so many structures, descriptions, configurations, and data models that can be faithfully described by a JSON object, with very little plumbing. The array- and dictionary-style collections are generally sufficient to describe a huge swath of problem spaces. (Some structures, like graphs, are harder to model in a hierarchy. Graphs are difficult to visualize in any serial text format.)

As we all know, JSON has some creaky bits. Some of them are merely annoying, like nitpicky punctuation--during a creative editing session, worrying about maintaining commas in just the right places while developing the structure is Flowus Interruptus. But some of JSON's shortcomings inhibit some more practical use cases, like the lack of comment support. Editing apps and other environments oftentimes include their own custom JSON parser, just to accept comments in configuration files, say.

I mean, JSON wasn't designed for these things. It was made to communicate between JavaScript processes over a wire. It doesn't *have* to be these things for me to appreciate what it can do. But that doesn't mean it should be the thing to do the things we do make it do. Let's design a language that can always do those things, and serve *all* its customers.

So there's YAML. It basically does what I want but its syntax is complex, and has spatial requirements that I feel are binding. Just more syntax to manage, and it's harder to modify its structure quickly by hand. Humon is more fluid and flexible, and that allows for quick changes in a text editor.

So there's TOML, which comes pretty close. It still has some structural requirements that I feel are unnecessary.

So, this project proposes Humon as a replacement for those tasks that JSON is performing but shouldn't, and YAML might be performing if it was more accessible. Specifically, those spaces where humans interact with structured text data, and those places where we could be, but are inhibited by inconvenience or a rigid format like .ini.

### Some terms
*Humon*: The format, API, implementation of the API, files memory blocks or data streams that contain expressions of data in the format, or that data.

*trove*: A tokenized and parsed, in-memory representation of Humon data which stems from a single root node, and against which Humon APIs are programmed.

*token stream*: The text of a Humon file or in-memory text string. I use this language to refer to Humon text content like in a file, as opposed to tokenized or parsed Humon data in a trove.

*token*: A word or sequence of words in a Humon token stream, which are meaningful to Humon and you at a granular level. `{` is a token, as is a value like `"foo"`, as is a whole comment like `/* I am just an utterance. */`. (Comments are considered single tokens for parsing purposes.) Every token is owned either by a single node, or (in special cases) by the trove itself. A token array is maintained by the trove, and provides content and position information (row, column) for each token, if that sort of thing is interesting to you.

*node*: A unit of structure in a Humon data hierarchy. There are three kinds of nodes: lists, dicts (dictionaries), and values. Lists and dicts can store any kinds of nodes as children. Nodes are the abstraction of the structure. The trove maintains an array of nodes parsed from the token array.

*list*: A node that contains a sequential collection of zero or more nodes as children.  In the language, the list is the bounding `[ ]` around the child nodes. In the API, a list's children can be accessed by index.

*dict*: A node that contains a sequential collection of zero or more nodes as children, where each node is associated with a key. In the language, the dict is the bounding `{ }` around the child nodes, each of which must be preceded by a key and `:`. In the API, a dict's children can be accessed by index or by key.

*key*: An associated string for a dict's child. A key must be a string of nonzero length. Within a dict, keys must be unique. There is no maximum length defined, and keys can be quoted. (Quoted values can be multiline strings, but that's a strange way to spec a key.) Any Unicode code point that is not whitespace or punctuation counts as a key character.

*value*: A node that contains a single string value. Values can be quoted; quoted values can be multiline strings. As with keys, any Unicode code point that is not whitespace or punctuation counts as a value character.

*comment*: A string of words that are not exposed as values in nodes. They're mainly for humans to make notes, but there are APIs to search for comments and get their associated nodes, in case that's useful.

*annotation*: One of a collection of key:value string pairs associated to a node or a trove. They are exposed as special values in nodes, and are globally searchable by key and/or value. Any node can have any number of annotations, as long as they have unique keys (like in a dict).

### The principles applied to the language

#### Brevity and simplicity
Punctuation in Humon consists entirely of:

    [ ] { } : " ' ` @ // /* */

As stated, there are three kinds of nodes in a Humon token stream: lists, dicts, and values. These correspond to the same object kinds in JSON, conceptually: A list stores a plurality of other nodes in ordered sequence. A dictionary stores a plurality of nodes in ordered sequence, each associated with a key. A value stores a single string. This structure allows for an arbitrary data hierarchy.

#### It works like you expect a smart thing to work
For the most part it looks like JSON, and nothing about it is surprising. However, there is one relatively smart aspect of the Humon spec, to do with how comments are associated to related nodes. We'll discuss the finer points of comments later but broadly, comments can be searched for, and their associated nodes referenced by APIs. Humon has smarts about associating comments as they appear in a token stream, relative to other tokens, in the same way your brain probably does.

Mmm, brain.

#### The hu in Humon
You're the human. Use it the way you want.

Humon is intended to be used to describe data in almost any structure. Few problems I can think of cannot be described by a structure like Humon (or JSON [or YAML (or XML)]). Humon seeks to be the least verbose of these, only stating the minimum about the structure it holds, and allowing the problem space it models to be whatever the human--you--want it to be.

To that end, Humon makes no assumptions about the meaning of values. There are no keywords. There are no special meanings to any tags or annotations or the text of comments. There are no automatic annotations for special nodes or anything. Nothing for you to memorize. Humon has little understanding of anything at all, which is *freeing*. It's up to the application using Humon to decide everything about structure without worrying about stumbling on some forgotten language rule.

I mean, JSON does all this too. There's nothing revolutionary here, but Humon's syntax is *human-usable*, in an editing context.

### Humon language rules
Okay, let's state some explicit rules, then.

**A trove contains zero or one root node.**
A blank token stream, or one with only comments and annotations (but no collections or values) contains no root node. Otherwise it must contain one top-level node, probably a list or dict with many children.

**The root node is what it is, which is what you say it is.**
There is no assumption made about the kind of the root node. If the first non-comment, non-annotation token in your token stream is a `[`, then the root is a list; a `{` starts a dict. If it's a value, then it's the only one allowed, since value nodes don't have children and a trove can only have one root. (Not a terribly useful Humon trove, but legal. Eh.)

**Lists are enclosed in [square brackets].**
Any kind of node can occupy any entry in any list. Lists don't have to contain nodes of a single kind; mix and match as you see fit.

**Dicts are enclosed in {curly braces}.**
Each dict entry must be a key:node pair. The node can be of any kind. Each key in a dict must be a string, and unique within the dict. There is no delimiting syntax between key:node pairs; just whitespace (maybe including commas if you like) if needed for disambiguation.

**Values are strings.**
A value is a contiguous non-punctuation, non-whitespace string of Unicode code points, or a quoted string of Unicode code points.

For non-quoted values, the first unescaped whitespace or punctuation character encountered ends the string, and begins a new token. This obviously includes newlines. Quotes that are encountered *within* the string (but not the beginning) are included with it, and are not matched against any other quotes later in the string. So, `value"with'quotes` is tokenized as one whole token string. Backslashes can be used to escape punctuation characters or whitespace within an unquoted string. So, `value\ with\[\ crazy\ stuff \]` is tokenized as one whole token string. Backslashes do not escape newlines though; for that, use a quoted string.

You can use any quote character (`'`, `"`, `` ` ``) to begin a string. It doesn't matter which you use; they all work the same way. A quoted string value starts after the quote, and ends just before the next corresponding, *unescaped* quote. The string can contain backslash-escaped quotes, like in most programming languages, and treats other kinds of quotes as normal token characters:

    'This isn\'t an "uncommon" example.'

Quoted string values can span multiple lines of text. Newlines are included in the value.

    "Fiery the angels rose, and as they rose deep thunder roll'd.
    Around their shores: indignant burning with the fires of Orc."

**Keys must be strings.**
A value is a contiguous non-punctuation, non-whitespace string of Unicode code points, or a quoted string of Unicode code points.

They can be numbers of course, but that's just a string to Humon. The rules for value strings also fully apply to keys. Examples below specify valid keys:

    {
        this: okay
        this one: nope
        this\ one: sure
        "this one here": "this is fine"
        "this
    one": yep
        1: "fine, but remember: numbers are just strings to Humon and they won't be sorted"
        Δημοσθένους: "Unicode is fully recognized."
    }

**Whitespace is ignored.**
The tokenizer ignores whitespace that isn't escaped or contained in a quoted string. Whitespace characters are not captured by tokens. All Unicode whitespace code points are recognized as whitespace.

**Commas are whitespace.**
Commas are regarded by the tokenizer as whitespace. They are completely optional. The following Humon objects are identical:

    [resistors caps ICs diodes MOSFETs]
    [resistors,caps,ICs,diodes,MOSFETs]
    [resistors, caps, ICs, diodes, MOSFETs]     // [1]
    [
        resistors
        caps
        ICs
        diodes
        MOSFETs
    ]
    [
        resistors,
        caps,
        ICs,
        diodes,
        MOSFETs,
    ]
    [
        resistors,
        caps,
        ICs,
        diodes,
        MOSFETs
    ]
    [resistors, caps, ICs, diodes, MOSFETs,]
    ,,,[,resistors, 
    caps    ,
         ICs, ,
                 diodes 
               MOSFETs,,,,,,, ],,,

I find myself sometimes using commas when placing list or dict elements on one line, as on line `[1]` above, and not using them elsewise. In other words, what comes natural to my typing is legal Humon, and requires no grammarizing. I like designing structure without worrying about the punctuation; indeed, this little nit was one of the main motivations behind Humon's development.

**Comments have no meaning.**
Any Unicode code points can be in any comments. Humon doesn't require your comments to contain any particular information. Because they have meaning to humans, they are not discarded by the parser; see below.

**Annotations are their own thing, and exist out of band.**
Annotations are the only sort of special syntax-y feature of Humon. The first working versions of the format didn't include them, but as I used Humon in those early days, I found that I wanted a convenient shorthand that sort of *jumped out of the structure* to provide context data, or specify certain rare conditions. Something formal and machineable, but that also didn't force modifications to the structure in place and were never strictly necessary. I settled on annotations.

Every node can have any number of annotations, which appear *after or within* the node--specifically, *after* any token belonging or associated to a node. The trove can have them too, if an annotation appears before any other objects. Annotations begin with an `@` symbol, followed by either one key:value pair, or a dict of key:value pairs:

    [
        nostromo @ movie-ref: alien
        sulaco @ { movie-ref: aliens, movie-director: cameron }
    ] @ { exhaustive: probablyNot }

Annotation values must be strings; an annotation value can't be a collection. That way lies some madness.

Like dict entries, annotation keys must be unique among annotations associated to a particular node. Different nodes in a trove can have annotations with identical keys though.

Practically, annotations aren't a necessary part of Humon. All their data could be baked into normal lists or dicts, and that's a fine way to design.

**Humon reads UTF-8, UTF-16, and UTF-32.**
Humon supports reading files or in-memory strings encoded as any of:
* UTF-8, with or without BOM
* UTF-16, little- or big-endian, with or without BOM
* UTF-32, little- or big-endian, with or without BOM

Humon respects all whitespace characters that Unicode specifies, and supports all code points. When loading Humon data, you can specify the encoding if you know it already, or specify `hu::Encoding::unknown` (which is the default when loading from file). In that case, Humon will attempt to figure out the encoding automatically, either according to the BOM or, if one is not present, by examining the bit pattern of the token stream. When loading from an in-memory string, the default encoding is `hu::Encoding::utf8`, and you'll need to specify something different if that's what you've got (or `hu::Encoding::unknown`).

There are performance implications for using `hu::Encoding::unknown`, especially if there is no BOM in the token stream. Humon has to examine bytes until it can determine the encoding, and then start over with a transcode operation. If you do know your encoding, do specify it.

Some encodings (aliases or overlong encodings in some UTF-n formats) can cause unsecure behavior in some applications. You can be strict about checking for encoding legality. The checks are specified in a `hu::DeserializeOptions` structure passed to `hu::fromString` or `hu::fromFile` or `hu::fromStream`, and are on by default. When checking legality, overlong sequences are converted to canonical forms, and code points outside legal ranges cause an error.

    auto desRes = hu::Trove::fromFile("samples/sampleFiles/materials.hu"sv, 
        { hu::Encoding::utf8, false });     // UTF-8, and disable Unicode checks
    if (auto trove = std::get_if<hu::Trove>(& desRes))
    {
        // ...

If you know your source data is UTF-8, and you know it contains only legal code units or you don't care, you can turn off strict Unicode checking. This allows Humon to indiscriminately load byte data without checking for overlong sequences, etc. It's a little faster. If you're accepting a token stream generated by a user, especially a remote user, consider always checking legality. A proper Unicode application should not emit these illegal codes.

Either way, special Unicode code points like continuations are simply considered word characters for keys or values or comments, and Humon doesn't *ever* check for Unicode's notions of semantic correctness for sequences of code points. To Humon, any code point is either whitespace, language punctuation (which is only ever a single UTF-8 byte), or a word-token character. That's all the tokenizer understands.

If this is all Groot to you, just rest assured that Humon can load any text file that any web server, browser, or basic text editor generally produces, regardless of platform / OS.

**Humon writes UTF-8.**
While Humon can read all the normal UTF-n formats, `Trove::toString`, `Trove::toFile`, and their sugar functions can only generate UTF-8, with or without BOM as you choose.

> Why only UTF-8? It's the ubiquitous encoding for the web and most Linux and OSX things, and Windows APIs can fully deal with it. But mainly, I just don't care enough about supporting these encodings beyond reading. Humon transcodes the token stream into UTF-8 internally, and slams memory out on a `Trove::to*` call. See the manifesto at [this page](http://utf8everywhere.org/) to read an opinionated opinion.

## The C-family programming interfaces
There are API specs for the C and C++ interfaces. The C API is fully featured but obtuse, as C APIs tend to be. The C++ API mainly wraps the C API, but also provides some nice features for more C++ish fun-time things. Since you'll usually be using the C++ API, we'll mainly discuss that.

### Getting a trove

Start with `#include <humon.hpp>`. The interface is contained in a namespace, `hu`. The C API is itself contained in a nested namespace, `hu::capi`, but you generally won't reference it yourself.

(Note: To use the C API, `#include <humon.h>` instead. Obviously there is no namespace there, because it's a pure C99 header.)

To deserialize a Humon token stream, invoke one of `hu::Trove`'s static member functions:

    auto desResFromRam = hu::Trove::fromString("{foo: [100, 200]}"sv);
    auto DesResFromFile = hu::Trove::fromFile("data/foo.hu"sv);

These each return a `std::variant<hu::Trove, hu::ErrorCode>` object. Once you have a trove, all the loading from source is finished, and it's fully ready to use. You'll use the trove to get access to nodes and their data.

> The error code is set if there was a problem loading the token stream before tokenization could begin. Bad parameters or an unusable encoding will disallow a trove from even being created. Beyond that point, a trove is made and returned, and will contain all the tokenization and parsing errors in the token stream.

The `hu::Trove` class is move-constructable and move-assignable. When it is destroyed, it will destroy the underlying trove data.

### Getting nodes

There are several ways to access a node. To get the root node, which is always at node index 0:

    auto & trove = std::get<hu::Trove>(desResFromRam);

    auto rootNode = trove.root();
    // or
    rootNode = trove.nodeByAddress("/");
    // or
    rootNode = trove.nodeByIndex(0);

These each return a `hu::Node` object that represents the root node. 

To get a node deeper in the tree:

    auto node = rootNode / "foo" / 0;
    // or
    node = trove / "foo" / 0;
    // or
    node = trove / "foo" / 1 / hu::Parent{} / 0;
    // or
    node = rootNode.nodeByAddress("foo/0");
    // or
    node = trove.nodeByAddress("/foo/0");
    // or
    node = rootNode.child("foo").child(0);

> Internally, a `hu::Trove` object just manages a pointer to a `capi::huTrove`, which is created on the heap. Moves are shallow, and all the member functions call C API functions on the stored pointer. `hu::Node` objects also just manage pointers, and are movable and copyable. Since underlying Humon objects are immutable and immovable, the C++ objects can be lightweight and breezy.

To check whether a node has a particular child node, use the `%` operator on the node with the desired key or index. This returns a bool which indicates whether the given key or index is valid. Note that for indices, this produces `true` if the given index is strictly smaller than the number of children, and is inclusive of index 0.

    // has a child with key 'foo'
    bool hasFoo = rootNode % "foo";
    // has a child with index 1 (so at least two children)
    bool hasFoosStuff = hasFoo && rootNode / "foo" % 1;

It's more efficient to store a reference to a node than to look it up successive times. Above, we're essentially looking up `/foo` twice. Better to get the base object and store it:

    auto fooNode = rootNode / "foo";
    hasFoosStuff = rootNode / "foo" % 1;

Humon APIs don't throw, but rather return nullish* objects, which have implicit `operator bool()`s for checking nullity. So even better than above:

    fooNode = rootNode / "foo";
    hasFoosStuff = fooNode ? fooNode % 1 : false;

Or even,

    hasFoosStuff = rootNode / "foo" % 1;

And, if you intend to actually use the data (probably, right?), it may be even better to just ask for it all the way without checking anything:

    auto foosStuff = rootNode / "foo" / 1;
    if (foosStuff) { ... }

Above, `foosStuff` will be a nullish object if any part of the path doesn't actually exist in the Humon source. It's all in one succinct line of code, though this is harder to debug; if the path is invalid, it may be hard to check which part of the path fell off of reality. So use this compact query style with caution.

> * *nullish* objects are `hu::Trove`, `hu::Node`, or `hu::Token` objects that manage a null pointer. `HU_NULLTROVE`, `HU_NULLNODE`, and `HU_NULLTOKEN` are aliases to `NULL`. Operations on these objects will return other nullish objects, or `NULL` or `nullptr` as appropriate.

Up to now, we've mostly been using `operator /` to get nodes. But there's a string-based addressing method as well. Every node has a unique address based on the progression of keys and indices used to get to it from the trove or another node. You can get a node's address easily enough:

    string address = node.address();

Node addresses are computed, not stored in the trove; as a result, the return value is a `std::string` rather than a `std::string_view` peering into the token stream. More on that in a sec.

The address of a node looks like:

    "/foo/20/baz/3"

The `/`s delimit the terms, and each term is either a key or integer index.

In Humon, dicts maintain their ordering like lists. When crafting an address string into Humon data, you can use an integer to index into a dict just like you do a list. (`hu::Node::address()` prefers keys over indices when generating addresses.) Note also that full paths from the trove must start with `/`, the root node. You can also put arbitrary whitespace in the address. For reasons.

You can look up a node by its address:

    node = trove.nodeByAddress("/foo/0");

You get a node object back, which is valid if the address is valid or nullish if not.

There are some finnicky bits about node addresses when considering that a dict's keys can be numbers in a token stream. They're still just string keys to Humon. But, `hu::Trove::nodeByAddress(address)` and `hu::Node::nodeByAddress(address)` interpret numeric terms in the address as indices. If you intend them to be read as keys, enquote that term in the address:

    "/foo/'20'/baz/"

`hu::Node::address()` always returns an address that is legal to use in `hu::Trove::nodeByAddress()` to find the node again. `hu::Node::address()` will appropriately `"`-enquoted terms if it needs to, and will escape already-present `"`s if it has to enquote such a term.

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

    ...

    auto requiredNode = trove / 0 / 0 / "required";
    cout << "required address: " << requiredNode.address() << "\n";

    ...

    $ runSample
    required's address: /bufferSources/"res/\"game\ assets\"/meshes.hu"/required

A relative address can be used to get from one node to another:

    auto requiredAddress = requiredNode.nodeByAddress("../.. / 1 / 0");

Notice the relative path does not start with `/`. The relative address is followed from the node, not from the trove.

There are also explicit member functions for getting nodes by child index or key or parentage:

    auto node = trove.root();
    
    node = node.child(1);
    
    node = node.child(R"(res/"game\ assets"/materials.hu)"sv);

    auto childNode = trove.root().firstChild();                 
    do
    {
        // do something with childNode
        childNode = childNode.nextSibling();
    }
    while (childNode);

    node = node.parent();

### Getting node data
    
So you've got a value node. Whooptie-doo. To get a value from it:

    node = trove / "bufferSources" / 0 / "monitoredForChanges";
    string_view valStr = node.value();          // [1]
    // or
    bool valBool = node / hu::val<bool>{};      // [2]

> There are numerous Humon APIs that return a `std::string_view`, which might raise some of y'all's flags--`std::string_view`s do not own their own data. But remember, Humon objects are immutable and don't ever move in memory; the C++ objects just wrap heap pointers. Once the trove is loaded, none of its references go bad until the trove is destroyed. The returned `std::string_view`s point to memory that is static and good until the trove is gone, so in the case of Humon APIs, returning a `std::string_view` is copacetic. Just keep the trove around.

Though `hu::Node::value()` is assigned to a `std::string_view` above, it actually returns a `hu::Token`, which has an implicit conversion to a `std::string_view`. A `hu::Token` has information about the token's representation in the token stream. You can get line / column data and the string value itself. Use token information if you want to report on the location of interesting data in a token stream.

In example [2] above, the `hu::val<T>` type is defined to extract typed data from the value string of a node.  It allows you to deserialize and return an arbitrary-typed value. There are definitions for the basic numeric types and `bool` (using English spelling of "true" to denote truth). This is how you'll get the vast majority of your data.

You can also define your own specialization of `hu::val<T>` for your own type. Start with the type you'd like to deserialize:

    template<int NumComponents>
    class Version
    {
    public:
        Version(std::string_view str) { ... }
        Version(std::initializer_list<int> init) { ... }
        bool operator < (Version const & rhs) const { ... }
        int operator [] (int compIdx) const { return components[compIdx]; }
        friend std::ostream & operator << (std::ostream & out, Version const & v) { ... }
    private:
        std::array<int, NumComponents> components;
    };

    using V3 = Version<3>;

And define the specialized extractor:

    template <>
    struct hu::val<V3>
    {
        static V3 extract(std::string_view valStr)
        {
            return V3(valStr);
        }
    };

Now you can use it:

    {
        dependencies: {
            gcc: 9.2.1
        }
    }
    
    ...

    auto gccVersion = trove / "dependencies" / "gcc" / hu::val<V3>{};

C++ will deduce tye type of `gccVersion` above from the `V3` template parameter passed to `hu::val<>`. `hu::val<T>` is a convenience which allows you to code the lookup and conversion in line, without grouping parentheses. You can also use the extractor member to convert strings by themselves:

    auto const literalVersion = "9.2.1";
    auto toolVersion = hu::val<V3>::extract(literalVersion);

### Getting annotation data

Annotations are described in detail below. They're essentially per-node metadata. Examine a node's annotations in several ways:

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

    ...

    auto node = trove.nodeByAddress("/definitions/player/maxHp/type");

    int numAnnos = node.numAnnotations();
    for (int i = 0; i < numAnnos; ++i)
    {
        auto [k, v] = node.annotation(i);
        if (k == "numBits"sv) { /*...*/ }
        else if (k == "numBytes"sv) { /*...*/ }
    }
    // or (slower; hu::Node::allAnnotations() allocates a std::vector):
    for (auto [k, v] : node.allAnnotations())
    {
        if (k == "numBits"sv) { /*...*/ }
        else if (k == "numBytes"sv) { /*...*/ }
    }

    // verify annotation by key
    bool hasNumBits = node.hasAnnotation("numBits"sv);

    // get annotation by key
    auto annoValue = node.annotation("numBits"sv);

    // many annotations in a single node might have the same value; run through them all
    for (auto & annoKey: node.annotationsWithValue("32"sv))
    {
        //...

Trove objects can have annotations too, separate from any node annotations, and feature similar APIs. There are also APIs for searching all a trove's nodes for annotations by key, value, or both; these return collections of `hu::Node`s.

    auto all32BitTypeNodes = trove.findNodesWithAnnotationKeyValue("numBits"sv, "32"sv);
    for (auto & node : all32BitTypes) { ... }

Similar APIs also exist for nodes and troves that search comment content and return associated nodes. See the API spec for these.

### Formatting a token stream

There are APIs for serializing a trove back to memory or a stream. There are three whitespace formatting options to serialize Humon objects:
1. **Cloned**: A direct copy of the original token stream is produced, including all comments and commas and whitespace. Just a brainless memory slam.
1. **Minimal**: This reduces whitespace to at most one character each to pare down length. Humon does as much as it can, but if you choose to preserve comments in the minified output, you may notice that not all newlines get replaced. This is because the next read operation on the resultant token stream must replicate the comment associations from the original, and that requires some comments to be on their own line. Also, C++-style `//comment`s end in a newline, and the token after must be on its own line, so those newlines are also preserved.
1. **Pretty**: This produces a clean, indented, eminently readable string.

You can generate a Humon token stream from a `hu::Trove` like so:

    hu::SerializeOptions opts {};
    auto tokStr = trove.toString(opts);

This will generate an uncolored, well-formatted token stream, or an error code. These are returned as a `std::variant<std::string, hu::ErrorCode>`. The `hu::SerializeOptions` class has formatting options to customize the token stream to you liking.

There are sugar functions to make it even easier:

    // Output the exact token stream used to build the trove. Fast.
    tokStr = trove.toClonedString();

    // Output the trove with minimal whitespace for most efficient storage/transmission. 
    // The first parameter is a null color table.
    // The second parameter directs Humon to strip comments from the stream, making it even more compact.
    tokStr = trove.toMinimalString({}, false);

    // Minimal whitespace, with old style HTML line breaks (default is "\n").
    tokStr = trove.toMinimalString({}, true, "<br />");

For printing with colors, specify a color table of type `hu::ColorTable`, which is just a type alias for `std::array<std::string_view, hu::capi::HU_COLORCODE_NUMCOLORS>`. You can set these values manually; each string corresponds index-wise to color codes in `hu::ColorCode`, and is inserted just before the appropriate token in the printed token stream. There are a few special values: `hu::ColorCode::tokenStreamBegin` which is placed before all other characters, `hu::ColorCode::tokenEnd` which is placed after *each* colored token, and `hu::ColorCode::tokenStreamEnd` which is placed after the *last* token in the token stream.

> Sometimes C-style enums can be more convenient than C++ enum classes. It's okay to admit it. You can use the analogous `hu::capi::huColorCode`, or `huColorCode` as defined in the `humon.h` C header.

Humon provides a function to make a `hu::ColorTable` with ANSI terminal color codes. This is useful for command-line printing of Humon content.

    hu::ColorTable colorTable = hu::getAnsiColorTable();

    // You can specify minimal whitespace and still use a color table for the tokens.
    tokStr = trove.toMinimalString(colorTable, false, "\n");
    if (auto str = std::get_if<std::string>(& tokStr))
        { cout << * str; }

    // Pretty. Use an indentation of 4 spaces to format nested depths.
    tokStr = trove.toPrettyString(4, false, colorTable, false, "\n");
    if (auto str = std::get_if<std::string>(& tokStr))
        { cout << * str; }

### The principles applied to the C-family APIs
Maybe you'd call them behaviors, but they embody the Humon principles.

**CRLF-style newlines (often made in Windows or Symbian OS) are regarded as one newline object.**
This is mostly for recording line numbers in token objects, for error reporting and other things that need token placement information. In general, it does what you expect.

**All non-whitespace, non-quote characters are part of some token.**
Only whitespace characters like spaces, newlines, commas, and quote characters around keys and values are discarded from tracking in the tokenizer.

**All tokens are part of some node, xor the trove.**
During a load, a Humon token stream is tokenized into a list of token objects, and then those tokens are parsed into a node hierarchy. Every single token object is owned by exactly one node, or by the trove itself in a few cases. A trove can completely reconstruct a Humon token stream from the nodes and tokens, including reconstructing comments and annotations with their appropriate associations.

**All keys and values are strings.**
As stated, Humon makes no assumptions about the data type of a value. `"True"`, `"true"`, and `true` are all the same type of thing to Humon: a four-character string. (Though, of course, case is preserved and considered when searching.)

Usually boolean and numeric values are computational though, and matter to the application using Humon. Convenience APIs are included in some language bindings to parse values (and in some cases let you define your own parsers).

**Humon objects are immutable.**
Once loaded, a Humon trove does not move or change. This has implications:
1. Accessing raw value data is quick. You basically get a pointer and size. Since the data behind it doesn't move or change, that value pointer is good for the life of the trove.
1. Raw string values returned by APIs are *not* NULL-terminated. (In C++, they translate directly to `std::string_view`s.)
1. Raw string values contain escapement characters, just as seen in the token stream. They do not contain any surrounding quotes. If strings contain CRLF newlines in the token stream, they'll appear that way in raw string accesses too.
1. Raw string values are UTF-8-encoded.
1. The whole token stream must be in contiguous memory. This is currently true at least, and means loading a whole token stream before tokenizing and parsing. It's unfortunate for concurrency, and I'm thinking about it. For most use cases like app configurations or database schemas, this isn't a real issue.
1. When serializing back to another stream or file from a Humon trove, the exact source can be spit out without any conversion, since the original string is still in memory (encoded as UTF-8). This is the most performant way to generate a Humon token stream from a trove.

**The C-family APIs don't signal by default; rather, they return null objects.**
Normally, trove and node functions that return other nodes or tokens will not throw exceptions on bad parameters or lookup terms, or if they're nullish objects. Instead they just return other nullish objects or degenerate values. In the C++ API, you can check `IsNullish()` on `hu::Trove`, `hu::Node`, and `hu::Token` objects to see whether they're managing null values. Most of the functions are marked `noexcept` by default as well.

Tou can turn turn on exceptions for the C++ API. Before your `#include <humon.hpp>` declaration, define any of these:
* `#define HUMON_USE_NULLISH_EXCEPTIONS` This will cause nullish `hu::Token` and `hu::Node` objects to throw when their member functions are called, instead of returning other nullish objects.
* `#define HUMON_USE_NODE_PATH_EXCEPTIONS` This will cause  `hu::Trove::operator/` and `hu::Node::operator/` to throw exceptions instead of returning nullish objects when called with bad indices or keys. It can help you track down erroneous lookups.
* `#define HUMON_SUPPRESS_NOEXCEPT` This turns off `noexcept` on most member functions, causing exceptions to be thrown instead of terminating the program.
* `#define HUMON_SUPPRESS_PATH_NOEXCEPT` This turns off `noexcept` on `hu::Trove::operator/` or `hu::Node::operator/` calls, allowing the program to throw instead of terminate when bad lookups occur.

So, if you want bad lookup sequences to throw exceptions, but all other functions to terminate the program when misused, you might write this:

    #define HUMON_USE_NULLISH_EXCEPTIONS
    // By not suppressing noexcept, those functions that fail by
    // throwing will instead just crash the program.

    // But here we allow the path functions to throw instead of
    // crash, since we might actually want to handle it with code.
    #define HUMON_USE_NODE_PATH_EXCEPTIONS
    #define HUMON_SUPPRESS_PATH_NOEXCEPT

    #include <humon.hpp>

**Objects in dicts remain in serial order.**
This is different from some JSON libraries, that don't preserve the order of key-object pairs in dicts. Humon guarantees that, when you access a dict's children by index (like you would a list), they'll be returned in the order you expect. Humon can maintain an extra table for accessing dict entries quickly by key. (Currently this is not implemented, and keys are searched in linear order. This is obviously on the fix-it list, but it's often the case that a linear search beats hash tables or binary searches for small numbers of entries.<sup>[citation needed]</sup>)

**Comments have no meaning, but they do have context.**
Humon doesn't know or care about your comments, as already stated in the language rules. But it doesn't discard them in the parsing process; they're preserved for searching and reserialization.

All comments are associated to either a node in the hierarchy, or the trove itself. Humon tries to be smart about associating comments. Here's an example; the comments indicate to which token and node they associate:

    // ↓ root dict node
    // ↓ root dict node
    {   // ← root dict node
        // ↓ element node
        key     // ← element node
            :   // ← element node
            // ↓ element node
            value // ← element node
                // ← element node; the following annotation is considered part of it
                @ { annoKey: annoValue } ← // element node
        // ↓ root dict node
    }   // ← root dict node
    // ↓ trove

You can get the comments on any node or the trove, and you can search for a comment and get its associated node back though APIs. Comments are associated to nodes per the following rules:

1. Any comment that starts on a line of text that contains no other non-comment tokens before it is *associated forward*. That means the next non-comment token encountered in the token stream specifies the node to which the comment will be associated. If there are no non-comment tokens left in the stream, the comment applies to the trove.

        [
            // craftsman
            "table saw"
            // delta
            "drill press"
            // dewalt
            "chop saw"
            // makita
            "impact driver"
        ]

1. Any comment that appears on a line of text *after* a non-comment token is *associated backward*. That means a comment which trails a value or punctuation symbol on the same line of text is associated to that value or collection, respectively. These are mainly for comments that trail elements in a vertical list:

        [
            "table saw"     // craftsman
            "drill press"   // delta
            "chop saw"      // dewalt
            "impact driver" // makita
        ]

These rules allow you to write comments naturally within the structure of the token stream, and their associations will be what you'd expect a human to interpret as well.

If you're never searching by comments, or are ignoring them altogether, then none of these rules really matter to you. But for such a mission, the associations can be handy. IDEs and document generators could machine on comments for smart document summaries, for example.

> Usually you would prefer annotations for searchable metadata, as comments are kind of "more optional" and likely more ephemeral--nobody expects comments to be required in any file, especially for machining purposes, and it may not occur to someone to maintain them for functional reasons. But I'm not here to tell you what to do.

**Annotations have context.**
Like comments, annotations are associated with nodes or the trove. You can access annotations by key from a node, or search all annotations by key or value (or both) and get their associated nodes.

Annotations are always *associated backward*. They modify the nearest non-comment node that appeared before the annotation, regardless of same-lineness. Any amount of whitespace or comments in between is ignored, as usual. If the previous token was a collection-starter or collection-ender (`[`, `{`, `]` or `}`), it modifies that list or dict node. In the following, `remoteStorage:true` annotates the player dict, not the userId value, because it follows the player dict's `{` opener:

    {
        player: {
            @remoteStorage: true
            userId:     int
            username:   string
            friends:    { type:vector of:string }
        }
    }

(Personally, I would place the annotation on the same line as the `player: {`, for clarity.)

If no nodes appear before an annotation, it applies to the trove. A great way to begin a Humon file spec is by specifying annotations for the application version or config version for which the file is good. Maybe you're developing a code-generation suite cleverly called "hudo", and want to ensure the correct version of the engine is called to consume every model file, even older ones. Hudo could look for a version annotation on the trove, before examining any nodes, and use the appropriate engine version to interpret the structure. Here we'll reuse the `using V3 = Version<3>;` class from earlier:

    @ { app: hudo, hudo-version: 0.1.1 }

    {
        player: { @remoteStorage: true
            userId:     int
            username:   string
            friends:    { type:vector of:string }
        }
    }

    ...

    desRes = hu::Trove::fromFile("samples/sampleFiles/hudo.hu"sv);
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

    $ runSample
    Using version 0.1.x

Like asserted earlier, annotations are 100% open in their use. Humon doesn't use any annotation keys or values and doesn't interpret them. Applications can use them or not, but all annotations that are legal are guaranteed to be parsed, even if the application doesn't know about or use them at all. In this way you can embed metadata about objects in a Humon file, and even old versions of Humon apps will correctly read (and ignore) them, because all official versions of Humon always have.

## The future of Humonity

Near-future features include:
* Language bindings for Python
* Language bindings for .NET
* Language bindings for Node
* Language bindings for Rust
* Accept alloc/free alternatives in the C lib
* Modules for C++
* Single-header-ify the C++ (as an alternative to modules)
* Constexpr All The Things in the single-header version. Kind of a research project.
* Type overrides for some of the runtime data elements like line/column data
* Better build process. Considering CMake, but that's not set in stone.
* C++ Range views for depth- and breadth-first node visitation
