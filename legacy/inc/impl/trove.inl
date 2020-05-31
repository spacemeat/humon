#pragma once

// This #include won't resolve because this file is actually inclued from humon2.h, but it's nice for the editor.
#include <sstream>
#include "humon2cpp.h"
#include <fmt/core.h>
#include "ansiTerm.h"


namespace humon
{
  constexpr auto nullName = "";

  template <typename idx_t>
  std::string makeMinimalString(std::string_view humonString, std::vector<token_t<idx_t>> const & tokens, bool includeComments)
  {
    token_t<idx_t> nullToken { 
      TokenKind::eof, nullName, 
      { nullName, 0 }, 
      0, 
      0
    };

    std::stringstream ss;
    token_t<idx_t> const * pt = tokens.data();
    token_t<idx_t> const * prev = & nullToken;
    token_t<idx_t> const * cur = prev;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
      if (includeComments == false && (pt + i)->kind == TokenKind::comment)
        { continue; }

      prev = cur;
      cur = pt + i;
      if (cur->kind == TokenKind::eof)
        { break; }

      if (cur->kind == TokenKind::word)
      {
        if (i > 0 && 
            prev->kind == TokenKind::word)
          { ss << " "; }
      }

      if (cur->kind != TokenKind::comment || includeComments)
      {
        ss << cur->value.on(humonString);

        if (cur->kind == TokenKind::comment &&
            (cur + 1)->kind != TokenKind::eof)
          { ss << "\n"; }
      }
    }

    return ss.str();
  }


  template <typename idx_t>
  std::string makePrettyString(std::string_view humonString, std::vector<token_t<idx_t>> const & tokens, bool includeComments, idx_t tabSize)
  {
    token_t<idx_t> nullToken { 
      TokenKind::eof, nullName, 
      { nullName, 0 }, 
      0, 
      0
    };
    
    std::stringstream ss;
    token_t<idx_t> const * pt = tokens.data();
    int depth = 0;
    bool annotatingDict = false;
    bool annotatingOne = false;
    token_t<idx_t> const * prev = & nullToken;
    token_t<idx_t> const * prevNonComment = & nullToken;
    token_t<idx_t> const * cur = prev;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
      if (includeComments == false && (pt + i)->kind == TokenKind::comment)
        { continue; }

      prev = cur;
      cur = pt + i;
      if (prev->kind != TokenKind::comment)
        { prevNonComment = prev; }
      
      std::string_view val = cur->value.on(humonString);

      switch(cur->kind)
      {
      case TokenKind::eof:
        i = tokens.size();
        break;
      
      case TokenKind::startDict:
        switch(prev->kind)
        {
        case TokenKind::eof:
        case TokenKind::startDict:
        case TokenKind::endDict:
        case TokenKind::startList:
        case TokenKind::endList:
        case TokenKind::keyValueSep:
          ss << "{";
          break;
        case TokenKind::comment:
          if (includeComments)
            { ss << std::string(depth * tabSize, ' '); }
          ss << "{";
          if (prevNonComment->kind == TokenKind::annotate)
            { annotatingDict = true; }
          break;
        case TokenKind::annotate:
          ss << "{";
          annotatingDict = true;
          break;
        default:
          ss << '\n' << std::string(depth * tabSize, ' ') << "{";
          break;
        }
        depth += 1;
        break;
      
      case TokenKind::endDict:
        depth -= 1;
        if (annotatingDict)
        {
          if (prev->kind == TokenKind::comment && includeComments)
            { ss << std::string(depth * tabSize, ' '); }

          ss << '}';
          annotatingDict = false;
          depth -= 1;
        }
        else
        {
          switch(prev->kind)
          {
          case TokenKind::comment:
            if (includeComments == false)
              { ss << '\n'; }
            ss << std::string(depth * tabSize, ' ') << '}';
            break;
          default:
            ss << '\n' << std::string(depth * tabSize, ' ') << '}';
            break;
          }
        }
        
        break;
      
      case TokenKind::startList:
        switch(prev->kind)
        {
        case TokenKind::eof:
        case TokenKind::startDict:
        case TokenKind::endDict:
        case TokenKind::startList:
        case TokenKind::endList:
        case TokenKind::keyValueSep:
          ss << "[";
          break;
        case TokenKind::comment:
          if (includeComments)
            { ss << std::string(depth * tabSize, ' '); }
          ss << "[";
          break;
        default:
          ss << '\n' << std::string(depth * tabSize, ' ') << "[";
        }
        depth += 1;
        break;

      case TokenKind::endList:
        depth -= 1;
        switch(prev->kind)
        {
        case TokenKind::comment:
          if (includeComments == false)
            { ss << '\n'; }
          ss << std::string(depth * tabSize, ' ') << ']';
          break;
        default:
          ss << '\n' << std::string(depth * tabSize, ' ') << ']';
          break;
        }
        break;

      case TokenKind::keyValueSep:
        if (prev->kind == TokenKind::comment && includeComments)
          { ss << std::string(depth * tabSize, ' '); }
        ss << ": ";
        break;

      case TokenKind::annotate:
        depth += 1;
        if (prev->kind == TokenKind::comment && includeComments)
          { ss << std::string(depth * tabSize, ' '); }
        else
          { ss << ' '; }
        ss << '@';
        break;

      case TokenKind::word:
        if (annotatingDict)
        {
          if (prev->kind == TokenKind::comment && includeComments)
            { ss << std::string(depth * tabSize, ' '); }
          if (prev->kind == TokenKind::word)
            { ss << ' '; }
          ss << val;
        }
        else if (annotatingOne)
        {
          if (prev->kind == TokenKind::comment && includeComments)
            { ss << std::string(depth * tabSize, ' '); }
          ss << val;
          if (prevNonComment->kind == TokenKind::keyValueSep)
            { annotatingOne = false; depth -= 1; }
        }
        else
        {
          switch(prev->kind)
          {
          case TokenKind::eof:
          case TokenKind::keyValueSep:
            ss << val;
            break;
          case TokenKind::comment:
            if (includeComments == false)
            {
              if (prevNonComment->kind == TokenKind::keyValueSep)
                { ss << val; }
              else
                { ss << '\n' << std::string(depth * tabSize, ' ') << val; }
            }
            else
            {
              ss << std::string(depth * tabSize, ' ') << val;
              if (prevNonComment->kind == TokenKind::annotate)
                { annotatingOne = true; }
            }
            break;
          case TokenKind::word:
          case TokenKind::startDict:
          case TokenKind::endDict:
          case TokenKind::startList:
          case TokenKind::endList:
            ss << '\n' << std::string(depth * tabSize, ' ') << val;
            break;
          case TokenKind::annotate:
            annotatingOne = true;
            ss << val;
            break;
          default:  
            ss << val;
            break;
          }
        }
        break;

      case TokenKind::comment:
        if (includeComments)
        {
          if (prev->kind != TokenKind::eof && prev->kind != TokenKind::keyValueSep)
            { ss << ' '; }
          ss << val;
          if ((cur + 1)->kind != TokenKind::eof)
            { ss << '\n'; }
        }
        break;
      }
    }

    return ss.str();
  }


  template <typename T>
  std::string Trove<T>::to_string(OutputFormat format, bool includeComments) const
  {
    switch(format)
    {
    case OutputFormat::preserved:
      return std::string(data);
    case OutputFormat::minimal:
      return makeMinimalString(data, tokens, includeComments);
    case OutputFormat::pretty:
      return makePrettyString(data, tokens, includeComments, outputTabSize);
    default:
      return "";
    }
  }


  template <typename T>
  void Trove<T>::reset()
  {
    data.clear();
    tokens.clear();
    nodes.clear();
  }


  template <typename T>
  void Trove<T>::setRoot(std::string_view rhs)
  {
    reset();
    data = rhs;
    tokenize();
    // We don't parse here. That's done on demand, because there are operations which don't require it (like merging).
  }


  template <typename T>
  void Trove<T>::addError(std::string_view errorMsg, 
    token_t<idx_t> const * token)
  {
    errors.push_back({ std::string(errorMsg), token - tokens.data() });

    auto & tok = tokens.back();

    std::cout << fmt::format("{}Error: {}{}{} token: {}{}{} line {}{}{} col {}{}{}\n",
      ansi::darkRed,
      ansi::lightYellow, errorMsg, ansi::darkRed,
      ansi::lightYellow, tok.value.on(data), ansi::darkRed,
      ansi::lightYellow, tok.line, ansi::darkRed,
      ansi::lightYellow, tok.col, ansi::off);
  }


  template <typename T>
  void Trove<T>::removeSubtree(idx_t nodeIdx)
  {
    auto & pn = getNode(nodeIdx);

    // Kill all child subtrees. All of their tokens are embedded in this node's full token set, so they'll all be removed automatically.
    idx_t firstTokenIdx = pn.getFirstTokenIdx();
    idx_t postTokenIdx = firstTokenIdx + pn.getNumTokens();
    idx_t numOldTokens = postTokenIdx - firstTokenIdx;
    
    idx_t dataPartASize = tokens[firstTokenIdx].value.data() - data.data();
    idx_t dataPartBOffset = tokens[postTokenIdx].value.data() - data.data();

    copy(data.begin() + dataPartBOffset, data.end(), data.begin() + dataPartASize);

    copy(tokens.begin() + postTokenIdx, tokens.end(), tokens.begin() + firstTokenIdx);
    tokens.resize(tokens.size() - numOldTokens);

    // nodes - for now, just clear it and force a full reparse. Later I'd like to explore merging the node lists.
    nodes.clear();
  }


  template <typename T>
  void Trove<T>::replaceSubtree(std::string_view humonString, idx_t nodeIdx)
  {
    auto ho = Trove<T>::fromString(humonString);
    auto & pn = getNode(nodeIdx);

    // Kill all child subtrees. All of their tokens are embedded in this node's full token set, so they'll all be removed automatically.
    idx_t firstTokenIdx = pn.getFirstTokenIdx();
    idx_t postTokenIdx = firstTokenIdx + pn.getNumTokens();
    
    idx_t sizeOfOldData = tokens[postTokenIdx].value.data() - tokens[firstTokenIdx].value.data();
    idx_t sizeOfNewData = ho.getData().size();
    idx_t numNewTokens = ho.getTokens().size();
    
    idx_t dataPartASize = tokens[firstTokenIdx].value.data() - data.data();
    idx_t dataPartBOffset = tokens[postTokenIdx].value.data() - data.data();
    idx_t dataPartBSize = data.size() - dataPartBOffset;

    // make new backing std::string
    std::string newData;
    newData.reserve(data.size() + sizeOfNewData - sizeOfOldData);
    // copy std::string part a
    copy(data.begin(), data.begin() + dataPartASize, back_inserter(newData));
    // copy new std::string
    auto & hoData = ho.getData();
    copy(hoData.begin(), hoData.end(), back_inserter(newData));
    // copy std::string part b
    copy(data.begin() + dataPartBOffset, data.end(), back_inserter(newData));

    // instead of retokenizing, just adjust and retarget what we have.

    // make new token buffer
    std::vector<std::string_view> newTokens;
    newTokens.reserve(tokens.size() + numNewTokens);
    // copy tokens part a
    copy(tokens.begin(), tokens.begin() + firstTokenIdx, back_inserter(newTokens));

    // copy each new token, adjusting by a.size
    auto & hoToks = ho.getTokens();
    for (auto & token : hoToks)
    {
      newTokens.emplace_back(
        token.kind, 
        { token.value.loc + dataPartASize, token.value.size() }, 
        token.line, 
        token.col);
    }

    // copy tokens part b, adjusting by sizeOfNewData - sizeOfOldData
    for (auto it = tokens.begin() + postTokenIdx; it != tokens.end(); ++it)
    {
      auto & token = * it;
      newTokens.emplace_back(
        token.kind, 
        { token.value.loc + sizeOfNewData - sizeOfOldData, it->value.size() }, 
        token.line, 
        token.col);
    }

    // nodes - for now, just clear it and force a full reparse. Later I'd like to explore merging the node lists.
    nodes.clear();
  }


  template <typename T>
  void Trove<T>::insertSubtrees(std::initializer_list<std::string_view> humonStrings, idx_t parentNodeIdx, idx_t insertTokenIdx)
  {
    std::vector<Trove<T>> v;
    v.reserve(humonStrings.size());
    for (auto & st : humonStrings)
      { v.emplace_back(st); }
    
    T sizeOfNewData = 0;
    T numNewTokens = 0;
    for(auto & ho : v)
    { 
      sizeOfNewData += ho.getData().size();
      numNewTokens += ho.getTokens().size();
    };

    T dataPartASize = tokens[insertTokenIdx].value.data() - data.data();
    //T dataPartBSize = data.size() - dataPartASize;

    // make new backing std::string
    std::string newData;
    newData.reserve(data.size() + sizeOfNewData);
    // copy std::string part a
    copy(data.begin(), data.begin() + dataPartASize, back_inserter(newData));
    // copy each new std::string
    for(auto & ho : v)
    {
      auto & data = ho.getData();
      copy(data.begin(), data.end(), back_inserter(newData));
    }
    // copy std::string part b
    copy(data.begin() + dataPartASize, data.end(), back_inserter(newData));

    // instead of retokenizing, just adjust and retarget what we have.

    // make new token buffer
    std::vector<std::string_view> newTokens;
    newTokens.reserve(tokens.size() + numNewTokens);
    // copy tokens part a
    copy(tokens.begin(), tokens.begin() + insertTokenIdx, back_inserter(newTokens));

    // copy each new token, adjusting by a.size
    for (auto & ho : v)
    {
      auto & hoToks = ho.getTokens();
      for (auto & token : hoToks)
      {
        newTokens.emplace_back(
          token.kind, 
          { token.value.loc + dataPartASize, token.value.size() }, 
          token.line, 
          token.col);
      }
    }

    // copy tokens part b, adjusting by sizeOfNewData
    for (auto it = tokens.begin() + insertTokenIdx; it != tokens.end(); ++it)
    {
      auto & token = * it;
      newTokens.emplace_back(
        token.kind, 
        { token.value.loc + sizeOfNewData, it->value.size() }, 
        token.line, 
        token.col);
    }

    // nodes - for now, just clear it and force a full reparse. Later I'd like to explore merging the node lists.
    nodes.clear();
  }
}
