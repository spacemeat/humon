#include "humon2.h"
#include <numeric>
#include <sstream>


using namespace std;
using namespace humon;


HuNode::HuNode(HumonObj * obj, size_t nodeIdx) noexcept
: obj(obj), nodeIdx(nodeIdx)
{
}


// mutators
HuNode & HuNode::operator = (std::string_view humonString)
{
  auto ho = HumonObj::fromString(humonString);
  auto pn = obj->getNode(parentNodeIdx);
  auto thisIdx = collectionNodeIdx;

  // kill all child subtrees
  for (auto chIdx : *this)
    { ho.removeSubtree(chIdx); }
  
  // TODO: Finish this
}


HuNode & HuNode::operator += (std::initializer_list<std::string_view> humonStrings)
{
  size_t newTokenIdx = appendTokenIdx;
  
  // TODO: Finish this
}


HuNode & HuNode::insertAt(size_t idx, std::initializer_list<std::string_view> humonStrings)
{
  size_t newTokenIdx = getChildNode(idx).firstTokenIdx;
  
  // TODO: Finish this
}


HuNode & HuNode::operator -= (std::initializer_list<size_t> idxs)
{
  
  // TODO: Finish this
}


HuNode & HuNode::operator -= (std::initializer_list<std::string_view> keys)
{
  
  // TODO: Finish this
}


void HuNode::setError(std::string_view msg)
{
  type = NodeType::error;
  errorMsg = msg;
}


void HuNode::setType(NodeType type)
{
  this->type = type;
}


void HuNode::setParentNodeIdx(size_t parentNodeIdx)
{
  this->parentNodeIdx = parentNodeIdx;
}


void HuNode::addChildNodeIdx(size_t newChildIdx)
{
  childNodeIdxs.push_back(newChildIdx);
}


void HuNode::setAnnotation(std::string_view key, size_t valueIdx)
{
  annotationTokenIdxs.emplace(key, valueIdx);
}


void HuNode::adjustTokenIdxs(long long offset) noexcept
{
  firstTokenIdx += offset;
  appendTokenIdx += offset;
  for (auto & kvp : annotationTokenIdxs)
    { kvp.second += offset; }
}


void HuNode::ensureKeysMapped()
{
  for (auto ch : *this)
  {
    auto key = obj->getNode(ch).getKey();
    childDictIdxs.emplace(key, obj->getNode(ch));
  }
}


string makeMinimalString(vector<token_t> const & tokens, bool includeComments)
{
  stringstream ss;
  token_t const * pt = tokens.data();
  token_t nullToken { TokenType::eof, ""sv, 0, 0 };
  token_t const * prev = & nullToken;
  token_t const * prevNonComment = & nullToken;
  token_t const * cur = prev;
  for (size_t i = 0; i < tokens.size(); ++i)
  {
    if (includeComments == false && (pt + i)->type == TokenType::comment)
      { continue; }

    prev = cur;
    cur = pt + i;
    if (prev->type != TokenType::comment)
      { prevNonComment = prev; }

    if (cur->type == TokenType::eof)
      { break; }

    if (cur->type == TokenType::word)
    {
      if (i > 0 && 
          prev->type == TokenType::word)
        { ss << " "; }
    }

    if (cur->type != TokenType::comment || includeComments)
    {
      ss << cur->value;

      if (cur->type == TokenType::comment &&
          (cur + 1)->type != TokenType::eof)
        { ss << "\n"; }
    }
  }

  return ss.str();
}


string makePrettyString(vector<token_t> const & tokens, bool includeComments, size_t tabSize)
{
  stringstream ss;
  token_t const * pt = tokens.data();
  int depth = 0;
  bool annotatingDict = false;
  bool annotatingOne = false;
  token_t nullToken { TokenType::eof, ""sv, 0, 0 };
  token_t const * prev = & nullToken;
  token_t const * prevNonComment = & nullToken;
  token_t const * cur = prev;
  for (size_t i = 0; i < tokens.size(); ++i)
  {
    if (includeComments == false && (pt + i)->type == TokenType::comment)
      { continue; }

    prev = cur;
    cur = pt + i;
    if (prev->type != TokenType::comment)
      { prevNonComment = prev; }

    switch(cur->type)
    {
    case TokenType::eof:
      i = tokens.size();
      break;
    
    case TokenType::startDict:
      switch(prev->type)
      {
      case TokenType::eof:
      case TokenType::startDict:
      case TokenType::endDict:
      case TokenType::startList:
      case TokenType::endList:
      case TokenType::keyValueSep:
        ss << "{";
        break;
      case TokenType::comment:
        if (includeComments)
          { ss << string(depth * tabSize, ' '); }
        ss << "{";
        if (prevNonComment->type == TokenType::annotate)
          { annotatingDict = true; }
        break;
      case TokenType::annotate:
        ss << "{";
        annotatingDict = true;
        break;
      default:
        ss << '\n' << string(depth * tabSize, ' ') << "{";
        break;
      }
      depth += 1;
      break;
    
    case TokenType::endDict:
      depth -= 1;
      if (annotatingDict)
      {
        if (prev->type == TokenType::comment && includeComments)
          { ss << string(depth * tabSize, ' '); }

        ss << '}';
        annotatingDict = false;
        depth -= 1;
      }
      else
      {
        switch(prev->type)
        {
        case TokenType::comment:
          if (includeComments == false)
            { ss << '\n'; }
          ss << string(depth * tabSize, ' ') << '}';
          break;
        default:
          ss << '\n' << string(depth * tabSize, ' ') << '}';
          break;
        }
      }
      
      break;
    
    case TokenType::startList:
      switch(prev->type)
      {
      case TokenType::eof:
      case TokenType::startDict:
      case TokenType::endDict:
      case TokenType::startList:
      case TokenType::endList:
      case TokenType::keyValueSep:
        ss << "[";
        break;
      case TokenType::comment:
        if (includeComments)
          { ss << string(depth * tabSize, ' '); }
        ss << "[";
        break;
      default:
        ss << '\n' << string(depth * tabSize, ' ') << "[";
      }
      depth += 1;
      break;

    case TokenType::endList:
      depth -= 1;
      switch(prev->type)
      {
      case TokenType::comment:
        if (includeComments == false)
          { ss << '\n'; }
        ss << string(depth * tabSize, ' ') << ']';
        break;
      default:
        ss << '\n' << string(depth * tabSize, ' ') << ']';
        break;
      }
      break;

    case TokenType::keyValueSep:
      if (prev->type == TokenType::comment && includeComments)
        { ss << string(depth * tabSize, ' '); }
      ss << ": ";
      break;

    case TokenType::annotate:
      depth += 1;
      if (prev->type == TokenType::comment && includeComments)
        { ss << string(depth * tabSize, ' '); }
      else
        { ss << ' '; }
      ss << '@';
      break;

    case TokenType::word:
      if (annotatingDict)
      {
        if (prev->type == TokenType::comment && includeComments)
          { ss << string(depth * tabSize, ' '); }
        if (prev->type == TokenType::word)
          { ss << ' '; }
        ss << cur->value;
      }
      else if (annotatingOne)
      {
        if (prev->type == TokenType::comment && includeComments)
          { ss << string(depth * tabSize, ' '); }
        ss << cur->value;
        if (prevNonComment->type == TokenType::keyValueSep)
          { annotatingOne = false; depth -= 1; }
      }
      else
      {
        switch(prev->type)
        {
        case TokenType::eof:
        case TokenType::keyValueSep:
          ss << cur->value;
          break;
        case TokenType::comment:
          if (includeComments == false)
          {
            if (prevNonComment->type == TokenType::keyValueSep)
              { ss << cur->value; }
            else
              { ss << '\n' << string(depth * tabSize, ' ') << cur->value; }
          }
          else
          {
            ss << string(depth * tabSize, ' ') << cur->value;
            if (prevNonComment->type == TokenType::annotate)
              { annotatingOne = true; }
          }
          break;
        case TokenType::word:
        case TokenType::startDict:
        case TokenType::endDict:
        case TokenType::startList:
        case TokenType::endList:
          ss << '\n' << string(depth * tabSize, ' ') << cur->value;
          break;
        case TokenType::annotate:
          annotatingOne = true;
          ss << cur->value;
          break;
        default:  
          ss << cur->value;
          break;
        }
      }
      break;

    case TokenType::comment:
      if (includeComments)
      {
        if (prev->type != TokenType::eof && prev->type != TokenType::keyValueSep)
          { ss << ' '; }
        ss << cur->value;
        if ((cur + 1)->type != TokenType::eof)
          { ss << '\n'; }
      }
      break;
    }
  }

  return ss.str();
}


string HumonObj::to_string(OutputFormat format, bool includeComments) const
{
  switch(format)
  {
  case OutputFormat::preserved:
    return string(data);
  case OutputFormat::minimal:
    return makeMinimalString(tokens, includeComments);
  case OutputFormat::pretty:
    return makePrettyString(tokens, includeComments, outputTabSize);
  default:
    return "";
  }
}


void HumonObj::reset()
{
  data.clear();
  tokens.clear();
  nodes.clear();
}


void HumonObj::setRoot(std::string_view rhs)
{
  reset();
  data = rhs;
  tokenize();
  // We don't parse here. That's done on demand, because there are operations which don't require it (like merging).
}


HuNode & HumonObj::addErrorNode(std::string_view errorMsg, 
  token_t const * token, size_t parentNodeIdx)
{
  nodes.emplace_back(this, nodes.size());
  HuNode & node = nodes.back();
  node.setParentNodeIdx(parentNodeIdx);
  if (parentNodeIdx != numeric_limits<size_t>::max())
  {
    auto & parentNode = getNode(parentNodeIdx);
    parentNode.addChildNodeIdx(node.getNodeIdx());
  }
  node.setError(errorMsg);
  error = true;

  return node;
}


void HumonObj::removeSubtree(size_t nodeIdx)
{
}


void HumonObj::insertSubtrees(std::initializer_list<std::string_view> humonStrings, size_t parentNodeIdx, size_t insertTokenIdx)
{
  vector<HumonObj> v;
  v.reserve(humonStrings.size());
  for (auto & st : humonStrings)
    { v.emplace_back(st); }
  
  size_t sizeOfNewData = 0;
  size_t numNewTokens = 0;
  for(auto & ho : v)
  { 
    sizeOfNewData += ho.getData().size();
    numNewTokens += ho.getTokens().size();
  };

  size_t dataPartASize = tokens[insertTokenIdx].value.data() - data.data();
  //size_t dataPartBSize = data.size() - dataPartASize;

  // make new backing string
  string newData;
  newData.reserve(data.size() + sizeOfNewData);
  // copy string part a
  copy(data.begin(), data.begin() + dataPartASize, newData.begin());
  // copy each new string
  for(auto & ho : v)
  {
    auto & data = ho.getData();
    copy(data.begin(), data.end(), newData.end());
  }
  // copy string part b
  copy(data.begin() + dataPartASize, data.end(), newData.end());

  // instead of retokenizing, just adjust and retarget what we have.

  // make new token buffer
  vector<string_view> newTokens;
  newTokens.reserve(tokens.size() + numNewTokens);
  // copy tokens part a
  for (auto it = tokens.begin(); it != tokens.begin() + insertTokenIdx; ++it)
  {
    auto offset = it->value.data() - data.data();
    newTokens.emplace_back(newData.data() + offset, it->value.size());
  }
  // copy each new token, adjusting by a.size
  for (auto & ho : v)
  {
    auto & toks = ho.getTokens();
    for (auto & token : toks)
    {
      auto offset = token.value.data() - ho.data.data();
      newTokens.emplace_back(newData.data() + offset + dataPartASize, token.value.size());
    }
  }
  // copy tokens part b, adjusting by all a.size
  for (auto it = tokens.begin() + insertTokenIdx; it != tokens.end(); ++it)
  {
    auto offset = it->value.data() - data.data();
    newTokens.emplace_back(newData.data() + offset + sizeOfNewData, it->value.size());
  }
}
