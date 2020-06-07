#include "humon.internal.h"


void assignTroveComment(huTrove * trove, huToken * pCurrentToken)
{
    huComment * comment = huGrowVector(& trove->comments, 1);
    comment->commentToken = pCurrentToken;
    comment->owner = NULL;
}


void assignComment(huNode * owner, huToken * pCurrentToken)
{
    huComment * comment = huGrowVector(& owner->comments, 1);
    comment->commentToken = pCurrentToken;
    comment->owner = owner;
}


void enqueueComment(huVector * commentQueue, huToken * comment)
{
    huToken ** newComment = huGrowVector(commentQueue, 1);
    * newComment = comment;
}


// owner can be NULL, for comments assigned to trove
void assignEnqueuedComments(huVector * commentQueue, huVector * commentVector, huNode * owner)
{
    if (commentQueue->numElements == 0)
        { return; }

    huComment * newCommentObj = huGrowVector(commentVector, commentQueue->numElements);

    // The first (earliest) one extends the node's first token to the comment token.
    if (owner != NULL && commentQueue->numElements > 0)
    {
        huComment * firstComment = huGetVectorElement(commentQueue, 0);
        owner->firstToken = firstComment->commentToken;
    }

    for (int i = 0; i < commentQueue->numElements; ++i)
    {
        huComment * comment = huGetVectorElement(commentQueue, i);
        (newCommentObj + i)->commentToken = comment->commentToken;
        (newCommentObj + i)->owner = owner;
    }

    huResetVector(commentQueue);
}


void assignSameLineComments(huNode * node, huToken ** ppCur)
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
void parseAnnotations(huTrove * trove, huNode * owner, huToken ** ppCur)
{
    huToken * key = NULL;
    huToken * value = NULL;
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
                    huAnnotation * annotation = huGrowVector(
                        & owner->annotations, 1);
                    annotation->key = key;
                    annotation->value = value;
                }
                else
                {
                    huAnnotation * annotation = huGrowVector(
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
                    huAnnotation * annotation = huGrowVector(
                        & owner->annotations, 1);
                    annotation->key = key;
                    annotation->value = value;
                }
                else
                {
                    huAnnotation * annotation = huGrowVector(
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

void parseTroveRecursive(huTrove * trove, huToken ** ppCur, int parentNodeIdx, int state, huVector * commentQueue)
{
    depth += 1;

    huNode * parentNode = (huNode *) huGetNode(trove, parentNodeIdx);

    bool scanning = true;
    while (scanning)
    {
        //printf("%sparseRec: parent: %d    depth: %d    state: %d    currentToken: %s%s\n",
        //    darkBlue, parentNode ? parentNode->nodeIdx : -1, depth, state, 
        //    huTokenKindToString((* ppCur)->tokenKind), off);
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
                huNode * newNode = NULL;

                if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
                        state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
                {
                    newNode = allocNewNode(trove, HU_NODEKIND_LIST, * ppCur);
                    newNode->firstToken = * ppCur;
                    newNode->firstValueToken = * ppCur;
                    parentNode = (huNode *) huGetNode(trove, parentNodeIdx);
                }

                if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
                {
                    // no state change

                    // parentage
                    newNode->parentNodeIdx = parentNodeIdx;
                    * (int *) huGrowVector((huVector *) & parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
                    newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;
                }
                else if (state == PS_IN_DICT_EXPECT_START_OR_VALUE)
                {
                    newNode = (huNode *) trove->nodes.buffer + trove->nodes.numElements - 1;
                    newNode->kind = HU_NODEKIND_LIST;
                    newNode->firstToken = * ppCur;
                    newNode->firstValueToken = * ppCur;

                    state = PS_IN_DICT_EXPECT_KEY_OR_END;

                    // parentage
                    newNode->parentNodeIdx = parentNodeIdx;
                    * (int *) huGrowVector((huVector *) & parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
                    newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

                    // dictionary key
                    huDictEntry * dictEntry = huGrowVector(
                        (huVector *) & parentNode->childDictKeys, 1);
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
                parentNode = (huNode *) huGetNode(trove, parentNodeIdx);
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
                huNode * newNode = NULL;

                if (state == PS_TOP_LEVEL_EXPECT_START_OR_VALUE ||
                        state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
                {
                    newNode = allocNewNode(trove, HU_NODEKIND_DICT, * ppCur);
                    newNode->firstToken = * ppCur;
                    newNode->firstValueToken = * ppCur;
                    parentNode = (huNode *) huGetNode(trove, parentNodeIdx);
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
                    newNode = (huNode *) trove->nodes.buffer + trove->nodes.numElements - 1;
                    newNode->kind = HU_NODEKIND_DICT;
                    newNode->firstToken = * ppCur;
                    newNode->firstValueToken = * ppCur;

                    state = PS_IN_DICT_EXPECT_KEY_OR_END;

                    // parentage
                    newNode->parentNodeIdx = parentNodeIdx;
                    * (int *) huGrowVector(& parentNode->childNodeIdxs, 1) = newNode->nodeIdx;
                    newNode->childIdx = parentNode->childNodeIdxs.numElements - 1;

                    // dictionary key
                    huDictEntry * dictEntry = (huDictEntry *) huGrowVector(
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
                parentNode = (huNode *) huGetNode(trove, parentNodeIdx);
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

                huNode * lastNode = (huNode *) trove->nodes.buffer + trove->nodes.numElements - 1;
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

                huNode * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, * ppCur);
                newNode->firstToken = * ppCur;
                newNode->firstValueToken = * ppCur;
                newNode->lastValueToken = * ppCur;
                newNode->lastToken = * ppCur;
                parentNode = (huNode *) huGetNode(trove, parentNodeIdx);
                * ppCur += 1;

                assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
                assignSameLineComments(newNode, ppCur);
                parseAnnotations(trove, newNode, ppCur);
            }
            else if (state == PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END)
            {
                // no state change

                huNode * newNode = allocNewNode(trove, HU_NODEKIND_VALUE, * ppCur);
                newNode->firstToken = * ppCur;
                newNode->firstValueToken = * ppCur;
                newNode->lastValueToken = * ppCur;
                newNode->lastToken = * ppCur;
                parentNode = (huNode *) huGetNode(trove, parentNodeIdx);
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

                huNode * newNode = allocNewNode(trove, HU_NODEKIND_NULL, * ppCur);
                newNode->keyToken = * ppCur;
                parentNode = (huNode *) huGetNode(trove, parentNodeIdx);
                * ppCur += 1;

                assignEnqueuedComments(commentQueue, & newNode->comments, newNode);
                assignSameLineComments(newNode, ppCur);
                parseAnnotations(trove, newNode, ppCur);
            }
            else if (state == PS_IN_DICT_EXPECT_START_OR_VALUE)
            {
                state = PS_IN_DICT_EXPECT_KEY_OR_END;

                // get the last node made when we found a key
                huNode * lastNode = (huNode *) trove->nodes.buffer + trove->nodes.numElements - 1;
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
                huDictEntry * dictEntry = huGrowVector(
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
                huNode * lastNode = NULL;
                if (huGetNumNodes(trove) != 0)
                {
                    lastNode = (huNode *) trove->nodes.buffer + trove->nodes.numElements - 1;
                }
                parseAnnotations(trove, lastNode, ppCur);
            }
            break;
        }

    }

    depth -= 1;

    // printf("%s    returning state %s%d%s\n",
    //    darkBlue, lightBlue, state, off);
}


void huParseTrove(huTrove * trove)
{
    huVector commentQueue;
    huInitVector(& commentQueue, sizeof(huToken *));

    huResetVector(& trove->nodes);
    huToken * pCur = trove->tokens.buffer;
    parseTroveRecursive(trove, & pCur, -1, PS_TOP_LEVEL_EXPECT_START_OR_VALUE, & commentQueue);
}


