#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "humon.internal.h"


void huInitNode(huNode * node, huTrove * trove)
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


void huDestroyNode(huNode * node)
{
    huDestroyVector(& node->childNodeIdxs);
    huDestroyVector(& node->childDictKeys);
    huDestroyVector(& node->annotations);
    huDestroyVector(& node->comments);
}


huNode * huGetParentNode(huNode * node)
{
    return huGetNode(node->trove, node->parentNodeIdx);
}


int huGetNumChildren(huNode * node)
{
    return node->childNodeIdxs.numElements;
}


huNode * huGetChildNodeByIndex(huNode * node, int childIdx)
{
    if (childIdx < 0 || childIdx >= huGetNumChildren(node))
        { return & humon_nullNode; }

    return huGetNode(node->trove, 
        * (int *) huGetVectorElement(& node->childNodeIdxs, childIdx));
}


huNode * huGetChildNodeByKeyZ(huNode * node, char const * key)
{
    return huGetChildNodeByKeyN(node, key, strlen(key));
}


huNode * huGetChildNodeByKeyN(huNode * node, char const * key, int keyLen)
{
    huStringView keyView = { key, keyLen };

    huDictEntry * dicts = (huDictEntry *) huGetVectorElement(
        & node->childDictKeys, 0);
    for (int i = 0; i < huGetNumChildren(node); ++i)
    {
        huDictEntry * dict = dicts + i;
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

bool huHasKey(huNode * node)
{
    return node->keyToken->tokenKind != HU_TOKENKIND_NULL;
}


huToken * huGetKey(huNode * node)
{
    return node->keyToken;
}


bool huHasValue(huNode * node)
{
    return node->firstValueToken->tokenKind != HU_TOKENKIND_NULL;
}


huToken * huGetValue(huNode * node)
{
    return node->firstValueToken;
}


huToken * huGetStartToken(huNode * node)
{
    return node->firstValueToken;
}


huToken * huGetEndToken(huNode * node)
{
    return node->lastValueToken;
}


huNode * huNextSibling(huNode * node)
{
    if (node->parentNodeIdx != -1)
    {
        huNode * parentNode = huGetParentNode(node);

        if (huGetNumChildren(parentNode) > node->childIdx + 1)
        {
           return huGetChildNodeByIndex(parentNode, node->childIdx + 1);
        }
    }

    return & humon_nullNode;
}


int huGetNumAnnotations(huNode * node)
{
    return node->annotations.numElements;
}


huAnnotation * huGetAnnotation(huNode * node, int annotationIdx)
{
    if (annotationIdx < node->annotations.numElements)
        { return (huAnnotation *) node->annotations.buffer + annotationIdx; }
    else
        { return NULL; }
}


int huGetNumAnnotationsByKeyZ(huNode * node, char const * key)
{
    return huGetNumAnnotationsByKeyN(node, key, strlen(key));
}


int huGetNumAnnotationsByKeyN(huNode * node, char const * key, int keyLen)
{
    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) node->annotations.buffer + i;
        if (strncmp(anno->key->value.str, key, keyLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huAnnotation * huGetAnnotationByKeyZ(huNode * node, char const * key, int annotationIdx)
{
    return huGetAnnotationByKeyN(node, key, strlen(key), annotationIdx);
}


huAnnotation * huGetAnnotationByKeyN(huNode * node, char const * key, int keyLen, int annotationIdx)
{
    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) node->annotations.buffer + i;
        if (strncmp(anno->key->value.str, key, keyLen) == 0)
        {
            if (matches == annotationIdx)
                { return anno; }

            matches += 1;
        }
    }

    return NULL;
}


int huGetNumAnnotationsByValueZ(huNode * node, char const * value)
{
    return huGetNumAnnotationsByValueN(node, value, strlen(value));
}


int huGetNumAnnotationsByValueN(huNode * node, char const * value, int valueLen)
{
    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) node->annotations.buffer + i;
        if (strncmp(anno->value->value.str, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huAnnotation * huGetAnnotationByValueZ(huNode * node, char const * value, int annotationIdx)
{
    return huGetAnnotationByValueN(node, value, strlen(value), annotationIdx);
}


huAnnotation * huGetAnnotationByValueN(huNode * node, char const * value, int valueLen, int annotationIdx)
{
    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) node->annotations.buffer + i;
        if (strncmp(anno->value->value.str, value, valueLen) == 0)
        {
            if (matches == annotationIdx)
                { return anno; }

            matches += 1;
        }
    }

    return NULL;
}


int huGetNumComments(huNode * node)
{
    return node->comments.numElements;
}


huComment * huGetComment(huNode * node, int commentIdx)
{
    if (commentIdx < node->comments.numElements)
        { return (huComment *) node->comments.buffer + commentIdx; }
    else
        { return NULL; }
}


void eatAddressWord(huCursor * cursor, int * len, int * col)
{
    // The first character is already confirmed a word char, so, next please.
    * len += cursor->codepointLength;
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
                * len += cursor->codepointLength;
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
            * len += cursor->codepointLength;
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
                * len += cursor->codepointLength;
                nextCharacter(cursor);
            }
        }
    }
}


void eatSingleQuotedAddressWord(huCursor * cur, int * len, int * col, int * error)
{
    eatQuotedAddressWord(cur, '\'', len, col, error);
}


void eatDoubleQuotedAddressWord(huCursor * cur, int * len, int * col, int * error)
{
    eatQuotedAddressWord(cur, '"', len, col, error);
}


void eatBackQuotedAddressWord(huCursor * cur, int * len, int * col, int * error)
{
    eatQuotedAddressWord(cur, '`', len, col, error);
}


huNode * huGetNodeByRelativeAddressZ(huNode * node, char const * address, int * error)
{
    return huGetNodeByRelativeAddressN(node, address, strlen(address), error);
}


/*
    foo/3/bar
    "foo"/"3"/"bar"
    foo\u2000/3/bar
*/
huNode * huGetNodeByRelativeAddressN(huNode * node, char const * address, int addressLen, int * error)
{
    if (error) { * error = HU_ERROR_NO_ERROR; }

    // When the last node is reached in the address, we're it.
    if (addressLen == 0)
        { return node; }

    if (addressLen < 0)
        { if (error) { * error = HU_ERROR_UNEXPECTED_EOF; } return & humon_nullNode; }

    // malformed
    if (address[0] == '/')
        { if (error) { * error = HU_ERROR_SYNTAX_ERROR; } return & humon_nullNode; }

    huCursor cur = 
        { .trove = NULL, 
          .character = address, 
          .codepointLength = getCodepointLength(address) };
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
        eatDoubleQuotedAddressWord(& cur, & len, & col, error);
        quoted = true;
        break;
    case '\'':
        eatSingleQuotedAddressWord(& cur, & len, & col, error);
        quoted = true;
        break;
    case '`':
        eatBackQuotedAddressWord(& cur, & len, & col, error);
        quoted = true;
        break;
    default:
        eatAddressWord(& cur, & len, & col);
        break;
    }

    if (error)
       { return & humon_nullNode; }

    huNode * childNode = & humon_nullNode;
    // if numeric, and not quoted, use as an index
    char * wordEnd;
    int index = strtol(address + col, & wordEnd, 10);
    if (quoted == false && wordEnd - wordStart == len)
        { childNode = huGetChildNodeByIndex(node, index); }
    else
        { childNode = huGetChildNodeByKeyN(node, wordStart, len); }   
    // If the key or index is invalid, childNode wil be set to & humon_nullNode.
    if (childNode->kind == HU_NODEKIND_NULL)
        { if (error) { * error = HU_ERROR_NOTFOUND; } return & humon_nullNode; }
    
    eatWs(& cur, 1, & line, & col);

    if (address[col] == '\0')
        { return childNode; }
    else if (address[col] == '/')
    {
        return huGetNodeByRelativeAddressN(
            childNode, address + col + 1, addressLen - col - 1, error);
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


huStringView huGetNodeAddress(huNode * node)
{
    // measure, alloc, fill
    int addressLen = 0;

    huNode * n = node;
    while (n != & humon_nullNode)
    {
        huNode * parentN = huGetParentNode(n);
        if (parentN->kind == HU_NODEKIND_LIST)
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
    while (n != & humon_nullNode)
    {
        huNode * parentN = huGetParentNode(n);
        if (parentN->kind == HU_NODEKIND_LIST)
        {
            huStringView * key = & huGetKey(n)->value;
            cur -= addressLen;
            memcpy(cur, key->str, key->size);
        }
        else if (parentN->kind == HU_NODEKIND_LIST)
        {
            cur -= log10i((unsigned int) n->childIdx) + 1;
            strtol(cur, NULL, 10);
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
