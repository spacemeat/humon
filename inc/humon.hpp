#pragma once

#include <string_view>
#include <tuple>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <charconv>
#include <optional>

namespace hu
{
    namespace capi
    {
#include "humon.h"
    }


    enum class TokenKind : int
    {
        null = capi::HU_TOKENKIND_NULL,
        eof = capi::HU_TOKENKIND_EOF,
        startList = capi::HU_TOKENKIND_STARTLIST,
        endList = capi::HU_TOKENKIND_ENDLIST,
        startDict = capi::HU_TOKENKIND_STARTDICT,
        endDict = capi::HU_TOKENKIND_ENDDICT,
        keyValueSep = capi::HU_TOKENKIND_KEYVALUESEP,
        annotate = capi::HU_TOKENKIND_ANNOTATE,
        word = capi::HU_TOKENKIND_WORD,
        comment = capi::HU_TOKENKIND_COMMENT
    };

    static inline char const * TokenKindToString(TokenKind rhs)
    {
        return capi::huTokenKindToString((int) rhs);
    }


    enum class NodeKind : int
    {
        null = capi::HU_NODEKIND_NULL,
        list = capi::HU_NODEKIND_LIST,
        dict = capi::HU_NODEKIND_DICT,
        value = capi::HU_NODEKIND_VALUE
    };

    static inline char const * huNodeKindToString(NodeKind rhs)
    {
        return capi::huNodeKindToString((int) rhs);
    }

    enum class ColorKind
    {
        none = capi::HU_COLORKIND_NONE,
        end = capi::HU_COLORKIND_END,
        puncList = capi::HU_COLORKIND_PUNCLIST,
        puncDict = capi::HU_COLORKIND_PUNCDICT,
        puncKeyValueSep = capi::HU_COLORKIND_PUNCKEYVALUESEP,
        puncAnnotate = capi::HU_COLORKIND_PUNCANNOTATE,
        puncAnnotateDict = capi::HU_COLORKIND_PUNCANNOTATEDICT,
        puncAnnotateKeyValueSep = capi::HU_COLORKIND_PUNCANNOTATEKEYVALUESEP,
        key = capi::HU_COLORKIND_KEY,
        value = capi::HU_COLORKIND_VALUE,
        comment = capi::HU_COLORKIND_COMMENT,
        annoKey = capi::HU_COLORKIND_ANNOKEY,
        annoValue = capi::HU_COLORKIND_ANNOVALUE,
        whitespace = capi::HU_COLORKIND_WHITESPACE,

        numColorKinds = capi::HU_COLORKIND_NUMCOLORKINDS
    };


    enum class OutputFormat : int
    {
        preserved = capi::HU_OUTPUTFORMAT_PRESERVED,
        minimal = capi::HU_OUTPUTFORMAT_MINIMAL,
        pretty = capi::HU_OUTPUTFORMAT_PRETTY
    };

    static inline char const * huOutputFormatToString(OutputFormat rhs)
    {
        return capi::huOutputFormatToString((int) rhs);
    }

    enum class ErrorCode : int
    {
        unexpectedEof = capi::HU_ERROR_UNEXPECTED_EOF,
        syntaxError = capi::HU_ERROR_SYNTAX_ERROR,
        startEndMismatch = capi::HU_ERROR_START_END_MISMATCH
    };

    static inline char const * huOutputErrorToString(ErrorCode rhs)
    {
        return capi::huOutputErrorToString((int) rhs);
    }


    // NOTE: This is really cool. If you provide an overload of std::from_chars that fills in a custom type T, you can extract a T directly from the value node like any of the specializations. You can alternately provide your own specialization on value<T>.
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
            // TODO: (gcc): Once from_chars(..float&) is complete, use it instead. We're making a useless string here and it maketh me to bite metal. Also make noexcept.
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
            // TODO: (gcc): Once from_chars(..float&) is complete, use it instead. We're making a useless string here and it maketh me to bite metal. Also make noexcept.
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


    static inline std::string_view make_sv(capi::huStringView const & husv)
    {
        return std::string_view(husv.str, husv.size);
    }


    class Token
    {
    public:
        Token(capi::huToken_t * ctoken) : ctoken(ctoken) { }
        bool isValid() const noexcept { return ctoken != nullptr; }
        TokenKind getKind() const noexcept { return static_cast<TokenKind>(ctoken->tokenKind); }
        std::string_view getValue() const noexcept { return make_sv(ctoken->value); }
        int getLine() const noexcept { return ctoken->line; }
        int getCol() const noexcept { return ctoken->col; }
        int getEndLine() const noexcept { return ctoken->endLine; }
        int getEndCol() const noexcept { return ctoken->endCol; }

    private:
        capi::huToken_t * ctoken;
    };


    class Node
    {
    public:
        Node(capi::huNode_t * cnode) : cnode(cnode) { }
        bool isValid() const noexcept { return cnode != nullptr; }
        // TODO: int getIdx() maybe
        NodeKind getKind() const noexcept { return static_cast<NodeKind>(cnode->kind); }
        Token getFirstToken() const noexcept { return Token(cnode->firstToken); }
        Token getFirstValueToken() const noexcept { return Token(cnode->firstValueToken); }
        Token getLastValueToken() const noexcept { return Token(cnode->lastValueToken); }
        Token getLastToken() const noexcept { return Token(cnode->lastToken); }
        int getChildOrdinal() const noexcept { return cnode->childIdx; }
        Node getParentNode() const noexcept { return Node(capi::huGetParentNode(cnode)); }
        int getNumChildren() const noexcept { return capi::huGetNumChildren(cnode); }
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node getChild(IntType idx) const noexcept { return capi::huGetChildNodeByIndex(cnode, idx); }
        Node getChild(std::string_view key) const noexcept 
            { return capi::huGetChildNodeByKey(cnode, key.data(), key.size()); }
        bool hasKey() const noexcept { return capi::huHasKey(cnode); }
        Token getKeyToken() const noexcept { return Token(capi::huGetKey(cnode)); }
        bool hasValue() const noexcept { return capi::huHasValue(cnode); }
        Token getValueToken() const noexcept { return Token(capi::huGetValue(cnode)); }
        Token getStartToken() const noexcept { return Token(capi::huGetStartToken(cnode)); }
        Token getEndToken() const noexcept { return Token(capi::huGetEndToken(cnode)); }
        Node getNextSibling() const noexcept { return Node(capi::huNextSibling(cnode)); }
        int getNumAnnotations() const noexcept { return capi::huGetNumAnnotations(cnode); }
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        std::tuple<Token, Token> getAnnotation(IntType idx) const noexcept 
        { 
            auto canno = capi::huGetAnnotation(cnode, idx); 
            return { Token(canno->key), Token(canno->value) };
        }
        int getNumComments() const noexcept { return capi::huGetNumComments(cnode); }
        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        std::tuple<Token, Node> getComment(IntType idx) const noexcept 
        {
            auto ccomm = capi::huGetComment(cnode, idx);
            return { Token(ccomm->commentToken), Node(ccomm->owner) };
        }

        std::string_view getKey() const noexcept 
        {
            if (capi::huHasKey(cnode))
            {
                return make_sv(cnode->keyToken->value);
            }

            return "";
        }

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

        bool operator % (std::string_view key) const noexcept
        {
            if (getKind() == NodeKind::dict)
                { return getChild(key).isValid(); }
            return false;
        }

        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const noexcept { return getChild(idx); }
        Node operator / (std::string_view key) const noexcept { return getChild(key); }

        // Lets you say:
        // auto sv = valueNode / h::value<string_view> {};
        // auto f = valueNode / h::value<float> {};
        template <class U>
        inline U operator / (value<U> ve) const
        {
            if (getKind() != NodeKind::value)
                { return U {}; }
            return ve.extract(getValue());
        }
        
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

    private:
        capi::huNode_t * cnode;
    };

    static inline void free_const(void const * t) noexcept
        { free(const_cast<void *>(t)); }

    template <typename T>
    using unique_ptr_free = std::unique_ptr<T, decltype(free_const)*>;

    class Trove
    {
    public:
        static Trove fromString(std::string_view name, std::string_view data,
            int inputTabSize = 4, int outputTabSize = 4) noexcept
        {
            return Trove(capi::huMakeTroveFromString(
                name.data(), data.data(), data.size(), inputTabSize, outputTabSize));
        }

        static Trove fromString(std::string_view data,
            int inputTabSize = 4, int outputTabSize = 4) noexcept
        {
            return Trove(capi::huMakeTroveFromString(
                NULL, data.data(), data.size(), inputTabSize, outputTabSize));
        }

        static Trove fromFile(std::string_view name, std::string_view path,
            int inputTabSize = 4, int outputTabSize = 4) noexcept
        {
            return Trove(capi::huMakeTroveFromFile(
                name.data(), path.data(), inputTabSize, outputTabSize));
        }
        
        static Trove fromFile(std::string_view path,
            int inputTabSize = 4, int outputTabSize = 4) noexcept
        {
            return Trove(capi::huMakeTroveFromFile(
                "", path.data(), inputTabSize, outputTabSize));
        }
        
        static Trove fromIstream(std::string_view name, std::istream & in,
            int inputTabSize = 4, int outputTabSize = 4) noexcept
        {
            // TODO: This currently loads the whole stream before tokenizing. It would
            // be better to load and tokenize and parse in parallel, for large file.
            std::stringstream buffer;
            buffer << in.rdbuf();
            return fromString(name, buffer.str(), inputTabSize, outputTabSize);
        }

        static Trove fromIstream(std::istream & in,
            int inputTabSize = 4, int outputTabSize = 4) noexcept
        {
            // TODO: This currently loads the whole stream before tokenizing. It would
            // be better to load and tokenize and parse in parallel, for large file.
            std::stringstream buffer;
            buffer << in.rdbuf();
            return fromString("", buffer.str(), inputTabSize, outputTabSize);
        }
    
    public:
        Trove() { }

    private:
        Trove(capi::huTrove_t * ctrove) : ctrove(ctrove) { }

    public:
        Trove(Trove const & rhs) = delete;
        Trove(Trove && rhs)
        {
            if (ctrove)
                { capi::huDestroyTrove(ctrove); }

            ctrove = rhs.ctrove;
            rhs.ctrove = nullptr;
        }
        ~Trove()
        {
            if (ctrove)
                { capi::huDestroyTrove(ctrove); }
        }
        Trove & operator = (Trove const & rhs) = delete;
        Trove & operator = (Trove && rhs)
        {
            if (ctrove)
                { capi::huDestroyTrove(ctrove); }

            ctrove = rhs.ctrove;
            rhs.ctrove = nullptr;
            
            return * this;
        }

        bool isValid() const noexcept { return ctrove != nullptr; }
        int getNumTokens() const noexcept { return capi::huGetNumTokens(ctrove); }
        Token getToken(int idx) const noexcept { return Token(capi::huGetToken(ctrove, idx)); }
        int getNumNodes() const noexcept { return capi::huGetNumNodes(ctrove); }
        Node getRootNode() const noexcept { return capi::huGetRootNode(ctrove); }
        Node getNode(int idx) const noexcept { return Node(capi::huGetNode(ctrove, idx)); }
        int getNumErrors() const noexcept { return capi::huGetNumErrors(ctrove); }
        std::tuple<ErrorCode, Token> getError(int idx) const noexcept 
        {
            auto cerr = capi::huGetError(ctrove, idx);
            return { static_cast<ErrorCode>(cerr->errorCode), Token(cerr->errorToken) };
        }
        int getNumAnnotations() const noexcept { return capi::huGetNumTroveAnnotations(ctrove); }
        std::tuple<Token, Token> getAnnotation(int idx) const noexcept 
        { 
            auto canno = capi::huGetTroveAnnotation(ctrove, idx); 
            return { Token(canno->key), Token(canno->value) };
        }
        int getNumComments() const noexcept { return capi::huGetNumTroveComments(ctrove); }
        std::tuple<Token, Node> getComment(int idx) const noexcept 
        {
            auto ccomm = capi::huGetTroveComment(ctrove, idx);
            return { Token(ccomm->commentToken), nullptr };
        }

        std::tuple<unique_ptr_free<char const>, int> toString(OutputFormat outputFormat, 
            bool excludeComments, std::vector<std::string> const & colorTable) const noexcept 
        {
            std::array<capi::huStringView_t, capi::HU_COLORKIND_NUMCOLORKINDS> colors;
            for (int i = 0; i < capi::HU_COLORKIND_NUMCOLORKINDS; ++i)
            {
                colors[i].str = colorTable[i].c_str();
                colors[i].size = colorTable[i].length();
            }
            auto str = capi::huTroveToString(ctrove, static_cast<int>(outputFormat), excludeComments, colors.data());
            auto ptr = unique_ptr_free<char const> { str.str, free_const };
            return { std::move(ptr), str.size };
        }

        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        bool operator % (IntType idx) const noexcept
        {
            return getRootNode() % idx;
        }

        bool operator % (std::string_view key) const noexcept
        {
            return getRootNode() % key;
        }

        template <class IntType, 
            typename std::enable_if<
                std::is_integral<IntType>::value, IntType>::type * = nullptr>
        Node operator / (IntType idx) const noexcept { return getRootNode() / idx; }
        Node operator / (std::string_view key) const noexcept { return getRootNode() / key; }

    private:
        capi::huTrove_t * ctrove = nullptr;
    };
}
