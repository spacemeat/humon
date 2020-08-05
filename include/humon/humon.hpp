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


// Defining this turns off noexcept decorations on most functions.
#ifdef HUMON_SUPPRESS_NOEXCEPT
#define HUMON_NOEXCEPT
#else
#define HUMON_NOEXCEPT noexcept
#endif

/// Defining this turns off noexcept decorations on path functions 
/// (operator /, etc).
#ifdef HUMON_SUPPRESS_PATH_NOEXCEPT
#define HUMON_PATH_NOEXCEPT
#else
#define HUMON_PATH_NOEXCEPT noexcept
#endif

/// Defining this enables throwin exceptions on hu::Token and hu::Trove member
/// functions. Normally these don't throw, but you can enable this to help find
/// where othewise silent failure are occurring.
/// #define HUMON_USE_NULLISH_EXCEPTIONS

/// Defining this enables exceptions to be thrown on unfound path calls 
/// (hu::Trove::nodeByAddress, huNode::nodeByAddress, hu::Trove::operator/
/// or hu::Node::operator/); Normally these silently fail, and it can be
/// difficult to determine where.
/// #define HUMON_USE_NODE_PATH_EXCEPTIONS


/// The Humon namespace.
namespace hu
{
    // This namespace contains the C API. Not included in doxygen on purpose.
    namespace capi
    {
#include "humon.h"
    }

    /// Specifies a UTFn text encoding.
    enum class Encoding : int
    {
        utf8 = capi::HU_ENCODING_UTF8,
        utf16be = capi::HU_ENCODING_UTF16_BE,
        utf16le = capi::HU_ENCODING_UTF16_LE,
        utf32be = capi::HU_ENCODING_UTF32_BE,
        utf32le = capi::HU_ENCODING_UTF32_LE,
        unknown = capi::HU_ENCODING_UNKNOWN
    };

    /// Return a string representation of a hu::Encoding.
    inline char const * to_string(Encoding rhs) HUMON_NOEXCEPT
    {
        return capi::huEncodingToString((int) rhs);
    }

    /// Specifies the kind of data represented by a particular hu::Token.
    enum class TokenKind : int
    {
        null = capi::HU_TOKENKIND_NULL,                 ///< Invalid token. Malformed, or otherwise nonexistent.
        eof = capi::HU_TOKENKIND_EOF,                   ///< The end of the token stream or string.
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
    inline char const * to_string(TokenKind rhs) HUMON_NOEXCEPT
    {
        return capi::huTokenKindToString((int) rhs);
    }

    /// Specifies the kind of node represented by a particular hu::Node.
    enum class NodeKind : int
    {
        null = capi::HU_NODEKIND_NULL,      ///< Invalid node. An invalid address returns a null node.
        list = capi::HU_NODEKIND_LIST,      ///< List node. The node contains a sequence of unassociated objects in maintained order.
        dict = capi::HU_NODEKIND_DICT,      ///< Dict node. The node contains a sequence of string-associated objects in maintained order.
        value = capi::HU_NODEKIND_VALUE     ///< Value node. The node contains a string value, and no children.
    };

    /// Return a string representation of a hu::NodeKind.
    inline char const * to_string(NodeKind rhs) HUMON_NOEXCEPT
    {
        return capi::huNodeKindToString((int) rhs);
    }

    /// Specifies the style of whitespacing in Humon text.
    enum class WhitespaceFormat : int
    {
        cloned = capi::HU_WHITESPACEFORMAT_CLONED,              ///< Byte-for-byte copy of the original.
        minimal = capi::HU_WHITESPACEFORMAT_MINIMAL,            ///< Reduces as much whitespace as possible.
        pretty = capi::HU_WHITESPACEFORMAT_PRETTY               ///< Formats the text in a standard, human-friendly way.
    };

    /// Return a string representation of a hu::WhitespaceFormat.
    inline char const * to_string(WhitespaceFormat rhs) HUMON_NOEXCEPT
    {
        return capi::huWhitespaceFormatToString((int) rhs);
    }

    /// Specifies a tokenizing or parsing error code, or lookup error.
    enum class ErrorCode : int
    {
        noError = capi::HU_ERROR_NOERROR,                       ///< No error.
        badEncoding = capi::HU_ERROR_BADENCODING,               ///< The Unicode encoding is malformed.
        unfinishedQuote = capi::HU_ERROR_UNFINISHEDQUOTE,       ///< The quoted text was not endquoted.
        unfinishedCStyleComment = 
            capi::HU_ERROR_UNFINISHEDCSTYLECOMMENT,             ///< The C-style comment was not closed.
        unexpectedEof = capi::HU_ERROR_UNEXPECTEDEOF,           ///< The text ended early.
        tooManyRoots = capi::HU_ERROR_TOOMANYROOTS,             ///< There is more than one root node detected.
        nonuniqueKey = capi::HU_ERROR_NONUNIQUEKEY,             ///< A non-unique key was encountered in a dict or annotation.
        syntaxError = capi::HU_ERROR_SYNTAXERROR,               ///< General syntax error.
        notFound = capi::HU_ERROR_NOTFOUND,                     ///< No node could be found at the address.
        illegal = capi::HU_ERROR_ILLEGAL,                       ///< The operation was illegal.
        badParameter = capi::HU_ERROR_BADPARAMETER,             ///< An API parameter is malformed or illegal.
        badFile = capi::HU_ERROR_BADFILE,                       ///< An attempt to open or operate on a file failed.
        outOfMemory = capi::HU_ERROR_OUTOFMEMORY,               ///< An internal memory allocation failed.
        troveHasErrors = capi::HU_ERROR_TROVEHASERRORS          ///< The loading function succeeded, but the loaded trove has errors.
    };

    /// Return a string representation of a hu::ErrorCode.
    inline char const * to_string(ErrorCode rhs) HUMON_NOEXCEPT
    {
        return capi::huOutputErrorToString((int) rhs);
    }

    /// Specifies how a trove responds to errors.
    enum class ErrorResponse
    {
        mum = capi::HU_ERRORRESPONSE_MUM,
        stdout = capi::HU_ERRORRESPONSE_STDOUT,
        stderr = capi::HU_ERRORRESPONSE_STDERR,
        stdoutAnsiColor = capi::HU_ERRORRESPONSE_STDOUTANSICOLOR,
        stderrAnsiColor = capi::HU_ERRORRESPONSE_STDERRANSICOLOR,
        numResponses = capi::HU_ERRORRESPONSE_NUMRESPONSES
    };

    /// Specifies a style ID for colorized printing.
    enum class ColorCode
    {
        tokenStreamBegin = capi::HU_COLORCODE_TOKENSTREAMBEGIN,     ///< Beginning-of-token stream color code.
        tokenStreamEnd = capi::HU_COLORCODE_TOKENSTREAMEND,         ///< End-of-token stream color code.
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

    /// Value extraction template for in-line grokking of value nodes.
    /** This template is a helper to extract machine values out of Humon value nodes.
     * Standard types are supported, and custom extractors can be specified in two
     * ways:
     * 1. Overload `std::from_chars()` for your type.
     * 2. Specialize `hu::val<T>` for your type `T`, and implement the `extract()`
     * function.
     * Usage: Use in conjunction with the `/` operator on a value node. This allows
     * you to extract a value in the natural flow of things:
     * `auto numEyes = node / "frogs" / "numEyes" / hu::val<int> {};`
     * `auto color = node / "frogs" / "colors" / 0 / hu::val<AnimalColorsEnum> {};`
     */
    template <class T>
    struct val
    { };

    /// Extractor for type int.
    template<>
    struct val<int>
    {
        static inline int extract(std::string_view valStr) HUMON_PATH_NOEXCEPT
        {
            int val;
            auto [p, ec] = std::from_chars(valStr.data(), valStr.data() + valStr.size(), val);
            if (ec == std::errc())
                { return val; }
            else
                { return int {}; }
        }
    };


    /// Extractor for type float.
    template <>
    struct val<float>
    {
        static inline float extract(std::string_view valStr) HUMON_PATH_NOEXCEPT
        {
            // TODO: (gcc): Once from_chars(..float&) is complete, use this or remove this specialization. We're making a useless string here and it maketh me to bite metal. Don't forget to HUMON_NOEXCEPT.
    #if 0
            float val;
            auto [p, ec] = std::from_chars(valStr.data(), valStr.data() + valStr.size(), val, std::chars_format::general);
            if (ec == std::errc())
                { return val; }
            else
                { return -1; }
    #endif
            return std::stof(std::string(valStr));
        }
    };

    /// Extractor for type double.
    template <>
    struct val<double>
    {
        static inline double extract(std::string_view valStr) HUMON_PATH_NOEXCEPT
        {
            // TODO: (gcc): Once from_chars(..double&) is complete, use this or remove this specialization. We're making a useless string here and it maketh me to bite metal. Don't forget to HUMON_NOEXCEPT.
    #if 0
            double val;
            auto [p, ec] = std::from_chars(valStr.data(), valStr.data() + valStr.size(), val, std::chars_format::general);
            if (ec == std::errc())
                { return val; }
            else
                { return -1; }
    #endif
            return std::stod(std::string(valStr));
        }
    };

    /// Extractor for type std::string_view.
    template <>
    struct val<std::string_view>
    {
        static inline std::string_view extract(std::string_view valStr) HUMON_PATH_NOEXCEPT
        {
            return valStr;
        }
    };

    /// Extractor for type std::string.
    template <>
    struct val<std::string>
    {
        static inline std::string extract(std::string_view valStr) HUMON_PATH_NOEXCEPT
        {
            return std::string(valStr);
        }
    };

    /// Extractor for type bool. Parses English spelling of "true" for truth.
    template <>
    struct val<bool>
    {
        static inline bool extract(std::string_view valStr) HUMON_PATH_NOEXCEPT
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

    // Conversion from capi::huStringView to std::string_view. Mostly internal, no doxygen.
    inline std::string_view make_sv(capi::huStringView const & husv) HUMON_NOEXCEPT
    {
        return std::string_view(husv.ptr, husv.size);
    }

    using ColorTable = std::array<std::string_view, capi::HU_COLORCODE_NUMCOLORS>;

    /// Encapsulates a selection of parameters to control how Humon interprets the input for loading.
    class DeserializeOptions
    {
    public:
        /// Construct with sane defaults.
        DeserializeOptions(Encoding encoding = Encoding::unknown, bool strictUnicode = true, int tabSize = 4) HUMON_NOEXCEPT
        {
            capi::huInitDeserializeOptions(& cparams, static_cast<int>(encoding), strictUnicode, tabSize);
        }

        /// Expect a particular Unicode encoding, or Encoding::unknown.
        void setEncoding(Encoding encoding) HUMON_NOEXCEPT { cparams.encoding = static_cast<int>(encoding); }
        /// Allow code points that are out of Unicode range.
        void setAllowOutOfRangeCodePoints(bool shallWe) HUMON_NOEXCEPT { cparams.allowOutOfRangeCodePoints = shallWe; }
        /// Allow unpaired surrogate code units in UTF-16.
        void setAllowUtf16UnmatchedSurrogates(bool shallWe) HUMON_NOEXCEPT { cparams.allowUtf16UnmatchedSurrogates = shallWe; }
        /// Use this tab size when computing the column of a token and the token stream contains tabs.
        void setTabSize(int tabSize) HUMON_NOEXCEPT { cparams.tabSize = tabSize; }

        /// Get the encoding to expect.
        int encoding() const HUMON_NOEXCEPT { return cparams.encoding; }
        /// Get whether out-of-range Unicode code points are allowed.
        bool allowOutOfRangeCodePoints() const HUMON_NOEXCEPT { return cparams.allowOutOfRangeCodePoints; }
        /// Get whether unpaired surrogates in UTF-16 are allowed.
        bool allowUtf16UnmatchedSurrogates() const HUMON_NOEXCEPT { return cparams.allowUtf16UnmatchedSurrogates; }
        /// Get the tab size used to compute column information in token streams that contain tabs.
        int tabSize() const HUMON_NOEXCEPT { return cparams.tabSize; }

        /// Aggregated C structure.
        capi::huDeserializeOptions cparams;
    };

    /// Encapsulates a selection of parameters to control the serialization of a trove.
    class SerializeOptions
    {
    public:
        /// Construct with sane defaults.
        SerializeOptions(WhitespaceFormat WhitespaceFormat, int indentSize = 4, bool indentWithTabs = false,
            std::optional<ColorTable> const & colors = {}, bool printComments = true, 
            std::string_view newline = "\n", bool printBom = false) HUMON_NOEXCEPT
        {
            capi::huInitSerializeOptionsN(& cparams, static_cast<int>(WhitespaceFormat), indentSize, indentWithTabs,
                false, capiColorTable, printComments, newline.data(), newline.size(), printBom);
            setColorTable(colors);
        }

        /// Set the whitespace formatting.
        void setFormat(WhitespaceFormat WhitespaceFormat) HUMON_NOEXCEPT { cparams.WhitespaceFormat = static_cast<int>(WhitespaceFormat); }
        /// Set the number of spaces to use for indentation.
        void setIndentSize(int indentSize) HUMON_NOEXCEPT { cparams.indentSize = indentSize; }
        /// Use tab instead of spaces for indentation.
        void setIndentWithTabs(bool shallWe) HUMON_NOEXCEPT { cparams.indentWithTabs = shallWe; }

        /// Use the given color table.
        void setColorTable(std::optional<ColorTable> const & colors) HUMON_NOEXCEPT
        {
            if (colors)
            {
                std::string_view const * sv = (* colors).data();
                for (size_t i = 0; i < capi::HU_COLORCODE_NUMCOLORS; ++i)
                {
                    capiColorTable[i].ptr = sv[i].data();
                    capiColorTable[i].size = sv[i].size();
                }
                cparams.usingColors = true;
            }
            else
                { cparams.usingColors = false; }
        }

        /// Set whether to print comment tokens in the output.
        void setPrintComments(bool shallWe) HUMON_NOEXCEPT { cparams.printComments = shallWe; }
        /// Set the character string to use for a newline.
        void setNewline(std::string_view newline) HUMON_NOEXCEPT { cparams.newline.ptr = newline.data(); cparams.newline.size = newline.size(); }

        /// Get the whitespace formatting.
        WhitespaceFormat whitespaceFormat() const HUMON_NOEXCEPT { return static_cast<WhitespaceFormat>(cparams.WhitespaceFormat); }
        /// Get the number of spaces to use for indentation.
        int indentSize() { return cparams.indentSize; }
        /// Get whether to use tabs instead of spaces for indentation.
        bool indentWithTabs() { return cparams.indentWithTabs; }

        /// Get the color table.
        std::optional<ColorTable> colorTable() const HUMON_NOEXCEPT
        {
            if (cparams.usingColors == false)
                { return std::nullopt; }
            
            ColorTable newColorTable;
            std::string_view * sv = newColorTable.data();
            for (int i = 0; i < capi::HU_COLORCODE_NUMCOLORS; ++i)
            {
                sv[i] = { capiColorTable[i].ptr,
                          (size_t) capiColorTable[i].size };
            }
            return newColorTable;
        }

        /// Get whether to print comments.
        bool printComments() const HUMON_NOEXCEPT { return cparams.printComments; }
        /// Get the newline sequence.
        std::string newline() const HUMON_NOEXCEPT { return { cparams.newline.ptr, (size_t) cparams.newline.size }; }

        /// Aggregated C struct.
        capi::huSerializeOptions cparams;
        capi::huStringView capiColorTable[capi::HU_COLORCODE_NUMCOLORS];
    };

    class Trove;

    typedef std::variant<Trove, ErrorCode> DeserializeResult;
    typedef std::variant<std::string, ErrorCode> SerializeResult;

    // Either throws or returns false.
    inline void checkNotNull(void const * cp) HUMON_NOEXCEPT
    {
#ifdef HUMON_USING_EXCEPTIONS
        if (cp == nullptr)
            { throw std::runtime_error("object is nullish"); }
#endif
    }

    /// Encodes a token read from Humon text.
    /** This class encodes file location and buffer location information about a
     * particular token in a Humon file. Every token is read and tracked with a 
     * hu::Token. */
    class Token
    {
    public:
        Token(capi::huToken const * ctoken) HUMON_NOEXCEPT : ctoken(ctoken) { }
        bool isValid() const HUMON_NOEXCEPT           ///< Returns whether the token is valid (not nullish).
            { return ctoken != nullptr; }
        bool isNullish() const HUMON_NOEXCEPT         ///< Returns whether the token is nullish (not valid).
            { return ctoken == nullptr; }
        operator bool() const HUMON_NOEXCEPT          ///< Implicit validity test.
            { return isValid(); }
        TokenKind kind() const HUMON_NOEXCEPT         ///< Returns the kind of token this is.
            { check(); return static_cast<TokenKind>(
                isValid() ? ctoken->kind : capi::HU_TOKENKIND_NULL); }
        std::string_view str() const HUMON_NOEXCEPT   ///< Returns the string value of the token.
            { check(); return isValid() ? make_sv(ctoken->str) : ""; }
        int line() const HUMON_NOEXCEPT               ///< Returns the line number of the first character of the token in the file.
            { check(); return isValid() ? ctoken->line : 0; }
        int col() const HUMON_NOEXCEPT                ///< Returns the column number of the first character of the token in the file.
            { check(); return isValid() ? ctoken->col : 0; }
        int endLine() const HUMON_NOEXCEPT            ///< Returns the line number of the last character of the token in the file.
            { check(); return isValid() ? ctoken->endLine : 0; }
        int endCol() const HUMON_NOEXCEPT             ///< Returns the column number of the last character of the token in the file.
            { check(); return isValid() ? ctoken->endCol : 0; }
        operator std::string_view() HUMON_NOEXCEPT    ///< String view conversion.
            { return str(); }

        friend std::ostream & operator <<(std::ostream & out, Token rhs) HUMON_NOEXCEPT
        {
            out << rhs.str();
            return out;
        }
    private:
        void check() const HUMON_NOEXCEPT { checkNotNull(ctoken); }

        capi::huToken const * ctoken;
    };

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
        Node() HUMON_NOEXCEPT : cnode(HU_NULLNODE) { }
        Node(capi::huNode const * cnode) HUMON_NOEXCEPT : cnode(cnode) { }
        /// Return whether two Node objects refer to the same node.
        friend bool operator == (Node const & lhs, Node const & rhs) HUMON_NOEXCEPT
            { return lhs.cnode == rhs.cnode; }
        /// Return whether two Node objects refer to the different nodes.
        friend bool operator != (Node const & lhs, Node const & rhs) HUMON_NOEXCEPT
            { return lhs.cnode != rhs.cnode; }
        // TODO: operator <=> when available.
        bool isValid() const HUMON_NOEXCEPT               ///< Returns whether the node is valid (not nullish).
            { return cnode != nullptr; }
        bool isNullish() const HUMON_NOEXCEPT             ///< Returns whether the node is nullish (not valid).
            { return cnode == nullptr; }
        operator bool()                             ///< Implicit validity test.
            { return isValid(); }
        bool isRoot() const HUMON_NOEXCEPT
            { check(); return nodeIndex() == 0; }
        int nodeIndex() const HUMON_NOEXCEPT              ///< Returns the node index within the trove. Rarely needed.
            { check(); return isValid() ? cnode->nodeIdx : -1; }
        NodeKind kind() const HUMON_NOEXCEPT              ///< Returns the kind of node this is.
            { check(); return static_cast<NodeKind>(
                isValid() ? cnode->kind : capi::HU_NODEKIND_NULL); }
        Token firstToken() const HUMON_NOEXCEPT           ///< Returns the first token which contributes to this node, including any annotation and comment tokens.
            { check(); return Token(isValid() ? cnode->firstToken : HU_NULLTOKEN); }
        Token firstValueToken() const HUMON_NOEXCEPT      ///< Returns the first token of this node's actual value; for a container, it points to the opening brac(e|ket).
            { check(); return Token(isValid() ? cnode->valueToken : HU_NULLTOKEN); }
        Token lastValueToken() const HUMON_NOEXCEPT       ///< Returns the last token of this node's actual value; for a container, it points to the closing brac(e|ket).
            { check(); return Token(isValid() ? cnode->lastValueToken : HU_NULLTOKEN); }
        Token lastToken() const HUMON_NOEXCEPT            ///< Returns the last token of this node, including any annotation and comment tokens.
            { check(); return Token(isValid() ? cnode->lastToken :  HU_NULLTOKEN); }
        Node parent() const HUMON_NOEXCEPT                ///< Returns the parent node of this node, or the null node if this is the root.
            { check(); return Node(capi::huGetParent(cnode)); }
        int childOrdinal() const HUMON_NOEXCEPT           ///< Returns the index of this node vis a vis its sibling nodes (starting at 0).
            { check(); return isValid() ? cnode->childOrdinal : -1; }
        int numChildren() const HUMON_NOEXCEPT            ///< Returns the number of children of this node.
            { check(); return capi::huGetNumChildren(cnode); }

        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node child(IntType idx) const HUMON_NOEXCEPT      ///< Returns the child node, by child index.
            { check(); return capi::huGetChildByIndex(cnode, idx); }
        Node child(std::string_view key) const HUMON_NOEXCEPT  ///< Returns the child node, by key (if this is a dict).
            { check(); return capi::huGetChildByKeyN(cnode, key.data(), key.size()); }
        Node firstChild() const HUMON_NOEXCEPT            ///< Returns the first child node of this node.
            { check(); return child(0); }
        Node nextSibling() const HUMON_NOEXCEPT        ///< Returns the node ordinally after this one in the parent's children, or the null node if it's the last.
            { check(); return Node(capi::huGetNextSibling(cnode)); }
        /// Access a node by an address relative to this node.
        /** A relative address is a single string, which contains as contents a `/`-delimited path
         * through the hierarchy. A key or index between the slashes indicates the child node to
         * access. */
        Node nodeByAddress(std::string_view relativeAddress) const HUMON_NOEXCEPT
            { check(); return capi::huGetRelativeN(cnode, relativeAddress.data(), relativeAddress.size()); }
        bool hasKey() const HUMON_NOEXCEPT                ///< Returns whether this node has a key. (If it's in a dict.)
            { check(); return capi::huHasKey(cnode); }
        Token key() const HUMON_NOEXCEPT                  ///< Returns the key token, or the null token if this is not in a dict.
            { check(); return Token(isValid() ? cnode->keyToken : HU_NULLTOKEN); }
        Token value() const HUMON_NOEXCEPT                ///< Returns the first value token that encodes this node.
            { check(); return Token(isValid() ? cnode->valueToken : HU_NULLTOKEN); }
        int numAnnotations() const HUMON_NOEXCEPT         ///< Returns the number of annotations associated to this node.
            { check(); return capi::huGetNumAnnotations(cnode); }

        /// Returns the `idx`th annotation.
        /** Returns a <Token, Token> referencing the key and value of the annotation
         * accessed by index. */
        std::tuple<Token, Token> annotation(int idx) const HUMON_NOEXCEPT 
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
        [[nodiscard]] std::vector<std::tuple<Token, Token>> allAnnotations() const HUMON_NOEXCEPT
        {
            check();
            std::vector<std::tuple<Token, Token>> vec;
            int numAnnos = numAnnotations();
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec.push_back(annotation(i)); }
            return vec;
        }

        /// Returns the number of annotations associated to this node, with the specified key.
        /** A node can have multiple annotations with the same key. This is legal in Humon, but
         * rare. This function returns the number of annotations associated to this node with
         * the specified key. */
        bool hasAnnotation(std::string_view key) const HUMON_NOEXCEPT
            { check(); return capi::huHasAnnotationWithKeyN(cnode, key.data(), key.size()); }

        /// Returns a Token referencing the value of the annotation accessed by key.
        Token const annotation(std::string_view key) const HUMON_NOEXCEPT 
        { 
            check();
            auto canno = capi::huGetAnnotationWithKeyN(cnode, key.data(), key.size());
            return Token(canno);
        }

        /// Returns the number of annotations associated to this node, with the specified key.
        /** A node can have multiple annotations, each with different keys which have the same 
         * value. This function returns the number of annotations associated to this node with
         * the specified value. */
        int numAnnotationsWithValue(std::string_view value) const HUMON_NOEXCEPT
            { check(); return capi::huGetNumAnnotationsWithValueN(cnode, value.data(), value.size()); }
        
        /// Returns a new collection of all this node's annotations with the specified value.
        /** Creates a new vector of Tokens, each referencing the key of an annotation that 
         * has the specified value. */
        [[nodiscard]] std::vector<Token> annotationsWithValue(std::string_view value) const HUMON_NOEXCEPT
        {
            check();
            std::vector<Token> vec;
            int cursor = 0;
            capi::huToken const * tok = HU_NULLTOKEN;
            do
            {
                tok = capi::huGetAnnotationWithValueN(cnode, value.data(), value.size(), & cursor);
                if (tok)
                    { vec.push_back(tok); }
            } while (tok != HU_NULLTOKEN);
            
            return vec;
        }

        /// Returns the number of comments associated to this node.
        int numComments() const HUMON_NOEXCEPT
            { check(); return capi::huGetNumComments(cnode); }
        /// Returns the `idx`th comment associated to this node.
        /** Returns a Token of the `idx`th ordinal comment associated with this node. */
        Token comment(int idx) const HUMON_NOEXCEPT 
            { check(); return capi::huGetComment(cnode, idx); }

        /// Returns a new collection of all comments associated to this node.
        [[nodiscard]] std::vector<Token> allComments() const HUMON_NOEXCEPT
        {
            check();
            std::vector<Token> vec;
            int numComms = numComments();
            vec.reserve(numComms);
            for (int i = 0; i < numComms; ++i)
                { vec.push_back(comment(i)); }
            return vec;
        }

        bool hasCommentsContaining(std::string_view containedString) const HUMON_NOEXCEPT
        {
            check();
            return capi::huHasCommentsContainingN(cnode, containedString.data(), containedString.size());
        }

        /// Returns a new collection of all comments associated to this node containing the specified substring.
        [[nodiscard]] std::vector<Token> commentsContaining(std::string_view containedString) const HUMON_NOEXCEPT
        {
            check();
            std::vector<Token> vec;
            int cursor = 0;
            capi::huToken const * comm = NULL;
            do
            {
                comm = capi::huGetCommentsContainingN(cnode, containedString.data(), containedString.size(), & cursor);
                if (comm)
                    { vec.emplace_back(comm); }
            } while (comm != HU_NULLTOKEN);

            return vec;
        }

        /// Returns whether the specified index is a valid child index of this list or dict node.
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        bool operator % (IntType idx) const HUMON_PATH_NOEXCEPT
        {
            check();
            int cidx = static_cast<int>(idx);
            if (isValid() && (kind() == NodeKind::dict || kind() == NodeKind::list))
                { return cidx >= 0 && cidx < numChildren(); }
            return false;
        }

        /// Returns whether the specified key is a valid child key of this dict node.
        bool operator % (std::string_view key) const HUMON_PATH_NOEXCEPT
        {
            check();
            if (kind() == NodeKind::dict)
                { return isValid() && child(key).isValid(); }
            return false;
        }

        /// Returns the `idx`th child of this node.
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const HUMON_PATH_NOEXCEPT
        {
            check();
            if (isValid())
            {
                auto ch = capi::huGetChildByIndex(cnode, idx);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
                if (ch == nullptr)
                    { throw std::runtime_error("Illegal path entry"); }
#endif
                return Node(ch);
            }
            return Node(HU_NULLNODE);
        }

        /// Returns the child of this node by key.
        Node operator / (std::string_view key) const HUMON_PATH_NOEXCEPT
        {
            check();
            if (isValid())
            {
                auto ch = capi::huGetChildByKeyN(cnode, key.data(), key.size());
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
                if (ch == nullptr)
                    { throw std::runtime_error("Illegal path entry"); }
#endif
                return Node(ch);
            }
            return Node(HU_NULLNODE);
        }

        /// Return the parent of this node.
        Node operator / (Parent p) const HUMON_PATH_NOEXCEPT
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
         * performed by passing a dummy object of type val<U> which, if implemented
         * for type U, will invoke that type's extract function. If there is no val<U>
         * defined, the default val<U> calls std::from_chars(... U&). Any type that
         * has an overload of std::from_chars (integers or floating point types), or a
         * specialization of val<U>, will return the converted value.
         * 
         * Users can implement a specialization of val<U> for a custom type, providing
         * the analogous extract() member function.
         * 
         * The purpose of val<T> is to allow users to write code like:
         *      auto sv = materialsTrove / "assets" / "brick-diffuse" / "src" / 
         *                0 / h::val<string_view> {};*/
        template <class U>
        [[nodiscard]] inline U operator / (val<U> ve) const HUMON_PATH_NOEXCEPT
        {
            check();
            if (kind() != NodeKind::value)
                { return U {}; }
            return ve.extract(make_sv(cnode->valueToken->str));
        }

        /// Returns the entire text contained by this node and all its children.
        /** The entire text of this node is returned, including all its children's 
         * texts, and any comments and annotations associated to this node. */
        std::string_view nestedValue() const HUMON_NOEXCEPT
        {
            check();
            auto sv = capi::huGetTokenStream(cnode);
            return make_sv(sv);
        }

        /// Generates and returns the address of this node.
        /** Each node in a trove has a unique address, separate from its node index, 
         * which is represented by a series of keys or index values from the root,
         * separated by `/`s. The address is guaranteed to work with 
         * Trove::nodeByAddress(). */
        [[nodiscard]] std::string address() const HUMON_NOEXCEPT
        {
            check();
            int len = 0;
            capi::huGetAddress(cnode, NULL, & len);
            std::string s;
            s.resize(len);
            capi::huGetAddress(cnode, s.data(), & len);
            return s;
        }

    private:
        void check() const HUMON_NOEXCEPT { checkNotNull(cnode); }

        capi::huNode const * cnode;
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
            ErrorResponse errorRespose = ErrorResponse::stderrAnsiColor) HUMON_NOEXCEPT
        {
            capi::huTrove const * trove = HU_NULLTROVE;
            int error = capi::huDeserializeTroveN(& trove, data.data(), data.size(), 
                & deserializeOptions.cparams, static_cast<int>(errorRespose));

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
        [[nodiscard]] static DeserializeResult fromString(char const * data, int dataLen, 
            DeserializeOptions deserializeOptions = { Encoding::utf8 }, 
            ErrorResponse errorRespose = ErrorResponse::stderrAnsiColor) HUMON_NOEXCEPT
        {
            capi::huTrove const * trove = HU_NULLTROVE;
            int error = capi::huDeserializeTroveN(& trove, data, dataLen, 
                & deserializeOptions.cparams, static_cast<int>(errorRespose));
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
            ErrorResponse errorRespose = ErrorResponse::stderrAnsiColor) HUMON_NOEXCEPT
        {
            capi::huTrove const * trove = HU_NULLTROVE;
            int error = capi::huDeserializeTroveFromFileN(& trove, path.data(), path.size(), 
                & deserializeOptions.cparams, static_cast<int>(errorRespose));
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
         * tokens. This function does not close the stream once it has finished reading.
         * 
         * \maxNumBytes: If 0, `fromIstream` reads the stream until EOF is encountered.*/
        [[nodiscard]] static DeserializeResult fromIstream(std::istream & in, 
            DeserializeOptions deserializeOptions = { Encoding::unknown }, size_t maxNumBytes = 0, 
            ErrorResponse errorResponse = ErrorResponse::stderrAnsiColor) HUMON_NOEXCEPT
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
    
    public:
        /// Construct a Trove which decorates a null trove. Useful for collections of troves.
        Trove() HUMON_NOEXCEPT { }
    private:
        /// Construction from static member functions.
        Trove(capi::huTrove const * ctrove) HUMON_NOEXCEPT : ctrove(ctrove) { }
 
    public:
        Trove(Trove && rhs) HUMON_NOEXCEPT
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

        Trove & operator = (Trove && rhs) HUMON_NOEXCEPT
        {
            if (ctrove)
            {
                capi::huDestroyTrove(ctrove);
                ctrove = HU_NULLTROVE;
            }
            std::swap(ctrove, rhs.ctrove);
            return * this;
        }

        friend bool operator == (Trove const & lhs, Trove const & rhs) HUMON_NOEXCEPT
            { return lhs.ctrove == rhs.ctrove; }
        friend bool operator != (Trove const & lhs, Trove const & rhs) HUMON_NOEXCEPT
            { return lhs.ctrove != rhs.ctrove; }
        // C++20: <=> when it's available.

        bool isValid() const HUMON_NOEXCEPT       ///< Returns whether the trove is null (not valid).
            { return ctrove != HU_NULLTROVE && numErrors() == 0; }
        bool isNull() const HUMON_NOEXCEPT        ///< Returns whether the trove is null (not valid).
            { return ctrove == nullptr; }
        operator bool () const HUMON_NOEXCEPT     ///< Returns whether the trove is valid (not null).
            { return isValid(); }
        int numTokens() const HUMON_NOEXCEPT      ///< Returns the number of tokens in the trove.
            { return capi::huGetNumTokens(ctrove); }
        Token token(int idx) const HUMON_NOEXCEPT ///< Returns a Token by index.
            { return Token(capi::huGetToken(ctrove, idx)); }
        int numNodes() const HUMON_NOEXCEPT       ///< Returns the number of nodes in the trove.
            { return capi::huGetNumNodes(ctrove); }
        bool hasRoot() const HUMON_NOEXCEPT       ///< Returns whether the trove has a root node.
            { return numNodes() > 0; }
        Node root() const HUMON_NOEXCEPT          ///< Returns the root node of the trove.
            { return capi::huGetRootNode(ctrove); }
        Node nodeByIndex(int idx) const HUMON_NOEXCEPT   ///< Returns a Node by index.
            { return Node(capi::huGetNodeByIndex(ctrove, idx)); }
        /// Gets a node in the trove by its address.
        /** Given a `/`-separated sequence of dict keys or indices, this function returns
         * a node in this trove which can be found by tracing nodes from the root. The address
         * must begin with a `/` when accessing from the Trove.
         * \return Returns the {node and hu::ErrorCode::NoError} or {null node and 
         * the appropriate hu::ErrorCode}, */
        Node nodeByAddress(std::string_view address) const HUMON_NOEXCEPT
            { return Node(capi::huGetNodeByAddressN(ctrove, address.data(), address.size())); }
        /// Returns the number of errors encountered when tokenizing and parsing the Humon.
        int numErrors() const HUMON_NOEXCEPT
            { return ctrove ? capi::huGetNumErrors(ctrove) : 0; }

        /// Returns the `idx`th error encountered when tokenizing and parsing the Humon.
        std::tuple<ErrorCode, Token> error(int idx) const HUMON_NOEXCEPT 
        {
            auto cerr = capi::huGetError(ctrove, idx);
            return { static_cast<ErrorCode>(cerr->errorCode), Token(cerr->token) };
        }

        /// Returns a new collection of all errors encountered when tokenizing and parsing the Humon.
        [[nodiscard]] std::vector<std::tuple<ErrorCode, Token>> errors() const HUMON_NOEXCEPT
        {
            std::vector<std::tuple<ErrorCode, Token>> vec;
            int numErr = numErrors();
            vec.reserve(numErr);
            for (int i = 0; i < numErr; ++i)
                { vec.push_back(error(i)); }
            return vec;
        }

        /// Returns the number of annotations associated to this trove (not to any node).
        int numTroveAnnotations() const HUMON_NOEXCEPT
            { return capi::huGetNumTroveAnnotations(ctrove); }

        /// Returns the `idx`th annotation associated to this trove (not to any node).
        std::tuple<Token, Token> troveAnnotation(int idx) const HUMON_NOEXCEPT 
        { 
            auto canno = capi::huGetTroveAnnotation(ctrove, idx); 
            return { Token(canno->key), Token(canno->value) };
        }

        /// Returns a new collection of all annotations associated to this trove (not to any node).
        [[nodiscard]] std::vector<std::tuple<Token, Token>> troveAnnotations() const HUMON_NOEXCEPT
        {
            std::vector<std::tuple<Token, Token>> vec;
            int numAnnos = numTroveAnnotations();
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec.push_back(troveAnnotation(i)); }
            return vec;
        }

        /// Return the number of trove annotations associated to this trove (not to any node) with 
        /// the specified key.
        bool hasTroveAnnotation(std::string_view key) const HUMON_NOEXCEPT
            { return capi::huTroveHasAnnotationWithKeyN(ctrove, key.data(), key.size()); }
        /// Returns the value of the `idx`th annotation associated to this trove (not to any node)
        /// with the specified key.
        Token troveAnnotation(std::string_view key) const HUMON_NOEXCEPT 
            { return capi::huGetTroveAnnotationWithKeyN(ctrove, key.data(), key.size()); }
        /// Return the number of trove annotations associated to this trove (not to any node) with 
        /// the specified value.
        int numTroveAnnotationsWithValue(std::string_view value) const HUMON_NOEXCEPT
            { return capi::huGetNumTroveAnnotationsWithValueN(ctrove, value.data(), value.size()); }

        /// Returns a new collection of all the keys of annotations associated to this trove (not 
        /// to any node) with the specified value.
        [[nodiscard]] std::vector<Token> troveAnnotationsWithValue(std::string_view value) const HUMON_NOEXCEPT
        {
            check();
            std::vector<Token> vec;
            int cursor = 0;
            capi::huToken const * tok = HU_NULLTOKEN;
            do
            {
                tok = capi::huGetTroveAnnotationWithValueN(ctrove, value.data(), value.size(), & cursor);
                if (tok)
                    { vec.push_back(tok); }
            } while (tok != HU_NULLTOKEN);
            
            return vec;
        }

        /// Returns the number of comments associated to this trove (not to any node).
        int numTroveComments() const HUMON_NOEXCEPT
            { return capi::huGetNumTroveComments(ctrove); }

        /// Returns the `idx`th comment associated to this trove (not to any node).
        std::tuple<Token, Node> troveComment(int idx) const HUMON_NOEXCEPT 
        {
            auto ccomm = capi::huGetTroveComment(ctrove, idx);
            return { Token(ccomm), nullptr };
        }

        /// Returns a new collection of all comments associated to this trove (not to any node).
        [[nodiscard]] std::vector<std::tuple<Token, Node>> allTroveComments() const HUMON_NOEXCEPT
        {
            std::vector<std::tuple<Token, Node>> vec;
            int numComms = numTroveComments();
            vec.reserve(numComms);
            for (int i = 0; i < numComms; ++i)
                { vec.push_back(troveComment(i)); }
            return vec;
        }        

        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key.
        [[nodiscard]] std::vector<Node> findNodesWithAnnotationKey(std::string_view key) const HUMON_NOEXCEPT
        {
            std::vector<Node> vec;
            int cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesWithAnnotationKeyN(ctrove, key.data(), key.size(), & cursor);
                if (node)
                    { vec.emplace_back(node); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified value.
        [[nodiscard]] std::vector<Node> findNodesWithAnnotationValue(std::string_view value) const HUMON_NOEXCEPT
        {
            std::vector<Node> vec;
            int cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesWithAnnotationValueN(ctrove, value.data(), value.size(), & cursor);
                if (node)
                    { vec.emplace_back(node); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key and value.
        [[nodiscard]] std::vector<Node> findNodesWithAnnotationKeyValue(std::string_view key, std::string_view value) const HUMON_NOEXCEPT
        {
            std::vector<Node> vec;
            int cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesWithAnnotationKeyValueNN(ctrove, key.data(), key.size(), value.data(), value.size(), & cursor);
                if (node)
                    { vec.emplace_back(node); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Returns a new collection of all nodes that are associated a comment containing
        /// the specified substring.
        [[nodiscard]] std::vector<Node> findNodesByCommentContaining(std::string_view containedText) const HUMON_NOEXCEPT
        {
            std::vector<Node> vec;
            int cursor = 0;
            capi::huNode const * node = HU_NULLNODE;
            do
            {
                node = capi::huFindNodesByCommentContainingN(ctrove, containedText.data(), containedText.size(), & cursor);
                if (node)
                    { vec.emplace_back(Node(node)); }
            } while(node != HU_NULLNODE);
            return vec;
        }

        /// Serializes a trove with the exact input token stream.
        [[nodiscard]] std::variant<std::string, ErrorCode> toClonedString(bool printBom = false) const HUMON_NOEXCEPT
        {
            SerializeOptions sp = { WhitespaceFormat::cloned, 0, false, std::nullopt, true, "", printBom };
            return toString(sp);
        }

        /// Serializes a trove with the minimum token stream necessary to accurately convey the data.
        [[nodiscard]] std::variant<std::string, ErrorCode> toMinimalString(std::optional<ColorTable> const & colors = {}, 
            bool printComments = true, std::string_view newline = "\n", bool printBom = false) const HUMON_NOEXCEPT
        {
            SerializeOptions sp = { WhitespaceFormat::minimal, 0, false, colors, printComments, newline, printBom };
            return toString(sp);
        }

        /// Serializes a trove with whitespace formatting suitable for readability.
        [[nodiscard]] std::variant<std::string, ErrorCode> toPrettyString(int indentSize = 4, 
            bool indentWithTabs = false, std::optional<ColorTable> const & colors = {}, bool printComments = true, 
            std::string_view newline = "\n", bool printBom = false) const HUMON_NOEXCEPT 
        {
            SerializeOptions sp = { WhitespaceFormat::pretty, indentSize, indentWithTabs, colors, printComments, newline, printBom };
            return toString(sp);
        }

        /// Serializes a trove to a UTF8-formatted string.
        /** Creates a UTF8 string which encodes the trove, as seen in a Humon file. 
         * The contents of the file are whitespace-formatted and colorized depending on
         * the parameters.
         * \return A variant containing either the encoded string, or an error code.*/
        [[nodiscard]] std::variant<std::string, ErrorCode> toString(SerializeOptions & SerializeOptions) const HUMON_NOEXCEPT 
        {
            int strLength = 0;
            std::string s;
            int error = capi::huSerializeTrove(ctrove, NULL, & strLength, & SerializeOptions.cparams);
            if (error != capi::HU_ERROR_NOERROR)
                { return static_cast<ErrorCode>(error); }
            
            s.resize(strLength);
            error = capi::huSerializeTrove(ctrove, s.data(), & strLength, & SerializeOptions.cparams);
            if (error != capi::HU_ERROR_NOERROR)
                { return static_cast<ErrorCode>(error); }

            return s;
        }

        /// Serializes a trove with the exact input token stream.
        [[nodiscard]] std::variant<int, ErrorCode> toClonedFile(std::string_view path, 
            bool printBom = false) const HUMON_NOEXCEPT
        {
            SerializeOptions sp = { WhitespaceFormat::cloned, 0, false, std::nullopt, true, "", printBom };
            return toFile(path, sp);
        }

        /// Serializes a trove with the minimum token stream necessary to accurately convey the data.
        [[nodiscard]] std::variant<int, ErrorCode> toMinimalFile(std::string_view path, 
            std::optional<ColorTable> const & colors = {}, bool printComments = true,
            std::string_view newline = "\n", bool printBom = false) const HUMON_NOEXCEPT
        {
            SerializeOptions sp = { WhitespaceFormat::minimal, 0, false, colors, printComments, newline, printBom };
            return toFile(path, sp);
        }

        /// Serializes a trove with whitespace formatting suitable for readability.
        [[nodiscard]] std::variant<int, ErrorCode> toPrettyFile(std::string_view path, 
            int indentSize = 4, bool indentWithTabs = false, std::optional<ColorTable> const & colors = {}, bool printComments = true, 
            std::string_view newline = "\n", bool printBom = false) const HUMON_NOEXCEPT 
        {
            SerializeOptions sp = { WhitespaceFormat::pretty, indentSize, indentWithTabs, colors, printComments, newline, printBom };
            return toFile(path, sp);
        }

        /// Serializes a trove to a UTF8-formatted file.
        /** Creates or overwrites a UTF8 file which encodes the trove, as seen in a Humon file. 
         * The contents of the file are whitespace-formatted and colorized depending on
         * the parameters.
         * \return A variant containing either the number of bytes written to the file, or the
         * an error code.
         */
        [[nodiscard]] std::variant<int, ErrorCode> toFile(std::string_view path, SerializeOptions & SerializeOptions) const HUMON_NOEXCEPT
        {
            int outputLength = 0;
            int error = capi::huSerializeTroveToFileN(ctrove, path.data(), path.size(), & outputLength, & SerializeOptions.cparams);
            if (error != capi::HU_ERROR_NOERROR)
                { return error; }

            return outputLength;
        }

        /// Returns whether `idx` is a valid child index of the root node. (Whether root has
        /// at least `idx`+1 children.)
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        bool operator % (IntType idx) const HUMON_NOEXCEPT
           { return hasRoot() && root() % idx; }
        /// Returns whether the root is a dict and has a child with the specified key.
        bool operator % (std::string_view key) const HUMON_NOEXCEPT
            { return hasRoot() && root() % key; }
        /// Returns the root node's `idx`th child.
        template <class IntType,
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const HUMON_PATH_NOEXCEPT
        {
            auto ch = capi::huGetRootNode(ctrove);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (ch == nullptr)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            if (ch == nullptr)
                { return Node(HU_NULLNODE); }
            ch = capi::huGetChildByIndex(ch, idx);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (ch == nullptr)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            return Node(ch);
        }
        /// Returns the root node's child with the specified key.
        Node operator / (std::string_view key) const HUMON_PATH_NOEXCEPT
        {
            auto ch = capi::huGetRootNode(ctrove);
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (ch == nullptr)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            if (ch == nullptr)
                { return Node(HU_NULLNODE); }
            ch = capi::huGetChildByKeyN(ch, key.data(), key.size());
#ifdef HUMON_USE_NODE_PATH_EXCEPTIONS
            if (ch == nullptr)
                { throw std::runtime_error("Illegal path entry"); }
#endif
            return Node(ch);
        }
    private:
        void check() const HUMON_NOEXCEPT { checkNotNull(ctrove); }

        capi::huTrove const * ctrove = nullptr;
    };


    /// Fills an array with string table values for ANSI color terminals.
    inline ColorTable getAnsiColorTable() HUMON_NOEXCEPT
    {
        ColorTable table;        
        capi::huStringView nativeTable[capi::HU_COLORCODE_NUMCOLORS];
        capi::huFillAnsiColorTable(nativeTable);
        for (size_t i = 0; i < capi::HU_COLORCODE_NUMCOLORS; ++i)
            { table[i] = {nativeTable[i].ptr, static_cast<size_t>(nativeTable[i].size)}; }

        return table;
    }
}
