#include "humon.internal.h"


void recordError(huTrove_t * trove, int errorCode, huToken_t * pCur)
{
  printf ("%sError%s: line: %d  col: %d  %s\n", lightRed, off, 
    pCur->line, pCur->col, huOutputErrorToString(errorCode));
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


// owner can be NULL, for comments assigned to trove
void assignEnqueuedComments(huVector_t * commentQueue, huVector_t * commentVector, huNode_t * owner)
{
  if (commentQueue->numElements == 0)
    { return; }

  huComment_t * commentObj = huGrowVector(commentVector, commentQueue->numElements);

  // The first (earliest) one extends the node's first token to the comment token.
  if (owner != NULL && commentQueue->numElements > 0)
  {
    owner->firstToken = commentObj->commentToken;
  }

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
  while ((* ppCur)->tokenKind == HU_TOKENKIND_COMMENT &&
    node->lastToken->line == (* ppCur)->line)
  {
    assignComment(node, * ppCur);
    node->lastToken = * ppCur;
    *ppCur += 1;
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
void parseAnnotations(huNode_t * owner, huToken_t ** ppCur)
{
  huToken_t * key = NULL;
  huToken_t * value = NULL;
  int state = APS_ANTICIPATE_ANNOTATE;

  bool scanning = true;
  while (scanning)
  {
    if (state == APS_ANTICIPATE_ANNOTATE && 
        (* ppCur)->tokenKind != HU_TOKENKIND_ANNOTATE)
      { break; }

    switch ((* ppCur)->tokenKind)
    {
    case HU_TOKENKIND_EOF:
      recordError(owner->trove, HU_ERROR_UNEXPECTED_EOF, * ppCur);
      scanning = false;
      break;
    
    case HU_TOKENKIND_ANNOTATE:
      state = APS_EXPECT_START_OR_KEY;
      break;
    
    case HU_TOKENKIND_COMMENT:
      assignComment(owner, * ppCur);
      owner->lastToken = * ppCur;
      * ppCur += 1;
      break;

    case HU_TOKENKIND_STARTDICT:
      if (state == APS_EXPECT_START_OR_KEY)
      {
        state = APS_IN_DICT_EXPECT_KEY_OR_END;
        owner->lastToken = * ppCur;
        * ppCur += 1;
      }
      else
      {
        recordError(owner->trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }
      break;
    
    case HU_TOKENKIND_ENDDICT:
      if (state == APS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = APS_ANTICIPATE_ANNOTATE;
        owner->lastToken = * ppCur;
        * ppCur += 1;
        scanning = false;
      }
      else
      {
        recordError(owner->trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }
      break;

    case HU_TOKENKIND_KEYVALUESEP:
      if (state == APS_IN_DICT_EXPECT_KVS)
      {
        state = APS_IN_DICT_EXPECT_VAL;
        owner->lastToken = * ppCur;
        * ppCur += 1;
      }
      else
      {
        recordError(owner->trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }
      break;
    
    case HU_TOKENKIND_WORD:
      if (state == APS_EXPECT_START_OR_KEY)
      {
        state = APS_EXPECT_KVS;
        key = * ppCur;
        owner->lastToken = * ppCur;
        * ppCur += 1;
      }
      else if (state == APS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = APS_IN_DICT_EXPECT_KVS;
        key = * ppCur;
        owner->lastToken = * ppCur;
        * ppCur += 1;
      }
      else if (state == APS_EXPECT_VAL)
      {
        state = APS_ANTICIPATE_ANNOTATE;
        value = * ppCur;
        owner->lastToken = * ppCur;
        * ppCur += 1;
        scanning = false;

        huAnnotation_t * annotation = huGrowVector(
          & owner->annotations, 1);
        annotation->key = key;
        annotation->value = value;
      }
      else if (state == APS_IN_DICT_EXPECT_VAL)
      {
        state = APS_IN_DICT_EXPECT_KEY_OR_END;
        value = * ppCur;
        owner->lastToken = * ppCur;
        * ppCur += 1;

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

huNode_t * parseTroveRecursive(huTrove_t * trove, huToken_t ** ppCur, huNode_t * parentNode, int state, huVector_t * commentQueue)
{
  depth += 1;

  huNode_t * parsedNode = NULL;

  bool scanning = true;
  while (scanning)
  {
    printf("%sparseRec: parent: %d  depth: %d  state: %d  currentToken: %s%s\n",
      darkBlue, parentNode ? parentNode->nodeIdx : -1, depth, state, 
      huTokenKindToString((* ppCur)->tokenKind), off);
    switch ((* ppCur)->tokenKind)
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
        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_LIST, * ppCur);
        int newNodeIdx = newNode->nodeIdx;
        newNode->valueToken = * ppCur;
        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, & * ppCur);
        parseAnnotations(newNode, & * ppCur);

        huNode_t * childNode = parseTroveRecursive(trove, ppCur, 
          newNode, PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);
        // Here we recompute newNode, just in case the above call reallocated trove->nodes.
        newNode = (huNode_t *) trove->nodes.buffer + newNodeIdx;

        // parentage
        if (childNode != NULL)
        {
          childNode->parentNodeIdx = newNodeIdx;
          * (int *) huGrowVector(& newNode->childNodeIdxs, 1) = childNode->nodeIdx;
          childNode->childIdx = newNode->childNodeIdxs.numElements - 1;
        }

        parsedNode = newNode;
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }
      break;

    case HU_TOKENKIND_ENDLIST:
      if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
      {
        scanning = false;
        parentNode->lastToken = * ppCur;

        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & parentNode->comments, parentNode);
        assignSameLineComments(parentNode, ppCur);
        parseAnnotations(parentNode, ppCur);
      }
      else
      {
        if (state == PS_IN_DICT_EXPECT_KEY_OR_END)
          { recordError(trove, HU_ERROR_START_END_MISMATCH, * ppCur); }
        else
          { recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur); }
        * ppCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_STARTDICT:
      if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
          state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END ||
          state == PS_IN_DICT_EXPECT_START_OR_VALUE)
      {
        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_DICT, * ppCur);
        int newNodeIdx = newNode->nodeIdx;
        
        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(newNode, ppCur);

        huNode_t * childNode = parseTroveRecursive(trove, ppCur, 
          newNode, PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);
        // Here we recompute newNode, just in case the above call reallocated trove->nodes.
        newNode = (huNode_t *) trove->nodes.buffer + newNodeIdx;

        // parentage
        if (childNode != NULL)
        {
          childNode->parentNodeIdx = newNodeIdx;
          * (int *) huGrowVector(& newNode->childNodeIdxs, 1) = childNode->nodeIdx;
          childNode->childIdx = newNode->childNodeIdxs.numElements - 1;

          // dictionary key
          huDictEntry_t * dictEntry = huGrowVector(
            & newNode->childDictKeys, 1);
          dictEntry->key = newNode->keyToken;
          dictEntry->idx = childNode->childIdx;
        }

        parsedNode = newNode;
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_ENDDICT:
      if (state == PS_IN_DICT_EXPECT_KEY_OR_END)
      {
        parentNode->lastToken = * ppCur;
        scanning = false;

        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & parentNode->comments, parentNode);
        assignSameLineComments(parentNode, ppCur);
        parseAnnotations(parentNode, ppCur);
      }
      else
      {
        if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
          { recordError(trove, HU_ERROR_START_END_MISMATCH, * ppCur); }
        else
          { recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur); }
        * ppCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_KEYVALUESEP:
      if (state == PS_IN_DICT_EXPECT_KVS)
      {
        state = PS_IN_DICT_EXPECT_START_OR_VALUE;

        huNode_t * lastNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
        lastNode->lastToken = * ppCur;
        * ppCur += 1;
        assignEnqueuedComments(commentQueue, & lastNode->comments, lastNode);
        assignSameLineComments(lastNode, ppCur);
        parseAnnotations(lastNode, ppCur);
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_WORD:
      if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE)
      {
        state = PS_DONE;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, * ppCur);
        newNode->valueToken = * ppCur;
        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(parentNode, ppCur);

        parsedNode = newNode;
      }
      else if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
      {
        // no state change

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, * ppCur);
        newNode->valueToken = * ppCur;
        * ppCur += 1;

        // parentage
//        newNode->parentNodeIdx = parentNode->nodeIdx;
//        * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
//        newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(parentNode, ppCur);

        parsedNode = newNode;
      }
      else if (state == PS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = PS_IN_DICT_EXPECT_KVS;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, * ppCur);
        newNode->keyToken = * ppCur;
        * ppCur += 1;

        // parentage
//        newNode->parentNodeIdx = parentNode->nodeIdx;
//        * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
//        newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

        // dictionary key
//        huDictEntry_t * dictEntry = huGrowVector(
//          & parentNode->childDictKeys, 1);
//        dictEntry->key = newNode->keyToken;
//        dictEntry->idx = newNode->childIdx;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(newNode, ppCur);

        parsedNode = newNode;
      }
      else if (state == PS_IN_DICT_EXPECT_START_OR_VALUE)
      {
        state = PS_IN_DICT_EXPECT_KEY_OR_END;

        // get the last node made when we found a key
        huNode_t * lastNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
        lastNode->kind = HU_NODEKIND_VALUE;
        lastNode->valueToken = * ppCur;
        lastNode->lastToken = * ppCur;
        * ppCur += 1;

        parsedNode = lastNode;

        assignEnqueuedComments(commentQueue, & lastNode->comments, lastNode);
        assignSameLineComments(lastNode, ppCur);
        parseAnnotations(lastNode, ppCur);
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }      
      break;

    case HU_TOKENKIND_COMMENT:
      // Only whole-line comments are picked up here *
      enqueueComment(commentQueue, * ppCur);
      * ppCur += 1;
      break;
    }
  }

  depth -= 1;

  printf("%s  returning state %s%d%s\n",
    darkBlue, lightBlue, state, off);

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


