#include "humon.internal.h"


void recordError(huTrove_t * trove, int errorCode, huToken_t * pCur)
{
  printf ("%sError%s: line: %d  col: %d  %s\n", lightRed, off, 
    pCur->line, pCur->col, huOutputErrorToString(errorCode));
  huError_t * error = huGrowVector(& trove->errors, 1);
  error->errorCode = errorCode;
  error->errorToken = pCur;
}


void assignTroveComment(huTrove_t * trove, huToken_t * pCurrentToken)
{
  huComment_t * comment = huGrowVector(& trove->comments, 1);
  comment->commentToken = pCurrentToken;
  comment->owner = NULL;
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

  huComment_t * newCommentObj = huGrowVector(commentVector, commentQueue->numElements);

  // The first (earliest) one extends the node's first token to the comment token.
  if (owner != NULL && commentQueue->numElements > 0)
  {
    huComment_t * firstComment = huGetElement(commentQueue, 0);
    owner->firstToken = firstComment->commentToken;
  }

  for (int i = 0; i < commentQueue->numElements; ++i)
  {
    huComment_t * comment = huGetElement(commentQueue, i);
    (newCommentObj + i)->commentToken = comment->commentToken;
    (newCommentObj + i)->owner = owner;
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
void parseAnnotations(huTrove_t * trove, huNode_t * owner, huToken_t ** ppCur)
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
    
    printf("state: %d\n", state);

    switch ((* ppCur)->tokenKind)
    {
    case HU_TOKENKIND_EOF:
      recordError(trove, HU_ERROR_UNEXPECTED_EOF, * ppCur);
      scanning = false;
      break;
    
    case HU_TOKENKIND_ANNOTATE:
      state = APS_EXPECT_START_OR_KEY;
      if (owner != NULL)
        { owner->lastToken = * ppCur; }
      * ppCur += 1;
      break;
    
    case HU_TOKENKIND_COMMENT:
      if (owner != NULL)
      {
        assignComment(owner, * ppCur);
        owner->lastToken = * ppCur;
      }
      else
      {
        assignTroveComment(trove, * ppCur);
      }
      * ppCur += 1;
      break;

    case HU_TOKENKIND_STARTDICT:
      if (state == APS_EXPECT_START_OR_KEY)
      {
        state = APS_IN_DICT_EXPECT_KEY_OR_END;
        if (owner != NULL)
          { owner->lastToken = * ppCur; }
        * ppCur += 1;
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }
      break;
    
    case HU_TOKENKIND_ENDDICT:
      if (state == APS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = APS_ANTICIPATE_ANNOTATE;
        if (owner != NULL)
          { owner->lastToken = * ppCur; }
        * ppCur += 1;
        scanning = false;
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }
      break;

    case HU_TOKENKIND_KEYVALUESEP:
      if (state == APS_EXPECT_KVS)
      {
        state = APS_EXPECT_VAL;
        if (owner != NULL)
          { owner->lastToken = * ppCur; }
        * ppCur += 1;
      }
      else if (state == APS_IN_DICT_EXPECT_KVS)
      {
        state = APS_IN_DICT_EXPECT_VAL;
        if (owner != NULL)
          { owner->lastToken = * ppCur; }
        * ppCur += 1;
      }
      else
      {
        recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
        * ppCur += 1;
        scanning = false;
      }
      break;
    
    case HU_TOKENKIND_WORD:
      if (state == APS_EXPECT_START_OR_KEY)
      {
        state = APS_EXPECT_KVS;
        key = * ppCur;
        if (owner != NULL)
          { owner->lastToken = * ppCur; }
        * ppCur += 1;
      }
      else if (state == APS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = APS_IN_DICT_EXPECT_KVS;
        key = * ppCur;
        if (owner != NULL)
          { owner->lastToken = * ppCur; }
        * ppCur += 1;
      }
      else if (state == APS_EXPECT_VAL)
      {
        state = APS_ANTICIPATE_ANNOTATE;
        value = * ppCur;
        if (owner != NULL)
        {
          owner->lastToken = * ppCur;
          huAnnotation_t * annotation = huGrowVector(
            & owner->annotations, 1);
          annotation->key = key;
          annotation->value = value;
        }
        else
        {
          huAnnotation_t * annotation = huGrowVector(
            & trove->annotations, 1);
          annotation->key = key;
          annotation->value = value;
        }
        * ppCur += 1;
        scanning = false;
      }
      else if (state == APS_IN_DICT_EXPECT_VAL)
      {
        state = APS_IN_DICT_EXPECT_KEY_OR_END;
        value = * ppCur;
        if (owner != NULL)
        {
          owner->lastToken = * ppCur;
          huAnnotation_t * annotation = huGrowVector(
            & owner->annotations, 1);
          annotation->key = key;
          annotation->value = value;
        }
        else
        {
          huAnnotation_t * annotation = huGrowVector(
            & trove->annotations, 1);
          annotation->key = key;
          annotation->value = value;
        }
        * ppCur += 1;
      }
      break;
    
    default:
      recordError(trove, HU_ERROR_SYNTAX_ERROR, * ppCur);
      * ppCur += 1;
      scanning = false;
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

void parseTroveRecursive(huTrove_t * trove, huToken_t ** ppCur, int parentNodeIdx, int state, huVector_t * commentQueue)
{
  depth += 1;

  huNode_t * parentNode = huGetNode(trove, parentNodeIdx);

  bool scanning = true;
  while (scanning)
  {
    // printf("%sparseRec: parent: %d  depth: %d  state: %d  currentToken: %s%s\n",
    //  darkBlue, parentNode ? parentNode->nodeIdx : -1, depth, state, 
    //  huTokenKindToString((* ppCur)->tokenKind), off);
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
        huNode_t * newNode = NULL;

        if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
            state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
        {
          newNode = allocNewNode(trove, HU_NODEKIND_LIST, * ppCur);
          newNode->firstToken = * ppCur;
          newNode->firstValueToken = * ppCur;
          parentNode = huGetNode(trove, parentNodeIdx);
        }

        if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
        {
          // no state change

          // parentage
          newNode->parentNodeIdx = parentNodeIdx;
          * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
          newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;
        }
        else if (state == PS_IN_DICT_EXPECT_START_OR_VALUE)
        {
          newNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
          newNode->kind = HU_NODEKIND_LIST;
          newNode->firstToken = * ppCur;
          newNode->firstValueToken = * ppCur;

          state = PS_IN_DICT_EXPECT_KEY_OR_END;

          // parentage
          newNode->parentNodeIdx = parentNodeIdx;
          * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
          newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

          // dictionary key
          huDictEntry_t * dictEntry = huGrowVector(
            & parentNode->childDictKeys, 1);
          dictEntry->key = newNode->keyToken;
          dictEntry->idx = newNode->childIdx;
        }

        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, & * ppCur);
        parseAnnotations(trove, newNode, & * ppCur);

        parseTroveRecursive(trove, ppCur, 
          newNode->nodeIdx, PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);
        // recursive call may realloc, so refresh parentNode
        parentNode = huGetNode(trove, parentNodeIdx);
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
        parentNode->lastToken = * ppCur;
        parentNode->lastValueToken = * ppCur;
        scanning = false;

        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & parentNode->comments, parentNode);
        // TODO: These can be arbitrarily ordered on one line. Also, fix this for all instances of these invocations.
        assignSameLineComments(parentNode, ppCur);
        parseAnnotations(trove, parentNode, ppCur);
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
        huNode_t * newNode = NULL;

        if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
            state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
        {
          newNode = allocNewNode(trove, HU_NODEKIND_DICT, * ppCur);
          newNode->firstToken = * ppCur;
          newNode->firstValueToken = * ppCur;
          parentNode = huGetNode(trove, parentNodeIdx);
        }

        if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
        {
          // no state change

          // parentage
          newNode->parentNodeIdx = parentNodeIdx;
          * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
          newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;
        }
        else if (state == PS_IN_DICT_EXPECT_START_OR_VALUE)
        {
          newNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
          newNode->kind = HU_NODEKIND_DICT;
          newNode->firstToken = * ppCur;
          newNode->firstValueToken = * ppCur;

          state = PS_IN_DICT_EXPECT_KEY_OR_END;

          // parentage
          newNode->parentNodeIdx = parentNodeIdx;
          * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
          newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

          // dictionary key
          huDictEntry_t * dictEntry = huGrowVector(
            & parentNode->childDictKeys, 1);
          dictEntry->key = newNode->keyToken;
          dictEntry->idx = newNode->childIdx;
        }

        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(trove, newNode, ppCur);

        parseTroveRecursive(trove, ppCur, 
          newNode->nodeIdx, PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);
        // recursive call may realloc, so refresh parentNode
        parentNode = huGetNode(trove, parentNodeIdx);
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
        parentNode->lastValueToken = * ppCur;
        scanning = false;

        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & parentNode->comments, parentNode);
        assignSameLineComments(parentNode, ppCur);
        parseAnnotations(trove, parentNode, ppCur);
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
        parseAnnotations(trove, lastNode, ppCur);
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
        newNode->firstToken = * ppCur;
        newNode->firstValueToken = * ppCur;
        newNode->lastValueToken = * ppCur;
        newNode->lastToken = * ppCur;
        parentNode = huGetNode(trove, parentNodeIdx);
        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(trove, newNode, ppCur);
      }
      else if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
      {
        // no state change

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, * ppCur);
        newNode->firstToken = * ppCur;
        newNode->firstValueToken = * ppCur;
        newNode->lastValueToken = * ppCur;
        newNode->lastToken = * ppCur;
        parentNode = huGetNode(trove, parentNodeIdx);
        * ppCur += 1;

        // parentage
        newNode->parentNodeIdx = parentNode->nodeIdx;
        * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
        newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(trove, newNode, ppCur);
      }
      else if (state == PS_IN_DICT_EXPECT_KEY_OR_END)
      {
        state = PS_IN_DICT_EXPECT_KVS;

        huNode_t * newNode = allocNewNode(trove, HU_NODEKIND_NULL, * ppCur);
        newNode->keyToken = * ppCur;
        parentNode = huGetNode(trove, parentNodeIdx);
        * ppCur += 1;

        assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
        assignSameLineComments(newNode, ppCur);
        parseAnnotations(trove, newNode, ppCur);
      }
      else if (state == PS_IN_DICT_EXPECT_START_OR_VALUE)
      {
        state = PS_IN_DICT_EXPECT_KEY_OR_END;

        // get the last node made when we found a key
        huNode_t * lastNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
        lastNode->kind = HU_NODEKIND_VALUE;
        lastNode->firstValueToken = * ppCur;
        lastNode->lastValueToken = * ppCur;
        lastNode->lastToken = * ppCur;
        * ppCur += 1;

        // parentage
        lastNode->parentNodeIdx = parentNode->nodeIdx;
        * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = lastNode->nodeIdx;
        lastNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

        // dictionary key
        huDictEntry_t * dictEntry = huGrowVector(
          & parentNode->childDictKeys, 1);
        dictEntry->key = lastNode->keyToken;
        dictEntry->idx = lastNode->childIdx;

        assignEnqueuedComments(commentQueue, & lastNode->comments, lastNode);
        assignSameLineComments(lastNode, ppCur);
        parseAnnotations(trove, lastNode, ppCur);
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

    case HU_TOKENKIND_ANNOTATE:
      {
        // Only annotations after other annotations OR trove annotations are picked up here
        huNode_t * lastNode = NULL;
        if (huGetNumNodes(trove) != 0)
        {
          lastNode = (huNode_t *) trove->nodes.buffer + trove->nodes.numElements - 1;
        }
        parseAnnotations(trove, lastNode, ppCur);
      }
      break;
    }

  }

  depth -= 1;

  // printf("%s  returning state %s%d%s\n",
  //  darkBlue, lightBlue, state, off);
}


void huParseTrove(huTrove_t * trove)
{
  huVector_t commentQueue;
  huInitVector(& commentQueue, sizeof(huToken_t *));

  huResetVector(& trove->nodes);
  huToken_t * pCur = trove->tokens.buffer;
  parseTroveRecursive(trove, & pCur, -1, PS_TOP_LEVEL_EXPECT_START_OR_VALUE, & commentQueue);
}


