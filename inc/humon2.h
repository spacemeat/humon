#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>
#include <limits>
#include <initializer_list>
#include <charconv>


namespace humon
{  
  enum class NodeKind
  {
    null,    
    error,
    list,
    dict,
    value,
    comment
  };

  inline std::string to_string(NodeKind rhs)
  {
    switch(rhs)
    {
    case NodeKind::null: return "null";
    case NodeKind::error: return "error";
    case NodeKind::list: return "list";
    case NodeKind::dict: return "dict";
    case NodeKind::value: return "value";
    case NodeKind::comment: return "comment";
    default: return "!!unknown!!";
    }
  }


  enum class TokenKind
  {
    eof,
    startDict,
    endDict,
    startList,
    endList,
    keyValueSep,
    annotate,
    word,
    comment
  };

  inline std::string to_string(TokenKind rhs)
  {
    switch(rhs)
    {
    case TokenKind::eof: return "eof";
    case TokenKind::startDict: return "startDict";
    case TokenKind::endDict: return "endDict";
    case TokenKind::startList: return "startList";
    case TokenKind::endList: return "endList";
    case TokenKind::keyValueSep: return "keyValueSep";
    case TokenKind::annotate: return "annotate";
    case TokenKind::word: return "word";
    case TokenKind::comment: return "comment";
    default: return "!!unknown!!";
    }
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
        { return -1; }
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
    inline std::string_view extract(std::string_view valStr)
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


  template <typename T>
  struct string_view_reloc
  {
    T loc;
    T size;

    string_view_reloc() = default;
    string_view_reloc(std::string_view rhsStr, std::string_view rhsView)
      : loc(rhsView.data() - rhsStr.data()),
        size(rhsView.size())
      { }

    inline std::string_view on(std::string_view str) const noexcept
    {
      return { str.data() + loc, size };
    }
  };


  template <typename T>
  struct token_t
  {
    TokenKind kind;
    string_view_reloc<T> value;
    T line;
    T col;

    token_t() = default;
    token_t(
      TokenKind kind,
      std::string_view str,
      std::string_view value,
      T line,
      T col)
    : kind(kind), value(str, value), line(line), col(col)
      { }
    token_t(const token_t<T> &) = default;
    token_t(token_t<T> &&) = default;
    ~token_t() = default;

    token_t<T> & operator =(const token_t<T> &) = default;
    token_t<T> & operator =(token_t<T> &&) = default;
  };


  template <typename T>
  class Trove;

  template <typename T>
  class Node
  {
  public:
    using idx_t = typename std::make_unsigned<T>::type;
    static const idx_t nullIdx = std::numeric_limits<idx_t>::max();

    Node(Trove<T> * trove, idx_t nodeIdx) noexcept;
    Node() = default;

    void print() const noexcept;

    inline Trove<T> * getObj() const noexcept;
    inline idx_t getNodeIdx() const noexcept;
    inline NodeKind getKind() const noexcept;

    inline idx_t getFirstTokenIdx() const noexcept;
    inline idx_t getNumTokens() const noexcept;

    inline idx_t getCollectionIdx() const noexcept; // TODO: kill?
    inline idx_t getParentNodeIdx() const noexcept;
    inline Node<T> const & getParentNode() const noexcept;
    inline Node<T> & getParentNode() noexcept;
    inline idx_t getNumChildren() const noexcept;
    inline idx_t getChildNodeIdx(idx_t idx) const noexcept;
    inline Node<T> const & getChildNode(idx_t idx) const noexcept;
    inline Node<T> & getChildNode(idx_t idx) noexcept;

    inline bool hasKey() const noexcept;
    inline std::string_view getKey() const noexcept;

    inline idx_t getIndex() const noexcept;

    inline std::string_view getValue() const noexcept;

    inline typename std::vector<idx_t>::const_iterator begin() const noexcept;
    inline typename std::vector<idx_t>::const_iterator end() const noexcept;

    inline bool operator %(std::string_view key) const noexcept;

    template <class IntType, 
      typename std::enable_if<
        std::is_integral<IntType>::value, IntType>::type * = nullptr>
    inline bool operator %(IntType idx) const noexcept;
    
    inline Node<T> const & operator /(std::string_view key) const;

    template <class IntType, 
      typename std::enable_if<
        std::is_integral<IntType>::value, IntType>::type * = nullptr>
    inline Node<T> const & operator /(IntType idx) const noexcept;

    template <class U>
    inline U operator /(value<U> ve) const noexcept;

    inline Node<T> const & nextSibling() const noexcept;

    inline operator bool() const noexcept;

    // mutators for internal Humon use only
    void _setKeyIdx(idx_t idx) noexcept;
    void _setFirstTokenIdx(idx_t idx) noexcept;
    void _setNumTokens(idx_t numTokens) noexcept;
    void _setKind(NodeKind kind);
    void _setParentNodeIdx(idx_t parentNodeIdx);
    void _setCollectionIdx(idx_t collectionIdx);
    void _addChildNodeIdx(idx_t newChildIdx);
    void _setCollectionAppendTokenIdx(idx_t appendIdx) noexcept;
    void _setAnnotation(std::string_view key, idx_t valueIdx);
    void _setAnnotationAppendTokenIdx(idx_t appendIdx) noexcept;

    // pseudo-mutator
  private:
    void ensureKeysMapped() const;

    Trove<T> * trove = nullptr;
    std::string errorMsg;

    idx_t nodeIdx = nullIdx;
    idx_t keyIdx = nullIdx;
    idx_t firstTokenIdx = nullIdx;
    idx_t numTokens = 0;

    idx_t collectionIdx = nullIdx;
    idx_t parentNodeIdx = nullIdx;
    idx_t collectionAppendTokenIdx = nullIdx;
    idx_t annotationAppendTokenIdx = nullIdx;

    NodeKind kind = NodeKind::null;
    std::vector<idx_t> childNodeIdxs;
    mutable std::unordered_map<std::string, idx_t> childDictIdxs;
    std::unordered_map<std::string, idx_t> annotationTokenIdxs;
  };


  enum class OutputFormat
  {
    preserved,
    minimal,
    pretty
  };

  inline std::string to_string(OutputFormat rhs)
  {
    switch(rhs)
    {
    case OutputFormat::preserved: return "preserved";
    case OutputFormat::minimal: return "minimal";
    case OutputFormat::pretty: return "pretty";
    default: return "!!unknown!!";
    }
  }


  template <typename T>
  class Trove
  {
  public:
    using idx_t = typename std::make_unsigned<T>::type;
    static const idx_t nullIdx = std::numeric_limits<idx_t>::max();

    static inline Trove<T> fromString(std::string_view humonString, std::string_view name = "");

    Trove() = default;
    Trove(std::string_view humonString)
      { setRoot(humonString); }
    ~Trove() = default;
    Trove(Trove<T> const & rhs) = default;
    Trove(Trove<T> && rhs) = default;
    Trove<T> & operator = (Trove<T> const & rhs) = default;
    Trove<T> & operator = (Trove<T> && rhs) = default;

    inline std::string const & getName() const noexcept
      { return name; }

    inline std::string const & getData() const noexcept
      { return data; }

    inline token_t<idx_t> const & getToken(idx_t tokenIdx) const noexcept
      { return tokens[tokenIdx]; }
    
    inline std::vector<token_t<idx_t>> const & getTokens() const noexcept
      { return tokens; }

    inline std::vector<token_t<idx_t>> & getTokens() noexcept
      { return tokens; }

    inline Node<T> const & getRoot() const noexcept
      { return getNode(0); }

    inline Node<T> & getRoot() noexcept
      { ensureParsed(); return getNode(0); }

    inline Node<T> const & getNode(idx_t nodeIdx) const noexcept
      { return nodes[nodeIdx]; }
    
    inline Node<T> & getNode(idx_t nodeIdx) noexcept
      { ensureParsed(); return nodes[nodeIdx]; }

    inline std::vector<Node<T>> const & getNodes() const noexcept
      { return nodes; }
    
    inline std::vector<Node<T>> & getNodes() noexcept
      { ensureParsed(); return nodes; }
    
    inline Node<T> const & getNull() const noexcept
      { return nullNode; }

    std::string to_string(OutputFormat format, bool includeComments) const;

    void reset();
    void setName(std::string_view name)
      { this->name = name; }

    void setRoot(std::string_view rhs);

    void addError(std::string_view errorMsg, token_t<idx_t> const * token);

    void removeSubtree(idx_t nodeIdx);
    void replaceSubtree(std::string_view humonString, idx_t nodeIdx);
    void insertSubtrees(std::initializer_list<std::string_view> humonStrings, idx_t parentNodeIdx, idx_t insertTokenIdx);

    void ensureParsed()
    {
      if (nodes.size() == 0)
        { parse(); }
    }

    // called internally to humon
  private:
    void tokenize();
    void parse();
    Node<T> & parseRec(token_t<idx_t> const *& cur);
    
    static inline Node<T> nullNode;
    std::string name;
    std::string data;
    std::vector<token_t<idx_t>> tokens;
    std::vector<Node<T>> nodes;
    std::vector<std::pair<std::string, idx_t>> errors;

    idx_t inputTabSize = 2;
    idx_t outputTabSize = 2;
  };


  template <typename T>
  inline Trove<T> Trove<T>::fromString(std::string_view humonString, std::string_view name)
  {
    Trove<T> ho;
    ho.setName(name);
    ho.setRoot(humonString);
    return ho;
  }
}


#include "impl/node.inl"
#include "impl/trove.inl"
#include "impl/tokenize.inl"
#include "impl/parse.inl"
