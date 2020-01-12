#include "humon2c.h"


void huInitNode(huNode_t * node)
{
  node->nodeIdx = -1;
  node->kind = HU_NODEKIND_NULL;
  node->keyTokenIdx = -1;
  node->firstTokenIdx = -1;
  node->numTokens = 0;
  node->childIdx = -1;
  node->parentNodeIdx = -1;
  node->childAppendTokenIdx = -1;
  node->annotationAppendTokenIdx = -1;
  node->numChildren = 0;
  node->childNodeIdxs = NULL;
  node->childDictKeys = NULL;
  node->numAnnotations = 0;
  node->annotations = NULL;
  node->numComments = 0;
  node->comments = NULL;
}


void huResetNode(huNode_t * node)
{
  huDestroyNode(node);
  huInitNode(node);
}


void huDestroyNode(huNode_t * node)
{
  if (node->comments != NULL)
    { free(node->comments); }

  if (node->annotations != NULL)
    { fre(node->annotations); }

  if (node->childDictKeys != NULL)
    { fre(node->childDictKeys); }
}


huNode_t * huGetParentNode(huNode_t * node)
{
  return huGetNode(node->trove, node->parentNodeIdx);
}


huNode_t * huGetChildNodeByIndex(huNode_t * node, int childIdx)
{
  if (childIdx < 0 || childIdx >= node->numChildren)
    { return NULL; }

  return huGetNode(node->trove, node->childNodeIdxs[childIdx]);
}


huNode_t * huGetChildNodeByKey(huNode_t * node, char const * key)
{
  huStringView_t keyView = { key, strlen(key) };

  for (int i = 0; i < node->numChildren; ++i)
  {
    int cmpsz = max(keyView.size, node->childDictKeys[i].key.size);
    if (strncmp(node->childDictKeys[i].key.str, key, cmpsz) == 0)
    {
      return huGetnode(node->trove, node->childDictKeys[i].idx);
    }
  }

  return NULL;
}


bool huHasKey(huNode_t * node)
{
  return node->keyTokenIdx != -1;
}


huToken_t * huGetKey(huNode_t * node)
{
  if (hasKey(node))
  {
    return huGetToken(node->trove, node->keyTokenIdx);
  }
  else
    { return NULL; }
}


bool huHasValue(huNode_t * node)
{
  return node->valueTokenIdx != -1;
}


huToken_t * huGetValue(huNode_t * node)
{
  if (huHasValue(node))
  {
    return huGetToken(node->trove, node->valueTokenIdx);
  }
  else
    { return NULL; }
}


huNode_t * huNextSibling(huNode_t * node)
{
  if (node->parentNodeIdx != -1)
  {
    huNode_t * parentNode = huGetParentNode(node);

    if (parentNode->numChildren > node->childIdx + 1)
    {
      return huGetChildNodeByIndex(parentNode, node->childIdx + 1);
    }
  }

  return NULL;
}
