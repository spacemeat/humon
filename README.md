# Humon

*Human-Usable Machine Object Notation*

*Note: Certain language in this document assumes you are human.*

## Introduction

To talk about Humon is to describe any of three things:
1. A general text-based, structured language format similar to JSON,
1. some programming interfaces that understand it, and
1. a reference implementation of said interface with bindings to several programming languages.

Mainly, it takes some of the roles that JSON has been taking lately, but with an eye towards the human side of the experience.

I use it for design. It's a great mind-mapping tool. I use it to model database tables, serialized structure definitions, app configuration. Designing a complex structure in Humon is a fun and interactive task, even in a text editor. The absense of much syntax makes authorship feel fluid, and I can keep my mind on my creative work.

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

No quotes except where necessary, and no commas required (they're optional, and treated like whitespace). So that's kind of nice. Strict JSON is fussy about all that punctuation.

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
        huTrove const * trove = huMakeTroveFromFileZ("src/samples/materials.hu", 4);
        if (trove != hu_nullTrove && huGetNumErrors(trove) == 0)
        {
            huNode const * extentsNode = huGetNodeByFullAddressZ(trove, "/assets/brick-diffuse/importData/extents");
            huNode const * valueNode = huGetChildByIndex(extentsNode, 0);
            huStringView const * sExt = valueNode ? & valueNode->valueToken->value : NULL;
            int extX = sExt ? strntol(sExt->str, sExt->size, NULL, 10) : 0;
            valueNode = huGetChildByIndex(extentsNode, 1);
            sExt = valueNode ? & valueNode->valueToken->value : NULL;
            int extY = sExt ? strntol(sExt->str, sExt->size, NULL, 10) : 0;
            ...

or in C++:

    #include <Humon.hpp>
    ...
        if (auto trove = hu::Trove::fromFile("src/samples/materials.hu");
            trove)
        {
            auto extentsNode = trove.getNode("/assets/brick-diffuse/importData/extents"sv);
            tuple xyExtents = { extentsNode / 0 / hu::value<int>{}, 
                                extentsNode / 1 / hu::value<int>{} };
            ...

### Installation
If you're programming with Humon, or rather want to be, [here's how we install it, and do all the CMake yakkity smack]. If you're just using Humon files with some app, you don't need to do anything. Also here's a TextMate colorizer.

## Design principles
We're going to keep referencing JSON, as it's the model that Humon takes after. But we want it to be better, so the obvious thing to do is just fix the problems with JSON. Once we commit to designing a new format / language, we're free to make the rules. Rules should come from guiding principles:

1. **We are serving humans with this language.**
Human authors and maintainers are one of the two primary customers. Reading, modifying, and writing Humon data in an editor should be as simple and hassle-free as possible. It must never come as a surprise what the language thinks you mean, or what is legal to state in the language. A minimal syntax promotes this principle. Humon token streams must be able to be large, storing lots of kinds of data, free to fulfill human needs tersely.
1. **We are serving machines with this language.**
Computing machines are the other primary customer. Software written against the language must run quick, and the language must promote simple interfaces and consistent operation. There must be adequate facilities for programmed machines to utilize the data exposed by Humon. Resource usage for grokking Humon objects must be reasonable.
1. **We are serving humans who are serving machines with this language.**
The APIs are the conjunction of human and machine. The machine has been taught how to use them. We can learn the same. API design must be convenient and unsurprising, consistent and unambiguous.

### Principles in the language
No more quotes. Commas are optional. You can use any quote character `` ' " ` ``. Values are just text data. Quoted values can contain newlines. Annotations and conmments can provide "human hooks" into the data without the need to muck with the structure design, and without polluting the data structure expected by an extant application. Humon files are *intended* to be authored and modified in text editing tools, by humans, while also simple enough for a machine to generate as well.

### Principles in the APIs
Most of the interesting use cases for Humon involve a machine *reading* data authored by either machines or humans. The performance of this implementation reflects this; loaded Humon troves are *immutable*. Searching, scanning, or scumming through the data is quick, and references never go bad for the life of the trove.

The base API is a C interface. Everything a human or machine needs to traverse the data heirarchically or by search is there and easy to use. Mainly it exists for other language bindings, but this makes Humon available to pure C code as well.

There is a C++ layer which wraps the C API, giving syntactic sugar and a more accessible interface. You'll almost certainly prefer it over the C API for C++ projects.

Language bindings forthcoming include Python, Node, Rust, and .NET.

## The language
I happen to love me some JSON. To my mind, it has the best syntax for describing structured data. It's incredibly expressive, incredibly simple, and open to just about any practical use. There are so many structures, descriptions, configurations, and data models that can be faithfully described by a JSON object, with very little plumbing. The array- and dictionary-style collections are generally sufficient to describe a huge swath of problems. (Some structures, like graphs, are harder to model in a heirarchy. Using labels helps bridge some gaps, but graphs are a problem in any serial text format.)

As we all know, JSON has some creaky bits. Some of them are merely annoying, like nitpicky punctuation--during a creative editing session, worrying about maintaining commas in just the right places while developing the structure is Flowus Interruptus. But some of JSON's shortcomings inhibit some practical use cases, like the lack of commenting or metadata. Editing apps and other environments oftentimes include their own custom JSON parser, just to accept comments in configuration files, say.

I mean, JSON wasn't designed for these things. It was made to communicate between JavaScript processes over a wire. It doesn't *have* to be these things for me to appreciate what it can do. But that doesn't mean it should be the thing to do the things we do make it do. Let's design a language that can always do that, and serve all its customers.

So there's YAML. It basically does what I want but its syntax is complex, and has spatial requirements that I feel are binding. Just more syntax to manage, and it's harder to modify its structure quickly. Humon is more fluid and flexible, and that allows for quick changes in a text editor.

So, this project proposes a replacement, or suggests a philosophy for such anyway, for those tasks that JSON is performing but shouldn't, and YAML might be performing if it was more accessible. Specifically, those spaces where humans interact with structured data, and those places where we could be, but are inhibited by inconvenience or a rigid format like .ini.

### Some terms
`Humon`: The format, API, implementation of the API, files or data streams that contain expressions of data in the format, or that data.

`trove`: A tokenized and parsed, in-memory representation of Humon data which stems from a single root node, and against which APIs are programmed.

`token`: A word or sequence of words in a Humon token stream, which are meaningful to Humon and you at a graunular level. `{` is a token, as is a value like `"foo"`, as is a whole comment like `/* I am just an utterance. */`. Every token is owned either by a single node, or (in special cases) by the trove itself. A token array is maintained by the trove, and provides content and position information (row, column) for each token, if that sort of thing is interesting to you.

`token stream`: The text of a Humon file or in-memory text string. I use this language to refer to Humon text content that is tokenized and parsed into a trove.

`node`: A unit of structure in a Humon heirarchy. There are three kinds of nodes: lists, dictionaries (dicts), and values. Lists and dicts can store any kinds of nodes as children. Nodes are the abstraction of the structure. The trove maintains an array of nodes parsed from the token array.

`list`: A node that contains a sequential collection of zero or more nodes as children.  In the language, the list is the bounding `[ ]` around the child nodes. In the API, a list's children can be accessed by index.

`dict`: A node that contains a sequential collection of zero or more nodes as children, where each node is associated with a key. In the language, the dict is the bounding `{ }` around the child nodes, each of which must be preceded by a key and `:`. In the API, a dict's children can be accessed by index or by key.

`key`: An associated key for a dict's child. A key must be a string of nonzero length. Within a dict, keys must be unique. There is no maximum length defined, and keys can be quoted. (Quoted values can be multline strings, but that's a strange way to spec a key.) Any UTF8 code point that is not whitespace or punctuation counts as a key character.

`value`: A node that contains a single string value. Values can be quoted; quoted values can be multline strings. As with keys, any UTF8 code point that is not whitespace or punctuation counts as a value character.

`comment`: A string of words that are not exposed as values in nodes. They're mainly for humans to make notes, but there are APIs to search for comments and get their associated nodes, in case that's useful.

`annotation`: One of a collection of key:value pairs associated to a node or a trove. They are exposed as special values in nodes, and are globally searchable by key and/or value. Any node can have any number of annotations, as long as they have unique keys (like in a dict).

### The principles applied to the language

#### Brevity and simplicity
Punctuation in Humon consists entirely of:

    [ ] { } : " ' ` @

There are three kinds of "nodes" in a Humon token stream: lists, dictionaries (dicts), and values. These correspond to the same object kinds in JSON, conceptually: A list stores a plurality of other nodes in ordered sequence. A dictionary stores a plurality of nodes in ordered sequence, and associated with string keys. A value stores a single string value. This structure allows for an arbitrary object heirarchy.

#### It works like you expect a smart thing to work
For the most part it looks like JSON, and nothing about it is surprising. However, there is one relatively smart aspect of the Humon spec, to do with how comments are associated to related nodes. We'll discuss the finer points of comments later but broadly, comments can be searched for, and their associated nodes referenced by APIs. Humon has smarts about associating comments as they appear in a token stream, relative to other tokens, in the same way your brain probably does.

Mmm, brains.

#### The hu in Humon
You're the human. Use it the way you want.

Humon is intended to be used to describe data in almost any structure. Few problems I can think of cannot be described by a structure like Humon (or JSON [or YAML {or XML}]). Humon seeks to be the least verbose of these, only stating the minimum about the structure it holds, and allowing the problem space it models to be whatever the human--you--want it to be.

To that end, Humon makes no assumptions about the meaning of values. There are no keywords. There are no special meanings to any tags or annotations or the text of comments. There are no automatic annotations for special nodes or anything. Nothing for you to memorize. Humon has little understanding of anything at all, which is *freeing*. It's up to the application using Humon to decide anything about structure without worrying about stumbling on some language rule.

I mean, JSON does all this too. There's nothing revolutionary here, but Humon's syntax is *human-usable*, in an editing context.

### Humon language rules
Okay, let's state some explicit rules, then.

**A trove contains zero or one root node.**
A blank token stream, or one with only comments and annotations (but no structures or values) contains no root node. Otherwise it must contain one top-level node, probably with many children.

**The root node is what it is, which is what you say it is.**
There is no assumption made about the kind of the root node. If the first non-comment token in your token stream is a `{`, then the root is a dict; a `[` starts a list. If it's a value, then it's the only one allowed, since value nodes don't have children. (Not a terribly useful Humon trove, but legal. Eh.)

**Lists are enclosed in [square brackets].**
Any kind of node can occupy any entry in any list. Lists don't have to contain nodes of a single kind; mix and match as you see fit.

**Dicts are enclosed in {curly braces}.**
Each dict entry must be a key:node pair. The node can be of any kind. Each key in a dict must be a string, and unique within the dict. There is no delimiting syntax between key:object pairs; just whitespace (maybe including commas).

**Keys must be strings.**
They can be numbers of course, but that's just a string to Humon. As written, a key as a token must be a sequence of value characters, quoted or unquoted, with the same rules as value node strings.

**Values are strings.**
A value is a contiguous non-punctuation, non-whitespace string of characters, or a quoted string of characters.

For non-quoted values, the first whitespace or punctuation character encountered ends the string, and begins a new token. This obviously includes newlines.

You can use any quote character (`'`, `"`, `` ` ``) to begin a string. It doesn't matter which you use, as they all work the same way. A quoted string value starts after the quote, and ends just before the next corresponding, *unescaped* quote. The string can contain backslash-escaped quotes, like in most programming languages, and treats other kinds of quotes as characters:

    'This isn\'t an "uncommon" example.'

Quoted string values can span multiple lines of text. Newlines are included in the value.

    "Fiery the angels rose, and as they rose deep thunder roll'd.
    Around their shores: indignant burning with the fires of Orc."

**Whitespace is ignored.**
The tokenizer ignores whitespace that isn't contained in a quoted string. Whitespace characters are not captured by tokens.

**Commas are whitespace.**
Commas are ignored by the tokenizer as whitespace. They are completely optional. The following Humon objects are identical:

    [resistors caps ICs diodes MOSFETs]
    [resistors,caps,ICs,diodes,MOSFETs]
    [resistors, caps, ICs, diodes, MOSFETs]     // foo
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

I find myself sometimes using commas when placing list or dict elements on one line, as on line `foo` above, and not using them elsewise. In other words, what comes natural to my typing is legal Humon, and requires no grammarizing. I like designing structure without worrying about the punctuation; indeed, this little nit was one of the main motivations behind Humon's development.

**Comments have no meaning.**
Humon doesn't know or care about your comments. Humon commentary is just noise. But because they have meaning to humans, they are not discarded; see below.

**Annotations are their own thing, and exist out of band.**
Annotations are the only sort of language-y syntax-y feature of Humon. The first specs didn't include them, but as I used Humon in those early days, I found that I wanted a convenient shorthand that sort of *jumped out of the structure* to provide context data, or specify certain rare conditions. Something formal and machineable, but that also didn't force modifications to the structure in place. I settled on annotations.

Any single node can have any number of annotations. The trove can have them too, if an annotation appears before any other objects. Annotations begin with an `@` symbol, followed by either one key:value pair, or a dict of key:value pairs:

    [
        nostromo @ movie-ref: alien
        sulaco @{movie-ref: aliens, movie-director: cameron}
    ]

Annotation values must be value strings only; an annotation can't be a collection. That way lies some madness.

Like dict entries, annotation keys must be unique among annotations associated to a particular node. Different nodes in a trove can have annotations with identical keys though.

Practically, annotations aren't a necessary part of Humon. All their data could be baked into objects themselves, and there was a time when Humon didn't have them. But they are useful for a number of things.

## The C-family programming interfaces
There are API specs for the C and C++ interfaces. The C API is pretty basic, but fully featured. The C++ API mainly wraps the C API, but also provides some nice features for more C++ish fun-time things.

### The principles applied to the C-family APIs
Maybe you'd call them behaviors, but they embody the Humon principles.

**Humon supports UTF8.**
UTF8 is quickly becoming the ubiquitous Unicode encoding, even in Asian locales. Humon supports UTF8 only, with or without BOM. Support for UTF16 and UTF32 may be considered in the future, but their relative worldwide use is pretty small. Humon respects all the whitespace characters that Unicode specifies, and supports all code points.

A code point is either whitespace, language punctuation (which is only ever a single byte), or a word-token character. That's all the tokenizer understands.

If this is all Groot to you, just rest assured that any modern text editor or web server or web browser reads and writes UTF8, and can handle any language text and emojis, and Humon speaks that encoding too.

**CRLF-style newlines (often made in Windows or Symbian OS) are regarded as one newline object.**
This is mostly for recording line numbers in token objects, for error reporting and other things that need token placement information. In general, it does what you expect.

**All non-whitespace, non-quote characters are part of some token.**
Only whitespace characters like spaces, newlines, commas, and quote characters around keys and values are discarded from tracking in the tokenizer.

**All tokens are part of some node, or the trove.**
During a load, a Humon token stream is tokenized into a list of token objects, and then those tokens are parsed into a node hierarchy. Every single token object is owned by exactly one node, or by the trove itself in a few cases. A trove can completely reconstruct a Humon token stream from the nodes and tokens, including reconstructing comments and annotations with their appropriate associations.

**All keys and values are strings.**
As stated, Humon makes no assumptions about the data type of a value. `"True"`, `"true"`, and `true` are all the same type of thing to Humon: a four-character string. (Though, of course, case is preserved and considered when searching.)

Usually boolean and numeric values are computational though, and matter to the application using Humon. Convenience APIs are included in some language bindings to parse values (and in some cases let you define your own parsers).

**Humon objects are immutable.**
Once loaded, a Humon trove does not move or change. This has implications:
1. Accessing raw value data is quick. You basially get a pointer and size. Since the data behind it doesn't move or change, that value pointer is good for the life of the trove.
1. Raw string values returned by APIs are not NULL-terminated. (In C++, they translate directly to `std::string_view`s.)
1. Raw string values contain escaped characters. They do not contain any surrounding quotes. If strings contain CRLF newlines in the token stream, they'll appear that way in raw string accesses too.
1. Raw string values are still UTF8-encoded.
1. Operations that insert text or nodes into a trove actually create a new trove, with a new text string and new token and node data. (These ops are coming soon.)
1. The whole token stream must be in contiguous memory. This is currently true at least, and means loading a whole token stream before tokenizing and parsing. It's unfortunate for concurrency, and I'm thinking about it. For most use cases like app configurations or database schemas, this isn't a real issue.
1. When serializing back to another stream or file from a Humon trove, the exact source can be spit out without any conversion, since the original string is still in memory. This is the most performant way to generate a Humon token stream from a trove.

**The C-family APIs don't signal by default; rather, they return null objects.**
Humon's initial use case was in the context of C++. C++ can deal with exceptions just fine, and I don't mind them, but they're not always appropriate. I wanted a generally `noexcept` option, at least for most of the API. When querying a trove or node, bad indices or keys do not cause exceptions; rather, they return null results.

There is a value in exceptions, and I'm considering providing an option to let the APIs throw on bad inputs. Perhaps one could enable it in debug builds. It's on the list to consider.

**Objects in dicts remain in serial order.**
This is different from some JSON libraries, that don't preserve the order of key-object pairs in dicts. Humon guarantees that, when you access a dict's children by index (like you would a list), they'll be returned in the order you expect. Humon can maintain an extra table for accessing dict entries quickly by key. (Currently this is not implemented, and keys are searched in linear order. This is obviously on the fix-it list, but it's often the case that a linear search beats hashtables or binary searches for small numbers of entries.<sup>[citation needed]</sup>)

**Comments have no meaning, but they do have context.**
Humon doesn't know or care about your comments, as already stated in the language rules. But it doesn't discard them in the parsing process; they're preserved for searching and reserialization.

All comments are associated to either a node in the heirarchy, or the trove itself. Humon tries to be smart about associating comments. Here's an example; the comments indicate to which entity they associate:

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

If you're never searching by comments, or are ignoring them altogether, then none of these rules really matter. But for such a mission, the associations can be handy. IDEs and document generators could machine on comments for smart document summaries, for example.

Note: Usually you would prefer annotations for searchable metadata, as comments are kind of "more optional" and likely more ephemeral--nobody expects comments to be required in any file, especially for machining purposes, and it may not occur to someone to maintain them for functional reasons. But I'm not here to tell you what to do.

**Annotations have context.**
Like comments, annotations are associated with nodes or the trove. You can access annotations by key from a node, or search all annotations by key or value and get their associated nodes.

Annotations are always *associated backward*. They modify the nearest non-comment node that appeared before the annotation, regardless of same-lineness. Any amount of whitespace or comments in between is ignored, as usual. If the previous token was a collection-starter or collection-ender (`[`, `{`, `]` or `}`), it modifies that list or dict node. In the following, `remoteStorage:true` annotates the player dict, not the userId value, because it follows the player dict's `{` opener:

    {
        player: {
            @remoteStorage: true
            userId:     int
            username:   string
            friends:    { type:vector of:string }
        }
    }

(Personally, I would place the annotation on the same line as the `{`, for clarity.)

If no nodes appear before an annotation, it applies to the trove. A great way to begin a Humon file spec is by specifying annnotations for the application version or config version for which the file is good. Maybe you're developing a code-generation suite cleverly called "hudo", and want to ensure the correct version of the engine is called to consume every model file, even older ones. Hudo could look for a version annotation on the trove, before examining any nodes, and use the appropriate engine version to interpret the structure:

*hudo.hu:*:

    @ { app: hudo, hudo-version: 0.1.1 }

    {
        player: { @remoteStorage: true
            userId:     int
            username:   string
            friends:    { type:vector of:string }
        }
    }

*loadModel.cpp:*

    if (auto trove = hu::Trove::fromFile("src/samples/hudo.hu");
        trove)
    {
        if (trove.getAnnotationWithKey("app") != "hudo"sv)
            { throw runtime_error("File is not a hudo file."); }

        auto versionString = trove.getAnnotationWithKey("hudo-version");
        auto version = Version<3> { versionString };
        if (version < Version<3> { 0, 1, 0 }) { ... }
        else if (version < Version<3> { 0, 2, 0 }) { ... }
        else ...

Like asserted earlier, annotations are 100% open in their use. Humon doesn't use any annotaton keys or values and doesn't interpret them. Applications can use them or not, but all annotations that are legal are guaranteed to be parsed, even if the application doesn't know about or use them at all. In this way you can embed metadata about objects in a Humon file, and even old versions of Humon apps will correctly read (and ignore) them.

**Serializing back to stream comes with options.**
The API allows for serializing a trove back to memory or a stream. There are three options to serialize Humon objects:
1. **Preserve**: A direct copy of the original token stream is produced, including all comments and commas and whitespace. Just a brainless memory slam.
1. **Minimal**: This reduces whitespace to at most one character each to pare down length. Humon does as much as it can, but if you choose to preserve comments in the minified output, you may notice that not all newlines get replaced. This is because the next read operation on the resultant token stream must replicate the comment associations from the original, and that requires some comments be on their own line. Also, C++-style //comments end in a newline, and tokens after must be on their own line, so those newlines are also preserved.
1. **Pretty**: This produces a clean, indented, eminently readable string.

Each of these options allows you to remove comments (perhaps for security), and also provide a color table for syntax highlighting. This is useful for producing colored output for a ANSI-based terminal, or perhaps HTML.

## Python
A Python3 module is forthcoming, which will consome and produce Humon text.

## Node
A node module is forthcoming, which will consome and produce Humon text.

## Rust
A rust module is forthcoming, which will consome and produce Humon text.

## .NET
A .NET module is forthcoming, which will consome and produce Humon text.
