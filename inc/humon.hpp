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
    static inline char const * TokenKindToString(TokenKind rhs)
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
    static inline char const * huNodeKindToString(NodeKind rhs)
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
    static inline char const * huOutputFormatToString(OutputFormat rhs)
    {
        return capi::huOutputFormatToString((int) rhs);
    }

    /// Specifies a tokenizing or parsing error code, or lookup error.
    enum class ErrorCode : int
    {
        noError = capi::HU_ERROR_NO_ERROR,                      ///< No error.
        unexpectedEof = capi::HU_ERROR_UNEXPECTED_EOF,          ///< The text ended early.
        unfinishedQuote = capi::HU_ERROR_UNFINISHED_QUOTE,      ///< The quoted text was not endquoted.
        unfinishedCStyleComment = 
            capi::HU_ERROR_UNFINISHED_CSTYLECOMMENT,            ///< The C-style comment was not closed.
        syntaxError = capi::HU_ERROR_SYNTAX_ERROR,              ///< General syntax error.
        startEndMismatch = capi::HU_ERROR_START_END_MISMATCH,   ///< Braces ({,}) or brackets ([,]) are not properly nested.
        notFound = capi::HU_ERROR_NOTFOUND                      ///< No node could be found at the address.
    };

    /// Return a string representation of a hu::ErrorCode.
    static inline char const * huOutputErrorToString(ErrorCode rhs)
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
     * 2. Specialize `hu::value<T>` for your type `T`, and implement the `extract()`
     * function.
     * Usage: Use in conjunction with the `/` operator on a value node. This allows
     * you to extract a value in the natural flow of things:
     * `auto numEyes = node / "frogs" / "numEyes" / hu::value<int> {};`
     * `auto color = node / "frogs" / "colors" / 0 / hu::value<AnimalColorsEnum> {};`
     */
    template <typename T>
    struct value
    {
        inline T extract(std::string_view valStr) noexcept
        {
            T val;
            auto [p, ec] = std::from_chars(valStr.data(), valStr.data() + valStr.size(), val);
            if (ec == std::errc())
                { return val; }
            else
                { return T {}; }
        }
    };


    template <>
    struct value<float>
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
    struct value<double>
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
    struct value<std::string_view>
    {
        inline std::string_view extract(std::string_view valStr) noexcept
        {
            return valStr;
        }
    };

    template <>
    struct value<std::string>
    {
        inline std::string extract(std::string_view valStr) noexcept
        {
            return std::string(valStr);
        }
    };

    template <>
    struct value<bool>
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

    // Frees a const*. That fun intersection of C and C++. 
    static inline void free_const(void const * t) noexcept
        { free(const_cast<void *>(t)); }

    /// Own a buffer created with malloc().
    /** Deterministically destroy an owned, malloc'd buffer. This is for 
     * returned objects allocated by the C API. These take the place of
     * calling `huDestroyVector()` or `huDestroyStringView()` where 
     * appropriate. Yay C++. Not intended to be used for anything but
     * Humon APIs, but I'm not your boss. Just be sure to match it with
     * `malloc()` instead of `new`. */
    template <typename T>
    using unique_ptr_free = std::unique_ptr<T, decltype(free_const)*>;

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
        TokenKind getKind() const noexcept          ///< Returns the kind of token this is.
            { return static_cast<TokenKind>(ctoken->tokenKind); }
        std::string_view getValue() const noexcept  ///< Returns the string value of the token.
            { return make_sv(ctoken->value); }
        int getLine() const noexcept                ///< Returns the line number of the first character of the token in the file.
            { return ctoken->line; }
        int getCol() const noexcept                 ///< Returns the column number of the first character of the token in the file.
            { return ctoken->col; }
        int getEndLine() const noexcept             ///< Returns the line number of the last character of the token in the file.
            { return ctoken->endLine; }
        int getEndCol() const noexcept              ///< Returns the column number of the last character of the token in the file.
            { return ctoken->endCol; }

        std::string_view operator ()()              ///< String view conversion.
            { return getValue(); }

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
        Node(capi::huNode const * cnode) : cnode(cnode) { }
        bool isValid() const noexcept                   ///< Returns whether the node is valid (not null).
            { return cnode != nullptr; }
        int getNodeIndex() const noexcept               ///< Returns the node index within the trove. Rarely needed.
            { return cnode->nodeIdx; }
        NodeKind getKind() const noexcept               ///< Returns the kind of node this is.
            { return static_cast<NodeKind>(cnode->kind); }
        Token getFirstToken() const noexcept            ///< Returns the first token which contributes to this node, including any annotation and comment tokens.
            { return Token(cnode->firstToken); }
        Token getFirstValueToken() const noexcept       ///< Returns the first token of this node's actual value; for a container, it points to the opening brac(e|ket).
            { return Token(cnode->firstValueToken); }
        Token getLastValueToken() const noexcept        ///< Returns the last token of this node's actual value; for a container, it points to the closing brac(e|ket).
            { return Token(cnode->lastValueToken); }
        Token getLastToken() const noexcept             ///< Returns the last token of this node, including any annotation and comment tokens.
            { return Token(cnode->lastToken); }
        Node getParentNode() const noexcept             ///< Returns the parent node of this node, or the null node if this is the root.
            { return Node(capi::huGetParentNode(cnode)); }
        int getChildOrdinal() const noexcept            ///< Returns the index of this node vis a vis its sibling nodes (starting at 0).
            { return cnode->childIdx; }
        int getNumChildren() const noexcept             ///< Returns the number of children of this node.
            { return capi::huGetNumChildren(cnode); }
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node getChild(IntType idx) const noexcept           ///< Returns the child node, by child index.
            { return capi::huGetChildNodeByIndex(cnode, idx); }
        Node getChild(std::string_view key) const noexcept  ///< Returns the child node, by key (if this is a dict).
            { return capi::huGetChildNodeByKeyN(cnode, key.data(), key.size()); }
        /// Access a node by relative address.
        /** A relative address is a single string, which contains as contents a `/`-delimited path
         * through the heirarchy. A key or index between the slashes indicates the child node to
         * access. */
        std::tuple<Node, ErrorCode> getNode(std::string_view relativeAddress) const noexcept
        {
            int ierror = capi::HU_ERROR_NO_ERROR;
            auto n = capi::huGetNodeByRelativeAddressN(cnode, relativeAddress.data(), relativeAddress.size(), & ierror);
            return { n, static_cast<ErrorCode>(ierror) };
        }
        bool hasKey() const noexcept            ///< Returns whether this node has a key. (If it's in a dict.)
            { return capi::huHasKey(cnode); }
        Token getKeyToken() const noexcept      ///< Returns the key token, or the null token if this is not in a dict.
            { return Token(capi::huGetKey(cnode)); }
        bool hasValue() const noexcept          ///< Returns whether the node has a value token. Should always be true.
            { return capi::huHasValue(cnode); }
        Token getValueToken() const noexcept    ///< Returns the first value token that encodes this node.
            { return Token(capi::huGetValue(cnode)); }
        Token getStartToken() const noexcept    ///< Returns the first token that encodes this node, including comments.
            { return Token(capi::huGetStartToken(cnode)); }
        Token getEndToken() const noexcept      ///< Returns the last token that encodes this node, including comments and annotations.
            { return Token(capi::huGetEndToken(cnode)); }
        Node getNextSibling() const noexcept    ///< Returns the node ordinally after this one in the parent's children, or the null node if it's the last.
            { return Node(capi::huNextSibling(cnode)); }

        int getNumAnnotations() const noexcept  ///< Returns the number of annotations associated to this node.
            { return capi::huGetNumAnnotations(cnode); }
        /// Returns the `idx`th annotation.
        /** Returns a <Token, Token> referencing the key and value of the annotaion
         * accessed by index. */
        std::tuple<Token, Token> getAnnotation(int idx) const noexcept 
        { 
            auto canno = capi::huGetAnnotation(cnode, idx); 
            return { Token(canno->key), Token(canno->value) };
        }
        /// Returns a new collection of all this node's annotations.
        /** Creates a new vector of <Token, Token> tuples, each referencing the
         * key and value of an annotation. */
        std::vector<std::tuple<Token, Token>> getAnnotations() const
        {
            std::vector<std::tuple<Token, Token>> vec;
            int numAnnos = getNumAnnotations();
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec.push_back(getAnnotation(i)); }
            return vec;
        }

        /// Returns the number of annotations associated to this node, with the specified key.
        /** A node can have multiple annotations with the same key. This is legal in Humon, but
         * rare. This function returns the number of annotations associated to this node with
         * the specified key. */
        bool hasNumAnnotationWithKey(std::string_view key) const noexcept
            { return capi::huHasAnnotationWithKeyN(cnode, key.data(), key.size()); }
        /// Returns a Token referencing the value of the annotaion accessed by index and key.
        Token const getAnnotationByKey(std::string_view key, int idx) const noexcept 
        { 
            auto canno = capi::huGetAnnotationByKeyN(cnode, key.data(), key.size(), idx);
            return Token(canno);
        }

        /// Returns the number of annotations associated to this node, with the specified key.
        /** A node can have multiple annotations, each with different keys which have the same 
         * value. This function returns the number of annotations associated to this node with
         * the specified value. */
        int getNumAnnotationsByValue(std::string_view value) const noexcept
            { return capi::huGetNumAnnotationsByValueN(cnode, value.data(), value.size()); }
        /// Returns a Token referencing the value of the annotaion accessed by index and value.
        Token getAnnotationKeyByValue(std::string_view value, int idx) const noexcept 
        { 
            auto canno = capi::huGetAnnotationByValueN(cnode, value.data(), value.size(), idx);
            return Token(canno->key);
        }
        /// Returns a new collection of all this node's annotations with the specified value.
        /** Creates a new vector of Tokens, each referencing the key of an annotation that 
         * has the specified value. */
        std::vector<Token> getAnnotationKeysByValue(std::string_view key) const
        {
            std::vector<Token> vec;
            int numAnnos = getNumAnnotationsByValue(key);
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec.push_back(getAnnotationKeyByValue(key, i)); }
            return vec;
        }

        /// Returns the number of comments associated to this node.
        int getNumComments() const noexcept
            { return capi::huGetNumComments(cnode); }
        /// Returns the `idx`th comment associated to this node.
        /** Returns a Token of the `idx`th ordinal comment associated with this node. */
        Token getComment(int idx) const noexcept 
        {
            auto ccomm = capi::huGetComment(cnode, idx);
            return Token(ccomm->commentToken);
        }
        /// Returns a new collection of all comments associated to this node.
        std::vector<Token> getComments() const
        {
            std::vector<Token> vec;
            int numComms = getNumComments();
            for (int i = 0; i < numComms; ++i)
                { vec.push_back(getComment(i)); }
            return vec;
        }
        /// Returns a new collection of all comments associated to this node containing the specified substring.
        std::vector<Token> getCommentsContaining(std::string_view containedString) const
        {
            std::vector<Token> vec;
            capi::huVector commsVec = capi::huGetCommentsContainingN(cnode, containedString.data(), containedString.size());
            vec.reserve(commsVec.numElements);
            for (int i = 0; i < capi::huGetVectorSize(& commsVec); ++i)
            {
                capi::huComment * comm = (capi::huComment *) capi::huGetVectorElement(& commsVec, i);
                vec.emplace_back(comm->commentToken);
            }
            return vec;
        }

        /// Returns this node's key, or a zero-length string.
        std::string_view getKey() const noexcept 
        {
            if (capi::huHasKey(cnode))
            {
                return make_sv(cnode->keyToken->value);
            }

            return "";
        }

        /// Returns whether the specified index is a valid child index of this list or dict node.
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        bool operator % (IntType idx) const noexcept
        {
            int cidx = static_cast<int>(idx);
            if (getKind() == NodeKind::dict || getKind() == NodeKind::list)
                { return cidx >= 0 && cidx < getNumChildren(); }
            return false;
        }

        /// Returns whether the specified key is a valid child key of this dict node.
        bool operator % (std::string_view key) const noexcept
        {
            if (getKind() == NodeKind::dict)
                { return getChild(key).isValid(); }
            return false;
        }

        /// Returns the `idx`th child of this node.
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const noexcept
            { return getChild(static_cast<int>(idx)); }
        /// Returns the child of this node by key.
        Node operator / (std::string_view key) const noexcept
            { return getChild(key); }

        /// Returns the converted value of this value node.
        /** Converts the string value of this value node into a `U`. The conversion is 
         * performed by passing a dummy object of type value<U> which, if implemented
         * for type U, will invoke that type's extract function. If there is no value<U>
         * defined, the default value<U> calls std::from_chars(... U&). Any type that
         * has an overload of std::from_chars, or a specialization of value<U>, will
         * return the converted value.
         * 
         * Users can overload std::from_chars() for custom types. That involves adding
         * a function to std, which is legal but makes some people uncomfortable. Users
         * can alternatively implement a specialization of value<U> for a custom type, 
         * providing the analogous extract() member function.
         * 
         * The purpose of value<T> is to allow users to write code like:
         *      auto sv = materialsTrove / "assets" / "brick-diffuse" / "src" / 
         *                0 / h::value<string_view> {};
         *      auto f = configTrove | "/config/display/aspectRatio" / h::value<float> {}; */
        template <class U>
        inline U operator / (value<U> ve) const
        {
            if (getKind() != NodeKind::value)
                { return U {}; }
            return ve.extract(getValue());
        }

        /// Returns the string value of this value node.
        std::string_view getValue() const noexcept 
        {
            if (capi::huHasValue(cnode))
            {
                return std::string_view(cnode->firstValueToken->value.str,
                    cnode->lastValueToken->value.str + cnode->lastValueToken->value.size - 
                    cnode->firstValueToken->value.str);
            }

            return "";
        }

        /// Generates and returns the address of this node.
        /** Each node in a trove has a unique address, separate from its node index, 
         * which is represented by a series of keys or index values from the root,
         * separated by `/`s. The address is guaranteeed to work with 
         * Trove::getNode(std::string_view). */
        std::tuple<unique_ptr_free<char const>, int> getAddress() const noexcept
        {
            auto str = capi::huGetNodeAddress(cnode);
            auto ptr = unique_ptr_free<char const> { str.str, free_const };
            return { std::move(ptr), str.size };
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
            return Trove(capi::huMakeTroveFromFile(
                path.data(), inputTabSize));
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
        Trove() : ctrove(& capi::humon_nullTrove) { }

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
            rhs.ctrove = & capi::humon_nullTrove;
        }
        /// Desttruct a Trove.
        ~Trove()
        {
            if (ctrove && ctrove != & capi::humon_nullTrove)
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
            rhs.ctrove = & capi::humon_nullTrove;
            
            return * this;
        }

        bool isValid() const noexcept           ///< Returns whether the trove is valid (not null).
            { return ctrove && ctrove != & capi::humon_nullTrove; }
        int getNumTokens() const noexcept       ///< Returns the number of tokens in the trove.
            { return capi::huGetNumTokens(ctrove); }
        Token getToken(int idx) const noexcept  ///< Returns a Token by index.
            { return Token(capi::huGetToken(ctrove, idx)); }
        int getNumNodes() const noexcept        ///< Returns the number of nodes in the trove.
            { return capi::huGetNumNodes(ctrove); }
        Node getNode(int idx) const noexcept    ///< Returns a Node by index.
            { return Node(capi::huGetNode(ctrove, idx)); }
        Node getRootNode() const noexcept       ///< Returns the root node of the trove.
            { return capi::huGetRootNode(ctrove); }
        /// Gets a node in the trove by its address.
        /** Given a `/`-separated sequence of dict keys or indices, this function returns
         * a node in this trove which can be found by tracing nodes from the root. The address
         * must begin with a `/` when accessing from the Trove.
         * \return Returns the {node and hu::ErrorCode::NoError} or {null node and 
         * the appropriate hu::ErrorCode}, */
        std::tuple<Node, ErrorCode> getNode(std::string_view address) const noexcept
        {
            int ierror = capi::HU_ERROR_NO_ERROR;
            auto n = Node(capi::huGetNodeByFullAddressN(ctrove, address.data(), address.size(), & ierror));
            return { n, static_cast<ErrorCode>(ierror) };
        }
        /// Returns the number of errors encountered when tokenizing and parsing the Humon.
        int getNumErrors() const noexcept
            { return capi::huGetNumErrors(ctrove); }
        /// Returns the `idx`th error encountered when tokenizing and parsing the Humon.
        std::tuple<ErrorCode, Token> getError(int idx) const noexcept 
        {
            auto cerr = capi::huGetError(ctrove, idx);
            return { static_cast<ErrorCode>(cerr->errorCode), Token(cerr->errorToken) };
        }
        /// Returns a new collection of all errors encountered when tokenizing and parsing the Humon.
        std::vector<std::tuple<ErrorCode, Token>> getErrors() const
        {
            std::vector<std::tuple<ErrorCode, Token>> vec;
            int numErrors = getNumErrors();
            vec.reserve(numErrors);
            for (int i = 0; i < numErrors; ++i)
                { vec[i] = getError(i); }
            return vec;
        }
        /// Returns the number of annotations associated to this trove (not to any node).
        int getNumAnnotations() const noexcept
            { return capi::huGetNumTroveAnnotations(ctrove); }
        /// Returns the `idx`th annotation associated to this trove (not to any node).
        std::tuple<Token, Token> getAnnotation(int idx) const noexcept 
        { 
            auto canno = capi::huGetTroveAnnotation(ctrove, idx); 
            return { Token(canno->key), Token(canno->value) };
        }
        /// Returns a new collection of all annotations associated to this trove (not to any node).
        std::vector<std::tuple<Token, Token>> getAnnotations() const
        {
            std::vector<std::tuple<Token, Token>> vec;
            int numAnnos = getNumAnnotations();
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec[i] = getAnnotation(i); }
            return vec;
        }
        /// Return the number of trove annotations associated to this trove (not to any node) with 
        /// the specified key.
        int getNumAnnotationsByKey(std::string_view key) const noexcept
            { return capi::huGetNumTroveAnnotationsByKeyN(ctrove, key.data(), key.size()); }
        /// Returns the value of the `idx`th annotation associated to this trove (not to any node)
        /// with the specified key.
        Token getAnnotationByKey(std::string_view key, int idx) const noexcept 
        { 
            auto canno = capi::huGetTroveAnnotationByKeyN(ctrove, key.data(), key.size(), idx);
            return Token(canno->value);
        }
        /// Returns a new collection of all annotation values associated to this trove (not to any node)
        /// with the specified key.
        /** A node or the trove object can have multiple annotations with the same key. Since 
         * annotations can be "out of domain" of a Humon structure, it may be useful for multiple
         * users of a file to each adopt their own annotation conventions. Key collisions will
         * be nondestructive in that case--though users must be on the lookout for -- oh fuck it, this policy sucks, let's just enforce unique keys. :(
         * 
         */
        std::vector<Token> getAnnotationsByKey(std::string_view key) const
        {
            std::vector<Token> vec;
            int numAnnos = getNumAnnotationsByKey(key);
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec[i] = getAnnotationByKey(key, i); }
            return vec;
        }
        /// Return the number of trove annotations associated to this trove (not to any node) with 
        /// the specified value.
        int getNumAnnotationsByValue(std::string_view value) const noexcept
            { return capi::huGetNumTroveAnnotationsByValueN(ctrove, value.data(), value.size()); }
        /// Returns the key of the `idx`th annotation associated to this trove (not to any node) 
        /// with the specified value.
        Token getAnnotationByValue(std::string_view value, int idx) const noexcept 
        { 
            auto canno = capi::huGetTroveAnnotationByValueN(ctrove, value.data(), value.size(), idx);
            return Token(canno->value);
        }
        /// Returns a new collection of all the keys of annotations associated to this trove (not 
        /// to any node) with the specified value.
        std::vector<Token> getAnnotationsByValue(std::string_view key) const
        {
            std::vector<Token> vec;
            int numAnnos = getNumAnnotationsByValue(key);
            vec.reserve(numAnnos);
            for (int i = 0; i < numAnnos; ++i)
                { vec[i] = getAnnotationByValue(key, i); }
            return vec;
        }
        /// Returns the number of comments associated to this trove (not to any node).
        int getNumComments() const noexcept
            { return capi::huGetNumTroveComments(ctrove); }
        /// Returns the `idx`th comment associated to this trove (not to any node).
        std::tuple<Token, Node> getComment(int idx) const noexcept 
        {
            auto ccomm = capi::huGetTroveComment(ctrove, idx);
            return { Token(ccomm->commentToken), nullptr };
        }
        /// Returns a new collection of all comments associated to this trove (not to any node).
        std::vector<std::tuple<Token, Node>> getComments() const
        {
            std::vector<std::tuple<Token, Node>> vec;
            int numComms = getNumComments();
            vec.reserve(numComms);
            for (int i = 0; i < numComms; ++i)
                { vec[i] = getComment(i); }
            return vec;
        }
        /// Serializes a trove to a UTF8-formatted string.
        /** Creates a UTF8 string which encodes the trove, as seen in a Humon file. 
         * The contents of the file are whitespace-formatted and colorized depending on
         * the parameters.
         * \return A tuple containing a unique (self-managing) string pointer and the
         * string's length.
         */
        std::tuple<unique_ptr_free<char const>, int> toString(OutputFormat outputFormat, 
            bool excludeComments, int outputTabSize, std::vector<std::string> const & colorTable) const noexcept 
        {
            std::array<capi::huStringView, capi::HU_COLORKIND_NUMCOLORKINDS> colors;
            for (int i = 0; i < capi::HU_COLORKIND_NUMCOLORKINDS; ++i)
            {
                colors[i].str = colorTable[i].c_str();
                colors[i].size = colorTable[i].length();
            }
            auto str = capi::huTroveToString(ctrove, static_cast<int>(outputFormat), excludeComments, outputTabSize, colors.data());
            auto ptr = unique_ptr_free<char const> { str.str, free_const };
            return { std::move(ptr), str.size };
        }
        /// Returns whether `idx` is a valid child index of the root node. (Whether root has
        /// at least `idx`+1 children.)
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        bool operator % (IntType idx) const noexcept
        {
            return getRootNode() % idx;
        }
        /// Returns whether the root is a dict and has a child with the specified key.
        bool operator % (std::string_view key) const noexcept
        {
            return getRootNode() % key;
        }
        /// Returns the root node's `idx`th child.
        template <class IntType, // TODO: Turn this into a contract
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const noexcept
            { return getRootNode() / idx; }
        /// Returns the root node's child with the specified key.
        Node operator / (std::string_view key) const noexcept
            { return getRootNode() / key; }
        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key.
        std::vector<Node> findNodesByAnnotationKey(std::string_view key) const
        {
            std::vector<Node> vec;
            auto huvec = capi::huFindNodesByAnnotationKeyN(ctrove, key.data(), key.size());
            for (int i = 0; i < huGetVectorSize(& huvec); ++i)
                { vec.emplace_back(reinterpret_cast<capi::huNode *>(
                    huGetVectorElement(& huvec, i))); }

            return vec;
        }
        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified value.
        std::vector<Node> findNodesByAnnotationValue(std::string_view value) const
        {
            std::vector<Node> vec;
            auto huvec = capi::huFindNodesByAnnotationValueN(ctrove, value.data(), value.size());
            for (int i = 0; i < huGetVectorSize(& huvec); ++i)
                { vec.emplace_back(reinterpret_cast<capi::huNode *>(
                    huGetVectorElement(& huvec, i))); }

            return vec;
        }
        /// Returns a new collection of all nodes that are associated an annotation with
        /// the specified key and value.
        std::vector<Node> findNodesByAnnotationKeyValue(std::string_view key, std::string_view value) const
        {
            std::vector<Node> vec;
            auto huvec = capi::huFindNodesByAnnotationKeyValueNN(ctrove, key.data(), key.size(), value.data(), value.size());
            for (int i = 0; i < huGetVectorSize(& huvec); ++i)
                { vec.emplace_back(reinterpret_cast<capi::huNode *>(
                    huGetVectorElement(& huvec, i))); }

            return vec;
        }
        /// Returns a new collection of all nodes that are associated a comment containing
        /// the specified substring.
        std::vector<Node> findNodesByCommentContaining(std::string_view containedText) const
        {
            std::vector<Node> vec;
            auto huvec = capi::huFindNodesByCommentContainingN(ctrove, containedText.data(), containedText.size());
            for (int i = 0; i < huGetVectorSize(& huvec); ++i)
                { vec.emplace_back(reinterpret_cast<capi::huNode *>(
                    huGetVectorElement(& huvec, i))); }

            return vec;
        }

    private:
        capi::huTrove const * ctrove = nullptr;
    };
}
