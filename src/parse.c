#include "humon.internal.h"


enum parseState
{
    PS_TOP_LEVEL_EXPECT_START_OR_VALUE,
    PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END,
    PS_IN_DICT_EXPECT_KEY_OR_END,
    PS_IN_DICT_EXPECT_KVS,
    PS_IN_DICT_EXPECT_START_OR_VALUE,
    PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY,
    PS_IN_ANNO_EXPECT_KVS,
    PS_IN_ANNO_EXPECT_VALUE,
    PS_IN_ANNODICT_EXPECT_KEY_OR_END,
    PS_IN_ANNODICT_EXPECT_KVS,
    PS_IN_ANNODICT_EXPECT_VALUE,
    PS_DONE
};


char const * parseStateToString(int rhs)
{
    switch(rhs)
    {
    case PS_TOP_LEVEL_EXPECT_START_OR_VALUE: return "topLevel";
    case PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END: return "inList";
    case PS_IN_DICT_EXPECT_KEY_OR_END: return "inDict-keyOrEnd";
    case PS_IN_DICT_EXPECT_KVS: return "inDict-kvs";
    case PS_IN_DICT_EXPECT_START_OR_VALUE: return "startDict-startOrValue";
    case PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY: return "anno-dictStartOrKey";
    case PS_IN_ANNO_EXPECT_KVS: return "anno-key";
    case PS_IN_ANNO_EXPECT_VALUE: return "anno-value";
    case PS_IN_ANNODICT_EXPECT_KEY_OR_END: return "annoDict-keyOrEnd";
    case PS_IN_ANNODICT_EXPECT_KVS: return "annoDict-kvs";
    case PS_IN_ANNODICT_EXPECT_VALUE: return "annoDict-value";
    default: return "!!unknown!!";
    }
}



void ensureNodeContains(huNode * node, huToken const * token)
{
    if (node == hu_nullNode || node == NULL)
        { return; }
    if (token < node->firstToken)
        { node->firstToken = token; }
    else if (token > node->lastToken)
        { node->lastToken = token; }
}


void associateComment(huTrove * trove, huNode * node, huToken const * tok)
{
    huComment * comment;

    int num = 1;
    if (node)
        { comment = huGrowVector(& node->comments, & num); }
    else
        { comment = huGrowVector(& trove->comments, & num); }

    if (num)
    {
        comment->commentToken = tok;
        comment->owner = node;

        if (node)
            { ensureNodeContains(node, tok); }
    }


#ifdef HUMON_CAVEPERSON_DEBUGGING
    char address[32] = { 0 };
    int addLen = 32;
    huGetNodeAddress(node, address, & addLen);
    printf("Associating comment: %s%.*s%s to node %s%s%s\n", 
        darkGreen, tok->str.size, tok->str.str, off,
        lightBlue, address, off );
#endif
}


void enqueueComment(huVector * commentQueue, huToken const * comment)
{
    huAppendToVector(commentQueue, & comment, 1);
    
#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf("Enqueuing comment: %s%.*s%s\n", darkGreen, comment->str.size, comment->str.str, off);
#endif
}


// owner can be NULL, for comments assigned to trove
void associateEnqueuedComments(huTrove * trove, huNode * node, huVector * commentQueue)
{
#ifdef HUMON_CAVEPERSON_DEBUGGING
    char address[32] = { 0 };
    int addLen = 32;
    huGetNodeAddress(node, address, & addLen);
    printf("Associating enqueued %scomments%s to node %s%s%s\n", darkGreen, off,
        lightBlue, address, off);
#endif

    if (commentQueue->numElements == 0)
        { return; }

    huVector * commentVector = NULL;
    if (node)
        { commentVector = & node->comments; }
    else
        { commentVector = & trove->comments; }

    int num = commentQueue->numElements;
    huComment * newCommentObj = huGrowVector(commentVector, & num);

    // The first (earliest) one extends the node's first token to the comment token.
    if (node != NULL && num > 0)
    {
        huComment * firstComment = huGetVectorElement(commentQueue, 0);
        node->firstToken = firstComment->commentToken;
    }

    // Now add all comments to the vector.
    for (int i = 0; i < num; ++i)
    {
        huComment * comment = huGetVectorElement(commentQueue, i);
        (newCommentObj + i)->commentToken = comment->commentToken;
        (newCommentObj + i)->owner = node;
    }

    huResetVector(commentQueue);
}


void setKeyToken(huNode * node, huToken const * tok)
{
    node->keyToken = tok;
    ensureNodeContains(node, tok);
}


void setValueToken(huNode * node, huToken const * tok)
{
    node->valueToken = tok;
    ensureNodeContains(node, tok);
}


void setLastValueToken(huNode * node, huToken const * tok)
{
    node->lastValueToken = tok;
    ensureNodeContains(node, tok);
}


void addChildNode(huNode * node, huNode * child)
{
    child->parentNodeIdx = node->nodeIdx;
    huAppendToVector(& node->childNodeIdxs, & child->nodeIdx, 1);
    child->childOrdinal = node->childNodeIdxs.numElements - 1;

#ifdef HUMON_CAVEPERSON_DEBUGGING
    char address[32] = { 0 };
    int addLen = 32;
    huGetNodeAddress(node, address, & addLen);
    printf("Adding child node to node %s%s%s\n",
        lightBlue, address, off);
#endif
}


void addAnnotation(huTrove * trove, huNode * node, huToken const * keyToken)
{
    huAnnotation * anno = NULL;
    int num = 1;
    if (node)
        { anno = huGrowVector(& node->annotations, & num); }
    else
        { anno = huGrowVector(& trove->annotations, & num); }
    
    if (num)
    {
        anno->key = keyToken;
        anno->value = NULL;

        if (node)
            { ensureNodeContains(node, keyToken); }
    }
}


void setLastAnnotationValue(huTrove * trove, huNode * node, huToken const * valueToken)
{
    huAnnotation * anno = NULL;
    if (node)
        { anno = huGetVectorElement(& node->annotations, huGetVectorSize(& node->annotations) - 1); }
    else
        { anno = huGetVectorElement(& trove->annotations, huGetVectorSize(& trove->annotations) - 1); }
    anno->value = valueToken;

    if (node)
        { ensureNodeContains(node, valueToken); }
}


// Pay special notice to whether we break or return from a particular case.
void parseTroveRecursive(huTrove * trove, int * tokenIdx, huNode * parentNode, int depth, int state, huVector * commentQueue)
{
    huNode * nodeCreatedThisState = NULL;

    while (* tokenIdx < trove->tokens.numElements)
    {
        huToken const * tok = huGetToken(trove, * tokenIdx);
        
#ifdef HUMON_CAVEPERSON_DEBUGGING
        char address[32] = { 0 };
        int addLen = 32;
        huGetNodeAddress(parentNode, address, & addLen);
        printf("PTR: tokenIdx: %d  token: '%.*s'  parentNode: %s  depth: %d  state: %s\n",
            * tokenIdx, tok->str.size, tok->str.str, address, depth, parseStateToString(state));
#endif
        * tokenIdx += 1;
        switch (state)
        {
        case PS_TOP_LEVEL_EXPECT_START_OR_VALUE:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: return;
            
            case HU_TOKENKIND_COMMENT:
                // if nodeCreatedThisState and comment is on the same line,
                //   associate to nodeCreatedThisState
                // else
                //   comment queue
                if (nodeCreatedThisState &&
                    tok->line == nodeCreatedThisState->lastToken->line)
                    { associateComment(trove, nodeCreatedThisState, tok); }
                else
                    { enqueueComment(commentQueue, tok); }
                break;

            case HU_TOKENKIND_STARTLIST:
                // if nodeCreatedThisState exists, err
                // make new list node
                // assign comment queue to it
                // nodeCreatedThisState = new node
                // recursive(PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, nodeCreatedThisState)
                if (nodeCreatedThisState)
                    { recordError(trove, HU_ERROR_TOOMANYROOTS, tok); }
                else
                {
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_LIST, tok);
                    int nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setValueToken(nodeCreatedThisState, tok);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNode(trove, nctsIdx);
                }
                break;

            case HU_TOKENKIND_STARTDICT:
                // if nodeCreatedThisState exists, err
                // make new dict node
                // assign comment queue to it
                // nodeCreatedThisState = new node
                // recursive(PS_IN_DICT_EXPECT_KEY_OR_END, nodeCreatedThisState)
                if (nodeCreatedThisState)
                    { recordError(trove, HU_ERROR_TOOMANYROOTS, tok); }
                else
                {
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_DICT, tok);
                    int nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setValueToken(nodeCreatedThisState, tok);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNode(trove, nctsIdx);
                }
                break;

            case HU_TOKENKIND_WORD:
                // if nodeCreatedThisState exists, err
                // make new value node
                // assign comment queue to it
                // nodeCreatedThisState = new node
                if (nodeCreatedThisState)
                    { recordError(trove, HU_ERROR_TOOMANYROOTS, tok); }
                else
                {
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_VALUE, tok);
                    int nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setValueToken(nodeCreatedThisState, tok);
                    setLastValueToken(nodeCreatedThisState, tok);

                    nodeCreatedThisState = (huNode *) huGetNode(trove, nctsIdx);
                }
                break;

            case HU_TOKENKIND_ANNOTATE:
                // if nodeCreatedThisState, assign comment queue to it
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, nodeCreatedThisState)
                if (nodeCreatedThisState)
                {
                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    ensureNodeContains(nodeCreatedThisState, tok);
                }
                parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                    PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                break;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // if on same line as parentNode, associate to that
                // else if nodeCreatedThisState and on the same line, associate to that
                // else comment queue
                if (nodeCreatedThisState &&
                    tok->line == nodeCreatedThisState->lastToken->line)
                    { associateComment(trove, nodeCreatedThisState, tok); }
                else if (tok->line == parentNode->lastToken->line)
                    { associateComment(trove, parentNode, tok); }
                else
                    { enqueueComment(commentQueue, tok); }
                break;

            case HU_TOKENKIND_STARTLIST:
                // make new list node
                // assign comment queue to it
                // assign new node to parentNode
                // nodeCreatedThisState = new node
                // recursive(PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, nodeCreatedThisState)
                {
                    int parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_LIST, tok);
                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                    int nctsIdx = nodeCreatedThisState->nodeIdx;

                    setValueToken(nodeCreatedThisState, tok);
                    addChildNode(parentNode, nodeCreatedThisState);
                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNode(trove, nctsIdx);
                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                }
                break;

            case HU_TOKENKIND_STARTDICT:
                // make new dict node
                // assign comment queue to it
                // assign new node to parentNode
                // nodeCreatedThisState = new node
                // recursive(PS_IN_DICT_EXPECT_KEY_OR_END, nodeCreatedThisState)
                {
                    int parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_DICT, tok);
                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                    int nctsIdx = nodeCreatedThisState->nodeIdx;

                    setValueToken(nodeCreatedThisState, tok);
                    addChildNode(parentNode, nodeCreatedThisState);
                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNode(trove, nctsIdx);
                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                }
                break;

            case HU_TOKENKIND_WORD:
                // make new value node
                // assign comment queue to it
                // assign new node to parentNode
                // nodeCreatedThisState = new node
                {
                    int parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_VALUE, tok);
                    parentNode = (huNode *) huGetNode(trove, parentIdx);

                    setValueToken(nodeCreatedThisState, tok);
                    setLastValueToken(nodeCreatedThisState, tok);
                    addChildNode(parentNode, nodeCreatedThisState);
                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                }
                break;

            case HU_TOKENKIND_ANNOTATE:
                // if nodeCreatedThisState, assign comment queue to it
                // else assign comment queue to parentNode
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, nodeCreatedThisState || parentNode)
                {
                    huNode * annoTarget = nodeCreatedThisState;
                    if (annoTarget == NULL)
                        { annoTarget = parentNode; }
                    ensureNodeContains(annoTarget, tok);
                    associateEnqueuedComments(trove, annoTarget, commentQueue);
                    parseTroveRecursive(trove, tokenIdx, annoTarget, depth + 1, 
                        PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                }
                break;

            case HU_TOKENKIND_ENDLIST:
                // assign comment queue to parentNode
                associateEnqueuedComments(trove, parentNode, commentQueue);
                setLastValueToken(parentNode, tok);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_DICT_EXPECT_KEY_OR_END:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // if on same line as parentNode, associate to that
                // else if nodeCreatedThisState and on the same line, associate to that
                // else comment queue
                if (nodeCreatedThisState &&
                    tok->line == nodeCreatedThisState->lastToken->line)
                    { associateComment(trove, nodeCreatedThisState, tok); }
                else if (tok->line == parentNode->lastToken->line)
                    { associateComment(trove, parentNode, tok); }
                else
                    { enqueueComment(commentQueue, tok); }
                break;

            case HU_TOKENKIND_WORD:
                // make new dummy node
                // assign key to it
                // assign comment queue to it
                // assign new node to parentNode
                // nodeCreatedThisState = new node
                // recursive(PS_IN_DICT_EXPECT_KVS, nodeCreatedThisState)
                {
                    int parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_NULL, tok);
                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                    int nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setKeyToken(nodeCreatedThisState, tok);
                    addChildNode(parentNode, nodeCreatedThisState);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_DICT_EXPECT_KVS, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNode(trove, nctsIdx);
                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                }
                break;

            case HU_TOKENKIND_ANNOTATE:
                // if nodeCreatedThisState, assign comment queue to it
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, nodeCreatedThisState || parentNode)
                {
                    huNode * annoTarget = nodeCreatedThisState;
                    if (annoTarget == NULL)
                        { annoTarget = parentNode; }
                    ensureNodeContains(annoTarget, tok);
                    associateEnqueuedComments(trove, annoTarget, commentQueue);
                    parseTroveRecursive(trove, tokenIdx, annoTarget, depth + 1, 
                        PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                }
                break;

            case HU_TOKENKIND_ENDDICT:
                // assign comment queue to parentNode
                associateEnqueuedComments(trove, parentNode, commentQueue);
                setLastValueToken(parentNode, tok);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_DICT_EXPECT_KVS:
            // parentNode is the node we created when we encountered the key token.
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_KEYVALUESEP:
                // recursive(PS_IN_DICT_EXPECT_START_OR_VALUE)
                {
                    int parentIdx = parentNode->nodeIdx;

                    ensureNodeContains(parentNode, tok);
                    parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                        PS_IN_DICT_EXPECT_START_OR_VALUE, commentQueue);

                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                }
                return;

            case HU_TOKENKIND_ANNOTATE:
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, parentNode)
                ensureNodeContains(parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                break;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_DICT_EXPECT_START_OR_VALUE:
            // parentNode is the node we created when we encountered the key token.
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_STARTLIST:
                // set parentNode to list kind
                // recursive(PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, parentNode)
                {
                    int parentIdx = parentNode->nodeIdx;

                    parentNode->kind = HU_NODEKIND_LIST;
                    setValueToken(parentNode, tok);
                    parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                        PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);

                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                }
                return;

            case HU_TOKENKIND_STARTDICT:
                // set parentNode to dict kind
                // recursive(PS_IN_DICT_EXPECT_KEY_OR_END, parentNode)
                {
                    int parentIdx = parentNode->nodeIdx;

                    parentNode->kind = HU_NODEKIND_DICT;
                    setValueToken(parentNode, tok);
                    parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                        PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);

                    parentNode = (huNode *) huGetNode(trove, parentIdx);
                }
                return;

            case HU_TOKENKIND_WORD:
                // set parentNode to value kind
                parentNode->kind = HU_NODEKIND_VALUE;
                setValueToken(parentNode, tok);
                setLastValueToken(parentNode, tok);
                return;

            case HU_TOKENKIND_ANNOTATE:
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, parentNode)
                ensureNodeContains(parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                break;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_STARTDICT:
                // recursive(PS_IN_ANNODICT_EXPECT_KEY_OR_END, parentNode)
                ensureNodeContains(parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNODICT_EXPECT_KEY_OR_END, commentQueue);
                return;

            case HU_TOKENKIND_WORD:
                // make new anno with key, null value
                // assign anno to parentNode
                // recursive(PS_IN_ANNO_EXPECT_KVS, parentNode)
                addAnnotation(trove, parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNO_EXPECT_KVS, commentQueue);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNO_EXPECT_KVS:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_KEYVALUESEP:
                // recursive(PS_IN_ANNO_EXPECT_VALUE, parentNode)
                ensureNodeContains(parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNO_EXPECT_VALUE, commentQueue);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNO_EXPECT_VALUE:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_WORD:
                // get parentNode's last anno, assign value to it
                setLastAnnotationValue(trove, parentNode, tok);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNODICT_EXPECT_KEY_OR_END:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_WORD:
                // make new anno with key, null value
                // assign anno to parentNode
                // recursive(PS_IN_ANNODICT_EXPECT_KVS, parentNode)
                addAnnotation(trove, parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNODICT_EXPECT_KVS, commentQueue);
                break;

            case HU_TOKENKIND_ENDDICT:
                ensureNodeContains(parentNode, tok);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNODICT_EXPECT_KVS:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_KEYVALUESEP:
                // recursive(PS_IN_ANNODICT_EXPECT_VALUE, parentNode)
                ensureNodeContains(parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNODICT_EXPECT_VALUE, commentQueue);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNODICT_EXPECT_VALUE:
            switch (tok->tokenKind)
            {
            case HU_TOKENKIND_EOF: 
                recordError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_WORD:
                // get parentNode's last anno, assign value to it
                setLastAnnotationValue(trove, parentNode, tok);
                return;

            default:
                // report error
                recordError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;
        }    
    }
}

void huParseTrove(huTrove * trove)
{
#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf("%sParsing:%s\n%s\n%s",
        darkRed, darkYellow, trove->dataString, off);
#endif

    huVector commentQueue;
    huInitVector(& commentQueue, sizeof(huToken *));

    int tokenIdx = 0;
    parseTroveRecursive(trove, & tokenIdx, NULL, 0, PS_TOP_LEVEL_EXPECT_START_OR_VALUE, & commentQueue);
    associateEnqueuedComments(trove, NULL, & commentQueue);
}
