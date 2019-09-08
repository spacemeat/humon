#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>
#include <limits>
#include <initializer_list>


#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0
#define REQUIRES_DEF(...) typename std::enable_if<(__VA_ARGS__), int>::type


namespace humon
{  
  enum class NodeType
  {
    null,    
    error,
    list,
    dict,
    value,
    comment
  };

  inline std::string to_string(NodeType rhs)
  {
    switch(rhs)
    {
    case NodeType::null: return "null";
    case NodeType::error: return "error";
    case NodeType::list: return "list";
    case NodeType::dict: return "dict";
    case NodeType::value: return "value";
    case NodeType::comment: return "comment";
    default: return "!!unknown!!";
    }
  }


  enum class TokenType
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

  inline std::string to_string(TokenType rhs)
  {
    switch(rhs)
    {
    case TokenType::eof: return "eof";
    case TokenType::startDict: return "startDict";
    case TokenType::endDict: return "endDict";
    case TokenType::startList: return "startList";
    case TokenType::endList: return "endList";
    case TokenType::keyValueSep: return "keyValueSep";
    case TokenType::annotate: return "annotate";
    case TokenType::word: return "word";
    case TokenType::comment: return "comment";
    default: return "!!unknown!!";
    }
  }

  /*
      {
        berries: [black blue crunch]
        papers: {
          important: notReally
          relevant: notRemotely
          recycled: notAnymore
        }
      }

        HuObj hu;
        auto huRoot = hu.setRootType(NodeType::dict);
        auto huBerries = huRoot.add("berries", NodeType::list);
        huBerries.add("black", "blue", "crunch");
        auto huPapers = huRoot.add("papers", NodeType::dict);
        huPapers.add({"important", "notReally"},
                     {"relevant", "notRemotely"},
                     {"recycled", "notAnymore"});

        HuObj hu;
        hu.root = "{}";
        hu.root += ("berries", "{}");        
        hu.root / "berries" += {"black", "blue", "crunch" };

        hu.root += ("papers", "{}");
        hu.root / "papers" += {
          {"important", "notReally"},
          {"relevant", "notRemotely"},
          {"recycled", "notAnymore"}
        };

        hu.root += "papers: {"
          "important: notReally "
          "relevant: notRemotely "
          "recycled: notAnymore"
          "}";                      // one arg


        hu.root += ("papers", 
          "important: notReally "
          "relevant: notRemotely "
          "recycled: notAnymore");  // two args

        hu.root += ("papers", 
          "important: notReally", 
          "relevant: notRemotely", 
          "recycled: notAnymore"); //  four args

        hu.root += ("papers", 
          {"important", "notReally"}, 
          {"relevant", "notRemotely"}, 
          {"recycled", "notAnymore"}); // four args
  */


  class HumonObj;

  class HuNode
  {
  public:
    HuNode(HumonObj * obj, size_t nodeIdx) noexcept;
    inline HumonObj * getObj() const noexcept;
    inline size_t getNodeIdx() const noexcept;
    inline NodeType getType() const noexcept;

    inline size_t getFirstTokenIdx() const noexcept;
    inline size_t getNumTokens() const noexcept;

    inline size_t getParentNodeIdx() const noexcept;
    inline HuNode const & getParentNode() const noexcept;
    inline HuNode & getParentNode() noexcept;
    inline size_t getNumChildren() const noexcept;
    inline size_t getChildNodeIdx(size_t idx) const noexcept;
    inline HuNode const & getChildNode(size_t idx) const noexcept;
    inline HuNode & getChildNode(size_t idx) noexcept;

    inline bool hasKey() const noexcept;
    inline std::string_view getKey() const noexcept;

    inline size_t getIndex() const noexcept;

    inline std::string_view getValue() const noexcept;

    inline std::vector<size_t>::const_iterator begin() const noexcept;
    inline std::vector<size_t>::const_iterator end() const noexcept;

    inline bool operator %(std::string_view key) const noexcept;

    template <class IntType, REQUIRES(std::is_integral<IntType>())>
    inline bool operator %(IntType idx) const noexcept;
    
    inline HuNode const & operator /(std::string_view key) const;

    template <class IntType, REQUIRES(std::is_integral<IntType>())>
    inline HuNode const & operator /(IntType idx) const noexcept;

    inline HuNode const & nextSibling() const noexcept;

    inline operator bool() const noexcept;

    // mutators
    HuNode & operator = (std::string_view humonString);
    HuNode & operator += (std::initializer_list<std::string_view> humonStrings);
    HuNode & insertAt (size_t idx, std::initializer_list<std::string_view> humonStrings);
    HuNode & operator -= (std::initializer_list<size_t> idxs);
    HuNode & operator -= (std::initializer_list<std::string_view> keys);

    // mutators for internal Humon use only
    void setError(std::string_view msg);
    void setType(NodeType type);
    void setParentNodeIdx(size_t parentNodeIdx);
    void addChildNodeIdx(size_t newChildIdx);
    void setAnnotation(std::string_view key, size_t valueIdx);
    void adjustTokenIdxs(long long offset) noexcept;

    // pseudo-mutator
  private:
    void ensureKeysMapped();

    HumonObj * obj = nullptr;
    size_t nodeIdx = std::numeric_limits<size_t>::max();
    std::string errorMsg;
    size_t firstTokenIdx = std::numeric_limits<size_t>::max();
    size_t numTokens = 0;
    NodeType type = NodeType::null;
    size_t collectionNodeIdx = std::numeric_limits<size_t>::max();
    size_t parentNodeIdx = std::numeric_limits<size_t>::max();
    std::vector<size_t> childNodeIdxs;
    std::unordered_map<std::string, size_t> childDictIdxs;
    size_t appendTokenIdx = 0;
    std::unordered_map<std::string, size_t> annotationTokenIdxs;
  };


  struct token_t
  {
    TokenType type;
    std::string_view value;
    size_t line;
    size_t col;

    token_t(
      TokenType type,
      std::string_view value,
      size_t line,
      size_t col)
    : type(type), value(value), line(line), col(col)
      { }
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


  class HumonObj
  {
  public:
    static inline HumonObj fromString(std::string_view humonString, std::string_view name = "");

    HumonObj() = default;
    HumonObj(std::string_view humonString)
      { setRoot(humonString); }
    ~HumonObj() = default;
    HumonObj(HumonObj const & rhs) = default;
    HumonObj(HumonObj && rhs) = default;
    HumonObj & operator = (HumonObj const & rhs) = default;
    HumonObj & operator = (HumonObj && rhs) = default;

    inline std::string const & getName() const noexcept
      { return name; }

    inline std::string const & getData() const noexcept
      { return data; }

    inline token_t getToken(size_t tokenIdx) const noexcept
      { return tokens[tokenIdx]; }
    
    inline std::vector<token_t> const & getTokens() const noexcept
      { return tokens; }

    inline std::vector<token_t> & getTokens() noexcept
      { return tokens; }

    inline HuNode const & getRoot() const noexcept
      { return getNode(0); }

    inline HuNode & getRoot() noexcept
      { ensureParsed(); return getNode(0); }

    inline HuNode const & getNode(size_t nodeIdx) const noexcept
      { return nodes[nodeIdx]; }
    
    inline HuNode & getNode(size_t nodeIdx) noexcept
      { ensureParsed(); return nodes[nodeIdx]; }

    inline std::vector<HuNode> const & getNodes() const noexcept
      { return nodes; }
    
    inline std::vector<HuNode> & getNodes() noexcept
      { ensureParsed(); return nodes; }
    
    inline HuNode const & getNull() const noexcept
      { return nullNode; }

    std::string to_string(OutputFormat format, bool includeComments) const;

    void reset();
    void setName(std::string_view name)
      { this->name = name; }

    void setRoot(std::string_view rhs);

    HuNode & addErrorNode(std::string_view errorMsg, token_t const * token, size_t parentNodeIdx);

    void removeSubtree(size_t nodeIdx);
    void insertSubtrees(std::initializer_list<std::string_view> humonStrings, size_t parentNodeIdx, size_t insertTokenIdx);

    void ensureParsed()
    {
      if (nodes.size() == 0)
        { parse(); }
    }

    // called internally to humon
  private:
    void tokenize();
    void parse();
    HuNode & parseRec(token_t const * cur, size_t parentNodeIdx);
    
    static HuNode nullNode;
    bool error = false;
    std::string name;
    std::string data;
    std::vector<token_t> tokens;
    mutable std::vector<HuNode> nodes;

    size_t inputTabSize = 2;
    size_t outputTabSize = 2;
  };


  inline HumonObj HumonObj::fromString(std::string_view humonString, std::string_view name)
  {
    HumonObj ho;
    ho.setName(name);
    ho.setRoot(humonString);
    return ho;
  }


  // HuNode inline impl

  inline HumonObj * HuNode::getObj() const noexcept
    { return obj; }
  

  inline size_t HuNode::getNodeIdx() const noexcept
    { return nodeIdx; }
  

  inline NodeType HuNode::getType() const noexcept
    { return type; }


  inline size_t HuNode::getFirstTokenIdx() const noexcept
    { return firstTokenIdx; }


  inline size_t HuNode::getNumTokens() const noexcept
    { return numTokens; }


  inline size_t HuNode::getParentNodeIdx() const noexcept
    { return parentNodeIdx; }


  inline HuNode const & HuNode::getParentNode() const noexcept
    { return obj->getNode(parentNodeIdx); }


  inline HuNode & HuNode::getParentNode() noexcept
    { return obj->getNode(parentNodeIdx); }


  inline size_t HuNode::getNumChildren() const noexcept
  {
    return childNodeIdxs.size();
  }


  inline size_t HuNode::getChildNodeIdx(size_t idx) const noexcept
  {
    return childNodeIdxs[idx];
  }


  inline HuNode const & HuNode::getChildNode(size_t idx) const noexcept
  {
    return obj->getNode(childNodeIdxs[idx]);
  }


  inline HuNode & HuNode::getChildNode(size_t idx) noexcept
  {
    return obj->getNode(childNodeIdxs[idx]);
  }


  inline bool HuNode::hasKey() const noexcept
  {
    return obj->getNode(parentNodeIdx).type ==
      NodeType::dict;
  }


  inline std::string_view HuNode::getKey() const noexcept
  {
    if (obj->getNode(parentNodeIdx).type == 
        NodeType::dict)
      { return obj->getToken(firstTokenIdx).value; }
    else
      { return {}; }
  }


  inline size_t HuNode::getIndex() const noexcept
  {
    return collectionNodeIdx;
  }


  inline std::string_view HuNode::getValue() const noexcept
  {
    if (type == NodeType::value)
      { return obj->getToken(firstTokenIdx).value; }
    else
      { return {}; }
  }


  inline std::vector<size_t>::const_iterator
    HuNode::begin() const noexcept
  {
    return childNodeIdxs.begin();
  }


  inline std::vector<size_t>::const_iterator
    HuNode::end() const noexcept
  {
    return childNodeIdxs.end();
  }


  inline bool HuNode::operator %(std::string_view key) const noexcept
  {
    if (type == NodeType::dict)
      { return childDictIdxs.find(std::string(key)) != childDictIdxs.end(); }
    
    return false;
  }


  template <class IntType, REQUIRES_DEF(std::is_integral<IntType>())>
  inline bool HuNode::operator %(IntType idx) const noexcept
  {
    if (type == NodeType::dict || type == NodeType::list)
      { return idx >= 0 && idx < childNodeIdxs.size(); }
    return false;
  }
  
  
  inline HuNode const & HuNode::operator /(std::string_view key) const
  {
    if (type == NodeType::dict)
    {
      auto const & idx = childDictIdxs.find(std::string(key));
      if (idx != childDictIdxs.end())
        { return obj->getNode(idx->second); }
    }

    return obj->getNull();
  }


  template <class IntType, REQUIRES_DEF(std::is_integral<IntType>())>
  inline HuNode const & HuNode::operator /(IntType idx) const noexcept
  {
    if (type == NodeType::list || type == NodeType::dict)
    {
      if (idx >= 0 && idx < childNodeIdxs.size())
        { return obj->getNode(childNodeIdxs[idx]); }
    }

    return obj->getNull();
  }


  inline HuNode const & HuNode::nextSibling() const noexcept
  {
    auto const & parent = obj->getNode(parentNodeIdx);
    if (collectionNodeIdx + 1 < parent.getNumChildren())
      { return parent / (collectionNodeIdx + 1); }
    
    return obj->getNull();
  }


  inline HuNode::operator bool() const noexcept
  {
    return obj != nullptr;
  }
}
