#ifndef HUMON_H
#define HUMON_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ostream>

#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0
#define REQUIRES_DEF(...) typename std::enable_if<(__VA_ARGS__), int>::type


namespace humon
{
  class HuNode;
  class HuList;
  class HuDict;
  class HuValue;

  using nodePtr_t = std::unique_ptr<HuNode>;

  extern nodePtr_t fromString(std::string const & humonString);
  extern std::ostream & operator << (std::ostream & stream, HuNode const & rhs);

  class HuNode
  {
  public:
    static nodePtr_t fromString(std::string const & humonString);
    HuNode(HuNode const & rhs) = delete;
    HuNode(HuNode && rhs) = delete;

  protected:
    static nodePtr_t fromParsing(char const *& str, size_t & len);
    HuNode();

  public:
    virtual ~HuNode();
    nodePtr_t clone() const;

    template <class ReturnType>
    ReturnType & as()
    {
      return dynamic_cast<ReturnType &>(*this);
    }

    template <class ReturnType>
    ReturnType const & as() const
    {
      return dynamic_cast<ReturnType const &>(*this);
    }

    virtual bool isList() const { return false; };
    virtual bool isDict() const { return false; };
    virtual bool isValue() const { return false; };

    HuList const & asList() const { return as<HuList const>(); }
    HuDict const & asDict() const { return as<HuDict>(); }
    HuValue const & asValue() const { return as<HuValue>(); }

    HuList & asList() { return as<HuList>(); }
    HuDict & asDict() { return as<HuDict>(); }
    HuValue & asValue() { return as<HuValue>(); }

    bool operator %(std::string const & key) const;
    template <class IntType, REQUIRES(std::is_integral<IntType>())>
    bool operator %(IntType idx) const;
    HuNode const & operator /(std::string const & key) const;
    template <class IntType, REQUIRES(std::is_integral<IntType>())>
    HuNode const & operator /(IntType idx) const;
    HuNode const & operator >>(bool & rhs) const;
    HuNode const & operator >>(long & rhs) const;
    HuNode const & operator >>(float & rhs) const;
    HuNode const & operator >>(std::string & rhs) const;

    bool operator %(std::string const & key)
      { return std::as_const(*this) % key; }
    template <class IntType, REQUIRES(std::is_integral<IntType>())>
    bool operator %(IntType idx)
      { return std::as_const(*this) % idx; }
    HuNode & operator /(std::string const & key)
      { return const_cast<HuNode &>(std::as_const(*this) / key); }
    template <class IntType, REQUIRES(std::is_integral<IntType>())>
    HuNode & operator /(IntType idx)
      { return const_cast<HuNode &>(std::as_const(*this) / idx); }
    HuNode & operator >>(bool & rhs)
      { return const_cast<HuNode &>(std::as_const(*this) >> rhs); }
    HuNode & operator >>(long & rhs)
      { return const_cast<HuNode &>(std::as_const(*this) >> rhs); }
    HuNode & operator >>(float & rhs)
      { return const_cast<HuNode &>(std::as_const(*this) >> rhs); }
    HuNode & operator >>(std::string & rhs)
      { return const_cast<HuNode &>(std::as_const(*this) >> rhs); }

    operator bool() const;
    operator long() const;
    operator float() const;
    operator std::string() const;

    std::string getReport() const;
    virtual void print(std::ostream & stream, int depth = 0,
      bool indentFirstLine = true) const = 0;
    virtual bool operator ==(HuNode const & rhs) const = 0;

  private:
    virtual nodePtr_t clone_impl() const = 0;
  };


  class HuList : public HuNode
  {
  public:
    HuList(char const *& str, size_t & len);
    HuList();
    HuList(HuList const & rhs) = delete;
    HuList(HuList && rhs) = delete;
    virtual ~HuList();

    virtual void print(std::ostream & stream, int depth, 
      bool indentFirstLine = true) const override;

    virtual bool operator ==(HuNode const & rhs) const override;

    virtual bool isList() const override { return true; };

    size_t size() const { return elems.size(); }

    HuNode & at(size_t idx)
    {
      return const_cast<HuNode &>(std::as_const(*this).at(idx));
    }

    HuNode const & at(size_t idx) const
    {
      return * elems.at(idx).get();
    }

    template <class ReturnType>
    ReturnType & at(size_t idx)
    {
      return const_cast<ReturnType &>(std::as_const(*this).at<ReturnType>(idx));
    }

    template <class ReturnType>
    ReturnType const & at(size_t idx) const
    {
      return elems.at(idx).get()->as<ReturnType>();
    }

    size_t indexOf(nodePtr_t node) const;

    void append(nodePtr_t node);
    void addAt(size_t index, nodePtr_t node);
    void removeAt(size_t index);

  private:
    virtual nodePtr_t clone_impl() const override;

    std::vector<nodePtr_t> elems;
  };


  class HuDict : public HuNode
  {
  public:
    HuDict(char const *& str, size_t & len);
    HuDict();
    HuDict(HuDict const & rhs) = delete;
    HuDict(HuDict && rhs) = delete;
    virtual ~HuDict();

    virtual void print(std::ostream & stream, int depth, 
      bool indentFirstLine = true) const override;

    virtual bool operator ==(HuNode const & rhs) const override;

    virtual bool isDict() const override { return true; };

    size_t size() const { return elems.size(); }
    bool hasKey(std::string const & key) const;

    std::string const & keyAt(size_t idx)
    {
      return keys.at(idx);
    }

    HuNode & at(size_t idx)
    {
      return const_cast<HuNode &>(std::as_const(*this).at(idx));
    }

    HuNode const & at(size_t idx) const
    {
      auto key = keys.at(idx);
      return at(key);
    }

    template <class ReturnType>
    ReturnType & at(size_t idx)
    {
      return const_cast<HuNode &>(std::as_const(*this).at<ReturnType>(idx));
    }

    template <class ReturnType>
    ReturnType const & at(size_t idx) const
    {
      auto key = keys.at(idx);
      return at<ReturnType>(key);
    }

    HuNode & at(std::string const & key)
    {
      return * elems.at(key).get();
    }

    HuNode const & at(std::string const & key) const
    {
      return * elems.at(key).get();
    }

    template <class ReturnType>
    ReturnType & at(std::string const & key)
    {
      return const_cast<ReturnType &>(std::as_const(*this).at<ReturnType>(key));
    }

    template <class ReturnType>
    ReturnType const & at(std::string const & key) const
    {
      return elems.at(key).get()->as<ReturnType>();
    }

    void add(std::string const & key, nodePtr_t node);
    void removeAt(std::string const & key);

  private:
    virtual nodePtr_t clone_impl() const override;

    std::map<std::string, nodePtr_t> elems;
    std::vector<std::string> keys;
  };


  class HuValue : public HuNode
  {
  public:
    HuValue(char const *& str, size_t & len);
    HuValue();
    HuValue(HuValue const & rhs) = delete;
    HuValue(HuValue && rhs) = delete;
    virtual ~HuValue();

    virtual void print(std::ostream & stream, int depth, 
      bool indentFirstLine = true) const override;

    virtual bool operator ==(HuNode const & rhs) const override;

    virtual bool isValue() const override { return true; };

    bool tryGet(bool & rv) const;
    bool tryGet(long & rv) const;
    bool tryGet(float & rv) const;

    bool getBool() const;
    long getLong() const;
    float getFloat() const;
    std::string const & getString() const;

    void setValue(bool val);
    void setValue(long val);
    void setValue(float val);
    void setValue(std::string const & val);

  private:
    virtual nodePtr_t clone_impl() const override;

    std::string value;
  };


  template <class IntType, REQUIRES_DEF(std::is_integral<IntType>())>
  bool HuNode::operator %(IntType idx) const
  {
    if (isList())
      { return static_cast<size_t>(idx) < asList().size(); }
    else
      { return static_cast<size_t>(idx) < asDict().size(); }
  }


  template <class IntType, REQUIRES_DEF(std::is_integral<IntType>())>
  HuNode const & HuNode::operator /(IntType idx) const
  {
    if (isList())
      { return asList().at(idx); }
    else
      { return asDict().at(idx); }
  }
}


#endif
