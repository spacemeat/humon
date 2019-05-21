#include "humon.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace humon;

using ss = stringstream;

// ----------- parsing love

int line = 1;

unsigned stos(std::string const & str, size_t * idx = 0, int base = 10) {
  long result = stol(str, idx, base);
  if (result < std::numeric_limits<short>::min() ||
      result > std::numeric_limits<short>::max())
    { throw out_of_range("stos"); }
  return result;
}


unsigned stous(std::string const & str, size_t * idx = 0, int base = 10) {
  unsigned long result = stoul(str, idx, base);
  if (result > std::numeric_limits<unsigned short>::max())
    { throw out_of_range("stous"); }
  return result;
}


unsigned stoui(std::string const & str, size_t * idx = 0, int base = 10) {
  unsigned long result = stoul(str, idx, base);
  if (result > std::numeric_limits<unsigned>::max()) 
    { throw out_of_range("stous"); }
  return result;
}


// Nom whitespace.
void eatWs(char const *& str, size_t & len)
{
  bool commenting = false;
  while (len > 0)
  {
    if (commenting)
    {
      if (str[0] == '\n') // TODO: proper endl symbol
      {
        commenting = false;
        line += 1;
      }
      str += 1;
      len -= 1;
    }

    else if (isspace(str[0]) || str[0] == ',')
    {
      if (str[0] == '\n')
      { line += 1; }
      str += 1;
      len -= 1;
    }

    else if (str[0] == '#')
    { 
      commenting = true;
    }

    else break;
  }
}

enum class TokenType
{
  listBegin,
  listEnd,
  dictBegin,
  dictEnd,
  keyValueDelim,
  value,
  end
};


char const * lookaheadStr = nullptr;
size_t lookaheadLen;
pair<TokenType, string> lookaheadToken;

// Returns empty string when we done.
pair<TokenType, string> getToken(char const *& str, size_t & len)
{
  if (lookaheadStr != nullptr)
  {
    str = lookaheadStr;
    len = lookaheadLen;
    lookaheadStr = nullptr;
    return lookaheadToken;
  }

  eatWs(str, len);

  auto tokenType = TokenType::end;
  string token;
  if (len > 0)
  {
    if (str[0] == '[' || 
        str[0] == ']' ||
        str[0] == '{' ||
        str[0] == '}' ||
        str[0] == ':')
    {
      token = str[0];

      switch(str[0])
      {
        case '[': tokenType = TokenType::listBegin; break;
        case ']': tokenType = TokenType::listEnd; break;
        case '{': tokenType = TokenType::dictBegin; break;
        case '}': tokenType = TokenType::dictEnd; break;
        case ':': tokenType = TokenType::keyValueDelim; break;
      }

      str += 1;
      len -= 1;
    }
    else if (str[0] == '"' || str[0] == '\'')
    {
      auto quoteChar = str[0];
      str += 1;
      len -= 1;
      tokenType = TokenType::value;

      // copy until the next [^\]"
      bool switching = false;
      while (len > 0)
      {
        if (str[0] == '\\')
        {
          switching = true;
          str += 1;
          len -= 1;
        }
        else
        {
          if (str[0] != quoteChar || switching)
          {
            token.push_back(str[0]);
            switching = false;
            str += 1;
            len -= 1;
          }
          else
          {
            // eat the last "
            switching = false;
            str += 1;
            len -= 1;
            break;
          }
        }
      }
    }
    else
    {
      tokenType = TokenType::value;
      bool switching = false;
      while (len > 0)
      {
        if (str[0] == '\\')
        {
          switching = true;
          str += 1;
          len -= 1;
        }
        else
        {
          if (switching ||
              (isspace(str[0]) == false && 
                str[0] != ',' &&
                str[0] != '[' &&
                str[0] != ']' &&
                str[0] != '{' &&
                str[0] != '}' &&
                str[0] != ':'))
          {
            switching = false;
            token.push_back(str[0]);
            str += 1;
            len -= 1;
          }
          else
          { 
            switching = false;
            break;
          }
        }
      }

      // if last char is a '\\'
      if (switching)
      {
        token.push_back('\\');
      }
    }
  }

  return make_pair(tokenType, token);
}

pair<TokenType, string> peekToken(char const * str, size_t len)
{
  eatWs(str, len);
  if (lookaheadStr == nullptr)
  {
    lookaheadToken = getToken(str, len);
    lookaheadStr = str;
    lookaheadLen = len;
  }

  return lookaheadToken;
}


template<class ... T>
string fmt(T ... t)
{
  stringstream ss;
  ((ss << t),  ...);
  return ss.str();
}


nodePtr_t humon::fromString(string const & humonString)
{
  return HuNode::fromString(humonString);
}


std::ostream & humon::operator <<(std::ostream & stream, HuNode const & rhs)
{
  rhs.print(stream, 0);
  return stream;
}


// ----------- HuNode class

nodePtr_t HuNode::fromString(string const & humonString)
{
  char const * str = humonString.c_str();
  size_t len = humonString.length();

  return fromParsing(str, len);
}


nodePtr_t HuNode::fromParsing(char const *& str, size_t & len)
{
  auto token = peekToken(str, len);
  
  if (token.first == TokenType::listBegin)
  {
    return make_unique<HuList>(str, len);
  }
  else if (token.first == TokenType::dictBegin)
  {
    return make_unique<HuDict>(str, len);
  }
  else if (token.first == TokenType::value)
  {
    return make_unique<HuValue>(str, len);
  }
  else
  {
    throw runtime_error(fmt("Invalid token on line ", line));
  }
}


HuNode::HuNode()
{
}


HuNode::~HuNode()
{
}


nodePtr_t HuNode::clone() const
{
  return clone_impl();
}


bool HuNode::operator %(std::string const & key) const
{
  return asDict().hasKey(key);
}


HuNode const & HuNode::operator /(std::string const & key) const
{
  return asDict().at(key);
}


HuNode const & HuNode::operator >>(bool & rhs) const
{
  rhs = asValue().getBool();
  return *this;
}


HuNode const & HuNode::operator >>(std::string & rhs) const
{
  rhs = asValue().getString();
  return *this;
}


HuNode::operator bool() const
{
  return asValue().getBool();
}


HuNode::operator short() const
{
  return asValue().getInt();
}


HuNode::operator unsigned short() const
{
  return asValue().getUint();
}


HuNode::operator int() const
{
  return asValue().getInt();
}


HuNode::operator unsigned int() const
{
  return asValue().getUint();
}


HuNode::operator long() const
{
  return asValue().getLong();
}


HuNode::operator unsigned long() const
{
  return asValue().getUlong();
}


HuNode::operator long long() const
{
  return asValue().getLonglong();
}


HuNode::operator unsigned long long() const
{
  return asValue().getUlonglong();
}


HuNode::operator float() const
{
  return asValue().getFloat();
}


HuNode::operator double() const
{
  return asValue().getDouble();
}


HuNode::operator long double() const
{
  return asValue().getLongdouble();
}


HuNode::operator std::string() const
{
  return asValue().getString();
}


string const & HuNode::keyAt(size_t idx) const
{
  return asDict().keyAt(idx);  
}


size_t HuNode::size() const
{
  if (isList())
    { return asList().size(); }
  else if (isDict())
    { return asDict().size(); }
  return 1;
}


string HuNode::getReport() const
{
  stringstream ss;
  print(ss);
  return ss.str();
}


// ----------- HuList class

HuList::HuList(char const *& str, size_t & len)
: HuNode()
{
  auto token = getToken(str, len);
  if (token.first != TokenType::listBegin)
  { throw runtime_error(fmt("Invalid token on line ", line)); }

  token = peekToken(str, len);
  while (token.first != TokenType::listEnd)
  {
    if (token.first == TokenType::end)
    { throw runtime_error(fmt("Unclosed list on line ", line)); }

    if (token.first != TokenType::listBegin &&
        token.first != TokenType::dictBegin &&
        token.first != TokenType::value)
    { throw runtime_error(fmt("Invalid token on line ", line)); }

    elems.emplace_back(HuNode::fromParsing(str, len));

    token = peekToken(str, len);
  }

  // consume the last ]
  getToken(str, len);
}


HuList::HuList()
{
}


HuList::~HuList()
{
}


void HuList::print(std::ostream & out, int depth, bool indentFirstLine) const
{
  auto indent = string(depth * 2, ' ');
  if (indentFirstLine)
    { out << indent; }
  out << '[' << endl;
  for (auto & elem : elems)
  {
    elem->print(out, depth + 1);
  }
  out << indent << "]" << endl;
}


bool HuList::operator ==(HuNode const & rhs) const
{
  if (rhs.isList())
  {
    return elems == static_cast<HuList const &>(rhs).elems;
  }
  else
  { return false; }
}


size_t HuList::indexOf(nodePtr_t node) const
{
  for (size_t i = 0; i < elems.size(); ++i)
  {
    if (elems[i] == node)
    {
      return i;
    }
  }

  throw invalid_argument("node");
}


void HuList::append(nodePtr_t node)
{
  elems.push_back(node->clone());
}


void HuList::addAt(size_t index, nodePtr_t node)
{
  elems.insert(elems.begin() + index, node->clone());
}


void HuList::removeAt(size_t index)
{
  elems.erase(elems.begin() + index);
}


nodePtr_t HuList::clone_impl() const
{
  auto newThis = make_unique<HuList>();
  newThis->elems.reserve(elems.size());
  for (auto & elem : elems)
  {
    newThis->elems.push_back(elem->clone());
  }

  return newThis;
}


// ----------- HuDict class

HuDict::HuDict(char const *& str, size_t & len)
{
  auto token = getToken(str, len);
  if (token.first != TokenType::dictBegin)
  { throw runtime_error(fmt("Invalid token on line ", line)); }

  token = peekToken(str, len);
  while (token.first != TokenType::dictEnd)
  {
    if (token.first != TokenType::value)
    { throw runtime_error(fmt("Invalid token on line ", line)); }
    auto key = token.second;
    getToken(str, len);

    token = peekToken(str, len);
    if (token.first != TokenType::keyValueDelim)
    { throw runtime_error(fmt("Invalid token on line ", line)); }
    getToken(str, len);

    token = peekToken(str, len);
    if (token.first != TokenType::listBegin &&
        token.first != TokenType::dictBegin &&
        token.first != TokenType::value)
    { throw runtime_error(fmt("Invalid token on line ", line)); }

    elems.emplace(key, HuNode::fromParsing(str, len));
    keys.push_back(key);

    token = peekToken(str, len);
  }

  // consume the last }
  getToken(str, len);
}


HuDict::HuDict()
{
}


HuDict::~HuDict()
{
}


void HuDict::print(std::ostream & out, int depth, bool indentFirstLine) const
{
  auto indent = string(depth * 2, ' ');
  if (indentFirstLine)
    { out << indent; }
  out << '{' << endl;
  for (auto & key : keys)
  {
    bool quote = any_of(key.begin(), key.end(), 
      [] (char ch) { return isspace(ch); });

    out << string((depth + 1) * 2, ' ');
    if (quote)
      { out << "\"" << key << "\": "; }
    else
      { out << key << ": "; }
    elems.at(key)->print(out, depth + 1, false);
  }
  out << indent << "}" << endl;
}


bool HuDict::operator ==(HuNode const & rhs) const
{
  if (rhs.isDict())
  {
    return elems == static_cast<HuDict const &>(rhs).elems;
  }
  else
  { return false; }
}


bool HuDict::hasKey(string const & key) const
{
  return elems.find(key) != elems.end();
}


void HuDict::add(string const & key, nodePtr_t node)
{
  elems.insert(make_pair(key, node->clone()));
  keys.push_back(key);
}


void HuDict::removeAt(string const & key)
{
  elems.erase(key);
  keys.erase(find(keys.begin(), keys.end(), key));
}


nodePtr_t HuDict::clone_impl() const
{
  auto newThis = make_unique<HuDict>();
  for (auto & key : keys)
  {
    newThis->elems.insert({key, elems.at(key)->clone()});
    newThis->keys.push_back(key);
  }

  return newThis;
}


// ----------- HuValue class

HuValue::HuValue(char const *& str, size_t & len)
{
  auto token = getToken(str, len);
  if (token.first != TokenType::value)
  { throw runtime_error(fmt("Invalid token on line ", line)); }

  value = token.second;
}


HuValue::HuValue()
{
}


HuValue::~HuValue()
{
}


void HuValue::print(std::ostream & out, int depth, bool indentFirstLine) const
{
  if (indentFirstLine)
  {
    auto indent = string(depth * 2, ' ');
    out << indent;
  }
  if (any_of(value.begin(), value.end(), 
    [] (char ch) { return isspace(ch); }))
  {
    out << '\"' << value << '\"' << endl;
  }
  else
  {
    out << value << endl;
  }
  
}


bool HuValue::operator ==(HuNode const & rhs) const
{
  if (rhs.isValue())
  {
    return value == static_cast<HuValue const &>(rhs).value;
  }
  else
  { return false; }
}


bool HuValue::tryGet(bool & rv) const
{
  if (value == "true")
  {
    rv = true;
    return true;
  }

  if (value == "false")
  {
    rv = false;
    return true;
  }

  rv = false;
  return false;
}


bool HuValue::tryGet(short & rv) const
{
  size_t matches = 0;
  rv = stoi(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(unsigned short & rv) const
{
  size_t matches = 0;
  rv = stoui(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(int & rv) const
{
  size_t matches = 0;
  rv = stoi(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(unsigned int & rv) const
{
  size_t matches = 0;
  rv = stoui(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(long & rv) const
{
  size_t matches = 0;
  rv = stol(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(unsigned long & rv) const
{
  size_t matches = 0;
  rv = stoul(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(long long & rv) const
{
  size_t matches = 0;
  rv = stoll(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(unsigned long long & rv) const
{
  size_t matches = 0;
  rv = stoull(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(float & rv) const
{
  size_t matches = 0;
  rv = stof(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(double & rv) const
{
  size_t matches = 0;
  rv = stod(value, & matches);
  return matches == value.size();
}


bool HuValue::tryGet(long double & rv) const
{
  size_t matches = 0;
  rv = stold(value, & matches);
  return matches == value.size();
}


bool HuValue::getBool() const
{
  if (value == "true")
  { return true; }

  if (value == "false")
  { return false; }

  throw runtime_error("Not a boolean value");
}


short HuValue::getShort() const
{
  short v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


unsigned short HuValue::getUshort() const
{
  unsigned short v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


int HuValue::getInt() const
{
  int v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


unsigned int HuValue::getUint() const
{
  unsigned int v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


long HuValue::getLong() const
{
  long v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


unsigned long HuValue::getUlong() const
{
  unsigned long v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


long long HuValue::getLonglong() const
{
  long long v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


unsigned long long HuValue::getUlonglong() const
{
  unsigned long long v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


float HuValue::getFloat() const
{
  float v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


double HuValue::getDouble() const
{
  double v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


long double HuValue::getLongdouble() const
{
  long double v = 0;
  if (tryGet(v))
    { return v; }

  throw runtime_error("Not a boolean value");
}


string const & HuValue::getString() const
{
  return value;
}


/*
void HuValue::setValue(bool val)
{
  if (val)
  {
    value = "true";
  }
  else
  {
    value = "false";
  }
}


void HuValue::setValue(long val)
{
  value = to_string(val);
}


void HuValue::setValue(float val)
{
  value = to_string(val);
}


void HuValue::setValue(std::string const & val)
{
  value = val;
}
*/
template <>
void HuValue::setValue(bool val)
{
  if (val)
  {
    value = "true";
  }
  else
  {
    value = "false";
  }
}


template <>
void HuValue::setValue(std::string const & val)
{
  value = val;
}


nodePtr_t HuValue::clone_impl() const
{
  auto newThis = make_unique<HuValue>();
  newThis->value = value;
  return newThis;
}
