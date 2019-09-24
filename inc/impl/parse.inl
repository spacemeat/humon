#pragma once

// This #include won't resolve because this file is actually inclued from humon2.h, but it's nice for the editor.
#include "humon2.h"
#include "fmt/core.h"
#include "ansiTerm.h"

namespace humon
{
  template <typename T>
  inline void eatComments(token_t<typename Trove<T>::idx_t> const * cur)
  {
    while (cur->kind == TokenKind::comment)
      { cur += 1; }
  }


  template <typename T>
  Node<T> & Trove<T>::parseRec(
    token_t<typename Trove<T>::idx_t> const *& cur)
  {
    eatComments<T>(cur);

    auto newNodeId = nodes.size();
    auto * newNode = & nodes.emplace_back(this, newNodeId);
    newNode->_setFirstTokenIdx(cur - tokens.data());

    std::cout << fmt::format("parseRec: {}\n", cur->value.on(data));

    auto keyIdx = nullIdx;
    bool scanningColl = false;
    switch(cur->kind)
    {
    case TokenKind::eof:
      addError("Unexpected end of data.", cur);
      break;
    
    case TokenKind::startList:
      cur += 1;
      newNode->_setKind(NodeKind::list);
      scanningColl = true;
      while (scanningColl)
      {
        eatComments<T>(cur);
        switch (cur->kind)
        {
        case TokenKind::eof:
          scanningColl = false;
          break;

        case TokenKind::endList:
          scanningColl = false;
          cur += 1;
          break;

        default:
          auto & chNode = parseRec(cur);
          newNode = nodes.data() + newNodeId;      // parseRec invalidate newNode reference, so refresh it
          chNode._setParentNodeIdx(newNode->getNodeIdx());
          newNode->_addChildNodeIdx(chNode.getNodeIdx());
          chNode._setCollectionIdx(newNode->getNumChildren());
          newNode->_setCollectionAppendTokenIdx(cur - tokens.data());
          chNode.print();
          break;
        }
      }
      break;

    case TokenKind::startDict:
      cur += 1;
      newNode->_setCollectionAppendTokenIdx(cur - tokens.data());
      newNode->_setKind(NodeKind::dict);
      scanningColl = true;
      while (scanningColl)
      {
        eatComments<T>(cur);

        switch (cur->kind)
        {
        case TokenKind::eof:
          scanningColl = false;
          break;

        case TokenKind::endDict:
          scanningColl = false;
          cur += 1;
          break;

        case TokenKind::word:
          // *cur should be a key. We defer building a map until it's needed.
          keyIdx = cur - tokens.data();
          cur += 1;
          eatComments<T>(cur);
          // ensure separator
          if (cur->kind != TokenKind::keyValueSep)
          {
            addError("Missing separator ':' for key-value pair in dict", cur);
          }
          else
          {
            cur += 1;
            auto & chNode = parseRec(cur);
            newNode = nodes.data() + newNodeId;   // parseRec may newNode reference, so refresh it
            chNode._setKeyIdx(keyIdx);
            chNode._setParentNodeIdx(newNode->getNodeIdx());
            newNode->_addChildNodeIdx(chNode.getNodeIdx());
            chNode._setCollectionIdx(newNode->getNumChildren());
            newNode->_setCollectionAppendTokenIdx(cur - tokens.data());
            chNode.print();
          }
          break;

        default:  
          addError("Entries in a dict must be key-value pairs. Only strings are valid keys.", cur);
          cur += 1;
          break;
        }
      }
      break;

    case TokenKind::word:
      // value node
      newNode->_setKind(NodeKind::value);
      cur += 1;
      break;
    
    default:
      addError("Invalid token", cur);
      cur += 1;
      break;
    }

    eatComments<T>(cur);
    newNode->_setAnnotationAppendTokenIdx(cur - tokens.data());

    // any element can have annotations
    while (cur->kind == TokenKind::annotate)
    {
      cur += 1;
      eatComments<T>(cur);
      std::string_view key;
      bool scanningAnno = true;
      switch (cur->kind)
      {
      case TokenKind::eof:
        addError("Unexpected end of data.", cur);
        break;

      case TokenKind::startDict:
        cur += 1;
        while (scanningAnno)
        {
          eatComments<T>(cur);
          if (cur->kind != TokenKind::word)
          {
            addError("Entries in a dict must be key-value pairs. Only strings are valid keys.", cur);
            scanningAnno = false;
            break;
          }
          key = cur->value.on(data);
          cur += 1;
          eatComments<T>(cur);
          if (cur->kind != TokenKind::keyValueSep)
          {
            addError("Missing separator ':' for key-value pair in dict", cur);
            scanningAnno = false;
            break;
          }
          cur += 1;
          eatComments<T>(cur);
          if (cur->kind != TokenKind::word)
          {
            addError("Entries in a dict must be key-value pairs. Only strings are valid values in an annotation.", cur);
            scanningAnno = false;
          }
          newNode->_setAnnotation(key, cur - tokens.data());
          cur += 1;

          eatComments<T>(cur);
          if (cur->kind == TokenKind::endDict)
          {
            newNode->_setAnnotationAppendTokenIdx(cur - tokens.data());
            cur += 1;
            scanningAnno = false;
            break;
          }
        }
        break;

      case TokenKind::word:
        key = cur->value.on(data);
        cur += 1;
        eatComments<T>(cur);
        if (cur->kind != TokenKind::keyValueSep)
        {
          addError("Annotations must be a single key-value pair or a '{}'-wrapped dict.", cur);
        }
        cur += 1;
        eatComments<T>(cur);
        if (cur->kind != TokenKind::word)
        {
          addError("Entries in a dict must be key-value pairs. Only strings are valid values in an annotation.", cur);
        }
        newNode->_setAnnotation(key, cur - tokens.data());
        cur += 1;
        newNode->_setAnnotationAppendTokenIdx(cur - tokens.data());
        break;
      
      default:
        addError("Invalid token.", cur);
        cur += 1;
        break;
      }
    }

    newNode->_setNumTokens((cur - tokens.data()) - newNode->getFirstTokenIdx());

    return * newNode;
  }


  template <typename T>
  void Trove<T>::parse()
  {
    for (idx_t i = 0; i < tokens.size(); ++i)
    {
      std::cout << fmt::format("{}{}    {}{}{}\n",
        ansi::darkYellow, i, ansi::lightYellow,
        tokens[i].value.on(data), ansi::off);
    }

    nodes.clear();
    token_t<idx_t> const * cur = tokens.data();
    auto & root = parseRec(cur);
    root.print();

    std::cout << "PARM\n\n";
    for (idx_t i = 0; i < nodes.size(); ++i)
    {
      nodes[i].print();
    }
  }
}
