/** @file
 *  @brief This is the main header for the Humon C++ API. \#include this from your C++ code.
 **/

#pragma once

#include <string_view>
#include <tuple>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <array>
#include <charconv>
#include <optional>
#include <variant>
#include <limits>

// This macro wraps the C API in namespace hu::capi to keep global space pristine.
// Because it's also extern "C", the namespace names are dropped from the linkage,
// so we can still link against the C library.
#define HUMON_USENAMESPACE
#include "humon.h"


/// Defining this enables exceptions to be thrown on unfound path calls
/// (hu::Trove::nodeByAddress, huNode::nodeByAddress, hu::Trove::operator/
/// or hu::Node::operator/); Normally these silently fail, and it can be
/// difficult to determine where. See the README.
/// \#define HUMON_USE_NODE_PATH_EXCEPTIONS


/// The Humon namespace.
namespace hu
{
    typedef hu::capi::huLine_t line_t;
    typedef hu::capi::huCol_t col_t;
    typedef hu::capi::huSize_t size_t;

    /// Specifies a UTFn text encoding.
    enum class Encoding
    {
        utf8 = capi::HU_ENCODING_UTF8,
        utf16be = capi::HU_ENCODING_UTF16_BE,
        utf16le = capi::HU_ENCODING_UTF16_LE,
        utf32be = capi::HU_ENCODING_UTF32_BE,
        utf32le = capi::HU_ENCODING_UTF32_LE,
        unknown = capi::HU_ENCODING_UNKNOWN
    };

    /// Return a string representation of a hu::Encoding.
    inline char const * to_string(Encoding rhs)
    {
        return capi::huEncodingToString(static_cast<capi::huEncoding>(rhs));
    }

    /// Specifies the kind of data represented by a particular hu::Token.
    enum class TokenKind
    {
        null = capi::HU_TOKENKIND_NULL,                 ///< Invalid token. Malformed, or otherwise nonexistent.
        eof = capi::HU_TOKENKIND_EOF,                   ///< The end of the source text.
        startList = capi::HU_TOKENKIND_STARTLIST,       ///< The opening '[' of a list.
        endList = capi::HU_TOKENKIND_ENDLIST,           ///< The closing ']' of a list.
        startDict = capi::HU_TOKENKIND_STARTDICT,       ///< The opening '{' of a dict.
        endDict = capi::HU_TOKENKIND_ENDDICT,           ///< The closing '}' of a dict.
        keyValueSep = capi::HU_TOKENKIND_KEYVALUESEP,   ///< The separating ':' of a key-value pair.
        annotate = capi::HU_TOKENKIND_ANNOTATE,         ///< The annotation mark '@'.
        word = capi::HU_TOKENKIND_WORD,                 ///< Any key or value string, quoted or unquoted.
        comment = capi::HU_TOKENKIND_COMMENT            ///< Any comment token. An entire comment is considered one token.
    };

    /// Return a string representation of a hu::TokenKind.
    inline char const * to_string(TokenKind rhs)
    {
        return capi::huTokenKindToString(static_cast<capi::huTokenKind>(rhs));
    }

    /// Specifies the kind of node represented by a particular hu::Node.
    enum class NodeKind
    {
        null = capi::HU_NODEKIND_NULL,      ///< Invalid node. An invalid address returns a null node.
        list = capi::HU_NODEKIND_LIST,      ///< List node. The node contains a sequence of unassociated objects in maintained order.
        dict = capi::HU_NODEKIND_DICT,      ///< Dict node. The node contains a sequence of string-associated objects in maintained order.
        value = capi::HU_NODEKIND_VALUE     ///< Value node. The node contains a string value, and no children.
    };

    /// Return a string representation of a hu::NodeKind.
    inline char const * to_string(NodeKind rhs)
    {
        return capi::huNodeKindToString(static_cast<capi::huNodeKind>(rhs));
    }

    /// Specifies the style of whitespacing in Humon text.
    enum class WhitespaceFormat
    {
        cloned = capi::HU_WHITESPACEFORMAT_CLONED,              ///< Byte-for-byte copy of the original.
        minimal = capi::HU_WHITESPACEFORMAT_MINIMAL,            ///< Reduces as much whitespace as possible.
        pretty = capi::HU_WHITESPACEFORMAT_PRETTY               ///< Formats the text in a standard, human-friendly way.
    };

    /// Return a string representation of a hu::WhitespaceFormat.
    inline char const * to_string(WhitespaceFormat rhs)
    {
        return capi::huWhitespaceFormatToString(static_cast<capi::huWhitespaceFormat>(rhs));
    }

    /// Specifies a tokenizing or parsing error code, or lookup error.
    enum class ErrorCode
    {
        noError = capi::HU_ERROR_NOERROR,                       ///< No error.
        badEncoding = capi::HU_ERROR_BADENCODING,               ///< The Unicode encoding is malformed.
        unfinishedQuote = capi::HU_ERROR_UNFINISHEDQUOTE,       ///< The quoted text was not endquoted.
        unfinishedCStyleComment =
            capi::HU_ERROR_UNFINISHEDCSTYLECOMMENT,             ///< The C-style comment was not closed.
        unexpectedEof = capi::HU_ERROR_UNEXPECTEDEOF,           ///< The text ended early.
        tooManyRoots = capi::HU_ERROR_TOOMANYROOTS,             ///< There is more than one root node detected.
        syntaxError = capi::HU_ERROR_SYNTAXERROR,               ///< General syntax error.
        notFound = capi::HU_ERROR_NOTFOUND,                     ///< No node could be found at the address.
        illegal = capi::HU_ERROR_ILLEGAL,                       ///< The operation was illegal.
        badParameter = capi::HU_ERROR_BADPARAMETER,             ///< An API parameter is malformed or illegal.
        badFile = capi::HU_ERROR_BADFILE,                       ///< An attempt to open or operate on a file failed.
        outOfMemory = capi::HU_ERROR_OUTOFMEMORY,               ///< An internal memory allocation failed.
        troveHasErrors = capi::HU_ERROR_TROVEHASERRORS          ///< The loading function succeeded, but the loaded trove has errors.
    };

    /// Return a string representation of a hu::ErrorCode.
    inline char const * to_string(ErrorCode rhs)
    {
        return capi::huOutputErrorToString(static_cast<capi::huErrorCode>(rhs));
    }

    /// Specifies how a trove responds to errors.
    enum class ErrorResponse
    {
        mum = capi::HU_ERRORRESPONSE_MUM,
        toStdout = capi::HU_ERRORRESPONSE_STDOUT,
        toStderr = capi::HU_ERRORRESPONSE_STDERR,
        stdoutAnsiColor = capi::HU_ERRORRESPONSE_STDOUTANSICOLOR,
        stderrAnsiColor = capi::HU_ERRORRESPONSE_STDERRANSICOLOR,
        numResponses = capi::HU_ERRORRESPONSE_NUMRESPONSES
    };

    /// Specifies a style ID for colorized printing.
    enum class ColorCode
    {
        sourceTextBegin = capi::HU_COLORCODE_TOKENSTREAMBEGIN,     ///< Beginning-of-source text color code.
        sourceTextEnd = capi::HU_COLORCODE_TOKENSTREAMEND,         ///< End-of-source text color code.
        tokenEnd = capi::HU_COLORCODE_TOKENEND,                     ///< End-of-color code.
        puncList = capi::HU_COLORCODE_PUNCLIST,                     ///< List punctuation style. ([,])
        puncDict = capi::HU_COLORCODE_PUNCDICT,                     ///< Dict punctuation style. ({,})
        puncKeyValueSep = capi::HU_COLORCODE_PUNCKEYVALUESEP,       ///< Key-value separator style. (:)
        puncAnnotate = capi::HU_COLORCODE_PUNCANNOTATE,             ///< Annotation mark style. (@)
        puncAnnotateDict = capi::HU_COLORCODE_PUNCANNOTATEDICT,     ///< Annotation dict punctuation style. ({,})
        puncAnnotateKeyValueSep =
            capi::HU_COLORCODE_PUNCANNOTATEKEYVALUESEP,             ///< Annotation key-value separator style. (:)
        key = capi::HU_COLORCODE_KEY,                               ///< Key style.
        value = capi::HU_COLORCODE_VALUE,                           ///< Value style.
        comment = capi::HU_COLORCODE_COMMENT,                       ///< Comment style.
        annoKey = capi::HU_COLORCODE_ANNOKEY,                       ///< Annotation key style.
        annoValue = capi::HU_COLORCODE_ANNOVALUE,                   ///< Annotation value style.
        whitespace = capi::HU_COLORCODE_WHITESPACE,                 ///< Whitespace style (including commas).

        numColors = capi::HU_COLORCODE_NUMCOLORS                    ///< One past the last style code.
    };

    enum class BufferManagement
    {
        copyAndOwn = capi::HU_BUFFERMANAGEMENT_COPYANDOWN,  ///< The trove should copy the input buffer, and free the copy when destroyed.
        moveAndOwn = capi::HU_BUFFERMANAGEMENT_MOVEANDOWN,  ///< The trove should use the input buffer without copying it, and free it when destroyed.
        move = capi::HU_BUFFERMANAGEMENT_MOVE               ///< The trove should use the input buffer without copying it, and do nothing to it when destroyed.
    };

    inline bool validateSize(std::size_t sz)
    {
        if constexpr (sizeof(std::size_t) > sizeof(hu::size_t))
        {
            return sz < static_cast<std::size_t>(
                std::numeric_limits<hu::size_t>::max());
        }
        return true;
    }

    /// Conversion from capi::huStringView to std::string_view.
    inline std::string_view make_sv(capi::huStringView const & husv)
    {
        return std::string_view(husv.ptr, husv.size);
    }

    /// Describes a color table for printing.
    using ColorTable = std::array<std::string_view, capi::HU_COLORCODE_NUMCOLORS>;

    class Allocator : public capi::huAllocator
    {
    public:
        Allocator(void * manager = NULL, capi::huMemAlloc memAlloc = NULL, capi::huMemRealloc memRealloc = NULL, capi::huMemFree memFree = NULL)
        : capi::huAllocator({manager, memAlloc, memRealloc, memFree})
        { }

        Allocator(capi::huAllocator const & alloc)
        : capi::huAllocator({alloc.manager, alloc.memAlloc, alloc.memRealloc, alloc.memFree})
        { }

        void setManager(void * manager) { this->manager = manager; }
        void setAllocFn(capi::huMemAlloc memAlloc = NULL) { this->memAlloc = memAlloc; }
        void setReallocFn(capi::huMemRealloc memRealloc = NULL) { this->memRealloc = memRealloc; }
        void setFreeFn(capi::huMemFree memFree = NULL) { this->memFree = memFree; }

        void * getManager() { return this->manager; }
        capi::huMemAlloc getAllocFn() { return this->memAlloc; }
        capi::huMemRealloc getReallocFn() { return this->memRealloc; }
        capi::huMemFree getFreeFn() { return this->memFree; }
    };

    /// Encapsulates a selection of parameters to control how Humon interprets the input for loading.
    class DeserializeOptions
    {
    public:
        /// Construct with sane defaults.
        DeserializeOptions(Encoding encoding = Encoding::unknown, bool strictUnicode = true, hu::col_t tabSize = 4, Allocator allocator = {}, BufferManagement bufferManagement = BufferManagement::copyAndOwn)
        {
            capi::huInitDeserializeOptions(& cparams, static_cast<capi::huEncoding>(encoding), strictUnicode, tabSize, & allocator, static_cast<capi::huBufferManagement>(bufferManagement));
        }

        /// Expect a particular Unicode encoding, or Encoding::unknown.
        void setEncoding(Encoding encoding) { cparams.encoding = static_cast<capi::huEncoding>(encoding); }
        /// Allow code points that are out of Unicode range.
        void setAllowOutOfRangeCodePoints(bool shallWe) { cparams.allowOutOfRangeCodePoints = shallWe; }
        /// Allow unpaired surrogate code units in UTF-16.
        void setAllowUtf16UnmatchedSurrogates(bool shallWe) { cparams.allowUtf16UnmatchedSurrogates = shallWe; }
        /// Use this tab size when computing the column of a token and the source text contains tabs.
        void setTabSize(hu::col_t tabSize) { cparams.tabSize = tabSize; }
        /// Customize the memory allocation functions used in all of Humon's API.
        void setAllocator(Allocator allocator)
        {
            cparams.allocator = allocator;
        }
        /// Get the encoding to expect.
        Encoding encoding() const { return static_cast<Encoding>(cparams.encoding); }
        /// Get whether out-of-range Unicode code points are allowed.
        bool allowOutOfRangeCodePoints() const { return cparams.allowOutOfRangeCodePoints; }
        /// Get whether unpaired surrogates in UTF-16 are allowed.
        bool allowUtf16UnmatchedSurrogates() const { return cparams.allowUtf16UnmatchedSurrogates; }
        /// Get the tab size used to compute column information in source text that contain tabs.
        hu::col_t tabSize() const { return cparams.tabSize; }
        /// Get the allocator used to handle memory.
        Allocator getAllocator() const { return Allocator { cparams.allocator }; }

        /// Aggregated C structure.
        capi::huDeserializeOptions cparams;
    };

    /// Encapsulates a selection of parameters to control the serialization of a trove.
    class SerializeOptions
    {
    public:
        /// Construct with sane defaults.
        SerializeOptions(WhitespaceFormat whitespaceFormat = WhitespaceFormat::pretty,
            hu::col_t indentSize = 4, bool indentWithTabs = false, std::optional<ColorTable> const & colors = {},
            bool printComments = true, std::string_view newline = "\n", Encoding encoding = Encoding::utf8, bool printBom = false)
        {
            std::size_t newlineSize = newline.size();
            if (! validateSize(newlineSize))
                { newlineSize = static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()); }

            capi::huStringView * colorTablePtr = NULL;
            if (colors.has_value())
            {
                setColorTable(colors);
                colorTablePtr = capiColorTable;
            }

            capi::huInitSerializeOptionsN(& cparams, static_cast<capi::huWhitespaceFormat>(whitespaceFormat), indentSize, indentWithTabs,
                false, colorTablePtr, printComments, newline.data(), static_cast<hu::size_t>(newlineSize), static_cast<capi::huEncoding>(encoding), printBom);
            setColorTable(colors);
        }

		/// Set the encoding.
		void setEncoding(Encoding encoding) { cparams.encoding = static_cast<capi::huEncoding>(encoding); }
        /// Set the whitespace formatting.
        void setFormat(WhitespaceFormat whitespaceFormat) { cparams.whitespaceFormat = static_cast<capi::huWhitespaceFormat>(whitespaceFormat); }
        /// Set the number of spaces to use for indentation.
        void setIndentSize(hu::col_t indentSize) { cparams.indentSize = indentSize; }
        /// Use tab instead of spaces for indentation.
        void setIndentWithTabs(bool shallWe) { cparams.indentWithTabs = shallWe; }

        /// Use the given color table.
        void setColorTable(std::optional<ColorTable> const & colors)
        {
            if (colors)
            {
                std::string_view const * sv = (* colors).data();
                for (int i = 0; i < capi::HU_COLORCODE_NUMCOLORS; ++i)
                {
                    std::size_t sz = sv[(size_t) i].size();
                    if (! validateSize(sz))
                        { sz = static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()); }
                    capiColorTable[(size_t) i].ptr = sv[(size_t) i].data();
                    capiColorTable[(size_t) i].size = static_cast<hu::size_t>(sz);
                }
                cparams.usingColors = true;
            }
            else
                { cparams.usingColors = false; }
        }

        /// Set whether to print comment tokens in the output.
        void setPrintComments(bool shallWe) { cparams.printComments = shallWe; }
        /// Set the character string to use for a newline.
        void setNewline(std::string_view newline)
        {
            std::size_t sz = newline.size();
            if (! validateSize(sz))
                { sz = static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()); }

            cparams.newline.ptr = newline.data();
            cparams.newline.size = static_cast<hu::size_t>(sz);
        }

		/// Get the encoding.
		Encoding encoding() const { return static_cast<Encoding>(cparams.encoding); }
        /// Get the whitespace formatting.
        WhitespaceFormat whitespaceFormat() const { return static_cast<WhitespaceFormat>(cparams.whitespaceFormat); }
        /// Get the number of spaces to use for indentation.
        hu::size_t indentSize() { return cparams.indentSize; }
        /// Get whether to use tabs instead of spaces for indentation.
        bool indentWithTabs() { return cparams.indentWithTabs; }

        /// Get the color table.
        std::optional<ColorTable> colorTable() const
        {
            if (cparams.usingColors == false)
                { return std::nullopt; }

            ColorTable newColorTable;
            std::string_view * sv = newColorTable.data();
            for (int i = 0; i < capi::HU_COLORCODE_NUMCOLORS; ++i)
            {
                sv[(size_t) i] = { capiColorTable[(size_t) i].ptr,
                                   static_cast<std::size_t>(capiColorTable[(size_t) i].size) };
            }
            return newColorTable;
        }

        /// Get whether to print comments.
        bool printComments() const { return cparams.printComments; }
        /// Get the newline sequence.
        std::string newline() const { return { cparams.newline.ptr, static_cast<std::size_t>(cparams.newline.size) }; }

        /// Aggregated C struct.
        capi::huSerializeOptions cparams;
        /// Aggregated C color table.
        capi::huStringView capiColorTable[capi::HU_COLORCODE_NUMCOLORS];
    };

    class Trove;

    /// Describes the result type of a deserialize (load) operation.
    typedef std::variant<Trove, ErrorCode> DeserializeResult;

    /// Describes the result type of a serialize (print) operation.
    typedef std::variant<std::string, ErrorCode> SerializeResult;

    // Either throws or returns false.
    inline void checkNotNull(void const * cp)
    {
#ifdef HUMON_USING_EXCEPTIONS
        if (cp == nullptr)
            { throw std::runtime_error("object is nullish"); }
#else
        (void) cp;  // unused, satisfy -Wextra
#endif
    }

    /// Encodes a token read from Humon text.
    /** This class encodes file location and buffer location information about a
     * particular token in a Humon file. Every token is read and tracked with a
     * hu::Token. */
    class Token
    {
    public:
        /// Create a nullish token.
        Token() { }
        /// Create a token that wraps a `huToken const *`.
        Token(capi::huToken const * ctoken) : ctoken(ctoken) { }
        bool isValid() const           ///< Returns whether the token is valid (not nullish).
            { return ctoken != nullptr; }
        bool isNullish() const         ///< Returns whether the token is nullish (not valid).
            { return ctoken == nullptr; }
        operator bool() const          ///< Implicit validity test.
            { return isValid(); }
        TokenKind kind() const         ///< Returns the kind of token this is.
            { check(); return isValid() ? static_cast<TokenKind>(huGetTokenKind(ctoken))
                                        : static_cast<TokenKind>(capi::HU_TOKENKIND_NULL); }
        std::string_view rawStr() const   ///< Returns the raw string value of the token, including quotes.
            { check(); return isValid() ? make_sv(* huGetRawString(ctoken)) : ""; }
        std::string_view str() const   ///< Returns the string value of the token.
            { check(); return isValid() ? make_sv(* huGetString(ctoken)) : ""; }
        hu::line_t line() const               ///< Returns the line number of the first character of the token in the file.
            { check(); return isValid() ? huGetLine(ctoken) : 0; }
        hu::col_t col() const                ///< Returns the column number of the first character of the token in the file.
            { check(); return isValid() ? huGetColumn(ctoken) : 0; }
        hu::line_t endLine() const            ///< Returns the line number of the last character of the token in the file.
            { check(); return isValid() ? huGetEndLine(ctoken) : 0; }
        hu::col_t endCol() const             ///< Returns the column number of the last character of the token in the file.
            { check(); return isValid() ? huGetEndColumn(ctoken) : 0; }
        operator std::string_view()    ///< String view conversion.
            { return str(); }

        /// Prints the string value of the token to a stream.
        friend std::ostream & operator <<(std::ostream & out, Token rhs)
        {
            out << rhs.str();
            return out;
        }
    private:
        void check() const { checkNotNull(ctoken); }

        capi::huToken const * ctoken = nullptr;
    };

    /// Value extraction template for in-line grokking of value nodes.
    /** This template is a helper to extract machine values out of Humon value nodes.
     * Standard types are supported, and custom extractors can be specified. Just
     * specialize `hu::val<T>` for your type `T`, and implement the `extract()`
     * function.
     * Usage: Use in conjunction with the `/` operator on a value node. This allows
     * you to extract a value in the natural flow of things:
     * `auto numEyes = node / "frogs" / "numEyes" % hu::val<int> {};`
     * `auto color = node / "frogs" / "colors" / 0 % hu::val<AnimalColorsEnum> {};`
     */
    template <class T, typename V = void>
    struct val
    { };

    /// References a node's parent in an object-based lookup.
    /** hu::Node::operator/() has an overload which takes a hu::Parent. This
     * allows you to use the / operator to get a hu::Node's parent.
     */
    class Parent { };

    /// Encodes a Humon data node.
    /** Humon nodes make up a hierarchical structure, stemming from a single root node.
     * Humon troves contain a reference to the root, and store all nodes in an indexable
     * array. A node is either a list, a dict, or a value node. Any number of comments
     * and annotations can be associated to a node. */
    class Node
    {
    public:
        /// Constructs a nullish node.
        Node() { }
        /// Constructs a node which wraps a `huNode const *`.
        Node(capi::huNode const * cnode) : cnode(cnode) { }
        /// Return whether two Node objects refer to the same node.
        friend bool operator == (Node const & lhs, Node const & rhs)
            { return lhs.cnode == rhs.cnode; }
        /// Return whether two Node objects refer to the different nodes.
        friend bool operator != (Node const & lhs, Node const & rhs)
            { return lhs.cnode != rhs.cnode; }
        // TODO: operator <=> when available.
        bool isValid() const               ///< Returns whether the node is valid (not nullish).
            { return cnode != nullptr; }
        bool isNullish() const             ///< Returns whether the node is nullish (not valid).
            { return cnode == nullptr; }
        operator bool() const                           ///< Implicit validity test.
            { return isValid(); }
        bool isRoot() const              ///< Returns whether is node is the root node.
            { check(); return nodeIndex() == 0; }
        hu::size_t nodeIndex() const              ///< Returns the node index within the trove. Rarely needed.
            { check(); return isValid() ? huGetNodeIndex(cnode) : -1; }
        NodeKind kind() const              ///< Returns the kind of node this is.
            { check(); return isValid() ? static_cast<NodeKind>(huGetNodeKind(cnode))
                                        : static_cast<NodeKind>(capi::HU_NODEKIND_NULL); }
        Token firstToken() const           ///< Returns the first token which contributes to this node, including any annotation and comment tokens.
            { check(); return Token(isValid() ? huGetFirstToken(cnode) : HU_NULLTOKEN); }
        Token firstValueToken() const      ///< Returns the first token of this node's actual value; for a container, it points to the opening brac(e|ket).
            { check(); return Token(isValid() ? huGetValueToken(cnode) : HU_NULLTOKEN); }
        Token lastValueToken() const       ///< Returns the last token of this node's actual value; for a container, it points to the closing brac(e|ket).
            { check(); return Token(isValid() ? huGetLastValueToken(cnode) : HU_NULLTOKEN); }
        Token lastToken() const            ///< Returns the last token of this node, including any annotation and comment tokens.
            { check(); return Token(isValid() ? huGetLastToken(cnode) : HU_NULLTOKEN); }
        Node parent() const                ///< Returns the parent node of this node, or the null node if this is the root.
            { check(); return Node(capi::huGetParent(cnode)); }
        hu::size_t childOrdinal() const           ///< Returns the index of this node vis a vis its sibling nodes (starting at 0).
            { check(); return isValid() ? huGetChildOrdinal(cnode) : -1; }
        hu::size_t numChildren() const            ///< Returns the number of children of this node.
            { check(); return capi::huGetNumChildren(cnode); }

        /// Returns the child node, by child index.
        template <class IntType,
            typename std::enable_if<
                std::is_integral_v<IntType>, IntType>::type * = nullptr>
        Node child(IntType idx) const
        {
            check();
            if (idx > 0 && static_cast<std::size_t>(idx) > static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()))
                { return Node(HU_NULLNODE); }
            return capi::huGetChildByIndex(cnode, static_cast<hu::size_t>(idx));
        }

        /// Returns the last child node with the specified key (if this is a dict).
        Node child(std::string_view key) const
        {
            check();
            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return Node(HU_NULLNODE); }
            return capi::huGetChildByKeyN(cnode, key.data(), static_cast<hu::size_t>(sz));
        }

        Node firstChild() const            ///< Returns the first child node of this node.
            { check(); return child(0); }
        Node nextSibling() const        ///< Returns the node ordinally after this one in the parent's children, or the null node if it's the last.
            { check(); return Node(capi::huGetNextSibling(cnode)); }

        Node firstChild(std::string_view key) const            ///< Returns the first child node of this node with the specified key.
        {
            check();
            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return Node(HU_NULLNODE); }
            return Node(capi::huGetFirstChildWithKeyN(cnode, key.data(), static_cast<hu::size_t>(sz)));
        }

        Node nextSibling(std::string_view key) const        ///< Returns the node with the specified key ordinally after this one in the parent's children, or the null node if it's the last.
        {
            check();
            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return Node(HU_NULLNODE); }
            return Node(capi::huGetNextSiblingWithKeyN(cnode, key.data(), static_cast<hu::size_t>(sz)));
        }

        /// Access a node by an address relative to this node.
        /** A relative address is a single string, which contains as contents a `/`-delimited path
         * through the hierarchy. A key or index between the slashes indicates the child node to
         * access. */
        Node nodeByAddress(std::string_view relativeAddress) const
        {
            check();
            std::size_t sz = relativeAddress.size();
            if (! validateSize(sz))
                { return Node(HU_NULLNODE); }
            return capi::huGetNodeByRelativeAddressN(cnode, relativeAddress.data(), static_cast<hu::size_t>(sz));
        }
        bool hasKey() const                ///< Returns whether this node has a key. (If it's in a dict.)
            { check(); return capi::huHasKey(cnode); }
        Token key() const                  ///< Returns the key token, or the null token if this is not in a dict.
            { check(); return Token(isValid() ? huGetKeyToken(cnode) : HU_NULLTOKEN); }
        Token value() const                ///< Returns the first value token that encodes this node.
            { check(); return Token(isValid() ? huGetValueToken(cnode) : HU_NULLTOKEN); }
        hu::size_t numAnnotations() const         ///< Returns the number of annotations associated to this node.
            { check(); return capi::huGetNumAnnotations(cnode); }

        /// Returns the `idx`th annotation.
        /** Returns a <Token, Token> referencing the key and value of the annotation
         * accessed by index. */
        std::tuple<Token, Token> annotation(hu::size_t idx) const
        {
            check();
            auto canno = capi::huGetAnnotation(cnode, idx);
            if (canno == nullptr)
                return { Token(HU_NULLTOKEN), Token(HU_NULLTOKEN) };
            return { Token(canno->key), Token(canno->value) };
        }

        /// Returns a new collection of all this node's annotations.
        /** Creates a new vector of <Token, Token> tuples, each referencing the
         * key and value of an annotation. */
        [[nodiscard]] std::vector<std::tuple<Token, Token>> allAnnotations() const
        {
            check();
            std::vector<std::tuple<Token, Token>> vec;
            hu::size_t numAnnos = numAnnotations();
            vec.reserve(numAnnos);
            for (hu::size_t i = 0; i < numAnnos; ++i)
                { vec.push_back(annotation(i)); }
            return vec;
        }

        /// Returns the number of annotations associated to this node, with the specified key.
        /** A node can have multiple annotations with the same key. This is legal in Humon, but
         * rare. This function returns the number of annotations associated to this node with
         * the specified key. */
		hu::size_t numAnnotationsWithKey(std::string_view key) const
        {
            check();
            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return 0; }
            return capi::huGetNumAnnotationsWithKeyN(cnode, key.data(), static_cast<hu::size_t>(sz));
        }

        /// Returns a new collection of all this node's annotations with the specified key.
        /** Creates a new vector of Tokens, each referencing the key of an annotation that
         * has the specified key. */
        [[nodiscard]] std::vector<Token> annotationsWithKey(std::string_view key) const
        {
            check();
            std::vector<Token> vec;
            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huToken const * tok = HU_NULLTOKEN;
            do
            {
                tok = capi::huGetAnnotationWithKeyN(cnode, key.data(), static_cast<hu::size_t>(sz), & cursor);
                if (tok)
                    { vec.push_back(tok); }
            } while (tok != HU_NULLTOKEN);

            return vec;
        }

        /// Returns the number of annotations associated to this node, with the specified key.
        /** A node can have multiple annotations, each with different keys which have the same
         * value. This function returns the number of annotations associated to this node with
         * the specified value. */
        hu::size_t numAnnotationsWithValue(std::string_view value) const
        {
            check();
            std::size_t sz = value.size();
            if (! validateSize(sz))
                { return 0; }
            return capi::huGetNumAnnotationsWithValueN(cnode, value.data(), static_cast<hu::size_t>(sz));
        }

        /// Returns a new collection of all this node's annotations with the specified value.
        /** Creates a new vector of Tokens, each referencing the key of an annotation that
         * has the specified value. */
        [[nodiscard]] std::vector<Token> annotationsWithValue(std::string_view value) const
        {
            check();
            std::vector<Token> vec;
            std::size_t sz = value.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huToken const * tok = HU_NULLTOKEN;
            do
            {
                tok = capi::huGetAnnotationWithValueN(cnode, value.data(), static_cast<hu::size_t>(sz), & cursor);
                if (tok)
                    { vec.push_back(tok); }
            } while (tok != HU_NULLTOKEN);

            return vec;
        }

        /// Returns the number of comments associated to this node.
        hu::size_t numComments() const
            { check(); return capi::huGetNumComments(cnode); }
        /// Returns the `idx`th comment associated to this node.
        /** Returns a Token of the `idx`th ordinal comment associated with this node. */
        Token comment(hu::size_t idx) const
            { check(); return capi::huGetComment(cnode, idx); }

        /// Returns a new collection of all comments associated to this node.
        [[nodiscard]] std::vector<Token> allComments() const
        {
            check();
            std::vector<Token> vec;
            hu::size_t numComms = numComments();
            vec.reserve(numComms);
            for (hu::size_t i = 0; i < numComms; ++i)
                { vec.push_back(comment(i)); }
            return vec;
        }

        /// Returns whether this node any comments containing the specified substring.
        bool hasCommentsContaining(std::string_view containedString) const
        {
            check();
            std::size_t sz = containedString.size();
            if (! validateSize(sz))
                { return false; }
            return capi::huHasCommentsContainingN(cnode, containedString.data(), static_cast<hu::size_t>(sz));
        }

        /// Returns a new collection of all comments associated to this node containing the specified substring.
        [[nodiscard]] std::vector<Token> commentsContaining(std::string_view containedString) const
        {
            check();
            std::vector<Token> vec;
            std::size_t sz = containedString.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huToken const * comm = NULL;
            do
            {
                comm = capi::huGetCommentsContainingN(cnode, containedString.data(), static_cast<hu::size_t>(sz), & cursor);
                if (comm)
                    { vec.emplace_back(comm); }
            } while (comm != HU_NULLTOKEN);
            return vec;
        }

        /// Returns whether the specified index is a valid child index of this list or dict node.
        template <class IntType,
            typename std::enable_if<
                std::is_integral_v<IntType>, IntType>::type * = nullptr>
        bool operator % (IntType idx) const
        {
            check();
            if (idx > 0 && static_cast<std::size_t>(idx) > static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()))
                { return false; }
            auto cidx = static_cast<hu::size_t>(idx);
            if (isValid() && (kind() == NodeKind::dict || kind() == NodeKind::list))
                { return cidx >= 0 && cidx < numChildren(); }
            return false;
        }

        /// Returns whether the specified key is a valid child key of this dict node.
        bool operator % (std::string_view key) const
        {
            check();
            if (kind() == NodeKind::dict)
                { return isValid() && child(key).isValid(); }
            return false;
        }

        /// Returns the `idx`th child of this node.
        template <class IntType,
            typename std::enable_if<
                std::is_integral_v<IntType>, IntType>::type * = nullptr>
        Node operator / (IntType idx) const
        {
            check();
            if (isValid())
            {
                auto ch = child(idx);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
                if (ch.isNullish())
                    { throw std::runtime_error("Illegal path entry"); }
#endif
                return Node(ch);
            }
            return Node(HU_NULLNODE);
        }

        /// Returns the child of this node by key.
        Node operator / (std::string_view key) const
        {
            check();
            if (isValid())
            {
                auto ch = child(key);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
                if (ch.isNullish())
                    { throw std::runtime_error("Illegal path entry"); }
#endif
                return Node(ch);
            }
            return Node(HU_NULLNODE);
        }

        /// Return the parent of this node.
        Node operator / (Parent) const
        {
            check();
            if (isValid())
            {
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
                if (isRoot() == 0)
                    { throw std::runtime_error("Illegal path entry"); }
#endif
                return parent();
            }
            return Node(HU_NULLNODE);
        }

        /// Returns the converted value of this value node.
        /** Converts the string value of this value node into a `U`. The conversion is
         * performed by passing a dummy object of type val<T> which, if implemented
         * for type U, will invoke that type's extract function. If there is no val<T>
         * defined, the default val<T> calls std::from_chars(... U&). Any type that
         * has an overload of std::from_chars (integers or floating point types), or a
         * specialization of val<T>, will return the converted value.
         *
         * Users can implement a specialization of val<T> for a custom type, providing
         * the analogous extract() member function.
         *
         * The purpose of val<T> is to allow users to write code like:
         *      auto sv = materialsTrove / "assets" / "brick-diffuse" / "src" /
         *                0 % h::val<string_view> {}; */
        template <class T>
        [[nodiscard]] inline T operator % (val<T> ve) const
        {
            check();
            return ve.extract(*this);
        }

        /// Returns the entire text contained by this node and all its children.
        /** The entire text of this node is returned, including all its children's
         * texts, and any comments and annotations associated to this node. */
        std::string_view sourceText() const
        {
            check();
            auto sv = capi::huGetSourceText(cnode);
            return make_sv(sv);
        }

        /// Generates and returns the address of this node.
        /** Each node in a trove has a unique address, separate from its node index,
         * which is represented by a series of keys or index values from the root,
         * separated by `/`s. The address is guaranteed to work with
         * Trove::nodeByAddress(). */
        [[nodiscard]] std::string address() const
        {
            check();
            hu::size_t len = 0;
            capi::huGetAddress(cnode, NULL, & len);
            std::string s;
            s.resize(len);
            capi::huGetAddress(cnode, s.data(), & len);
            return s;
        }

        capi::huNode const * cNode() const { return cnode; }

    private:
        void check() const { checkNotNull(cnode); }

        capi::huNode const * cnode = nullptr;
    };


    /// Extractor for type int.
    template <class T>
    struct val<T, typename std::enable_if_t<std::is_integral_v<T>>>
    {
        /// Extract the value from the node.
        static inline T extract(Node const & node)
        {
            return extract(node.value().str());
        }

        /// Extract the value from the string.
        static inline T extract(std::string_view valStr)
        {
            T value;
            auto [p, ec] = std::from_chars(valStr.data(), valStr.data() + valStr.size(), value);
            if (ec == std::errc())
                { return value; }
            else
                { return T {}; }
        }
    };

    /// Extractor for type float.
    template <class T>
    struct val<T, typename std::enable_if_t<std::is_floating_point_v<T>>>
    {
        /// Extract the value from the node.
        static inline T extract(Node const & node)
        {
            return extract(node.value().str());
        }

        /// Extract the value from the string.
        static inline T extract(std::string_view valStr)
        {
            // TODO: (gcc): Once std::from_chars(..T&) is complete, use this or remove this specialization. We're making a useless string here and it maketh me to bite metal. Don't forget to.
    #if 0
            T val;
            auto [p, ec] = std::from_chars(valStr.data(), valStr.data() + valStr.size(), val, std::chars_format::general);
            if (ec == std::errc())
                { return val; }
            else
                { return T {}; }
    #else
            return static_cast<T>(std::stod(std::string(valStr)));
    #endif
        }
    };

    /// Extractor for type std::string_view.
    template <>
    struct val<std::string_view>
    {
        /// Extract the value from the node.
        static inline std::string_view extract(Node const & node)
        {
            return node.value().str();
        }

        /// Extract the value from the string.
        static inline std::string_view extract(std::string_view valStr)
        {
            return valStr;
        }
    };

    /// Extractor for type std::string.
    template <>
    struct val<std::string>
    {
        /// Extract the value from the node.
        static inline std::string extract(Node const & node)
        {
            return std::string(node.value().str());
        }

        /// Extract the value from the string.
        static inline std::string extract(std::string_view valStr)
        {
            return std::string(valStr);
        }
    };

    /// Extractor for type bool. Parses English spelling of "true" for truth.
    template <>
    struct val<bool>
    {
        /// Extract the value from the node.
        static inline bool extract(Node const & node)
        {
            return extract(node.value().str());
        }

        /// Extract the value from the string.
        static inline bool extract(std::string_view valStr)
        {
            if (valStr[0] != 't' && valStr[0] != 'T')
                { return false; }

            if (valStr.size() == 1)
                { return true; }

            if (valStr.size() != 4)
                { return false; }

            return valStr == "true" ||
                    valStr == "True" ||
                    valStr == "TRUE";
        }
    };

    class ChangeSet
    {
    public:
        ChangeSet(Trove const & trove); // defined after Trove

        ~ChangeSet()
        {
            capi::huDestroyChangeSet(& cchangeSet);
        }

        /// Record a node replacement / removal.
	    hu::size_t replaceNode(Node const & node, std::string_view newString)
        {
            return capi::huReplaceNodeN(& cchangeSet, node.cNode(), newString.data(), newString.size());
        }

        /// Record appending to a list or dict.
	    hu::size_t append(Node const & node, std::string_view newString)
        {
            return capi::huAppendN(& cchangeSet, node.cNode(), newString.data(), newString.size());
        }

        /// Record inserting into a list or dict.
	    hu::size_t insertAtIndex(Node const & node, hu::size_t idx, std::string_view newString)
        {
            return capi::huInsertAtIndexN(& cchangeSet, node.cNode(), idx, newString.data(), newString.size());
        }

        capi::huChangeSet & cChangeSet() { return cchangeSet; }
        capi::huTrove const * cTrove() const { return ctrove; }

    private:
        capi::huChangeSet cchangeSet;
        capi::huTrove const * ctrove;
    };


    /// Encodes a Humon trove.
    /** A trove contains all the tokens and nodes that make up a Humon text. You can
     * gain access to nodes in the hierarchy, and search for nodes with certain
     * annotations or comment content. */
    class Trove
    {
    public:
        /// Creates a Trove from a UTF8 string.
        /** This function makes a new Trove object from the given string. If the string
         * is in a legal Humon format, the Trove will come back without errors, and fully
         * ready to use. Otherwise the Trove will be in an erroneous state; it will not
         * be a null trove, but rather will be loaded with no nodes, and errors marking
         * tokens. */
        [[nodiscard]] static DeserializeResult fromString(std::string_view data,
            DeserializeOptions deserializeOptions = { Encoding::utf8 },
            ErrorResponse errorResponse = ErrorResponse::stderrAnsiColor)
        {
            capi::huTrove * trove = HU_NULLTROVE;

            std::size_t sz = data.size();
            if (! validateSize(sz))
                { return ErrorCode::badParameter; }

            auto error = capi::huDeserializeTroveN(& trove, data.data(),
                static_cast<hu::size_t>(sz),
                & deserializeOptions.cparams, static_cast<capi::huErrorResponse>(errorResponse));

            if (error != capi::HU_ERROR_NOERROR &&
                error != capi::HU_ERROR_TROVEHASERRORS)
                { return static_cast<ErrorCode>(error); }
            else
                { return Trove(trove); }
        }

        /// Creates a Trove from a UTFn-encoded string.
        /** This function makes a new Trove object from the given string. If the string
         * is in a legal Humon format, the Trove will come back without errors, and fully
         * ready to use. Otherwise the Trove will be in an erroneous state; it will not
         * be a null trove, but rather will be loaded with no nodes, and errors marking
         * tokens. */
        [[nodiscard]] static DeserializeResult fromString(char const * data, hu::size_t dataLen,
            DeserializeOptions deserializeOptions = { Encoding::utf8 },
            ErrorResponse errorResponse = ErrorResponse::stderrAnsiColor)
        {
            capi::huTrove * trove = HU_NULLTROVE;
            auto error = capi::huDeserializeTroveN(& trove, data, dataLen,
                & deserializeOptions.cparams, static_cast<capi::huErrorResponse>(errorResponse));
            if (error != capi::HU_ERROR_NOERROR &&
                error != capi::HU_ERROR_TROVEHASERRORS)
                { return static_cast<ErrorCode>(error); }
            else
                { return Trove(trove); }
        }

        /// Creates a Trove from a UTF8 file.
        /** This function makes a new Trove object from the given file. If the file
         * is in a legal Humon format, the Trove will come back without errors, and fully
         * ready to use. Otherwise the Trove will be in an erroneous state; it will not
         * be a null trove, but rather will be loaded with no nodes, and errors marking
         * tokens. */
        [[nodiscard]] static DeserializeResult fromFile(std::string_view path,
            DeserializeOptions deserializeOptions = { Encoding::unknown },
            ErrorResponse errorResponse = ErrorResponse::stderrAnsiColor)
        {
            capi::huTrove * trove = HU_NULLTROVE;

            std::size_t sz = path.size();
            if (! validateSize(sz))
                { return ErrorCode::badParameter; }

            auto error = capi::huDeserializeTroveFromFile(& trove, path.data(),
                & deserializeOptions.cparams, static_cast<capi::huErrorResponse>(errorResponse));
            if (error != capi::HU_ERROR_NOERROR &&
                error != capi::HU_ERROR_TROVEHASERRORS)
                { return static_cast<ErrorCode>(error); }
            else
                { return Trove(trove); }
        }

        /// Creates a Trove from a UTF8 stream.
        /** This function makes a new Trove object from the given stream. If the stream
         * is in a legal Humon format, the Trove will come back without errors, and fully
         * ready to use. Otherwise the Trove will be in an erroneous state; it will not
         * be a null trove, but rather will be loaded with no nodes, and errors marking
         * tokens. This function does **not** close the stream once it has finished reading.
         *
         * If maxNumBytes == 0, `fromIstream` reads the stream until EOF is encountered.*/
        [[nodiscard]] static DeserializeResult fromIstream(std::istream & in,
            DeserializeOptions deserializeOptions = { Encoding::unknown }, hu::size_t maxNumBytes = 0,
            ErrorResponse errorResponse = ErrorResponse::stderrAnsiColor)
        {
            if (maxNumBytes == 0)
            {
                std::stringstream buffer;
                buffer << in.rdbuf();
                return fromString(buffer.str(), deserializeOptions, errorResponse);
            }
            else
            {
                std::string buffer;
                buffer.reserve(maxNumBytes + 1);
                in.read(buffer.data(), maxNumBytes);
                buffer[maxNumBytes] = '\0'; // TODO: Necessary?
                return fromString(buffer.data(), deserializeOptions, errorResponse);
            }
        }

        /// Create a new trove, using recorded changes.
        [[nodiscard]] static DeserializeResult fromChanges(ChangeSet & changes)
        {
            capi::huTrove * newTrove = HU_NULLTROVE;
            auto error = capi::huMakeChangedTrove(& newTrove, changes.cTrove(), & changes.cChangeSet());
            if (error != capi::HU_ERROR_NOERROR &&
                error != capi::HU_ERROR_TROVEHASERRORS)
                { return static_cast<ErrorCode>(error); }
            else
                { return Trove(newTrove); }
        }
    public:
        /// Construct a nullish Trove.
        Trove() { }
    private:
        /// Construction from static member functions.
        Trove(capi::huTrove * ctrove) : ctrove(ctrove) { }

    public:
        /// Move-construct a temporary trove object.
        Trove(Trove && rhs) noexcept
        {
            ctrove = HU_NULLTROVE;
            std::swap(ctrove, rhs.ctrove);
        }

        /// Destruct a Trove.
        ~Trove()
        {
            if (ctrove && ctrove != HU_NULLTROVE)
                { capi::huDestroyTrove(ctrove); }
        }

        Trove(Trove const & rhs) = delete;
        Trove & operator = (Trove const & rhs) = delete;

        /// Move-assign a temporary trove object.
        Trove & operator = (Trove && rhs)
        {
            if (ctrove)
            {
                capi::huDestroyTrove(ctrove);
                ctrove = HU_NULLTROVE;
            }
            std::swap(ctrove, rhs.ctrove);
            return * this;
        }

        /// Returns whether two trove objects reference the same `huTrove const *`.
        friend bool operator == (Trove const & lhs, Trove const & rhs)
            { return lhs.ctrove == rhs.ctrove; }
        /// Returns whether two trove objects reference different `huTrove const *`s.
        friend bool operator != (Trove const & lhs, Trove const & rhs)
            { return lhs.ctrove != rhs.ctrove; }
        // C++20: <=> when it's available.

        bool isValid() const       ///< Returns whether the trove is null (not valid).
            { return ctrove != HU_NULLTROVE && numErrors() == 0; }
        bool isNull() const        ///< Returns whether the trove is null (not valid).
            { return ctrove == nullptr; }
        operator bool () const     ///< Returns whether the trove is valid (not null).
            { return isValid(); }
        hu::size_t numTokens() const      ///< Returns the number of tokens in the trove.
            { return capi::huGetNumTokens(ctrove); }

        template <class IntType,
            typename std::enable_if<
                std::is_integral_v<IntType>, IntType>::type * = nullptr>
        Token token(IntType idx) const ///< Returns a Token by index.
        {
            if (idx > 0 && static_cast<std::size_t>(idx) > static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()))
                { return Token(HU_NULLTOKEN); }
            return Token(capi::huGetToken(ctrove, idx));
        }
        hu::size_t numNodes() const       ///< Returns the number of nodes in the trove.
            { return capi::huGetNumNodes(ctrove); }
        bool hasRoot() const       ///< Returns whether the trove has a root node.
            { return numNodes() > 0; }
        Node root() const          ///< Returns the root node of the trove.
            { return capi::huGetRootNode(ctrove); }

        template <class IntType,
            typename std::enable_if<
                std::is_integral_v<IntType>, IntType>::type * = nullptr>
        Node nodeByIndex(IntType idx) const   ///< Returns a Node by index.
        {
            if (idx > 0 && static_cast<std::size_t>(idx) > static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()))
                { return Node(HU_NULLNODE); }
            return Node(capi::huGetNodeByIndex(ctrove, static_cast<hu::size_t>(idx)));
        }

        /// Gets a node in the trove by its address.
        /** Given a `/`-separated sequence of dict keys or indices, this function returns
         * a node in this trove which can be found by tracing nodes from the root. The address
         * must begin with a `/` when accessing from the Trove.
         * \return Returns the {node and hu::ErrorCode::NoError} or {null node and
         * the appropriate hu::ErrorCode}, */
        Node nodeByAddress(std::string_view address) const
        {
            std::size_t sz = address.size();
            if (! validateSize(sz))
                { return Node(HU_NULLNODE); }
            return Node(capi::huGetNodeByAddressN(
                ctrove, address.data(), static_cast<hu::size_t>(sz)));
        }

        /// Returns the number of errors encountered when tokenizing and parsing the Humon.
        hu::size_t numErrors() const
            { return ctrove ? capi::huGetNumErrors(ctrove) : 0; }

        /// Returns the `idx`th error encountered when tokenizing and parsing the Humon.
        std::tuple<ErrorCode, Token> error(hu::size_t idx) const
        {
            auto cerr = capi::huGetError(ctrove, idx);
            return { static_cast<ErrorCode>(cerr->errorCode), Token(cerr->token) };
        }

        /// Returns a new collection of all errors encountered when tokenizing and parsing the Humon.
        [[nodiscard]] std::vector<std::tuple<ErrorCode, Token>> errors() const
        {
            std::vector<std::tuple<ErrorCode, Token>> vec;
            auto numErr = numErrors();
            vec.reserve(numErr);
            for (hu::size_t i = 0; i < numErr; ++i)
                { vec.push_back(error(i)); }
            return vec;
        }

        /// Returns the number of annotations associated to this trove (not to any node).
        hu::size_t numTroveAnnotations() const
            { return capi::huGetNumTroveAnnotations(ctrove); }

        /// Returns the `idx`th annotation associated to this trove (not to any node).
        std::tuple<Token, Token> troveAnnotation(hu::size_t idx) const
        {
            auto canno = capi::huGetTroveAnnotation(ctrove, idx);
            return { Token(canno->key), Token(canno->value) };
        }

        /// Returns a new collection of all annotations associated to this trove (not to any node).
        [[nodiscard]] std::vector<std::tuple<Token, Token>> troveAnnotations() const
        {
            std::vector<std::tuple<Token, Token>> vec;
            auto numAnnos = numTroveAnnotations();
            vec.reserve(numAnnos);
            for (hu::size_t i = 0; i < numAnnos; ++i)
                { vec.push_back(troveAnnotation(i)); }
            return vec;
        }

        /// Return the number of trove annotations associated to this trove (not to any node) with
        /// the specified key.
        hu::size_t numTroveAnnotationsWithKey(std::string_view key) const
        {
            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return 0; }
            return capi::huGetNumTroveAnnotationsWithKeyN(
                ctrove, key.data(), static_cast<hu::size_t>(sz));
        }

        /// Returns a new collection of all the keys of annotations associated to this trove (not
        /// to any node) with the specified key.
        [[nodiscard]] std::vector<Token> troveAnnotationsWithKey(std::string_view key) const
        {
            check();
            std::vector<Token> vec;

            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huToken const * tok = HU_NULLTOKEN;
            do
            {
                tok = capi::huGetTroveAnnotationWithKeyN(
                    ctrove, key.data(), static_cast<hu::size_t>(sz), & cursor);
                if (tok)
                    { vec.push_back(tok); }
            } while (tok != HU_NULLTOKEN);

            return vec;
        }

        /// Return the number of trove annotations associated to this trove (not to any node) with
        /// the specified value.
        hu::size_t numTroveAnnotationsWithValue(std::string_view value) const
        {
            std::size_t sz = value.size();
            if (! validateSize(sz))
                { return 0; }
            return capi::huGetNumTroveAnnotationsWithValueN(
                ctrove, value.data(), static_cast<hu::size_t>(sz));
        }

        /// Returns a new collection of all the keys of annotations associated to this trove (not
        /// to any node) with the specified value.
        [[nodiscard]] std::vector<Token> troveAnnotationsWithValue(std::string_view value) const
        {
            check();
            std::vector<Token> vec;

            std::size_t sz = value.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huToken const * tok = HU_NULLTOKEN;
            do
            {
                tok = capi::huGetTroveAnnotationWithValueN(
                    ctrove, value.data(), static_cast<hu::size_t>(sz), & cursor);
                if (tok)
                    { vec.push_back(tok); }
            } while (tok != HU_NULLTOKEN);

            return vec;
        }

        /// Returns the number of comments associated to this trove (not to any node).
        hu::size_t numTroveComments() const
            { return capi::huGetNumTroveComments(ctrove); }

        /// Returns the `idx`th comment associated to this trove (not to any node).
        std::tuple<Token, Node> troveComment(hu::size_t idx) const
        {
            auto ccomm = capi::huGetTroveComment(ctrove, idx);
            return { Token(ccomm), nullptr };
        }

        /// Returns a new collection of all comments associated to this trove (not to any node).
        [[nodiscard]] std::vector<std::tuple<Token, Node>> allTroveComments() const
        {
            std::vector<std::tuple<Token, Node>> vec;
            auto numComms = numTroveComments();
            vec.reserve(numComms);
            for (hu::size_t i = 0; i < numComms; ++i)
                { vec.push_back(troveComment(i)); }
            return vec;
        }

        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key.
        [[nodiscard]] std::vector<Node> findNodesWithAnnotationKey(std::string_view key) const
        {
            std::vector<Node> vec;

            std::size_t sz = key.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesWithAnnotationKeyN(
                    ctrove, key.data(), static_cast<hu::size_t>(sz), & cursor);
                if (node)
                    { vec.emplace_back(node); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified value.
        [[nodiscard]] std::vector<Node> findNodesWithAnnotationValue(std::string_view value) const
        {
            std::vector<Node> vec;

            std::size_t sz = value.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesWithAnnotationValueN(
                    ctrove, value.data(), static_cast<hu::size_t>(sz), & cursor);
                if (node)
                    { vec.emplace_back(node); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key and value.
        [[nodiscard]] std::vector<Node> findNodesWithAnnotationKeyValue(std::string_view key, std::string_view value) const
        {
            std::vector<Node> vec;

            std::size_t szk = key.size();
            if (! validateSize(szk))
                { return vec; }

            std::size_t szv = value.size();
            if (! validateSize(szv))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesWithAnnotationKeyValueNN(
                    ctrove, key.data(), static_cast<hu::size_t>(szk),
                    value.data(), static_cast<hu::size_t>(szv), & cursor);
                if (node)
                    { vec.emplace_back(node); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Returns a new collection of all nodes that are associated a comment containing
        /// the specified substring.
        [[nodiscard]] std::vector<Node> findNodesByCommentContaining(std::string_view containedText) const
        {
            std::vector<Node> vec;

            std::size_t sz = containedText.size();
            if (! validateSize(sz))
                { return vec; }

            hu::size_t cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesByCommentContainingN(
                    ctrove, containedText.data(), static_cast<hu::size_t>(sz), & cursor);
                if (node)
                    { vec.emplace_back(Node(node)); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Returns the entire source text of a trove (its text), including all nodes and all comments and annotations.
        /** This function returns the stored text as a view. It does not allocate or copy memory,
         * and cannot format the string.*/
        std::string_view soureText() const
        {
            check();
            auto sv = capi::huGetTroveSourceText(ctrove);
            return make_sv(sv);
        }

        /// Serializes a trove with the exact input source text.
        [[nodiscard]] std::variant<std::string, ErrorCode> toClonedString(Encoding encoding = Encoding::utf8, bool printBom = false) const
        {
            SerializeOptions sp = { WhitespaceFormat::cloned, 0, false, std::nullopt, true, "", encoding, printBom };
            return toString(sp);
        }

        /// Serializes a trove with the minimum source text necessary to accurately convey the data.
        [[nodiscard]] std::variant<std::string, ErrorCode> toMinimalString(std::optional<ColorTable> const & colors = {},
            bool printComments = true, std::string_view newline = "\n", Encoding encoding = Encoding::utf8, bool printBom = false) const
        {
            SerializeOptions sp = { WhitespaceFormat::minimal, 0, false, colors, printComments, newline, encoding, printBom };
            return toString(sp);
        }

        /// Serializes a trove with whitespace formatting suitable for readability.
        [[nodiscard]] std::variant<std::string, ErrorCode> toPrettyString(hu::col_t indentSize = 4,
            bool indentWithTabs = false, std::optional<ColorTable> const & colors = {}, bool printComments = true,
            std::string_view newline = "\n", Encoding encoding = Encoding::utf8, bool printBom = false) const
        {
            SerializeOptions sp = { WhitespaceFormat::pretty, indentSize, indentWithTabs, colors, printComments, newline, encoding, printBom };
            return toString(sp);
        }

        /// Serializes a trove to a UTF8-formatted string.
        /** Creates a UTF8 string which encodes the trove, as seen in a Humon file.
         * The contents of the file are whitespace-formatted and colorized depending on
         * the parameters.
         * \return A variant containing either the encoded string, or an error code.*/
        [[nodiscard]] std::variant<std::string, ErrorCode> toString(SerializeOptions & serializeOptions) const
        {
            hu::size_t strLength = 0;
            std::string s;
            auto error = capi::huSerializeTrove(ctrove, NULL, & strLength, & serializeOptions.cparams);
            if (error != capi::HU_ERROR_NOERROR)
                { return static_cast<ErrorCode>(error); }

            s.resize(strLength);
            error = capi::huSerializeTrove(ctrove, s.data(), & strLength, & serializeOptions.cparams);
            if (error != capi::HU_ERROR_NOERROR)
                { return static_cast<ErrorCode>(error); }

            return s;
        }

        /// Serializes a trove with the exact input source text.
        [[nodiscard]] std::variant<hu::size_t, ErrorCode> toClonedFile(std::string_view path,
            Encoding encoding = Encoding::utf8, bool printBom = false) const
        {
            SerializeOptions sp = { WhitespaceFormat::cloned, 0, false, std::nullopt, true, "", encoding, printBom };
            return toFile(path, sp);
        }

        /// Serializes a trove with the minimum source text necessary to accurately convey the data.
        [[nodiscard]] std::variant<hu::size_t, ErrorCode> toMinimalFile(std::string_view path,
            std::optional<ColorTable> const & colors = {}, bool printComments = true,
            std::string_view newline = "\n", Encoding encoding = Encoding::utf8, bool printBom = false) const
        {
            SerializeOptions sp = { WhitespaceFormat::minimal, 0, false, colors, printComments, newline, encoding, printBom };
            return toFile(path, sp);
        }

        /// Serializes a trove with whitespace formatting suitable for readability.
        [[nodiscard]] std::variant<hu::size_t, ErrorCode> toPrettyFile(std::string_view path,
            hu::col_t indentSize = 4, bool indentWithTabs = false, std::optional<ColorTable> const & colors = {}, bool printComments = true,
            std::string_view newline = "\n", Encoding encoding = Encoding::utf8, bool printBom = false) const
        {
            SerializeOptions sp = { WhitespaceFormat::pretty, indentSize, indentWithTabs, colors, printComments, newline, encoding, printBom };
            return toFile(path, sp);
        }

        /// Serializes a trove to a UTF8-formatted file.
        /** Creates or overwrites a UTF8 file which encodes the trove, as seen in a Humon file.
         * The contents of the file are whitespace-formatted and colorized depending on
         * the parameters.
         * \return A variant containing either the number of bytes written to the file, or the
         * an error code.
         */
        [[nodiscard]] std::variant<hu::size_t, ErrorCode> toFile(std::string_view path, SerializeOptions & SerializeOptions) const
        {
            hu::size_t outputLength = 0;
            std::size_t sz = path.size();
            if (! validateSize(sz))
                { return ErrorCode::badParameter; }

            auto error = capi::huSerializeTroveToFile(ctrove, path.data(), & outputLength, & SerializeOptions.cparams);
            if (error != capi::HU_ERROR_NOERROR)
                { return error; }

            return outputLength;
        }

        /// Returns whether `idx` is a valid child index of the root node. (Whether root has
        /// at least `idx`+1 children.)
        template <class IntType,
            typename std::enable_if<
                std::is_integral_v<IntType>, IntType>::type * = nullptr>
        bool operator % (IntType idx) const
           { return hasRoot() && root() % idx; }

        /// Returns whether the root is a dict and has a child with the specified key.
        bool operator % (std::string_view key) const
            { return hasRoot() && root() % key; }

        /// Returns the root node's `idx`th child.
        template <class IntType,
            typename std::enable_if<
                std::is_integral_v<IntType>, IntType>::type * = nullptr>
        Node operator / (IntType idx) const
        {
            auto ch = root();
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (! ch)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            if (! ch || idx < 0)
                { return Node(HU_NULLNODE); }

            ch = ch.child(idx);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (! ch)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            return ch;
        }

        /// Returns the root node's child with the specified key.
        Node operator / (std::string_view key) const
        {
            auto ch = root();
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (! ch)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            if (! ch)
                { return Node(HU_NULLNODE); }

            if (key.size() > static_cast<std::size_t>(std::numeric_limits<hu::size_t>::max()))
                { return Node(HU_NULLNODE); }

            ch = ch.child(key);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (! ch)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            return ch;
        }

        capi::huTrove const * getCTrove() const { return ctrove; }

    private:
        void check() const { checkNotNull(ctrove); }

        capi::huTrove * ctrove = nullptr;
    };

    /// Fills an array with string table values for ANSI color terminals.
    inline ColorTable getAnsiColorTable()
    {
        ColorTable table;
        capi::huStringView nativeTable[capi::HU_COLORCODE_NUMCOLORS];
        capi::huFillAnsiColorTable(nativeTable);
        for (int i = 0; i < capi::HU_COLORCODE_NUMCOLORS; ++i)
            { table[(size_t) i] = {nativeTable[(size_t) i].ptr, static_cast<std::size_t>(nativeTable[(size_t) i].size)}; }

        return table;
    }

    inline ChangeSet::ChangeSet(Trove const & trove)
    {
        ctrove = trove.getCTrove();
        capi::huInitChangeSet(& cchangeSet, huGetAllocator(ctrove));
    }

}
