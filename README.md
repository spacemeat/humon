# humon

*Human-Usable Machine Object Notation*

*Note: Certain language in this document assumes you are human.*

## introduction

Humon is primarily three things: 1) A general text-based, structured language format, 2) some programming interfaces that understand it, and 3) a reference implementation of said interface with bindings to several programming languages. Mainly, it takes some of the roles that JSON has been taking increasingly lately, but with an eye towards the human side of the experience.

I use it for design. It's a great mind-mapping tool. I use it to model database tables, serialized structure definitions, app configuration. Designing a complex structure is a fun and interactive task, even in a text editor. The absense of much syntax makes authorship feel fluid, and I can keep my mind on my creative work.

### examples

JSON looks like this, as we all know:

    {
        "someDict": {
            "cow": {
                "says": "moo",
                "source": "mouth"
            }
            "cat": {
                "say": "meow",
                "source": "mouth"
            }
        }
        "someList": [
            "first", 1, true
        ]
        "someValue": "bug"
    }

It's not very friendly to composition by hand. Minimal humon looks pretty close, but without some nits:

    {
        someDict: {
            cow: {
                says: moo
                source: mouth
            }
        } {
            cat: {
                says: meow
                source: mouth
            }
        }
        someList: [
            first 1 true
        ]
        someValue: bug
    }

No quotes except where necessary, and no commas required (they're optional, and treated like whitespace). So that's kind of nice. More useful humon might look like this:

    @ engineVersion: 0.1.0
    assets: {
        brick-diffuse: {
            src: [brick-diffuse.png]
            importData: {
                type: image
                extents: [0 0 0]   # extract from src
                numMipLevels: 0    # all the mips
                format: R8G8B8A8Unorm # there should be a way to say "whatever we're using, man" Maybe "Unknown".
            }
        }
    }

The `@` syntax is an *annotation*. We'll discuss that, but it's one of the only real syntax-y parts of humon, and completely optional.

Using the APIs is straightforward. To get values above, we might invoke the following in C:

    #include <humon.h>
    ...
        huTrove_t * trove = huMakeTroveFromStringZ(NULL, humonStr, 4, 4);
        huNode_t * node = huGetRootNode(trove);
        node = huGetChildByKeyZ(node, "someList");
        node = huGetChildByIndex(node, 0);
        huStringValue_t * sVal = huGetValue(node); // has str and size

or in C++:

    #include <humon.hpp>
    ...
        auto trove = hu::Trove::fromFile("config.hu");
        auto intVal = trove / "someList" / 1 / hu::value<int>{}; // returns int

### installation
If you're programming with humon, or rather want to be, [here's how we install it, and do all the CMake yakkity smack]. If you're just using humon files with some app, you don't need to do anything. Also here's a VSCode colorizer.

## principles of the design
We're going to keep referencing JSON, as it's the model that humon takes after. But we want it to be better, so the obvious thing to do is just fix the problems with JSON. Once we commit to designing a new format / language, we're free to make the rules. Rules should come from guiding principles:

1. **We are serving humans with this language.**
Human authors and maintainers are one of the two primary customers. Reading, modifying, and writing humon data in an editor should be as simple and approachable as possible. It must never come as a surprise what the language thinks you mean, or what is legal to state in the language. A minimal syntax promotes this principle. Humon files must be able to be large, storing lots of kinds of data, free to fulfill human needs tersely.
1. **We are serving machines with this language.**
Computing machines are the other primary customer. Software written against the language must run quick, and the language must promote simple interfaces and consistent operation. There must be adequate facilities for programmed machines to utilize the data exposed by humon. Resource usage for grokking humon objects must be reasonable.
1. **We are serving humans who are serving machines with this language.**
The APIs are the conjunction of human and machine. The machine has been taught how to use them. We learn the same. API design must be convenient and unsurprising, consistent and unspecialized.

### principles in the language
No more quotes. Commas are optional. You can use any quote character `' " \``. Values are just character data. Quoted values can contain newlines. Annotations and conmments can provide "human hooks" into the data without the need to muck with the structure design, and without polluting the data structure expected by an extant application. Humon files are *intended* to be authored and modified in text editing tools, by humans.

### principles in the APIs
The base API is a C interface. Everything a human or machine needs to traverse the data heirarchically or by search is there and easy to use. Mainly it exists for other language bindings, but this makes humon available to C code as well.

There is a C++ layer which wraps the C API, giving syntactic sugar and a more accessible interface. You'll almost certainly prefer it over the C API for C++ projects.

Language bindings forthcoming include Python, Node, Rust, and .NET.

## the language
I happen to love me some JSON. To my mind, it has the best syntax for describing structured data. It's incredibly expressive, incredibly simple, and open to just about any practical use. There are so many structures, descriptions, configurations, and data models that can be faithfully described by a JSON file, with very little plumbing. The array- and dictionary-style collections are generally sufficient to describe a huge swath of problems. (Some structures, like graphs, are harder to model in a heirarchy. Using labels helps bridge some gaps, but graphs are a problem in any serial text format.)

But, as we all know, JSON has some creaky bits. Some of them are merely annoying, like nitpicky punctuation--during an editing session, this is Flowus Interruptus. But some of JSON's shortcomings inhibit some practical use cases, like the lack of commenting or other metadata that can take you "out of the structure". Editing apps and other environments oftentimes include their own custom JSON parser, just to accept comments in configuration files, say.

I mean, JSON wasn't designed for these things. It was made to communicate between JavaScript processes over a wire. It doesn't *have* to be these things for me to appreciate what it can do. But, that doesn't mean it should be the thing to do the things we do make it do. Let's design one language that can always do that, and serve all its customers.

So, this project proposes a replacement, or suggests a good philosophy for such anyway, for those tasks that JSON is performing but shouldn't. Specifically, those spaces where humans interact with the structured data, and those places where we could be but are maybe inhibited by inconvenience or a rigid format like .ini, or something complex like YAML.

### some terms
`humon`: The language, API, implementation of the API, files or data streams that contain expressions of data in the language, or that data.

`trove`: A parsed, in-memory representation of humon data which stems from a single root node.

`node`: A unit of structure in a humon heirarchy. There are three kinds of nodes: lists, dictionaries (dicts), and values. Lists and dicts can store any other kinds of nodes as children.

`token`: A word or sequence of words in a humon file, which are meaningful to humon and you at a graunular level. `{` is a token, as is a value like `"foo"`, as is a whole comment like `/* I am just an utterance. */`. Every token is owned either by a single node, or (in special cases) by the trove.

`list`: A node that contains a sequential collection of nodes as children.  In the language, the list is the bounding `[ ]`s around the child nodes. In the API, a list's children can be accessed by index.

`dict`: A node that contains a sequential collection of nodes as children, where each node is associated with a key. In the language, the dict is the bounding `{ }`s around the child nodes, each of which must be preceded by a key and `:`. In the API, a dict's children can be accessed by index, or by key.

`key`: An associated key for a dict's child. It must be a string of nonzero length. There is no maximum length defined.

`value`: A node that contains a single string value. All values are strings.

`comment`: A string of words that are not exposed as values in nodes, but are parsed and searchable within a trove.

`annotation`: One of a collection of key:value pairs applied to a node or a trove. They are exposed as special values in nodes, and are globally searchable by key and value.

### the principles applied to the language

#### brevity and simplicity
Punctuation in humon consists entirely of:

    [ ] { } : " ' ` @

There are three kinds of "nodes" in a humon file: lists, dictionaries (dicts), and values as strings. These correspond to the same object kinds in JSON, conceptually: A list stores a plurality of other nodes in ordered sequence. A dictionary stores a plurality of nodes in ordered sequence, and associated with string keys. This structure makes for the obvious object heirarchy.

#### it works like you expect a smart thing to work
For the most part, it looks like JSON structure, and nothing about it is surprising. However, there is one relatively smart aspect of the humon spec, to do with how comments are associated to related nodes. We'll discuss the finer points of comments later, but broadly, comments can be searched-for, and their associated nodes referenced by APIs. Humon has smarts about associating comments as they appear in a file, relative to other tokens.

#### the hu in humon
You're the human. Use it the way you want.

Humon is intended to be used to describe data in almost any structure. Few problems I can think of cannot be described by a structure like humon (or JSON [or YAML [[or XML]]]). Humon seeks to be the least verbose of these, only stating the minimum about the structure it holds, and allowing the problem space it models to be whatever the human--you--want it to be.

To that end, Humon makes no assumptions about the meaning of values. There are no keywords. There are no special meanings to any tags or annotations or the text of comments. Humon has little understanding of anything, really, which is *freeing*. It's up to the application using Humon to decide just about anything about structure without worrying about serialization. I mean, just like JSON does. Nothing revolutionary there, but the syntax is *human-usable*.

### humon language rules
Okay, let's state some explicit rules, then.

**A trove contains zero or one root node.**
A blank file, or a file with only comments and annotations (but no structures or values) contains no root node. Any other file must contain only one top-level node, probably with many children.

**The root node is what it is, which is what you say it is.**
There is no assumption made about the kind of the root node. If the first non-comment token in your humon stream or file is a '{', then the root is a dict; a '[' starts a list. If it's a value, then it's the only one allowed, since value nodes don't have children.

**List are enclosed in [square brackets].**
Any object kind can occupy any entry in any list. Lists don't have to contain objects of any one kind; mix and match as you see fit.

**Dicts are enclosed in {curly braces}.**
Each dict entry must be a key:object pair. The value portion can be any kind of object. Each key in a dict must be unique.

**Keys must be strings.**
They can be numbers of course, but that's just a string to humon. Other language bindings like ES6 might interpret them their own way, in their own sensible context. But as written, a key as a token must be a sequence of value characters.

**Values are strings.**
A value is a contiguous non-punctuation, non-whitespace string of characters, or a quoted string of characters.

For non-quoted values, the first whitespace or punctuation character encountered ends the string, and begins a new token. This obviously includes newlines.

You can use any quote character (', ", `) to begin a string. It doesn't matter which you use, as they all work the same way. A quoted string value starts at the quote (not after it), and ends at the next corresponding, *unescaped* quote. The string can contain backslash-escaped quotes, like in most programming languages, and treats other kinds of quotes as characters:

    'This isn\'t an "uncommon" example.'

Quoted string values can span multiple lines of text. Newlines are included in the value.

    "Fiery the angels rose, and as they rose deep thunder roll'd.
    Around their shores: indignant burning with the fires of Orc."

**Whitespace is ignored.**
The tokenizer ignores whitespace that isn't contained in a quoted string. Whitespace characters are not captured by tokens.

**Commas are whitespace.**
Commas are ignored by the tokenizer, like whitespace. They are completely optional. The following humon objects are identical:

    [resistors caps ICs diodes MOSFETs]
    [resistors,caps,ICs,diodes,MOSFETs]
    [resistors, caps, ICs, diodes, MOSFETs]
    [resistors, caps, ICs, diodes, MOSFETs, ]
    ,,,[,resistors, 
    caps    ,
         ICs, ,
                 diodes 
               MOSFETs,,,,,,, ],,,

**Comments have no meaning.**
Humon doesn't know or care about your comments. Humon commentary is just noise.

**Annotations are their own thing, and mean nothing.**
Annotations are the only sort of language-y syntax-y components of humon. The first specs didn't include them, but as I used humon in those early days, I found that I wanted a convenient shorthand that sort of "jumped out of the structure" to provide context data, or specify certain rare conditions. Something formal and machineable, but that also didn't force modifications to the structure in place. I settled on annotations.

Any node can have any number of annotations. The trove can have them too, if an annotation appears before any other objects. Annotations begin with an `@` symbol, followed by either one key:value pair, or a dict of key:value pairs:

    [
        nostromo @movie: alien
        sulaco @{movie: aliens director: cameron}
    ]

The values must be value strings only; an annotation can't be a collection. That way lies some madness.

Unlike dict entries, you can have multiple annotation values associated with the same key:

    ripley @ movie: alien @ movie: aliens @ movie: "alien 3" @ movie: "alien 4"

The APIs can return all of the values above by the `movie` key and an index.

Practically, annotations aren't a necessary part of humon. All their data could be baked into objects themselves, and there was a time when humon didn't have them. But they provide a way to mark up old data without modifying the structure, and provide out-of-band associations that allows a humon app to be almsot as free-form as humon itself. They're recommended over comments as associated, searchable metadata.

## the C-family programming interfaces
There are API specs for the C and C++ interfaces. The C API is pretty basic, but fully featured. The C++ API mainly wraps the C API, but also provides some nice features for more C++ish fun-time things.

### the principles applied to the C-family APIs
Maybe you'd call them behaviors, but they embody the humon principles.

**Humon supports UTF8.**
UTF8 is quickly becoming the ubiquitous Unicode encoding, even in Asian locales. Humon supports UTF8 only, with or without BOM. Support for UTF16 and UTF32 may be considered in the future, but their relative worldwide use is pretty small. Humon respects all the whitespace characters that Unicode specifies, and supports all code points.

A code point is either whitespace, language punctuation (which is only ever a single byte), or a word-token character. That's all the tokenizer understands.

If this is all Greek to you, just rest assured that any modern text editor or web server or web browser reads and writes UTF8, and can handle foreign language text and emojis, and humon speaks that encoding too.

**CRLF-style newlines (often made in Windows or Symbian OS) are regarded as one newline object.**
This is mostly for recording line numbers in token objects, for error reporting and other things that need token placement information. In general, it does what you expect.

**All non-whitespace characters are part of some token.**
Only whitespace characters like spaces, newlines, commas, and quote characters around keys and values are discarded from tracking in the tokenizer.

**All tokens are part of some node.**
During a load, a humon file is tokenized into a list of token objects, and then those tokens are parsed into a node hierarchy. Every single token object is owned by exactly one node, or by the trove itself in a few cases. The API implementation can completely reconstruct a humon file from nodes and tokens, including reconstructing comments and annotations.

**All keys and values are strings.**
As stated, humon makes no assumptions about the data type of a value. `"True"`, `"true"`, and `true` are all the same type of thing to humon: a four-character string. (Though, of course, case is preserved and considered when searching.)

Usually boolean and numeric values are computational though, and matter to the application using humon. Convenience APIs are included in some language bindings to parse values (and in some cases let you define your own parsers).

**Humon doesn't copy data for reading.**
Some of the design decisions stem from the implementation: Humon doesn't copy data when loading and exposing keys and values from a file. It stores the file contents itself, and returns pointers to that content. This means several things:
1. Raw string values contain escapes. They do not contain any surrounding quotes. If strings contain CRLF newlines in the file, they'll appear that way in raw string accesses too.
1. Raw string values are still UTF8-encoded.
1. Troves are immutable. Operations that insert text or nodes into a trove actually create a new trove, with a new (final) text string and new token and node data. (These ops are coming soon.)
1. Accessing raw value data is quick. You basially get a pointer and size. Since the data behind it doesn't move or change, that value pointer is good for the life of the trove.
1. The whole file must be in contiguous memory. This is currently true at least, and means loading a whole file before tokenizing and parsing. It's unfortunate for concurrency, and I'm thinking about it. For most use cases, this isn't a real issue.
1. When serializing back to another stream or file from a humon trove, the exact source can be spit out without any conversion, since the original string is still in memory.

**The C-family APIs don't signal by default; rather, they return null objects.**
Humon's initial use case was in the context of C++. C++ can deal with exceptions just fine, and I don't mind them, but they're not always appropriate. I wanted a generally `noexcept` option, at least for most of the API. When querying a trove or node, bad indices or keys do not cause exceptions; rather, they return special object instances which represent null values. You can query these objects and get more null objects (really, the same null objects), and get legal (but empty) strings.

There is a value in exceptions, and I'm considering providing an option to let the APIs throw on bad inputs. Perhaps one could enable it in debug builds. It's on the list.

**Objects in dicts remain in serial order.**
This is different from some JSON libraries, that don't preserve the order of key-object pairs in dicts. Humon guarantees that, when you access a dict's children by index (like you would a list), they'll be returned in the order you expect. Humon can maintain an extra table for accessing dict entries quickly by key. (Currently this is not implemented, and keys are searched in linear order. This is obviously on the fix-it list, but it's often the case that a linear search beats hashtables for small numbers of entries and small keys. <sup>[citation needed]</sup>)

**Comments have no meaning, but they do have context.**
Humon doesn't know or care about your comments, as already stated in the language rules. But it doesn't discard them in the parsing process; they're preserved for searching and reserialization.

All comments are associated to either a node in the heirarchy, or the trove itself. Humon tries to be smart about assigning comments. Here's an example; the comments indicate to which entity they belong:

    // root dict node
    {   // root dict node
    // element node
        key     // element node
            :   // element node
    // element node
            value // element node
        // root dict node
    }   // root dict node
    // trove

You can get the comments on any node or the trove, and you can search for a comment and get its associated node back. Comments are associated to objects per the following rules:

1. Any comment that appears on a line of text that contains no other non-comment tokens is *associated forward*. That means the next non-comment token encountered in the token stream specifies the owner of the comment. If there are no non-comment tokens left in the stream, the comment applies to the trove. These are mainly for whole-line comments that act as headers.

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

These rules allow you to write comments naturally within the structure of the file, and their associations will be what you'd expect a human to interpret as well.

If you're never searching by comments, or are ignoring them altogether, then none of these rules really matters. But for such a mission, the associations can be handy.

Note: Usually you would prefer annotations for searchable metadata, as comments are kind of "more optional" and likely more ephemeral--nobody expects comments to be required in any file, for machining purposes. But I'm not here to tell you how to live your life.

**Annotations have context.**
Like comments, annotations are associated with nodes or the trove. You can access annotations by key from a node, or search all annotations by key or value and get their associated nodes.

Annotations are always *associated backward*. They modify the nearest non-comment node that appeared before the annotation, regardless of same-lineness. Any amount of whitespace or comments in between is ignored, as usual. If the previous token was a collection-starter or collection-ender (`[`, `{`, `]` or `}`), it modifies that list or dict. In the following, `testAcct:true` annotates the player dict, not the userId value:

    {
        player: {
            @testAcct: true
            userId:     int
            username:   string,
            friends:    vector<string>
        }
    }

If no nodes appear before an annotation, it applies to the trove. A great way to begin a humon file spec is by specifying an application version or config version for which the file is good. Maybe you're developing a code-generation suite cleverly called "hudo", and want to ensure the correct version of the engine is called to consume every model file, even older ones. Hudo could look for a version annotation on the trove, before examining any nodes, and use the appropriate engine version to interpret a structure:

*playerData*:

    @ {app: hudo, version: 0.1.1}

    {
        player: { @testAcct: true
            userId:     int
    ...

*loadPlayer.cpp:*

    auto version = trove.getAnnotation("app") == "hudo"
                 ? Version<3> { trove.getAnnotation("version") }
                 : Version<3> { };
    if (version < Version { 0, 1, 0 }) { ... }
    else if (version < Version { 0, 2, 0 }) { ... }
    else ...

Like asserted earlier, annotations are 100% open in their use. Humon doesn't use any annotaton keys or values and doesn't interpret them. Applications can use them or not, but all annotations that are legal are guaranteed to be parsed, even if the application doesn't know about or use them at all. In this way you can embed metadata about objects in the humon file, and even old versions of humon apps will correctly read (and ignore) them.

**Serializing back to stream comes with options.**
The API allows for serializing a trove back to memory or a stream. There are three options to serialize humon objects:
1. **Preserve**: A direct copy of the original file is produced, including all comments and commas and whitespace. Just a brainless memory slam.
1. **Minimal**: This reduces whitespace to at most one character each to pare down length. Humon does as much as it can, but if you choose to preserve comments in the minified output, you may notice that not all newlines get replaced. This is because the next read operation on the resultant file must replicate the comment associations from the original, and that requires some comments be on their own line. Also, C++-style //comments end in a newline, and tokens after must be on their own line, so those newlines are also preserved.
1. **Pretty**: This produces a clean, indented, possibly colorized, eminently readable string.

Each of these options allows you to remove comments (perhaps for security), and also provide a color table for syntax highlighting. This is useful for producing colored output for a text-based shell, or perhaps HTML.

## python
A Python3 module is forthcoming, which will consome and produce humon text.

## node
A node module is forthcoming, which will consome and produce humon text.

## rust
A rust module is forthcoming, which will consome and produce humon text.

## .NET
A .NET module is forthcoming, which will consome and produce humon text.
