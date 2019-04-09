#include "humon.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace humon;

using ss = stringstream;

/*
bool operator ==(node_t const & lhs, node_t const & rhs)
{
  if (lhs->isList() && rhs->isList())
  {
    // Normally these static casts would be in violation, but we know 
    // what we're doing here.
    auto lhsl = static_cast<HumonList const *>(lhs.get());
    auto rhsl = static_cast<HumonList const *>(rhs.get());
    auto numElements = lhsl->getNumElements();
    if (numElements == rhsl->getNumElements())
    {
      for (int i = 0; i < numElements; ++i)
      {
        if ((*lhsl)[i] != (*rhsl)[i])
        { return false; }
      }

      return true;
    }

    return false;
  }

  if (lhs->isDict() == rhs->isDict())
  {
    auto lhsd = static_cast<HumonDict const *>(lhs.get());
    auto rhsd = static_cast<HumonDict const *>(rhs.get());
    auto numElements = lhsd->getNumElements();
    if (numElements == rhsd->getNumElements())
    {
      for (int i = 0; i < numElements; ++i)
      {
        auto key = lhsd->keyAt(i);
        if (rhsd->hasKey(key) == false)
        { return false; }

        if ((*lhsd)[key] != (*rhsd)[key])
        { return false; }
      }

      return true;
    }

    return false;
  }

  if (lhs->isValue() == rhs->isValue())
  {
    auto lhsv = static_cast<HumonValue const *>(lhs.get());
    auto rhsv = static_cast<HumonValue const *>(rhs.get());
    return lhsv->getString() == rhsv->getString();
  }
}
*/

// ----------- parsing love

int line = 1;

// Nom whitespace.
void eatWs(char const *& str, size_t & len)
{
  bool slashing = false;
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
      slashing = false;
    }

    else if (str[0] == '/')
    { 
      if (slashing)
      { slashing = false; commenting = true; }
      else
      { slashing = true; }
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
      while (len > 0)
      {
        if (isspace(str[0]) == false && 
            str[0] != '[' &&
            str[0] != ']' &&
            str[0] != '{' &&
            str[0] != '}' &&
            str[0] != ':')
        {
          token.push_back(str[0]);
          str += 1;
          len -= 1;
        }
        else
        { break; }
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


node_t humon::fromString(string const & humonString)
{
  return Humon::fromString(humonString);
}


std::ostream & humon::operator << (std::ostream & stream, Humon const & rhs)
{
  rhs.print(stream, 0);
  return stream;
}


// ----------- Humon class

node_t Humon::fromString(string const & humonString)
{
  char const * str = humonString.c_str();
  size_t len = humonString.length();

  return fromParsing(str, len);
}


node_t Humon::fromParsing(char const *& str, size_t & len)
{
  auto token = peekToken(str, len);
  
  if (token.first == TokenType::listBegin)
  {
    return make_unique<HumonList>(str, len);
  }
  else if (token.first == TokenType::dictBegin)
  {
    return make_unique<HumonDict>(str, len);
  }
  else if (token.first == TokenType::value)
  {
    return make_unique<HumonValue>(str, len);
  }
  else
  {
    throw runtime_error(fmt("Invalid token on line ", line));
  }
}


Humon::Humon()
{
}


Humon::~Humon()
{
}


node_t Humon::clone() const
{
  return clone_impl();
}


// ----------- HumonList class

HumonList::HumonList(char const *& str, size_t & len)
: Humon()
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

    elems.push_back(Humon::fromParsing(str, len));

    token = peekToken(str, len);
  }

  // consume the last ]
  getToken(str, len);
}


HumonList::HumonList()
{
}


HumonList::~HumonList()
{
}


void HumonList::print(std::ostream & out, int depth, bool indentFirstLine) const
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


bool HumonList::operator == (Humon const & rhs) const
{
  if (rhs.isList())
  {
    return elems == static_cast<HumonList const &>(rhs).elems;
  }
  else
  { return false; }
}


node_t const & HumonList::operator [](size_t idx) const
{
  return elems[idx];
}


size_t HumonList::indexOf(node_t node) const
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


void HumonList::append(node_t node)
{
  elems.push_back(node->clone());
}


void HumonList::addAt(size_t index, node_t node)
{
  elems.insert(elems.begin() + index, node->clone());
}


void HumonList::removeAt(size_t index)
{
  elems.erase(elems.begin() + index);
}


node_t HumonList::clone_impl() const
{
  auto newThis = make_unique<HumonList>();
  newThis->elems.reserve(elems.size());
  for (auto & elem : elems)
  {
    newThis->elems.push_back(elem->clone());
  }

  return newThis;
}


// ----------- HumonDict class

HumonDict::HumonDict(char const *& str, size_t & len)
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

    elems[key] = Humon::fromParsing(str, len);

    token = peekToken(str, len);
  }

  // consume the last }
  getToken(str, len);
}


HumonDict::HumonDict()
{
}


HumonDict::~HumonDict()
{
}


void HumonDict::print(std::ostream & out, int depth, bool indentFirstLine) const
{
  auto indent = string(depth * 2, ' ');
  if (indentFirstLine)
    { out << indent; }
  out << '{' << endl;
  for (auto & elem : elems)
  {
    out << string((depth + 1) * 2, ' ') << elem.first << ": ";
    elem.second->print(out, depth + 1, false);
  }
  out << indent << "}" << endl;
}


bool HumonDict::operator == (Humon const & rhs) const
{
  if (rhs.isDict())
  {
    return elems == static_cast<HumonDict const &>(rhs).elems;
  }
  else
  { return false; }
}


bool HumonDict::hasKey(string const & key) const
{
  return elems.find(key) != elems.end();
}


node_t const & HumonDict::operator [](string const & key) const
{
  return elems.at(key);
}


void HumonDict::add(string const & key, node_t node)
{
  elems.insert(make_pair(key, node->clone()));
}


void HumonDict::removeAt(string const & key)
{
  elems.erase(key);
}


node_t HumonDict::clone_impl() const
{
  auto newThis = make_unique<HumonDict>();
  for (auto & elem : elems)
  {
    newThis->elems.insert({elem.first, elem.second->clone()});
  }

  return newThis;
}


// ----------- HumonValue class

HumonValue::HumonValue(char const *& str, size_t & len)
{
  auto token = getToken(str, len);
  if (token.first != TokenType::value)
  { throw runtime_error(fmt("Invalid token on line ", line)); }

  value = token.second;
}


HumonValue::HumonValue()
{
}


HumonValue::~HumonValue()
{
}


void HumonValue::print(std::ostream & out, int depth, bool indentFirstLine) const
{
  if (indentFirstLine)
  {
    auto indent = string(depth * 2, ' ');
    out << indent;
  }
  out << value << endl;
}


bool HumonValue::operator == (Humon const & rhs) const
{
  if (rhs.isValue())
  {
    return value == static_cast<HumonValue const &>(rhs).value;
  }
  else
  { return false; }
}


bool HumonValue::tryGet(bool & rv) const
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


bool HumonValue::tryGet(long & rv) const
{
  size_t matches = 0;
  rv = stol(value, & matches);
  return matches == value.size();
}


bool HumonValue::tryGet(float & rv) const
{
  size_t matches = 0;
  rv = stof(value, & matches);
  return matches == value.size();
}


bool HumonValue::getBool() const
{
  if (value == "true")
  { return true; }

  if (value == "false")
  { return false; }

  throw runtime_error("Not a boolean value");
}


long HumonValue::getLong() const
{
  size_t matches = 0;
  auto val = stol(value, & matches);
  if (matches == value.size())
  { throw runtime_error("Not a long value"); }
  return val;
}


float HumonValue::getFloat() const
{
  size_t matches = 0;
  auto val = stof(value, & matches);
  if (matches == value.size())
  { throw runtime_error("Not a float value"); }
  return val;
}


string const & HumonValue::getString() const
{
  return value;
}


node_t HumonValue::clone_impl() const
{
  auto newThis = make_unique<HumonValue>();
  newThis->value = value;
  return newThis;
}
