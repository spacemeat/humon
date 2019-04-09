#ifndef HUMON_H
#define HUMON_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ostream>

namespace humon
{
  class Humon;
  using node_t = std::unique_ptr<Humon>;

  //bool operator ==(node_t const & lhs, node_t const & rhs);

  extern node_t fromString(std::string const & humonString);
  extern std::ostream & operator << (std::ostream & stream, Humon const & rhs);

  class Humon
  {
  public:
    static node_t fromString(std::string const & humonString);

  protected:
    static node_t fromParsing(char const *& str, size_t & len);
    Humon();

  public:
    virtual ~Humon();
    node_t clone() const;

    virtual void print(std::ostream & stream, int depth, 
      bool indentFirstLine = true) const = 0;
    virtual bool operator == (Humon const & rhs) const = 0;

    virtual bool isList() const { return false; };
    virtual bool isDict() const { return false; };
    virtual bool isValue() const { return false; };

  private:
    virtual node_t clone_impl() const = 0;
  };


  class HumonList : public Humon
  {
  public:
    HumonList(char const *& str, size_t & len);
    HumonList();
    virtual ~HumonList();

    virtual void print(std::ostream & stream, int depth, 
      bool indentFirstLine = true) const override;

    virtual bool operator == (Humon const & rhs) const override;

    virtual bool isList() const override { return true; };

    size_t getNumElements() const { return elems.size(); }

    node_t const & operator [](size_t idx) const;
    size_t indexOf(node_t node) const;

    void append(node_t node);
    void addAt(size_t index, node_t node);
    void removeAt(size_t index);

  private:
    virtual node_t clone_impl() const override;

    std::vector<node_t> elems;
  };


  class HumonDict : public Humon
  {
  public:
    HumonDict(char const *& str, size_t & len);
    HumonDict();
    virtual ~HumonDict();

    virtual void print(std::ostream & stream, int depth, 
      bool indentFirstLine = true) const override;

    virtual bool operator == (Humon const & rhs) const override;

    virtual bool isDict() const override { return true; };

    size_t getNumElements() const { return elems.size(); }
    bool hasKey(std::string const & key) const;

    node_t const & operator [](std::string const & key) const;

    void add(std::string const & key, node_t node);
    void removeAt(std::string const & key);

  private:
    virtual node_t clone_impl() const override;

    std::map<std::string, node_t> elems;
  };


  class HumonValue : public Humon
  {
  public:
    HumonValue(char const *& str, size_t & len);
    HumonValue();
    virtual ~HumonValue();

    virtual void print(std::ostream & stream, int depth, 
      bool indentFirstLine = true) const override;

    virtual bool operator == (Humon const & rhs) const override;

    virtual bool isValue() const override { return true; };

    bool tryGet(bool & rv) const;
    bool tryGet(long & rv) const;
    bool tryGet(float & rv) const;

    bool getBool() const;
    long getLong() const;
    float getFloat() const;
    std::string const & getString() const;

  private:
    virtual node_t clone_impl() const override;

    std::string value;
  };
}

#endif
