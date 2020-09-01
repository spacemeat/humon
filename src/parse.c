#include <string.h>
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


char const * parseStateToString(huEnumType_t rhs)
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


void ensureContains(huTrove * trove, huNode * node, huToken const * token)
{
    if (node == HU_NULLNODE)
    {
        if (token > trove->lastAnnoToken)
            { trove->lastAnnoToken = token; }
    }

    else if (token < node->firstToken)
        { node->firstToken = token; }
    else if (token > node->lastToken)
        { node->lastToken = token; }
}


void associateComment(huTrove * trove, huNode * node, huToken const * tok)
{
    huComment * comment;

    huSize_t num = 1;
    if (node)
        { comment = growVector(& node->comments, & num); }
    else
        { comment = growVector(& trove->comments, & num); }

    if (num)
    {
        comment->token = tok;
        comment->node = node;
        ensureContains(trove, node, tok);
    }

#ifdef HUMON_CAVEPERSON_DEBUGGING
    char address[HUMON_ADDRESS_BLOCKSIZE];
    huSize_t addLen = HUMON_ADDRESS_BLOCKSIZE;
    if (node)
    {
        huGetAddress(node, address, & addLen);
        printf("Associating comment: '%s%.*s%s' to node %s%.*s%s\n", 
            ansi_darkGreen, (int)tok->str.size, tok->str.ptr, ansi_off,
            ansi_lightBlue, (int)addLen, address, ansi_off );
    }
    else
    {
        printf("Associating comment: '%s%.*s%s' to trove\n", 
            ansi_darkGreen, (int)tok->str.size, tok->str.ptr, ansi_off);
    }
#endif
}


void enqueueComment(huVector * commentQueue, huToken const * comment)
{
    appendToVector(commentQueue, & comment, 1);
    
#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf("Enqueuing comment: '%s%.*s%s'\n", ansi_darkGreen, (int)comment->str.size, comment->str.ptr, ansi_off);
#endif
}


// node can be NULL, for comments assigned to trove
void associateEnqueuedComments(huTrove * trove, huNode * node, huVector * commentQueue)
{
    if (commentQueue->numElements == 0)
        { return; }

    huVector * commentVector = NULL;
    if (node)
        { commentVector = & node->comments; }
    else
        { commentVector = & trove->comments; }

    huSize_t num = commentQueue->numElements;

#ifdef HUMON_CAVEPERSON_DEBUGGING
    if (num > 0)
    {
        char address[HUMON_ADDRESS_BLOCKSIZE];
        huSize_t addLen = HUMON_ADDRESS_BLOCKSIZE;
        if (node)
        {
            huGetAddress(node, address, & addLen);
            printf("Associating %s%lld%s enqueued comments to node %s%.*s%s\n",
                ansi_white, (long long) num, ansi_off,
                ansi_lightBlue, (int)addLen, address, ansi_off);
        }
        else
            { printf("Associating %s%lld%s enqueued comments to trove\n", 
                ansi_white, (long long) num, ansi_off); }
    }
#endif

    huComment * newCommentObj = growVector(commentVector, & num);

    // The first (earliest) one extends the node's first token to the comment token.
    if (node != NULL && num > 0)
    {
        huComment * firstComment = getVectorElement(commentQueue, 0);
        node->firstToken = firstComment->token;
    }

    // Now add all comments to the vector.
    for (huSize_t i = 0; i < num; ++i)
    {
        huComment * comment = getVectorElement(commentQueue, i);
        (newCommentObj + i)->token = comment->token;
        (newCommentObj + i)->node = node;
    }

    resetVector(commentQueue);
}


void setKeyToken(huNode * node, huToken const * tok)
{
    node->keyToken = tok;
    ensureContains(NULL, node, tok);
}


void setValueToken(huNode * node, huToken const * tok)
{
    node->valueToken = tok;
    ensureContains(NULL, node, tok);
}


void setLastValueToken(huNode * node, huToken const * tok)
{
    node->lastValueToken = tok;
    ensureContains(NULL, node, tok);
}


void addChildNode(huTrove * trove, huNode * node, huNode * child)
{
    // check for duplicate keys
    if (node->kind == HU_NODEKIND_DICT)
    {
        huStringView const * key = & child->keyToken->str;
        if (huGetChildByKeyN(node, key->ptr, key->size) != HU_NULLNODE)
            { recordParseError(trove, HU_ERROR_NONUNIQUEKEY, child->keyToken); }
    }

    // all claer, adopt the child
    child->parentNodeIdx = node->nodeIdx;
    appendToVector(& node->childNodeIdxs, & child->nodeIdx, 1);
    child->childOrdinal = node->childNodeIdxs.numElements - 1;

#ifdef HUMON_CAVEPERSON_DEBUGGING
    char address[HUMON_ADDRESS_BLOCKSIZE];
    huSize_t addLen = HUMON_ADDRESS_BLOCKSIZE;
    huGetAddress(node, address, & addLen);
    printf("Adding child node to node %s%.*s%s\n",
        ansi_lightBlue, (int)addLen, address, ansi_off);
#endif
}


void addAnnotation(huTrove * trove, huNode * node, huToken const * keyToken)
{
    // check for duplicate keys
    huStringView const * key = & keyToken->str;
    if (node)
    {
        if (huHasAnnotationWithKeyN(node, key->ptr, key->size))
            { recordParseError(trove, HU_ERROR_NONUNIQUEKEY, keyToken); }
    }
    else
    {
        if (huTroveHasAnnotationWithKeyN(trove, key->ptr, key->size))
            { recordParseError(trove, HU_ERROR_NONUNIQUEKEY, keyToken); }
    }    

    huAnnotation * anno = NULL;
    huSize_t num = 1;
    if (node)
        { anno = growVector(& node->annotations, & num); }
    else
        { anno = growVector(& trove->annotations, & num); }
    
    if (num)
    {
        anno->key = keyToken;
        anno->value = NULL;

        ensureContains(trove, node, keyToken);
    }
}


void setLastAnnotationValue(huTrove * trove, huNode * node, huToken const * valueToken)
{
    huAnnotation * anno = NULL;
    if (node)
        { anno = getVectorElement(& node->annotations, getVectorSize(& node->annotations) - 1); }
    else
        { anno = getVectorElement(& trove->annotations, getVectorSize(& trove->annotations) - 1); }
    anno->value = valueToken;

    ensureContains(trove, node, valueToken);
}


// Pay special notice to whether we break or return from a particular case.
void parseTroveRecursive(huTrove * trove, huSize_t * tokenIdx, huNode * parentNode, huSize_t depth, huEnumType_t state, huVector * commentQueue)
{
    huNode * nodeCreatedThisState = NULL;

    while (* tokenIdx < trove->tokens.numElements)
    {
        huToken const * tok = huGetToken(trove, * tokenIdx);
        
#ifdef HUMON_CAVEPERSON_DEBUGGING
        char address[HUMON_ADDRESS_BLOCKSIZE];
        huSize_t addLen = HUMON_ADDRESS_BLOCKSIZE;
        if (parentNode)
            { huGetAddress(parentNode, address, & addLen); }
        else
        {
            memcpy(address, "null", 4);
            addLen = 4;
        }

        printf("PTR: tokenIdx: %s%lld%s  token: '%s%.*s%s'  parentNode: %s%.*s%s  depth: %s%lld%s  state: %s%s%s\n",
            ansi_darkYellow, (long long) * tokenIdx, ansi_off,
            ansi_white, (int)tok->str.size, tok->str.ptr, ansi_off,
            ansi_lightBlue, (int)addLen, address, ansi_off,
            ansi_white, (long long) depth, ansi_off,
            ansi_darkBlue, parseStateToString(state), ansi_off);
        
#endif
        * tokenIdx += 1;
        switch (state)
        {
        case PS_TOP_LEVEL_EXPECT_START_OR_VALUE:
            switch (tok->kind)
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
                else if (trove->lastAnnoToken && 
                    tok->line == trove->lastAnnoToken->line)
                    { associateComment(trove, NULL, tok); }
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
                    { recordParseError(trove, HU_ERROR_TOOMANYROOTS, tok); }
                else
                {
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_LIST, tok);
                    huSize_t nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setValueToken(nodeCreatedThisState, tok);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNodeByIndex(trove, nctsIdx);
                }
                break;

            case HU_TOKENKIND_STARTDICT:
                // if nodeCreatedThisState exists, err
                // make new dict node
                // assign comment queue to it
                // nodeCreatedThisState = new node
                // recursive(PS_IN_DICT_EXPECT_KEY_OR_END, nodeCreatedThisState)
                if (nodeCreatedThisState)
                    { recordParseError(trove, HU_ERROR_TOOMANYROOTS, tok); }
                else
                {
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_DICT, tok);
                    huSize_t nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setValueToken(nodeCreatedThisState, tok);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNodeByIndex(trove, nctsIdx);
                }
                break;

            case HU_TOKENKIND_WORD:
                // if nodeCreatedThisState exists, err
                // make new value node
                // assign comment queue to it
                // nodeCreatedThisState = new node
                if (nodeCreatedThisState)
                    { recordParseError(trove, HU_ERROR_TOOMANYROOTS, tok); }
                else
                {
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_VALUE, tok);
                    huSize_t nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setValueToken(nodeCreatedThisState, tok);
                    setLastValueToken(nodeCreatedThisState, tok);

                    nodeCreatedThisState = (huNode *) huGetNodeByIndex(trove, nctsIdx);
                }
                break;

            case HU_TOKENKIND_ANNOTATE:
                // if nodeCreatedThisState, assign comment queue to it
                // else assign comment queue to the trove (we encountered a trove annotation)
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, nodeCreatedThisState)

                // NOTE: nodeCreatedThisState will be NULL if we're in a trove annotation!
                associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                ensureContains(trove, nodeCreatedThisState, tok);
                parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                    PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                break;

            default:
                // report error
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
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
                    huSize_t parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_LIST, tok);
                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                    huSize_t nctsIdx = nodeCreatedThisState->nodeIdx;

                    setValueToken(nodeCreatedThisState, tok);
                    addChildNode(trove, parentNode, nodeCreatedThisState);
                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNodeByIndex(trove, nctsIdx);
                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                }
                break;

            case HU_TOKENKIND_STARTDICT:
                // make new dict node
                // assign comment queue to it
                // assign new node to parentNode
                // nodeCreatedThisState = new node
                // recursive(PS_IN_DICT_EXPECT_KEY_OR_END, nodeCreatedThisState)
                {
                    huSize_t parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_DICT, tok);
                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                    huSize_t nctsIdx = nodeCreatedThisState->nodeIdx;

                    setValueToken(nodeCreatedThisState, tok);
                    addChildNode(trove, parentNode, nodeCreatedThisState);
                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNodeByIndex(trove, nctsIdx);
                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                }
                break;

            case HU_TOKENKIND_WORD:
                // make new value node
                // assign comment queue to it
                // assign new node to parentNode
                // nodeCreatedThisState = new node
                {
                    huSize_t parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_VALUE, tok);
                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);

                    setValueToken(nodeCreatedThisState, tok);
                    setLastValueToken(nodeCreatedThisState, tok);
                    addChildNode(trove, parentNode, nodeCreatedThisState);
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
                    ensureContains(trove, annoTarget, tok);
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
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_DICT_EXPECT_KEY_OR_END:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF:
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
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
                    huSize_t parentIdx = parentNode->nodeIdx;
                    nodeCreatedThisState = allocNewNode(trove, HU_NODEKIND_NULL, tok);
                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                    huSize_t nctsIdx = nodeCreatedThisState->nodeIdx;

                    associateEnqueuedComments(trove, nodeCreatedThisState, commentQueue);
                    setKeyToken(nodeCreatedThisState, tok);
                    addChildNode(trove, parentNode, nodeCreatedThisState);
                    parseTroveRecursive(trove, tokenIdx, nodeCreatedThisState, depth + 1, 
                        PS_IN_DICT_EXPECT_KVS, commentQueue);

                    nodeCreatedThisState = (huNode *) huGetNodeByIndex(trove, nctsIdx);
                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                }
                break;

            case HU_TOKENKIND_ANNOTATE:
                // if nodeCreatedThisState, assign comment queue to it
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, nodeCreatedThisState || parentNode)
                {
                    huNode * annoTarget = nodeCreatedThisState;
                    if (annoTarget == NULL)
                        { annoTarget = parentNode; }
                    ensureContains(trove, annoTarget, tok);
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
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_DICT_EXPECT_KVS:
            // parentNode is the node we created when we encountered the key token.
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_KEYVALUESEP:
                // recursive(PS_IN_DICT_EXPECT_START_OR_VALUE)
                {
                    huSize_t parentIdx = parentNode->nodeIdx;

                    ensureContains(trove, parentNode, tok);
                    parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                        PS_IN_DICT_EXPECT_START_OR_VALUE, commentQueue);

                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                }
                return;

            case HU_TOKENKIND_ANNOTATE:
                // recursive(PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, parentNode)
                ensureContains(trove, parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                break;

            default:
                // report error
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_DICT_EXPECT_START_OR_VALUE:
            // parentNode is the node we created when we encountered the key token.
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_STARTLIST:
                // set parentNode to list kind
                // recursive(PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, parentNode)
                {
                    huSize_t parentIdx = parentNode->nodeIdx;

                    parentNode->kind = HU_NODEKIND_LIST;
                    setValueToken(parentNode, tok);
                    parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                        PS_IN_LIST_EXPECT_START_OR_VALUE_OR_END, commentQueue);

                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
                }
                return;

            case HU_TOKENKIND_STARTDICT:
                // set parentNode to dict kind
                // recursive(PS_IN_DICT_EXPECT_KEY_OR_END, parentNode)
                {
                    huSize_t parentIdx = parentNode->nodeIdx;

                    parentNode->kind = HU_NODEKIND_DICT;
                    setValueToken(parentNode, tok);
                    parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                        PS_IN_DICT_EXPECT_KEY_OR_END, commentQueue);

                    parentNode = (huNode *) huGetNodeByIndex(trove, parentIdx);
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
                ensureContains(trove, parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY, commentQueue);
                break;

            default:
                // report error
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNO_EXPECT_DICTSTART_OR_KEY:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_STARTDICT:
                // recursive(PS_IN_ANNODICT_EXPECT_KEY_OR_END, parentNode)
                ensureContains(trove, parentNode, tok);
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
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNO_EXPECT_KVS:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_KEYVALUESEP:
                // recursive(PS_IN_ANNO_EXPECT_VALUE, parentNode)
                ensureContains(trove, parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNO_EXPECT_VALUE, commentQueue);
                return;

            default:
                // report error
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNO_EXPECT_VALUE:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
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
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNODICT_EXPECT_KEY_OR_END:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
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
                ensureContains(trove, parentNode, tok);
                return;

            default:
                // report error
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNODICT_EXPECT_KVS:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
                return;

            case HU_TOKENKIND_COMMENT:
                // associate to parentNode
                associateComment(trove, parentNode, tok);
                break;

            case HU_TOKENKIND_KEYVALUESEP:
                // recursive(PS_IN_ANNODICT_EXPECT_VALUE, parentNode)
                ensureContains(trove, parentNode, tok);
                parseTroveRecursive(trove, tokenIdx, parentNode, depth + 1, 
                    PS_IN_ANNODICT_EXPECT_VALUE, commentQueue);
                return;

            default:
                // report error
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;

        case PS_IN_ANNODICT_EXPECT_VALUE:
            switch (tok->kind)
            {
            case HU_TOKENKIND_EOF: 
                recordParseError(trove, HU_ERROR_UNEXPECTEDEOF, tok);
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
                recordParseError(trove, HU_ERROR_SYNTAXERROR, tok);
                break;
            }
            break;
        }    
    }
}

void parseTrove(huTrove * trove)
{
#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf("%sParsing:%s\n%s\n%s",
        ansi_darkGreen, ansi_darkGray, trove->dataString, ansi_off);
#endif

    huVector commentQueue;
    initGrowableVector(& commentQueue, sizeof(huToken *), & trove->allocator);

    huSize_t tokenIdx = 0;
    parseTroveRecursive(trove, & tokenIdx, NULL, 0, PS_TOP_LEVEL_EXPECT_START_OR_VALUE, & commentQueue);
    associateEnqueuedComments(trove, NULL, & commentQueue);
}
