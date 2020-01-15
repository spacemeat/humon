#include "humon.internal.h"


void recordError(huTrove_t * trove, int errorCode, huToken_t * pCur)
{
  huError_t * error = huGrowVector(& trove->errors, 1);
  error->errorCode = errorCode;
  error->errorToken = pCur;
}


void assignComment(huNode_t * owner, huToken_t * pCurrentToken)
{
  huComment_t * comment = huGrowVector(& owner->comments, 1);
  comment->commentToken = pCurrentToken;
  comment->owner = owner;
}


void enqueueComment(huVector_t * commentQueue, huToken_t * comment)
{
  huToken_t ** newComment = huGrowVector(commentQueue, 1);
  * newComment = comment;
}


void assignEnqueuedComments(huVector_t * commentQueue, huVector_t * commentVector, huNode_t * owner)
{
  huComment_t * commentObj = huGrowVector(commentVector, commentQueue->numElements);
  for (int i = 0; i < commentQueue->numElements; ++i)
  {
    (commentObj + i)->commentToken = 
      * ((huToken_t **) commentQueue->buffer + i);
    (commentObj + i)->owner = owner;
  }
  huResetVector(commentQueue);
}


void assignSameLineComments(huNode_t * node, huToken_t ** ppCur)
{
  huToken_t * pCur = * ppCur;

  while (pCur->tokenKind == HU_TOKENKIND_COMMENT &&
    node->firstToken->line == pCur->line)
  { 
    assignComment(node, pCur);
    pCur += 1;
  }
}


enum annotationParseState
{
  APS_ANTICIPATE_ANNOTATE,
  APS_EXPECT_START_OR_KEY,
  APS_IN_DICT_EXPECT_KEY_OR_END,
  APS_IN_DICT_EXPECT_KVS,
  APS_IN_DICT_EXPECT_VAL,
  APS_EXPECT_KEY,
  APS_EXPECT_KVS,
  APS_EXPECT_VAL
};

// annotations are like @ foo: bar or @ { foo: bar baz: beer }
void parseAnnotations(huNode_t * owner, huToken_t ** ppCurrentToken)
{
  huToken_t * pCur = * ppCurrentToken;
  huToken_t * key = NULL;
  huToken_t * value = NULL;
  int state = APS_ANTICIPATE_ANNOTATE;

  bool scanning = true;
  while (scanning)
  {
    if (state == APS_ANTICIPATE_ANNOTATE && 
        pCur->tokenKind != HU_TOKENKIND_ANNOTATE)
      { break; }

    switch (pCur->tokenKind)
    {
    case HU_TOKENKIND_EOF:
      recordError(owner->trove, HU_ERROR_UNEXPECTED_EOF, pCur);
      scanning = false;
      break;
    
    case HU_TOKENKIND_ANNOTATE:
      state = APS_EXPECT_START_OR_KEY;
      break;
    
    case HU_TOKENKIND_COMMENT:
      assignComment(owner, pCur);
      pCur += 1;
      break;

    case HU_TOKENKIND_STARTDICT:
      if (state == APS_EXPECT_START_OR_KEY)
      {
        state = APS_IN_DICT_EXPECT_KEY_OR_END;
        pCur += 1;
      }
      else
      {
        recordError(owner->trove, HU_ERROR_SYNTAX_ERROR, pCur);
        pCur += 1;
        scanning = false;
      }
      break;
    
    case HU_TOKENKIND_ENDDICT:
      if (state == APS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = APS_ANTICIPATE_ANNOTATE;
        pCur += 1;
        scanning = false;
      }
      else
      {
        recordError(owner->trove, HU_ERROR_SYNTAX_ERROR, pCur);
        pCur += 1;
        scanning = false;
      }
      break;

    case HU_TOKENKIND_KEYVALUESEP:
      if (state == APS_IN_DICT_EXPECT_KVS)
      {
        state = APS_IN_DICT_EXPECT_VAL;
        pCur += 1;
      }
      else
      {
        recordError(owner->trove, HU_ERROR_SYNTAX_ERROR, pCur);
        pCur += 1;
        scanning = false;
      }
      break;
    
    case HU_TOKENKIND_WORD:
      if (state == APS_EXPECT_START_OR_KEY)
      {
        state = APS_EXPECT_KVS;
        key = pCur;
        pCur += 1;
      }
      else if (state == APS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = APS_IN_DICT_EXPECT_KVS;
        key = pCur;
        pCur += 1;
      }
      else if (state == APS_EXPECT_VAL)
      {
        state = APS_ANTICIPATE_ANNOTATE;
        value = pCur;
        pCur += 1;
        scanning = false;

        huAnnotation_t * annotation = huGrowVector(
          & owner->annotations, 1);
        annotation->key = key;
        annotation->value = value;
      }
      else if (state == APS_IN_DICT_EXPECT_VAL)
      {
        state = APS_IN_DICT_EXPECT_KEY_OR_END;
        value = pCur;
        pCur += 1;

        huAnnotation_t * annotation = huGrowVector(
          & owner->annotations, 1);
        annotation->key = key;
        annotation->value = value;
      }
      break;
    }
  }
}


enum parseState
{
  PS_TOP_LEVEL_EXPECT_START_OR_VALUE,
  PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END,
  PS_IN_DICT_EXPECT_KEY_OR_END,
  PS_IN_DICT_EXPECT_KVS,
  PS_IN_DICT_EXPECT_START_OR_VALUE,
  PS_DONE
};


int depth = 0;

huNode_t * parseTroveRecursive(huTrove_t * trove, huToken_t ** ppCurrentToken, huNode_t * parentNode, int state, huVector_t * commentQueue)
{
  depth += 1;

  huToken_t * pCur = * ppCurrentToken;
  huNode_t * parsedNode = NULL;

  bool scanning = true;
  while (scanning)
  {
    printf("parent: %d  depth: %d  state: %d  currentToken: %s\n",
      parentNode ? parentNode->nodeIdx : -1, depth, state, 
      huTokenKindToString(pCur->tokenKind));
    switch (pCur->tokenKind)
    {
    case HU_TOKENKIND_EOF:
      if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
          state == PS_DONE)
      {
        scanning = false;
        assignEnqueuedComments(commentQueue, & trove->comments, NULL);
      }
      break;

    case HU_TOKENKIND_STARTLIST:
      if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
          state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END ||
          state == PS_IN_DICT_EXPECT_START_OR_VALUE)
      {
        if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE)
          { state = PS_DONE; }
        pCur += 1;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_LIST, pCur);
        int newNodeIdx = newNode->nodeIdx;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, & pCur);
        parseAnnotations(newNode, & pCur);

        huNode_t * childNode = parseTroveRecursive(trove, & pCur, 
          newNode, PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);
        // Here we recompute newNode, just in case the above call reallocated trove->nodes.
        newNode = (huNode_t *) trove->nodes.buffer + newNodeIdx;

        // parentage
        childNode->parentNodeIdx = newNodeIdx;
        * (int *) huGrowVector(& newNode->childNodeIdxs, 1) = childNode->nodeIdx;
        childNode->childIdx = newNode->childNodeIdxs.numElements - 1;
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, pCur);
        pCur += 1;
        scanning = false;
      }
      break;

    case HU_TOKENKIND_ENDLIST:
      if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
      {
        pCur += 1;
        parsedNode = parentNode;
        scanning = false;

        assignEnqueuedComments(commentQueue, & parentNode->comments, parentNode);
        assignSameLineComments(parentNode, & pCur);
        parseAnnotations(parentNode, & pCur);
      }
      else
      {
        recordError(trove, HU_ERROR_START_END_MISMATCH, pCur);
        pCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_STARTDICT:
      if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
          state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END ||
          state == PS_IN_DICT_EXPECT_START_OR_VALUE)
      {
        if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE)
          { state = PS_DONE; }
        pCur += 1;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_DICT, pCur);
        int newNodeIdx = newNode->nodeIdx;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, & pCur);
        parseAnnotations(newNode, & pCur);

        huNode_t * childNode = parseTroveRecursive(trove, & pCur, 
          newNode, PS_IN_DICT_EXPECT_START_OR_VALUE, commentQueue);
        // Here we recompute newNode, just in case the above call reallocated trove->nodes.
        newNode = (huNode_t *) trove->nodes.buffer + newNodeIdx;

        // parentage
        childNode->parentNodeIdx = newNodeIdx;
        * (int *) huGrowVector(& newNode->childNodeIdxs, 1) = childNode->nodeIdx;
        childNode->childIdx = newNode->childNodeIdxs.numElements - 1;

        // dictionary key
        huDictEntry_t * dictEntry = huGrowVector(
          & newNode->childDictKeys, 1);
        dictEntry->key = newNode->keyToken;
        dictEntry->idx = childNode->childIdx;
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, pCur);
        pCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_ENDDICT:
      if (state == PS_IN_DICT_EXPECT_KEY_OR_END)
      {
        pCur += 1;
        parsedNode = parentNode;
        scanning = false;

        assignEnqueuedComments(commentQueue, & parentNode->comments, parentNode);
        assignSameLineComments(parentNode, & pCur);
        parseAnnotations(parentNode, & pCur);
      }
      else
      {
        recordError(trove, HU_ERROR_START_END_MISMATCH, pCur);
        pCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_KEYVALUESEP:
      if (state == PS_IN_DICT_EXPECT_KVS)
      {
        state = PS_IN_DICT_EXPECT_START_OR_VALUE;
        pCur += 1;

        huNode_t * lastNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
        parseAnnotations(lastNode, & pCur);
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, pCur);
        pCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_WORD:
      if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE)
      {
        state = PS_DONE;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, pCur);
        newNode->valueToken = pCur;
        pCur += 1;

        parsedNode = newNode;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, & pCur);
        parseAnnotations(parentNode, & pCur);
      }
      else if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
      {
        // no state change
        scanning = false;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, pCur);
        newNode->valueToken = pCur;
        pCur += 1;

        parsedNode = newNode;

        // parentage
        newNode->parentNodeIdx = parentNode->nodeIdx;
        * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
        newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, & pCur);
        parseAnnotations(parentNode, & pCur);
      }
      else if (state == PS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = PS_IN_DICT_EXPECT_KVS;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, pCur);
        newNode->keyToken = pCur;
        pCur += 1;

        // parentage
        newNode->parentNodeIdx = parentNode->nodeIdx;
        * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
        newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

        // dictionary key
        huDictEntry_t * dictEntry = huGrowVector(
          & parentNode->childDictKeys, 1);
        dictEntry->key = newNode->keyToken;
        dictEntry->idx = newNode->childIdx;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, & pCur);
        parseAnnotations(newNode, & pCur);
      }
      else if (state == PS_IN_DICT_EXPECT_START_OR_VALUE)
      {
        state = PS_IN_DICT_EXPECT_KEY_OR_END;
        scanning = false;

        // get the last node made when we found a key
        huNode_t * lastNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
        lastNode->valueToken = pCur;
        pCur += 1;

        parsedNode = lastNode;

        parseAnnotations(lastNode, & pCur);
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, pCur);
        pCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_COMMENT:
      // Only whole-line comments are picked up here *
      enqueueComment(commentQueue, pCur);
      pCur += 1;
      break;
    }
  }

  depth -= 1;

  return parsedNode;
}


void huParseTrove(huTrove_t * trove)
{
  huVector_t commentQueue;
  huInitVector(& commentQueue, sizeof(huToken_t *));

  huResetVector(& trove->nodes);
  huToken_t * pCur = trove->tokens.buffer;
  parseTroveRecursive(trove, & pCur, NULL, PS_TOP_LEVEL_EXPECT_START_OR_VALUE, & commentQueue);
}


