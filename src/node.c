#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "humon.internal.h"


void huInitNode(huNode * node, huTrove const * trove)
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
    huInitVector(& node->childDictKeys, sizeof(huDictEntry));
    huInitVector(& node->annotations, sizeof(huAnnotation));
    huInitVector(& node->comments, sizeof(huComment));
}


void huDestroyNode(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return; }
#endif

    huDestroyVector(& node->childNodeIdxs);
    huDestroyVector(& node->childDictKeys);
    huDestroyVector(& node->annotations);
    huDestroyVector(& node->comments);
}


huNode const * huGetParentNode(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return & humon_nullNode; }
#endif

    return huGetNode(node->trove, node->parentNodeIdx);
}


int huGetNumChildren(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return 0; }
#endif

    return node->childNodeIdxs.numElements;
}


huNode const * huGetChildNodeByIndex(huNode const * node, int childIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || childIdx < 0)
        { return & humon_nullNode; }
#endif

    if (childIdx >= huGetNumChildren(node))
        { return & humon_nullNode; }

    return huGetNode(node->trove, 
        * (int *) huGetVectorElement(& node->childNodeIdxs, childIdx));
}


huNode const * huGetChildNodeByKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return & humon_nullNode; }
#endif

    return huGetChildNodeByKeyN(node, key, strlen(key));
}


huNode const * huGetChildNodeByKeyN(huNode const * node, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || key == NULL || keyLen < 0)
        { return & humon_nullNode; }
#endif

    if (node->kind != HU_NODEKIND_DICT)
        { return & humon_nullNode; }

    huDictEntry * dictEntries = (huDictEntry *) huGetVectorElement(
        & node->childDictKeys, 0);
    for (int i = 0; i < huGetNumChildren(node); ++i)
    {
        huDictEntry * dictEntry = dictEntries + i;
        if (keyLen == dictEntry->key->value.size &&
            strncmp(dictEntry->key->value.str, key, keyLen) == 0)
        {
           return huGetChildNodeByIndex(node, dictEntry->idx);
        }
    }

    return & humon_nullNode;
}


bool huHasKey(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return false; }
#endif

    return node->keyToken->tokenKind != HU_TOKENKIND_NULL;
}


huToken const * huGetKey(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return & humon_nullToken; }
#endif

    return node->keyToken;
}


bool huHasValue(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return false; }
#endif

    return node->firstValueToken->tokenKind != HU_TOKENKIND_NULL;
}


huNode const * huNextSibling(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return & humon_nullNode; }
#endif

    if (node->parentNodeIdx != -1)
    {
        huNode const * parentNode = huGetParentNode(node);

        if (huGetNumChildren(parentNode) > node->childIdx + 1)
        {
           return huGetChildNodeByIndex(parentNode, node->childIdx + 1);
        }
    }

    return & humon_nullNode;
}


int huGetNumAnnotations(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return 0; }
#endif

    return node->annotations.numElements;
}


huAnnotation const * huGetAnnotation(huNode const * node, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || annotationIdx < 0)
        { return NULL; }
#endif

    if (annotationIdx < node->annotations.numElements)
        { return (huAnnotation *) node->annotations.buffer + annotationIdx; }
    else
        { return NULL; }
}


bool huHasAnnotationWithKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return false; }
#endif

    return huHasAnnotationWithKeyN(node, key, strlen(key));
}


bool huHasAnnotationWithKeyN(huNode const * node, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || key == NULL || keyLen < 0)
        { return false; }
#endif

    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation *) node->annotations.buffer + i;
        if (keyLen == anno->key->value.size &&
            strncmp(anno->key->value.str, key, keyLen) == 0)
            { return true; }
    }

    return false;
}


huToken const * huGetAnnotationByKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return & humon_nullToken; }
#endif

    return huGetAnnotationByKeyN(node, key, strlen(key));
}


huToken const * huGetAnnotationByKeyN(huNode const * node, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || key == NULL || keyLen < 0)
        { return & humon_nullToken; }
#endif

    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (keyLen == anno->key->value.size &&
            strncmp(anno->key->value.str, key, keyLen) == 0)
            { return anno->value; }
    }

    return & humon_nullToken;
}


int huGetNumAnnotationsByValueZ(huNode const * node, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    return huGetNumAnnotationsByValueN(node, value, strlen(value));
}


int huGetNumAnnotationsByValueN(huNode const * node, char const * value, int valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || value == NULL || valueLen < 0)
        { return 0; }
#endif

    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (valueLen == anno->value->value.size &&
            strncmp(anno->value->value.str, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetAnnotationByValueZ(huNode const * node, char const * value, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return & humon_nullToken; }
#endif

    return huGetAnnotationByValueN(node, value, strlen(value), annotationIdx);
}


huToken const * huGetAnnotationByValueN(huNode const * node, char const * value, int valueLen, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || value == NULL || valueLen < 0 || annotationIdx < 0)
        { return & humon_nullToken; }
#endif

    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation *) node->annotations.buffer + i;
        if (valueLen == anno->value->value.size &&
            strncmp(anno->value->value.str, value, valueLen) == 0)
        {
            if (matches == annotationIdx)
                { return anno->key; }

            matches += 1;
        }
    }

    return & humon_nullToken;
}


int huGetNumComments(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
        { return 0; }
#endif

    return node->comments.numElements;
}


huToken const * huGetComment(huNode const * node, int commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || commentIdx < 0)
        { return & humon_nullToken; }
#endif

    if (commentIdx < node->comments.numElements)
        { return ((huComment const *) node->comments.buffer + commentIdx)->commentToken; }
    else
        { return & humon_nullToken; }
}


huVector huGetCommentsContainingZ(huNode const * node, char const * containedText)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
    {
        huVector nodesVect;
        huInitVector(& nodesVect, sizeof(huNode *));
        return nodesVect;
    }
#endif

    return huGetCommentsContainingN(node, containedText, strlen(containedText));
}


huVector huGetCommentsContainingN(huNode const * node, char const * containedText, int containedTextLen)
{
    huVector nodesVect;
    huInitVector(& nodesVect, sizeof(huToken *));

#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || containedText == NULL || containedTextLen < 0)
        { return nodesVect; }
#endif

    int na = huGetNumComments(node);
    for (int i = 0; i < na; ++i)
    {
        huToken const * comm = huGetComment(node, i);
        if (stringInString(comm->value.str, comm->value.size, 
                containedText, containedTextLen))
        {
            huToken const ** pn = huGrowVector(& nodesVect, 1);                
            * pn = comm;
        }
    }

    return nodesVect;
}


void eatAddressWord(huCursor * cursor, int * len, int * col)
{
    // The first character is already confirmed a word char, so, next please.
    * len += cursor->charLength;
    * col += 1;
    nextCharacter(cursor);

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->character[0] == '\0')
            { eating = false; }
        else if (cursor->ws_line || cursor->ws_col)
            { eating = false; }
        else
        {
            switch(cursor->character[0])
            {
            case '{': case '}': case '[': case ']':
            case ':': case '@': case '#': case '/': // adding '/' here because address
                eating = false;
                break;
            default:
                * len += cursor->charLength;
                * col += 1;
                nextCharacter(cursor);
                break;
            }
        }
    }    
}


void eatQuotedAddressWord(huCursor * cursor, char quoteChar, int * len, int * col, int * error)
{
    // The first character is already confirmed quoteChar, so, next please.
    * col += 1;
    nextCharacter(cursor);

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->character[0] == '\0')
        { 
          if (error)
            { * error = HU_ERROR_UNFINISHED_QUOTE; } 
          eating = false;
        }
        else if (cursor->ws_line)
        {
            * len += cursor->charLength;
            * col = 1;
            nextCharacter(cursor);
        }
        else
        {
            * col += 1;

            if (cursor->character[0] == quoteChar)
            {
                nextCharacter(cursor);
                eating = false;
            }
            else if (cursor->character[0] != '\\' && 
                cursor->character[1] == quoteChar)
            {
                nextCharacter(cursor);
                * len += 1;
                * col += 1;
                nextCharacter(cursor);
                eating = false;
            }
            else
            {
                * len += cursor->charLength;
                nextCharacter(cursor);
            }
        }
    }
}


huNode const * huGetNodeByRelativeAddressZ(huNode const * node, char const * address, int * error)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
    {
        if (error) { * error = HU_ERROR_ILLEGAL; }
        return & humon_nullNode;
    }
#endif

    return huGetNodeByRelativeAddressN(node, address, strlen(address), error);
}


/*
    foo/3/bar
    "foo"/"3"/"bar"
    foo\u2000/3/bar
    error can be NULL
*/
huNode const * huGetNodeByRelativeAddressN(huNode const * node, char const * address, int addressLen, int * error)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode || address == NULL || addressLen < 0)
    {
        if (error) { * error = HU_ERROR_ILLEGAL; }
        return & humon_nullNode;
    }
#endif

    if (error) { * error = HU_ERROR_NO_ERROR; }

    // When the last node is reached in the address, we're it.
    if (addressLen == 0)
        { return node; }

    // malformed
    if (address[0] == '/')
        { if (error) { * error = HU_ERROR_SYNTAX_ERROR; } return & humon_nullNode; }

    huCursor cur = 
        { .trove = NULL, 
          .character = address, 
          .charLength = getcharLength(address) };
    int line = 0;  // unused
    int col = 0;

    eatWs(& cur, 1, & line, & col);
    int len = 0;
    char const * wordStart = address + col;
    bool quoted = false;
    switch(* cur.character)
    {
    case '\0':
        return node;
    case '"':
        eatQuotedAddressWord(& cur, '"', & len, & col, error);
        wordStart += 1;
        quoted = true;
        break;
    case '\'':
        eatQuotedAddressWord(& cur, '\'', & len, & col, error);
        wordStart += 1;
        quoted = true;
        break;
    case '`':
        eatQuotedAddressWord(& cur, '`', & len, & col, error);
        wordStart += 1;
        quoted = true;
        break;
    default:
        eatAddressWord(& cur, & len, & col);
        break;
    }

    if (* error != HU_ERROR_NO_ERROR)
       { return & humon_nullNode; }

    huNode const * nextNode = & humon_nullNode;
    // if '..', go up a level if we can
    if (quoted == false && len == 2 && wordStart[0] == '.' && wordStart[1] == '.')
        { nextNode = huGetParentNode(node); }
    else
    {
        // if numeric, and not quoted, use as an index
        char * wordEnd;
        int index = strtol(wordStart, & wordEnd, 10);
        if (quoted == false && wordEnd - wordStart == len)
            { nextNode = huGetChildNodeByIndex(node, index); }
        else
            { nextNode = huGetChildNodeByKeyN(node, wordStart, len); }   
    }
    // If the key or index is invalid, nextNode wil be set to & humon_nullNode.
    if (nextNode->kind == HU_NODEKIND_NULL)
        { if (error) { * error = HU_ERROR_NOTFOUND; } return & humon_nullNode; }
    
    eatWs(& cur, 1, & line, & col);

    if (address[col] == '\0')
        { return nextNode; }
    else if (address[col] == '/')
    {
        return huGetNodeByRelativeAddressN(
            nextNode, address + col + 1, addressLen - col - 1, error);
    }
    else
      { if (error) { * error = HU_ERROR_SYNTAX_ERROR; } return & humon_nullNode; }
}


// This is kinda fugly. But for most cases (x < 1000) it's probably fine.
int log10i(unsigned int x)
{
         if (x < 1 * 10) { return 0; }
    else if (x < 1 * 100) { return 1; }
    else if (x < 1 * 1000) { return 2; }
    else if (x < 1 * 1000 * 10) { return 3; }
    else if (sizeof(int) == 2) { return 4; }
    else if (x < 1 * 1000 * 100) { return 4; }
    else if (x < 1 * 1000 * 1000) { return 5; }
    else if (x < 1 * 1000 * 1000 * 10) { return 6; }
    else if (x < 1 * 1000 * 1000 * 100) { return 7; }
    else if (x < 1 * 1000 * 1000 * 1000) { return 8; }
    else if (sizeof(int) == 4) { return 9; }
    else if (x < 1L * 1000 * 1000 * 1000 * 10) { return 9; }
    else if (x < 1L * 1000 * 1000 * 1000 * 100) { return 10; }
    else if (x < 1L * 1000 * 1000 * 1000 * 1000) { return 11; }
    else if (x < 1L * 1000 * 1000 * 1000 * 1000 * 10) { return 12; }
    else if (x < 1L * 1000 * 1000 * 1000 * 1000 * 100) { return 13; }
    else if (x < 1L * 1000 * 1000 * 1000 * 1000 * 1000) { return 14; }
    else if (x < 1L * 1000 * 1000 * 1000 * 1000 * 1000 * 10) { return 15; }
    else if (x < 1L * 1000 * 1000 * 1000 * 1000 * 1000 * 100) { return 16; }
    else if (x < 1L * 1000 * 1000 * 1000 * 1000 * 1000 * 1000) { return 17; }
    else { return 18; }
}

// TODO: doc how this guy returns a null string
huStringView huGetNodeAddress(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == & humon_nullNode)
    {
        huStringView sv = { .str = NULL, .size = 0 };
        return sv;
    }
#endif

    // measure, alloc, fill
    int addressLen = 0;

    huNode const * n = node;
    if (n->parentNodeIdx == -1)
        { addressLen = 1; }

    while (n->parentNodeIdx != -1)
    {
        huNode const * parentN = huGetParentNode(n);
        if (parentN->kind == HU_NODEKIND_DICT)
        {
            addressLen += huGetKey(n)->value.size;
        }
        else if (parentN->kind == HU_NODEKIND_LIST)
        {
            addressLen += log10i((unsigned int) n->childIdx) + 1;
        }

        addressLen += 1;  // for the '/'!

        n = parentN;
    }

    char * str = malloc(addressLen + 1);
    if (str == NULL)
    {
        huStringView sv = { .str = NULL, .size = 0 };
        return sv;
    }

    str[addressLen] = '\0';
    huStringView sv = { .str = str, .size = addressLen };
    char * cur = str + addressLen;
    n = node;
    if (n->parentNodeIdx == -1)
    {
        str[0] = '/';
        cur = str;
    }
    while (n->parentNodeIdx != -1)
    {
        huNode const * parentN = huGetParentNode(n);
        if (parentN->kind == HU_NODEKIND_DICT)
        {
            huStringView const * key = & huGetKey(n)->value;
            cur -= key->size;
            memcpy(cur, key->str, key->size);
        }
        else if (parentN->kind == HU_NODEKIND_LIST)
        {
            int numBytes = log10i((unsigned int) n->childIdx) + 1;
            int cv = n->childIdx;
            for (int i = 0; i < numBytes; ++i)
            {
                cur -= 1;
                * cur = '0' + cv % 10;
                cv /= 10;
            }
        }

        cur -= 1;
        * cur = '/';

        n = parentN;
    }

    if (str[addressLen] != '\0' || cur != str)
    {
        free((void *) sv.str);
        sv.str = NULL;
        sv.size = 0;
        return sv;
    }

    return sv;
}
