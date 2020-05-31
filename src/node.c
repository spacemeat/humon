#include <string.h>
#include "humon.internal.h"


void huInitNode(huNode_t * node, huTrove_t * trove)
{
  node->trove = trove;
  node->nodeIdx = -1;
  node->kind = HU_NODEKIND_NULL;
  node->firstToken = & humon_nullToken;
  node->keyToken = & humon_nullToken;
  node->firstValueToken = & humon_nullToken;
  node->lastValueToken = & humon_nullToken;
  node->lastToken = & humon_nullToken;
  node->childIdx = 0;
  node->parentNodeIdx = -1;
  huInitVector(& node->childNodeIdxs, sizeof(int));
  huInitVector(& node->childDictKeys, sizeof(huDictEntry_t));
  huInitVector(& node->annotations, sizeof(huAnnotation_t));
  huInitVector(& node->comments, sizeof(huComment_t));
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
    { return & humon_nullNode; }

  return huGetNode(node->trove, 
    * (int *) huGetVectorElement(& node->childNodeIdxs, childIdx));
}


huNode_t * huGetChildNodeByKey(huNode_t * node, char const * key, int keyLen)
{
  huStringView_t keyView = { key, keyLen };

  huDictEntry_t * dicts = (huDictEntry_t *) huGetVectorElement(
    & node->childDictKeys, 0);
  for (int i = 0; i < huGetNumChildren(node); ++i)
  {
    huDictEntry_t * dict = dicts + i;
    int cmpsz = dict->key->value.size;
    if (cmpsz > keyView.size)
      { cmpsz = keyView.size; }
    if (strncmp(dict->key->value.str, key, cmpsz) == 0)
    {
      return huGetChildNodeByIndex(node, dict->idx);
    }
  }

  return & humon_nullNode;
}


bool huHasKey(huNode_t * node)
{
  return node->keyToken->tokenKind != HU_TOKENKIND_NULL;
}


huToken_t * huGetKey(huNode_t * node)
{
  return node->keyToken;
}


bool huHasValue(huNode_t * node)
{
  return node->firstValueToken->tokenKind != HU_TOKENKIND_NULL;
}


huToken_t * huGetValue(huNode_t * node)
{
  return node->firstValueToken;
}


huToken_t * huGetStartToken(huNode_t * node)
{
  return node->firstValueToken;
}


huToken_t * huGetEndToken(huNode_t * node)
{
  return node->lastValueToken;
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

  return & humon_nullNode;
}


int huGetNumAnnotations(huNode_t * node)
{
  return node->annotations.numElements;
}


huAnnotation_t * huGetAnnotation(huNode_t * node, int annotationIdx)
{
  if (annotationIdx < node->annotations.numElements)
    { return (huAnnotation_t *) node->annotations.buffer + annotationIdx; }
  else
    { return NULL; }  
}


int huGetNumComments(huNode_t * node)
{
  return node->comments.numElements;
}


huComment_t * huGetComment(huNode_t * node, int commentIdx)
{
  if (commentIdx < node->comments.numElements)
    { return (huComment_t *) node->comments.buffer + commentIdx; }
  else
    { return NULL; }
}
