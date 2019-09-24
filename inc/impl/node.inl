#pragma once

// This #include won't resolve because this file is actually inclued from humon2.h, but it's nice for the editor.
#include "humon2.h"
#include "fmt/core.h"
#include "ansiTerm.h"


namespace humon
{
  template <typename T>
  Node<T>::Node(Trove<T> * trove, idx_t nodeIdx) noexcept
  : trove(trove), nodeIdx(nodeIdx)
  {
  }


  template <typename T>
  void Node<T>::print() const noexcept
  {
    auto lo = ansi::darkBlue;
    auto hi = ansi::lightBlue;

    std::cout << fmt::format(
R"({}Node kind: {}{}{} - Node idx: {}{}{}
Ptr: {}{}{}
First token idx: {}{}{} - Num tokens: {}{}{}
Parent node idx: {}{}{} - Num children: {}{}{}
Child node idxs: {})",
      lo, hi, to_string(kind), lo, hi, nodeIdx, lo,
      hi, static_cast<void const*>(this), lo,
      hi, firstTokenIdx, lo, hi, numTokens, lo,
      hi, parentNodeIdx, lo, hi, childNodeIdxs.size(), lo,
      hi);
    
    for (auto ch : childNodeIdxs)
      { std::cout << ch << ' '; }

    std::cout << fmt::format(
R"(
{}Collection append token idx: {}{}{} - Annotation tokens:
)",
      lo, hi, collectionAppendTokenIdx, lo);

    for (auto & [key, val] : annotationTokenIdxs)
    {
      std::cout << fmt::format("{}{}{} ({}{}{})\n",
        hi, key, lo, hi, val, lo);
    }

    std::cout << fmt::format("Annotation append token idx: {}{}{}\n\n", 
      hi, annotationAppendTokenIdx, ansi::off);
  }


  template <typename T>
  void Node<T>::_setKeyIdx(idx_t idx) noexcept
  {
    keyIdx = idx;
  }


  template <typename T>
  void Node<T>::_setFirstTokenIdx(idx_t idx) noexcept
  {
    firstTokenIdx = idx;
  }


  template <typename T>
  void Node<T>::_setNumTokens(idx_t numTokens) noexcept
  {
    this->numTokens = numTokens;
  }


  template <typename T>
  void Node<T>::_setKind(NodeKind kind)
  {
    this->kind = kind;
  }


  template <typename T>
  void Node<T>::_setParentNodeIdx(idx_t parentNodeIdx)
  {
    this->parentNodeIdx = parentNodeIdx;
  }


  template <typename T>
  void Node<T>::_setCollectionIdx(idx_t collectionIdx)
  {
    this->collectionIdx = collectionIdx;
  }


  template <typename T>
  void Node<T>::_addChildNodeIdx(idx_t newChildIdx)
  {
    childNodeIdxs.push_back(newChildIdx);
  }


  template <typename T>
  void Node<T>::_setCollectionAppendTokenIdx(idx_t appendIdx) noexcept
  {
    collectionAppendTokenIdx = appendIdx;
  }


  template <typename T>
  void Node<T>::_setAnnotation(std::string_view key, idx_t valueIdx)
  {
    annotationTokenIdxs.emplace(key, valueIdx);
  }


  template <typename T>
  void Node<T>::_setAnnotationAppendTokenIdx(idx_t appendIdx) noexcept
  {
    annotationAppendTokenIdx = appendIdx;
  }


  template <typename T>
  void Node<T>::ensureKeysMapped() const
  {
    if (childDictIdxs.size() == 0)
    {
      for (auto chIdx : childNodeIdxs)
      {
        auto & chNode = trove->getNode(chIdx);
        auto key = chNode.getKey();
        childDictIdxs.emplace(key, chIdx);
      }
    }
  }


  template <typename T>
  inline Trove<T> * Node<T>::getObj() const noexcept
    { return trove; }
  

  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getNodeIdx() const noexcept
    { return nodeIdx; }
  

  template <typename T>
  inline NodeKind Node<T>::getKind() const noexcept
    { return kind; }


  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getFirstTokenIdx() const noexcept
    { return firstTokenIdx; }


  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getNumTokens() const noexcept
    { return numTokens; }


  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getCollectionIdx() const noexcept
    { return collectionIdx; }
    

  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getParentNodeIdx() const noexcept
    { return parentNodeIdx; }


  template <typename T>
  inline Node<T> const & Node<T>::getParentNode() const noexcept
    { return trove->getNode(parentNodeIdx); }


  template <typename T>
  inline Node<T> & Node<T>::getParentNode() noexcept
    { return trove->getNode(parentNodeIdx); }


  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getNumChildren() const noexcept
  {
    return childNodeIdxs.size();
  }


  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getChildNodeIdx(Node<T>::idx_t idx) const noexcept
  {
    return childNodeIdxs[idx];
  }


  template <typename T>
  inline Node<T> const & Node<T>::getChildNode(Node<T>::idx_t idx) const noexcept
  {
    return trove->getNode(childNodeIdxs[idx]);
  }


  template <typename T>
  inline Node<T> & Node<T>::getChildNode(idx_t idx) noexcept
  {
    return trove->getNode(childNodeIdxs[idx]);
  }


  template <typename T>
  inline bool Node<T>::hasKey() const noexcept
  {
    return trove->getNode(parentNodeIdx).kind ==
      NodeKind::dict;
  }


  template <typename T>
  inline std::string_view Node<T>::getKey() const noexcept
  {
    if (keyIdx != nullIdx)
      { return trove->getToken(keyIdx).value.on(trove->getData()); }
    else
      { return {}; }
  }


  template <typename T>
  inline typename Node<T>::idx_t Node<T>::getIndex() const noexcept
  {
    return collectionIdx;
  }


  template <typename T>
  inline std::string_view Node<T>::getValue() const noexcept
  {
    if (kind == NodeKind::value)
      { return trove->getToken(firstTokenIdx).value; }
    else
      { return {}; }
  }


  template <typename T>
  inline typename std::vector<typename Node<T>::idx_t>::const_iterator
    Node<T>::begin() const noexcept
  {
    return childNodeIdxs.begin();
  }


  template <typename T>
  inline typename std::vector<typename Node<T>::idx_t>::const_iterator
    Node<T>::end() const noexcept
  {
    return childNodeIdxs.end();
  }


  template <typename T>
  inline bool Node<T>::operator %(std::string_view key) const noexcept
  {
    ensureKeysMapped();

    if (kind == NodeKind::dict)
      { return childDictIdxs.find(std::string(key)) != childDictIdxs.end(); }
    
    return false;
  }


  template <typename T>
  template <class IntType, 
    typename std::enable_if<
    std::is_integral<IntType>::value, IntType>::type *>

  inline bool Node<T>::operator %(IntType idx) const noexcept
  {
    auto idxc = static_cast<idx_t>(idx);
    if (kind == NodeKind::dict || 
        kind == NodeKind::list)
      { return idxc >= 0 && idxc < childNodeIdxs.size(); }
    return false;
  }
  
  
  template <typename T>
  inline Node<T> const & Node<T>::operator /(std::string_view key) const
  {
    ensureKeysMapped();
    if (kind == NodeKind::dict)
    {
      auto idx = childDictIdxs.find(std::string(key));
      if (idx != childDictIdxs.end())
        { return trove->getNode(idx->second); }
    }

    return trove->getNull();
  }


  template <typename T>
  template <class IntType, 
    typename std::enable_if<
    std::is_integral<IntType>::value, IntType>::type *>
  inline Node<T> const & Node<T>::operator /(IntType idx) const noexcept
  {
    auto idxc = static_cast<idx_t>(idx);
    if (kind == NodeKind::list || 
        kind == NodeKind::dict)
    {
      if (idxc >= 0 && idxc < childNodeIdxs.size())
        { return trove->getNode(childNodeIdxs[idxc]); }
    }

    return trove->getNull();
  }


  template <typename T>
  template <class U>
  inline U Node<T>::operator /(value<U> ve) const noexcept
  {
    if (kind != NodeKind::value)
      { return U {}; }

    return ve.extract(trove->getToken(firstTokenIdx).value
      .on(trove->getData()));
  }


  template <typename T>
  inline Node<T> const & Node<T>::nextSibling() const noexcept
  {
    auto const & parent = trove->getNode(parentNodeIdx);
    if (collectionIdx + 1 < parent.getNumChildren())
      { return parent / (collectionIdx + 1); }
    
    return trove->getNull();
  }


  template <typename T>
  inline Node<T>::operator bool() const noexcept
  {
    return trove != nullptr;
  }
}
