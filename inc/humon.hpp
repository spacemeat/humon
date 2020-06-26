#pragma once

#include <string_view>
#include <tuple>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <charconv>

/// The Humon namespace.
namespace hu
{
    // This namespace contains the C API. Not included in doxygen on purpose.
    namespace capi
    {
#include "humon.h"
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
    static inline char const * to_string(TokenKind rhs)
    {
        return capi::huTokenKindToString((int) rhs);
    }

    /// Specifies the kind of node represented by a particular hu::Node.
    enum class NodeKind : int
    {
        null = capi::HU_NODEKIND_NULL,      ///< Invalid node. And invalid address returns a null node.
        list = capi::HU_NODEKIND_LIST,      ///< List node. The node ontains a sequence of unassociated objects in maintained order.
        dict = capi::HU_NODEKIND_DICT,      ///< Dict node. The node ontains a sequence of string-associated objects in maintained order.
        value = capi::HU_NODEKIND_VALUE     ///< Value node. The node ontains a string value, and no children.
    };

    /// Return a string representation of a hu::NodeKind.
    static inline char const * to_string(NodeKind rhs)
    {
        return capi::huNodeKindToString((int) rhs);
    }

    /// Specifies the style of whitespacing in Humon text.
    enum class OutputFormat : int
    {
        preserved = capi::HU_OUTPUTFORMAT_PRESERVED,    ///< Preserves the original whitespacing as loaded.
        minimal = capi::HU_OUTPUTFORMAT_MINIMAL,        ///< Reduces as much whitespace as possible.
        pretty = capi::HU_OUTPUTFORMAT_PRETTY           ///< Formats the text in a standard, human-friendly way.
    };

    /// Return a string representation of a hu::OutputFormat.
    static inline char const * to_string(OutputFormat rhs)
    {
        return capi::huOutputFormatToString((int) rhs);
    }

    /// Specifies a tokenizing or parsing error code, or lookup error.
    enum class ErrorCode : int
    {
        noError = capi::HU_ERROR_NOERROR,                       ///< No error.
        unfinishedQuote = capi::HU_ERROR_UNFINISHEDQUOTE,       ///< The quoted text was not endquoted.
        unfinishedCStyleComment = 
            capi::HU_ERROR_UNFINISHEDCSTYLECOMMENT,             ///< The C-style comment was not closed.
        unexpectedEof = capi::HU_ERROR_UNEXPECTEDEOF,           ///< The text ended early.
        tooManyRoots = capi::HU_ERROR_TOOMANYROOTS,             ///< Too many root nodes.
        syntaxError = capi::HU_ERROR_SYNTAXERROR,               ///< General syntax error.
        notFound = capi::HU_ERROR_NOTFOUND,                     ///< No node could be found at the address.
        illegal = capi::HU_ERROR_ILLEGAL                        ///< The operation was illegal.
    };

    /// Return a string representation of a hu::ErrorCode.
    static inline char const * to_string(ErrorCode rhs)
    {
        return capi::huOutputErrorToString((int) rhs);
    }


    /// Specifies a style ID for colorized printing.
    enum class ColorKind
    {
        none = capi::HU_COLORKIND_NONE,                             ///< No color
        end = capi::HU_COLORKIND_END,                               ///< End-of-color code.
        puncList = capi::HU_COLORKIND_PUNCLIST,                     ///< List punctuation style. ([,]) 
        puncDict = capi::HU_COLORKIND_PUNCDICT,                     ///< Dict punctuation style. ({,})
        puncKeyValueSep = capi::HU_COLORKIND_PUNCKEYVALUESEP,       ///< Key-value separator style. (:)
        puncAnnotate = capi::HU_COLORKIND_PUNCANNOTATE,             ///< Annotation mark style. (@)
        puncAnnotateDict = capi::HU_COLORKIND_PUNCANNOTATEDICT,     ///< Annotation dict punctuation style. ({,})
        puncAnnotateKeyValueSep = 
            capi::HU_COLORKIND_PUNCANNOTATEKEYVALUESEP,             ///< Annotation key-value separator style. (:)
        key = capi::HU_COLORKIND_KEY,                               ///< Key style.
        value = capi::HU_COLORKIND_VALUE,                           ///< Value style.
        comment = capi::HU_COLORKIND_COMMENT,                       ///< Comment style.
        annoKey = capi::HU_COLORKIND_ANNOKEY,                       ///< Annotation key style.
        annoValue = capi::HU_COLORKIND_ANNOVALUE,                   ///< Annotation value style.
        whitespace = capi::HU_COLORKIND_WHITESPACE,                 ///< Whitespace style (including commas).

        numColorKinds = capi::HU_COLORKIND_NUMCOLORKINDS            ///< One past the last style code.
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


    template<>
    struct val<int>
    {
        inline int extract(std::string_view valStr) noexcept
        {
            int val;
            auto [p, ec] = std::from_chars(valStr.data(), valStr.data() + valStr.size(), val);
            if (ec == std::errc())
                { return val; }
            else
                { return int {}; }
        }
    };


    template <>
    struct val<float>
    {
        inline float extract(std::string_view valStr)
        {
            // TODO: (gcc): Once from_chars(..float&) is complete, use this or remove this specialization. We're making a useless string here and it maketh me to bite metal. Don't forget to noexcept.
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

    template <>
    struct val<double>
    {
        inline double extract(std::string_view valStr)
        {
            // TODO: (gcc): Once from_chars(..double&) is complete, use this or remove this specialization. We're making a useless string here and it maketh me to bite metal. Don't forget to noexcept.
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

    template <>
    struct val<std::string_view>
    {
        inline std::string_view extract(std::string_view valStr) noexcept
        {
            return valStr;
        }
    };

    template <>
    struct val<std::string>
    {
        inline std::string extract(std::string_view valStr) noexcept
        {
            return std::string(valStr);
        }
    };

    template <>
    struct val<bool>
    {
        inline bool extract(std::string_view valStr) noexcept
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
    static inline std::string_view make_sv(capi::huStringView const & husv)
    {
        return std::string_view(husv.str, husv.size);
    }

    /// Encodes a token read from Humon text.
    /** This class encodes file location and buffer location information about a
     * particular token in a Humon file. Every token is read and tracked with a 
     * hu::Token. */
    class Token
    {
    public:
        Token(capi::huToken const * ctoken) : ctoken(ctoken) { }
        bool isValid() const noexcept               ///< Returns whether the token is valid (non-null).
            { return ctoken != nullptr; }
        bool isNull() const noexcept                ///< Returns whether the token is null (not valid).
            { return ctoken == nullptr; }
        TokenKind kind() const noexcept          ///< Returns the kind of token this is.
            { return static_cast<TokenKind>(isValid() ? ctoken->tokenKind : capi::HU_TOKENKIND_NULL); }
        std::string_view str() const noexcept  ///< Returns the string value of the token.
            { return isNull() ? "" : make_sv(ctoken->str); }
        int line() const noexcept                ///< Returns the line number of the first character of the token in the file.
            { return isNull() ? 0: ctoken->line; }
        int col() const noexcept                 ///< Returns the column number of the first character of the token in the file.
            { return isNull() ? 0: ctoken->col; }
        int endLine() const noexcept             ///< Returns the line number of the last character of the token in the file.
            { return isNull() ? 0: ctoken->endLine; }
        int endCol() const noexcept              ///< Returns the column number of the last character of the token in the file.
            { return isNull() ? 0: ctoken->endCol; }
        operator std::string_view()                 ///< String view conversion.
            { return str(); }

    private:
        capi::huToken const * ctoken;
    };

    /// Encodes a Humon data node.
    /** Humon nodes make up a heirarchical structure, stemming from a single root node.
     * Humon troves contain a reference to the root, and store all nodes in an indexable
     * array. A node is either a list, a dict, or a value node. Any number of comments 
     * and annotations can be associated to a node. */
    class Node
    {
    public:
        Node() : cnode(capi::hu_nullNode) { }
        Node(capi::huNode const * cnode) : cnode(cnode) { }
        bool isValid() const noexcept                   ///< Returns whether the node is valid (not null).
            { return cnode != nullptr; }
        bool isNull() const noexcept                   ///< Returns whether the node is null (not valid).
            { return cnode == nullptr; }
        int nodeIndex() const noexcept               ///< Returns the node index within the trove. Rarely needed.
            { return isNull() ? -1 : cnode->nodeIdx; }
        NodeKind kind() const noexcept               ///< Returns the kind of node this is.
            { return static_cast<NodeKind>(isNull() ? capi::HU_NODEKIND_NULL : cnode->kind); }
        Token firstToken() const noexcept            ///< Returns the first token which contributes to this node, including any annotation and comment tokens.
            { return Token(isNull() ? capi::hu_nullToken : cnode->firstToken); }
        Token firstValueToken() const noexcept       ///< Returns the first token of this node's actual value; for a container, it points to the opening brac(e|ket).
            { return Token(isNull() ? capi::hu_nullToken : cnode->valueToken); }
        Token lastValueToken() const noexcept        ///< Returns the last token of this node's actual value; for a container, it points to the closing brac(e|ket).
            { return Token(isNull() ? capi::hu_nullToken : cnode->lastValueToken); }
        Token lastToken() const noexcept             ///< Returns the last token of this node, including any annotation and comment tokens.
            { return Token(isNull() ? capi::hu_nullToken : cnode->lastToken); }
        Node parentNode() const noexcept             ///< Returns the parent node of this node, or the null node if this is the root.
            { return Node(capi::huGetParentNode(cnode)); }
        int childOrdinal() const noexcept            ///< Returns the index of this node vis a vis its sibling nodes (starting at 0).
            { return isNull() ? -1 : cnode->childOrdinal; }
        int numChildren() const noexcept             ///< Returns the number of children of this node.
            { return capi::huGetNumChildren(cnode); }
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node child(IntType idx) const noexcept           ///< Returns the child node, by child index.
            { return capi::huGetChildByIndex(cnode, idx); }
        Node child(std::string_view key) const noexcept  ///< Returns the child node, by key (if this is a dict).
            { return capi::huGetChildByKeyN(cnode, key.data(), key.size()); }
        Node firstChild() const noexcept       ///< Returns the first child node of this node.
            { return child(0); }
        Node getNextSibling() const noexcept    ///< Returns the node ordinally after this one in the parent's children, or the null node if it's the last.
            { return Node(capi::huGetNextSibling(cnode)); }
        /// Access a node by relative address.
        /** A relative address is a single string, which contains as contents a `/`-delimited path
         * through the heirarchy. A key or index between the slashes indicates the child node to
         * access. */
        Node relative(std::string_view relativeAddress) const noexcept
            { return capi::huGetNodeByRelativeAddressN(cnode, relativeAddress.data(), relativeAddress.size()); }
        bool hasKey() const noexcept            ///< Returns whether this node has a key. (If it's in a dict.)
            { return capi::huHasKey(cnode); }
        Token key() const noexcept      ///< Returns the key token, or the null token if this is not in a dict.
            { return Token(hasKey() ? cnode->keyToken : capi::hu_nullToken); }
        bool hasValue() const noexcept          ///< Returns whether the node has a value token. Should always be true.
            { return capi::huHasValue(cnode); }
        Token value() const noexcept    ///< Returns the first value token that encodes this node.
            { return Token(hasValue() ? cnode->valueToken : capi::hu_nullToken); }

        int numAnnotations() const noexcept  ///< Returns the number of annotations associated to this node.
            { return capi::huGetNumAnnotations(cnode); }
        /// Returns the `idx`th annotation.
        /** Returns a <Token, Token> referencing the key and value of the annotaion
         * accessed by index. */
        std::tuple<Token, Token> annotation(int idx) const noexcept 
        { 
            auto canno = capi::huGetAnnotation(cnode, idx); 
            return { Token(canno->key), Token(canno->value) };
        }
        /// Returns a new collection of all this node's annotations.
        /** Creates a new vector of <Token, Token> tuples, each referencing the
         * key and value of an annotation. */
        std::vector<std::tuple<Token, Token>> allAnnotations() const
        {
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
        bool numAnnotationWithKey(std::string_view key) const noexcept
            { return capi::huHasAnnotationWithKeyN(cnode, key.data(), key.size()); }
        /// Returns a Token referencing the value of the annotaion accessed by key.
        Token const annotationByKey(std::string_view key) const noexcept 
        { 
            auto canno = capi::huGetAnnotationByKeyN(cnode, key.data(), key.size());
            return Token(canno);
        }

        /// Returns the number of annotations associated to this node, with the specified key.
        /** A node can have multiple annotations, each with different keys which have the same 
         * value. This function returns the number of annotations associated to this node with
         * the specified value. */
        int numAnnotationsByValue(std::string_view value) const noexcept
            { return capi::huGetNumAnnotationsByValueN(cnode, value.data(), value.size()); }
        /// Returns a Token referencing the value of the annotaion accessed by index and value.
        Token annotationKeyByValue(std::string_view value, int idx) const noexcept 
            { return capi::huGetAnnotationByValueN(cnode, value.data(), value.size(), idx); }
        /// Returns a new collection of all this node's annotations with the specified value.
        /** Creates a new vector of Tokens, each referencing the key of an annotation that 
         * has the specified value. */
        std::vector<Token> annotationKeysByValue(std::string_view key) const
        {
            std::vector<Token> vec;
            int numAnnos = numAnnotationsByValue(key);
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec.push_back(annotationKeyByValue(key, i)); }
            return vec;
        }

        /// Returns the number of comments associated to this node.
        int numComments() const noexcept
            { return capi::huGetNumComments(cnode); }
        /// Returns the `idx`th comment associated to this node.
        /** Returns a Token of the `idx`th ordinal comment associated with this node. */
        Token comment(int idx) const noexcept 
            { return capi::huGetComment(cnode, idx); }
        /// Returns a new collection of all comments associated to this node.
        std::vector<Token> allComments() const
        {
            std::vector<Token> vec;
            int numComms = numComments();
            for (int i = 0; i < numComms; ++i)
                { vec.push_back(comment(i)); }
            return vec;
        }
        /// Returns a new collection of all comments associated to this node containing the specified substring.
        std::vector<Token> commentsContaining(std::string_view containedString) const
        {
            std::vector<Token> vec;
            capi::huToken const * comm = capi::huGetCommentsContainingN(cnode, containedString.data(), containedString.size(), NULL);
            while (comm != capi::hu_nullToken)
            {
                vec.emplace_back(comm);
                comm = capi::huGetCommentsContainingN(cnode, containedString.data(), containedString.size(), comm);
            }
            return vec;
        }
        /// Return whether two Node objects refer to the same node.
        friend bool operator == (Node const & lhs, Node const & rhs)
            { return lhs.cnode == rhs.cnode; }
        /// Return whether two Node objects refer to the different nodes.
        friend bool operator != (Node const & lhs, Node const & rhs)
            { return lhs.cnode != rhs.cnode; }
        // TODO: operator <=> when available.
        /// Returns whether the specified index is a valid child index of this list or dict node.
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        bool operator % (IntType idx) const noexcept
        {
            int cidx = static_cast<int>(idx);
            if (kind() == NodeKind::dict || kind() == NodeKind::list)
                { return cidx >= 0 && cidx < numChildren(); }
            return false;
        }
        /// Returns whether the specified key is a valid child key of this dict node.
        bool operator % (std::string_view key) const noexcept
        {
            if (kind() == NodeKind::dict)
                { return child(key).isValid(); }
            return false;
        }
        /// Returns the `idx`th child of this node.
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const noexcept
            { return child(static_cast<int>(idx)); }
        /// Returns the child of this node by key.
        Node operator / (std::string_view key) const noexcept
            { return child(key); }
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
         *                0 / h::val<string_view> {};
         *      auto f = configTrove | "/config/display/aspectRatio" / h::val<float> {}; */
        template <class U>
        inline U operator / (val<U> ve) const
        {
            if (kind() != NodeKind::value)
                { return U {}; }
            return ve.extract(make_sv(cnode->valueToken->str));
        }
        /// Returns the entire text contained by this node and all its children.
        /** The entire text of this node is returned, including all its children's 
         * texts, and any comments and annotations associated to this node. */
        std::string_view wholeText() const noexcept
        {
            return std::string_view(cnode->firstToken->str.str,
                cnode->lastToken->str.str + cnode->lastToken->str.size - 
                cnode->firstToken->str.str);
        }

        /// Generates and returns the address of this node.
        /** Each node in a trove has a unique address, separate from its node index, 
         * which is represented by a series of keys or index values from the root,
         * separated by `/`s. The address is guaranteeed to work with 
         * Trove::getNode(std::string_view). */
        [[nodiscard]] std::string address() const noexcept
        {
            int len = 0;
            capi::huGetNodeAddress(cnode, NULL, & len);
            std::string s;
            s.resize(len);
            capi::huGetNodeAddress(cnode, s.data(), & len);
            return s;
        }

    private:
        capi::huNode const * cnode;
    };


    /// Encodes a Humon trove.
    /** A trove contains all the tokens and nodes that make up a Humon text. You can
     * gain access to nodes in the heirarchy, and search for nodes with certain
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
        static Trove fromString(std::string_view data,
            int inputTabSize = 4) noexcept
        {
            return Trove(capi::huMakeTroveFromStringN(
                data.data(), data.size(), inputTabSize));
        }

        /// Creates a Trove from a UTF8 file.
        /** This function makes a new Trove object from the given file. If the file
         * is in a legal Humon format, the Trove will come back without errors, and fully
         * ready to use. Otherwise the Trove will be in an erroneous state; it will not 
         * be a null trove, but rather will be loaded with no nodes, and errors marking 
         * tokens. */
        static Trove fromFile(std::string_view path,
            int inputTabSize = 4) noexcept
        {
            return Trove(capi::huMakeTroveFromFileN(
                path.data(), path.size(), inputTabSize));
        }
                
        /// Creates a Trove from a UTF8 stream.
        /** This function makes a new Trove object from the given stream. If the stream
         * is in a legal Humon format, the Trove will come back without errors, and fully
         * ready to use. Otherwise the Trove will be in an erroneous state; it will not 
         * be a null trove, but rather will be loaded with no nodes, and errors marking 
         * tokens. This function does not close the stream once it has finished reading.
         * 
         * \maxNumBytes: If 0, `fromIstream` reads the stream until EOF is encountered.
         * \inputTabSize: If the Humon contains tabs, this value modulates the tokens'
         * reported column values to match what a text editor with this tabstop would
         * display.
         * \outputTabSize: When pretty printing with `hu::Trove::toString()`, this value
         * sets the tab spacing for the output. */
        static Trove fromIstream(std::istream & in, size_t maxNumBytes = 0,
            int inputTabSize = 4) noexcept
        {
            if (maxNumBytes == 0)
            {
                std::stringstream buffer;
                buffer << in.rdbuf();
                return fromString(buffer.str(), inputTabSize);
            }
            else
            {
                std::string buffer;
                buffer.reserve(maxNumBytes + 1);
                in.read(buffer.data(), maxNumBytes);
                buffer[maxNumBytes] = '\0';
                return fromString(buffer.data(), inputTabSize);
            }
        }
    
    public:
        /// Construct a Trove which decorates a null trove. Useful for collections of troves.
        Trove() : ctrove(capi::hu_nullTrove) { }

    private:
        /// Construction from static member functions.
        Trove(capi::huTrove const * ctrove) : ctrove(ctrove) { }
        /// Cannot copy a Trove. They're immutable anyway.
        Trove(Trove const & rhs) = delete;

    public:
        /// Move-construct a Trove.
        Trove(Trove && rhs)
        {
            if (ctrove)
                { capi::huDestroyTrove(ctrove); }

            ctrove = rhs.ctrove;
            rhs.ctrove = capi::hu_nullTrove;
        }
        /// Desttruct a Trove.
        ~Trove()
        {
            if (ctrove && ctrove != capi::hu_nullTrove)
                { capi::huDestroyTrove(ctrove); }
        }
        /// Cannot copy-assign a Trove.
        Trove & operator = (Trove const & rhs) = delete;
        /// Move-assign a Trove.
        Trove & operator = (Trove && rhs)
        {
            if (ctrove)
                { capi::huDestroyTrove(ctrove); }

            ctrove = rhs.ctrove;
            rhs.ctrove = capi::hu_nullTrove;
            
            return * this;
        }

        operator bool () const noexcept           ///< Returns whether the trove is valid (not null).
            { return ctrove && ctrove != capi::hu_nullTrove && numErrors() == 0; }
        bool isNull() const noexcept            ///< Returns whether the trove is null (not valid).
            { return ctrove == nullptr; }
        int numTokens() const noexcept       ///< Returns the number of tokens in the trove.
            { return capi::huGetNumTokens(ctrove); }
        Token token(int idx) const noexcept  ///< Returns a Token by index.
            { return Token(capi::huGetToken(ctrove, idx)); }
        int numNodes() const noexcept        ///< Returns the number of nodes in the trove.
            { return capi::huGetNumNodes(ctrove); }
        Node node(int idx) const noexcept    ///< Returns a Node by index.
            { return Node(capi::huGetNode(ctrove, idx)); }
        Node rootNode() const noexcept       ///< Returns the root node of the trove.
            { return capi::huGetRootNode(ctrove); }
        /// Gets a node in the trove by its address.
        /** Given a `/`-separated sequence of dict keys or indices, this function returns
         * a node in this trove which can be found by tracing nodes from the root. The address
         * must begin with a `/` when accessing from the Trove.
         * \return Returns the {node and hu::ErrorCode::NoError} or {null node and 
         * the appropriate hu::ErrorCode}, */
        Node node(std::string_view address) const noexcept
            { return Node(capi::huGetNodeByFullAddressN(ctrove, address.data(), address.size())); }
        /// Returns the number of errors encountered when tokenizing and parsing the Humon.
        int numErrors() const noexcept
            { return capi::huGetNumErrors(ctrove); }
        /// Returns the `idx`th error encountered when tokenizing and parsing the Humon.
        std::tuple<ErrorCode, Token> error(int idx) const noexcept 
        {
            auto cerr = capi::huGetError(ctrove, idx);
            return { static_cast<ErrorCode>(cerr->errorCode), Token(cerr->errorToken) };
        }
        /// Returns a new collection of all errors encountered when tokenizing and parsing the Humon.
        std::vector<std::tuple<ErrorCode, Token>> errors() const
        {
            std::vector<std::tuple<ErrorCode, Token>> vec;
            int numErr = numErrors();
            vec.reserve(numErr);
            for (int i = 0; i < numErr; ++i)
                { vec[i] = error(i); }
            return vec;
        }
        /// Returns the number of annotations associated to this trove (not to any node).
        int numAnnotations() const noexcept
            { return capi::huGetNumTroveAnnotations(ctrove); }
        /// Returns the `idx`th annotation associated to this trove (not to any node).
        std::tuple<Token, Token> annotation(int idx) const noexcept 
        { 
            auto canno = capi::huGetTroveAnnotation(ctrove, idx); 
            return { Token(canno->key), Token(canno->value) };
        }
        /// Returns a new collection of all annotations associated to this trove (not to any node).
        std::vector<std::tuple<Token, Token>> annotations() const
        {
            std::vector<std::tuple<Token, Token>> vec;
            int numAnnos = numAnnotations();
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec[i] = annotation(i); }
            return vec;
        }
        /// Return the number of trove annotations associated to this trove (not to any node) with 
        /// the specified key.
        bool hasAnnotationWithKey(std::string_view key) const noexcept
            { return capi::huTroveHasAnnotationWithKeyN(ctrove, key.data(), key.size()); }
        /// Returns the value of the `idx`th annotation associated to this trove (not to any node)
        /// with the specified key.
        Token annotationWithKey(std::string_view key) const noexcept 
            { return capi::huGetTroveAnnotationWithKeyN(ctrove, key.data(), key.size()); }
        /// Return the number of trove annotations associated to this trove (not to any node) with 
        /// the specified value.
        int numAnnotationsByValue(std::string_view value) const noexcept
            { return capi::huGetNumTroveAnnotationsByValueN(ctrove, value.data(), value.size()); }
        /// Returns the key of the `idx`th annotation associated to this trove (not to any node) 
        /// with the specified value.
        Token annotationByValue(std::string_view value, int idx) const noexcept 
            { return capi::huGetTroveAnnotationByValueN(ctrove, value.data(), value.size(), idx); }
        /// Returns a new collection of all the keys of annotations associated to this trove (not 
        /// to any node) with the specified value.
        std::vector<Token> annotationsByValue(std::string_view key) const
        {
            std::vector<Token> vec;
            int numAnnos = numAnnotationsByValue(key);
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec[i] = annotationByValue(key, i); }
            return vec;
        }
        /// Returns the number of comments associated to this trove (not to any node).
        int numComments() const noexcept
            { return capi::huGetNumTroveComments(ctrove); }
        /// Returns the `idx`th comment associated to this trove (not to any node).
        std::tuple<Token, Node> comment(int idx) const noexcept 
        {
            auto ccomm = capi::huGetTroveComment(ctrove, idx);
            return { Token(ccomm), nullptr };
        }
        /// Returns a new collection of all comments associated to this trove (not to any node).
        std::vector<std::tuple<Token, Node>> allComments() const
        {
            std::vector<std::tuple<Token, Node>> vec;
            int numComms = numComments();
            vec.reserve(numComms);
            for (int i = 0; i < numComms; ++i)
                { vec[i] = comment(i); }
            return vec;
        }
        /// Serializes a trove to a UTF8-formatted string.
        /** Creates a UTF8 string which encodes the trove, as seen in a Humon file. 
         * The contents of the file are whitespace-formatted and colorized depending on
         * the parameters.
         * \return A tuple containing a unique (self-managing) string pointer and the
         * string's length.
         */
        std::string toString(OutputFormat outputFormat = OutputFormat::pretty, bool excludeComments = false, 
            int outputTabSize = 4, std::string_view newline = "\n", std::vector<std::string_view> const & colorTable = {}) const noexcept 
        {
            std::array<capi::huStringView, capi::HU_COLORKIND_NUMCOLORKINDS> colors;
            capi::huStringView * nativeColorTable = NULL;
            if (colorTable.size() == capi::HU_COLORKIND_NUMCOLORKINDS)
            {
                for (size_t i = 0; i < capi::HU_COLORKIND_NUMCOLORKINDS && i < colorTable.size(); ++i)
                {
                    colors[i].str = colorTable[i].data();
                    colors[i].size = colorTable[i].size();
                }
                nativeColorTable = colors.data();
            }
            int strLength = 0;
            capi::huTroveToString(ctrove, NULL, & strLength, static_cast<int>(outputFormat), excludeComments, outputTabSize, newline.data(), newline.size(), nativeColorTable);
            std::string s;
            s.resize(strLength);
            capi::huTroveToString(ctrove, s.data(), & strLength, static_cast<int>(outputFormat), excludeComments, outputTabSize, newline.data(), newline.size(), nativeColorTable);
            return s;
        }
        friend bool operator == (Trove const & lhs, Trove const & rhs)
            { return lhs.ctrove == rhs.ctrove; }
        friend bool operator != (Trove const & lhs, Trove const & rhs)
            { return lhs.ctrove != rhs.ctrove; }
        // TODO: <=> when it's available.
        /// Returns whether `idx` is a valid child index of the root node. (Whether root has
        /// at least `idx`+1 children.)
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        bool operator % (IntType idx) const noexcept
           { return rootNode() % idx; }
        /// Returns whether the root is a dict and has a child with the specified key.
        bool operator % (std::string_view key) const noexcept
            { return rootNode() % key; }
        /// Returns the root node's `idx`th child.
        template <class IntType, // TODO: Turn this into a contract
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const noexcept
            { return rootNode() / idx; }
        /// Returns the root node's child with the specified key.
        Node operator / (std::string_view key) const noexcept
            { return rootNode() / key; }
        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key.
        std::vector<Node> findNodesByAnnotationKey(std::string_view key) const
        {
            std::vector<Node> vec;
            auto node = capi::huFindNodesByAnnotationKeyN(ctrove, key.data(), key.size(), NULL);
            while (node != capi::hu_nullNode)
            {
                vec.emplace_back(node);
                node = capi::huFindNodesByAnnotationKeyN(ctrove, key.data(), key.size(), node);
            }
            return vec;
        }
        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified value.
        std::vector<Node> findNodesByAnnotationValue(std::string_view value) const
        {
            std::vector<Node> vec;
            auto node = capi::huFindNodesByAnnotationValueN(ctrove, value.data(), value.size(), NULL);
            while (node != capi::hu_nullNode)
            {
                vec.emplace_back(node);
                node = capi::huFindNodesByAnnotationValueN(ctrove, value.data(), value.size(), node);
            }
            return vec;
        }
        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key and value.
        std::vector<Node> findNodesByAnnotationKeyValue(std::string_view key, std::string_view value) const
        {
            std::vector<Node> vec;
            auto node = capi::huFindNodesByAnnotationKeyValueNN(ctrove, key.data(), key.size(), value.data(), value.size(), NULL);
            while (node != capi::hu_nullNode)
            {
                vec.emplace_back(node);
                node = capi::huFindNodesByAnnotationKeyValueNN(ctrove, key.data(), key.size(), value.data(), value.size(), node);
            }
            return vec;
        }
        /// Returns a new collection of all nodes that are associated a comment containing
        /// the specified substring.
        std::vector<Node> findNodesByCommentContaining(std::string_view containedText) const
        {
            std::vector<Node> vec;
            auto node = capi::huFindNodesByCommentContainingN(ctrove, containedText.data(), containedText.size(), NULL);
            while (node != capi::hu_nullNode)
            {
                vec.emplace_back(node);
                node = capi::huFindNodesByCommentContainingN(ctrove, containedText.data(), containedText.size(), node);
            }
            return vec;
        }

    private:
        capi::huTrove const * ctrove = nullptr;
    };

    /// Fills a vector with string table values for ANSI color terminals.
    static inline void getAnsiColorTable(std::vector<std::string_view> & table)
    {
        auto numColors = static_cast<size_t const>(ColorKind::numColorKinds);
        if (table.size() < numColors)
            { table.resize(numColors); }
        capi::huStringView nativeTable[numColors];
        capi::huFillAnsiColorTable(nativeTable);
        for (size_t i = 0; i < numColors; ++i)
            { table[i] = {nativeTable[i].str, static_cast<size_t>(nativeTable[i].size)}; }
    }
}
