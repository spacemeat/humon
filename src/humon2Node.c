#include <string.h>
#include "humon.internal.h"


void huInitNode(huNode_t * node, huTrove_t * trove)
{
  node->trove = trove;
  node->nodeIdx = -1;
  node->kind = HU_NODEKIND_NULL;
  node->firstToken = NULL;
  node->lastToken = NULL;
  node->keyToken = NULL;
  node->valueToken = NULL;
  node->childIdx = -1;
  node->parentNodeIdx = -1;
  huInitVector(& node->childNodeIdxs, sizeof(int));
  huInitVector(& node->childDictKeys, sizeof(huDictEntry_t));
  huInitVector(& node->annotations, sizeof(huAnnotation_t));
  huInitVector(& node->comments, sizeof(huComment_t));
}


void huResetNode(huNode_t * node)
{
  huTrove_t * trove = node->trove;
  huDestroyNode(node);
  huInitNode(node, trove);
}


void huDestroyNode(huNode_t * node)
{
  huDestroyVector(& node->childNodeIdxs);
  huDestroyVector(& node->childDictKeys);
  huDestroyVector(& node->annotations);
  huDestroyVector(& node->comments);
}


huNode_t * huGetParentNode(huNode_t * node)
{
  return huGetNode(node->trove, node->parentNodeIdx);
}


int huGetNumChildren(huNode_t * node)
{
  return node->childNodeIdxs.numElements;
}


huNode_t * huGetChildNodeByIndex(huNode_t * node, int childIdx)
{
  if (childIdx < 0 || childIdx >= huGetNumChildren(node))
    { return NULL; }

  return huGetNode(node->trove, 
    * (int *) huGetElement(& node->childNodeIdxs, childIdx));
}


huNode_t * huGetChildNodeByKey(huNode_t * node, char const * key)
{
  huStringView_t keyView = { key, strlen(key) };

  for (int i = 0; i < huGetNumChildren(node); ++i)
  {
    huDictEntry_t * dict = (huDictEntry_t *) huGetElement(& node->childDictKeys, i);
    int cmpsz = dict->key->value.size;
    if (keyView.size > cmpsz)
      { cmpsz = keyView.size; }
    if (strncmp(dict->key->value.str, key, cmpsz) == 0)
    {
      return huGetNode(node->trove, dict->idx);
    }
  }

  return NULL;
}


bool huHasKey(huNode_t * node)
{
  return node->keyToken != NULL;
}


huToken_t * huGetKey(huNode_t * node)
{
  return node->keyToken;
}


bool huHasValue(huNode_t * node)
{
  return node->valueToken != NULL;
}


huToken_t * huGetValue(huNode_t * node)
{
  return node->valueToken;
}


huNode_t * huNextSibling(huNode_t * node)
{
  if (node->parentNodeIdx != -1)
  {
    huNode_t * parentNode = huGetParentNode(node);

    if (huGetNumChildren(parentNode) > node->childIdx + 1)
    {
      return huGetChildNodeByIndex(parentNode, node->childIdx + 1);
    }
  }

  return NULL;
}


int huGetNumAnnotations(huNode_t * node)
{
  return node->annotations.numElements;
}


huAnnotation_t * huGetAnnotation(huNode_t * node, int annotationIdx)
{
  return (huAnnotation_t *) node->annotations.buffer + annotationIdx;
}


int huGetNumComments(huNode_t * node)
{
  return node->comments.numElements;
}


huComment_t * huGetComment(huNode_t * node, int commentIdx)
{
  return (huComment_t *) node->annotations.buffer + commentIdx;
}
