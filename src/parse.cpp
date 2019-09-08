#include "humon2.h"


using namespace std;
using namespace humon;


inline void eatComments(token_t const * cur)
{
  while (cur->type == TokenType::comment)
    { cur += 1; }
}


HuNode & HumonObj::parseRec(token_t const * cur, size_t parentNodeIdx)
{
  nodes.emplace_back(this, nodes.size());
  HuNode & newNode = nodes.back();

  bool scanning = true;
  while (scanning)
  {
    bool scanningColl = false;
    switch(cur->type)
    {
    case TokenType::eof:
      addErrorNode("Unexpected end of data.", cur, parentNodeIdx);
      break;
    
    case TokenType::comment:
      cur += 1;
      newNode.setType(NodeType::comment);
      break;

    case TokenType::startList:
      cur += 1;
      newNode.setType(NodeType::list);
      scanningColl = true;
      while (scanningColl)
      {
        switch (cur->type)
        {
        case TokenType::eof:
          scanning = false;
          scanningColl = false;
          break;
        case TokenType::comment:
          cur += 1;
          break;
        case TokenType::endList:
          scanningColl = false;
          cur += 1;
          break;
        default:
          auto & chNode = parseRec(cur, newNode);
          newNode.addChildNodeIdx(chNode.getNodeIdx());
          break;
        }
      }
      break;

    case TokenType::startDict:
      cur += 1;
      scanningColl = true;
      newNode.setType(NodeType::dict);
      while (scanningColl)
      {
        eatComments(cur);

        switch (cur->type)
        {
        case TokenType::eof:
          scanning = false;
          scanningColl = false;
          break;
        case TokenType::endDict:
          scanningColl = false;
          cur += 1;
          break;
        case TokenType::word:
          // *cur should be a key. We defer building a map until it's needed.
          cur += 1;
          eatComments(cur);
          // ensure separator
          if (cur->type != TokenType::keyValueSep)
          {
            addErrorNode("Missing separator ':' for key-value pair in dict", cur, parentNodeIdx);
          }
          else
          {
            cur += 1;
            auto & chNode = parseRec(cur, newNode);
            chNode.setParentNodeIdx(newNode.getNodeIdx());
            newNode.addChildNodeIdx(chNode.getNodeIdx());
          }
          break;
        default:
            addErrorNode("Entries in a dict must be key-value pairs. Only strings are valid keys.", cur, parentNodeIdx);
          break;
        }
      }
      break;

    case TokenType::word:
      // value node
      newNode.setType(NodeType::value);
      break;
    
    default:
      addErrorNode("Invalid token", cur, parentNodeIdx);
    }

    eatComments(cur);

    // any element can have annotations
    while (cur->type == TokenType::annotate)
    {
      cur += 1;
      eatComments(cur);
      string_view key;
      bool scanningAnno = true;
      switch (cur->type)
      {
      case TokenType::eof:
        addErrorNode("Unexpected end of data.", cur, parentNodeIdx);
        break;
      case TokenType::startDict:
        cur += 1;
        while (scanningAnno)
        {
          eatComments(cur);
          if (cur->type != TokenType::word)
          {
            addErrorNode("Entries in a dict must be key-value pairs. Only strings are valid keys.", cur, parentNodeIdx);
            scanningAnno = false;
          }
          key = cur->value;
          cur += 1;
          eatComments(cur);
          if (cur->type != TokenType::keyValueSep)
          {
            addErrorNode("Missing separator ':' for key-value pair in dict", cur, parentNodeIdx);
            scanningAnno = false;
          }
          cur += 1;
          eatComments(cur);
          if (cur->type != TokenType::word)
          {
            addErrorNode("Entries in a dict must be key-value pairs. Only strings are valid values in an annotation.", cur, parentNodeIdx);
            scanningAnno = false;
          }
          newNode.setAnnotation(key, cur - tokens.data());
          cur += 1;
        }
        eatComments(cur);
        if (cur->type != TokenType::endDict)
        {
          addErrorNode("Dicts must end with a '}'.", cur, parentNodeIdx);
        }
        cur += 1;
        break;
      case TokenType::word:
        eatComments(cur);
        if (cur->type != TokenType::keyValueSep)
        {
          addErrorNode("Annotations must be a single key-value pair or a '{}'-wrapped dict.", cur, parentNodeIdx);
        }
        key = cur->value;
        cur += 1;
        eatComments(cur);
        if (cur->type != TokenType::word)
        {
          addErrorNode("Entries in a dict must be key-value pairs. Only strings are valid values in an annotation.", cur, parentNodeIdx);
        }
        newNode.setAnnotation(key, cur - tokens.data());
        cur += 1;
        break;
      
      default:
        addErrorNode("Invalid token.", cur, parentNodeIdx);
      }
    }
  }

  return newNode;
}


void HumonObj::parse()
{
  nodes.clear();
  token_t const * cur = tokens.data();
  parseRec(cur, numeric_limits<size_t>::max());
}
