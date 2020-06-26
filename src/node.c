#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "humon.internal.h"


void huInitNode(huNode * node, huTrove const * trove)
{
    node->trove = trove;
    node->nodeIdx = -1;
    node->kind = HU_NODEKIND_NULL;
    node->firstToken = hu_nullToken;
    node->keyToken = hu_nullToken;
    node->valueToken = hu_nullToken;
    node->lastValueToken = hu_nullToken;
    node->lastToken = hu_nullToken;
    node->childOrdinal = 0;
    node->parentNodeIdx = -1;
    huInitVector(& node->childNodeIdxs, sizeof(int));
    huInitVector(& node->annotations, sizeof(huAnnotation));
    huInitVector(& node->comments, sizeof(huComment));
}


void huDestroyNode(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return; }
#endif

    huDestroyVector(& node->childNodeIdxs);
    huDestroyVector(& node->annotations);
    huDestroyVector(& node->comments);
}


huNode const * huGetParentNode(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return hu_nullNode; }
#endif

    return huGetNode(node->trove, node->parentNodeIdx);
}


int huGetNumChildren(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return 0; }
#endif

    return node->childNodeIdxs.numElements;
}


huNode const * huGetChildByIndex(huNode const * node, int childOrdinal)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || childOrdinal < 0)
        { return hu_nullNode; }
#endif

    if (childOrdinal >= huGetNumChildren(node))
        { return hu_nullNode; }

    return huGetNode(node->trove, 
        * (int *) huGetVectorElement(& node->childNodeIdxs, childOrdinal));
}


huNode const * huGetChildByKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return hu_nullNode; }
#endif

    return huGetChildByKeyN(node, key, strlen(key));
}


huNode const * huGetChildByKeyN(huNode const * node, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || key == NULL || keyLen < 0)
        { return hu_nullNode; }
#endif

    if (node->kind != HU_NODEKIND_DICT)
        { return hu_nullNode; }

    for (int i = 0; i < huGetNumChildren(node); ++i)
    {
        huNode const * childNode = huGetChildByIndex(node, i);
        if (keyLen == childNode->keyToken->str.size &&
            strncmp(childNode->keyToken->str.str, key, keyLen) == 0)
            { return childNode; }
    }

    return hu_nullNode;
}


huNode const * huGetFirstChild(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return hu_nullNode; }
#endif

    if (huGetNumChildren(node) == 0)
        { return hu_nullNode; }

    return huGetNode(node->trove, 
        * (int *) huGetVectorElement(& node->childNodeIdxs, 0));
}


huNode const * huGetNextSibling(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return hu_nullNode; }
#endif

    if (node->parentNodeIdx != -1)
    {
        huNode const * parentNode = huGetParentNode(node);

        if (huGetNumChildren(parentNode) > node->childOrdinal + 1)
        {
           return huGetChildByIndex(parentNode, node->childOrdinal + 1);
        }
    }

    return hu_nullNode;
}


bool huHasKey(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return false; }
#endif

    return node->keyToken != hu_nullToken;
}


bool huHasValue(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return false; }
#endif

    return node->valueToken != hu_nullToken;
}


int huGetNumAnnotations(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return 0; }
#endif

    return node->annotations.numElements;
}


huAnnotation const * huGetAnnotation(huNode const * node, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || annotationIdx < 0)
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
    if (node == NULL || node == hu_nullNode || key == NULL || keyLen < 0)
        { return false; }
#endif

    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation *) node->annotations.buffer + i;
        if (keyLen == anno->key->str.size &&
            strncmp(anno->key->str.str, key, keyLen) == 0)
            { return true; }
    }

    return false;
}


huToken const * huGetAnnotationByKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return hu_nullToken; }
#endif

    return huGetAnnotationByKeyN(node, key, strlen(key));
}


huToken const * huGetAnnotationByKeyN(huNode const * node, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || key == NULL || keyLen < 0)
        { return hu_nullToken; }
#endif

    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (keyLen == anno->key->str.size &&
            strncmp(anno->key->str.str, key, keyLen) == 0)
            { return anno->value; }
    }

    return hu_nullToken;
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
    if (node == NULL || node == hu_nullNode || value == NULL || valueLen < 0)
        { return 0; }
#endif

    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (valueLen == anno->value->str.size &&
            strncmp(anno->value->str.str, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetAnnotationByValueZ(huNode const * node, char const * value, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return hu_nullToken; }
#endif

    return huGetAnnotationByValueN(node, value, strlen(value), annotationIdx);
}


huToken const * huGetAnnotationByValueN(huNode const * node, char const * value, int valueLen, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || value == NULL || valueLen < 0 || annotationIdx < 0)
        { return hu_nullToken; }
#endif

    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation *) node->annotations.buffer + i;
        if (valueLen == anno->value->str.size &&
            strncmp(anno->value->str.str, value, valueLen) == 0)
        {
            if (matches == annotationIdx)
                { return anno->key; }

            matches += 1;
        }
    }

    return hu_nullToken;
}


int huGetNumComments(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return 0; }
#endif

    return node->comments.numElements;
}


huToken const * huGetComment(huNode const * node, int commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || commentIdx < 0)
        { return hu_nullToken; }
#endif

    if (commentIdx < node->comments.numElements)
        { return ((huComment const *) node->comments.buffer + commentIdx)->commentToken; }
    else
        { return hu_nullToken; }
}


huToken const * huGetCommentsContainingZ(huNode const * node, char const * containedText, huToken const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
        { return hu_nullToken; }
#endif

    return huGetCommentsContainingN(node, containedText, strlen(containedText), startWith);
}


huToken const * huGetCommentsContainingN(huNode const * node, char const * containedText, int containedTextLen, huToken const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || containedText == NULL || containedTextLen < 0)
        { return hu_nullToken; }
#endif

    bool foundLast = startWith == NULL;

    int na = huGetNumComments(node);
    for (int i = 0; i < na; ++i)
    {
        huToken const * comm = huGetComment(node, i);
        if (foundLast && stringInString(comm->str.str, comm->str.size, 
                containedText, containedTextLen))
            { return comm; }

        if (foundLast == false && comm == startWith)
            { foundLast = true; }
    }

    return hu_nullToken;
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


void eatQuotedAddressWord(huCursor * cursor, char quoteChar, int * len, int * col)
{
    // The first character is already confirmed quoteChar, so, next please.
    * col += 1;
    nextCharacter(cursor);

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->character[0] == '\0')
            { eating = false; }
        else if (cursor->ws_line)
        {
            * len += cursor->charLength;
            * col = 1;
            nextCharacter(cursor);
        }
        else
        {
            if (cursor->character[0] == quoteChar)
            {
                * col += 1;
                nextCharacter(cursor);
                eating = false;
            }
            else if (cursor->character[0] == '\\' && 
                cursor->character[1] == quoteChar)  // TODO: ensure not running off the end
            {
                * col += 1;
                * len += 1;
                nextCharacter(cursor);
                * col += 1;
                * len += 1;
                nextCharacter(cursor);
            }
            else
            {
                * col += 1;
                * len += cursor->charLength;
                nextCharacter(cursor);
            }
        }
    }
}


huNode const * huGetNodeByRelativeAddressZ(huNode const * node, char const * address)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
        { return hu_nullNode; }
#endif

    return huGetNodeByRelativeAddressN(node, address, strlen(address));
}


/*
    foo/3/bar
    "foo"/"3"/"bar"
    foo\u2000/3/bar
    error can be NULL
*/
huNode const * huGetNodeByRelativeAddressN(huNode const * node, char const * address, int addressLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || address == NULL || addressLen < 0)
        { return hu_nullNode; }
#endif

    // When the last node is reached in the address, we're it.
    if (addressLen == 0)
        { return node; }

    // malformed
    if (address[0] == '/')
        { return hu_nullNode; }

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
    bool doubleQuoted = false;
    switch(* cur.character)
    {
    case '\0':
        return node;
    case '"':
        eatQuotedAddressWord(& cur, '"', & len, & col);
        wordStart += 1;
        quoted = true;
        doubleQuoted = true;
        break;
    case '\'':
        eatQuotedAddressWord(& cur, '\'', & len, & col);
        wordStart += 1;
        quoted = true;
        break;
    case '`':
        eatQuotedAddressWord(& cur, '`', & len, & col);
        wordStart += 1;
        quoted = true;
        break;
    default:
        eatAddressWord(& cur, & len, & col);
        break;
    }

    if (len == 0)
       { return hu_nullNode; }
    
    huNode const * nextNode = hu_nullNode;
    // if '..', go up a level if we can
    if (quoted == false && len == 2 && wordStart[0] == '.' && wordStart[1] == '.')
        { nextNode = huGetParentNode(node); }
    else
    {
        // if numeric, and not quoted, use as an index
        char * wordEnd;
        int index = strtol(wordStart, & wordEnd, 10);
        if (quoted == false && wordEnd - wordStart == len)
            { nextNode = huGetChildByIndex(node, index); }
        else
        {
            if (doubleQuoted)
            {
                // process a quoted string into a proper key
                if (len < 32)
                {
                    char trKey[32];
                    int trCursor = 0;
                    for (char const * pc = wordStart; pc < wordStart + len; ++pc)
                    {
                        if (* pc == '\\' && pc < wordStart + len - 1 && * (pc + 1) == '"')
                            { trKey[trCursor++] = '"'; pc += 1; }
                        else
                            { trKey[trCursor++] = * pc; }
                    }
                    nextNode = huGetChildByKeyN(node, trKey, trCursor);
                }
                else
                {
                    char * trKey = malloc(len);
                    int trCursor = 0;
                    for (char const * pc = wordStart; pc < wordStart + len; ++pc)
                    {
                        if (* pc == '\\' && pc < wordStart + len - 1 && * (pc + 1) == '"')
                            { trKey[trCursor++] = '"'; pc += 1; }
                        else
                            { trKey[trCursor++] = * pc; }
                    }
                    nextNode = huGetChildByKeyN(node, trKey, trCursor);
                    free(trKey);
                }
            }
            else
                { nextNode = huGetChildByKeyN(node, wordStart, len); }
        }
    }
    // If the key or index is invalid, nextNode wil be set to hu_nullNode.
    if (nextNode == hu_nullNode)
        { return hu_nullNode; }
    
    eatWs(& cur, 1, & line, & col);

    if (address[col] == '\0')
        { return nextNode; }
    else if (address[col] == '/')
    {
        return huGetNodeByRelativeAddressN(
            nextNode, address + col + 1, addressLen - col - 1);
    }
    else
      { return hu_nullNode; }
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


int huGetNodeAddressLength(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode)
        { return 0; }
#endif

    int addressLen = 0; 

    huNode const * n = node;
    if (n->parentNodeIdx == -1)
        { return 1; } // for the root if node is root
    while (n->parentNodeIdx != -1)
    {
        huNode const * parentN = huGetParentNode(n);
        if (parentN->kind == HU_NODEKIND_DICT)
        {
            addressLen += n->keyToken->str.size;
        }
        else if (parentN->kind == HU_NODEKIND_LIST)
        {
            addressLen += log10i((unsigned int) n->childOrdinal) + 1;
        }

        addressLen += 1;  // for the node '/'
        n = parentN;
    }

    return addressLen;
}


void getNodeAddressRec(huNode const * node, huVector * str)
{
    if (node->parentNodeIdx != -1)
        { getNodeAddressRec(huGetParentNode(node), str); }
    else
        { return; }
    
    huNode const * parentNode = huGetParentNode(node);

    appendString(str, "/", 1);    
    
    if (parentNode->kind == HU_NODEKIND_LIST)
    {
        int numBytes = log10i((unsigned int) node->childOrdinal) + 1;
        char * nn = huGrowVector(str, & numBytes);

        // If we're printing the string and not just counting,
        if (str->elementSize > 0)
        {
            nn += numBytes; // set nn to past the end of the new bits so we can print it in reverse
            int cv = node->childOrdinal;
            for (int i = 0; i < numBytes; ++i)
            {
                nn -= 1;
                * nn = '0' + cv % 10;
                cv /= 10;
            }
        }
    }
    else if (parentNode->kind == HU_NODEKIND_DICT)
    {
        huStringView const * key = & node->keyToken->str;

        //  if key is not quoted, and contains a '/',
        //      append "
        //      for the whole key,
        //          append string up to any " in the string
        //          if we're at a ",
        //              append \"
        //      append "

        bool isQuoted = node->keyToken->quoteChar != '\0';

        if (! isQuoted)
        {
            char * slashIdx = memchr(key->str, '/', key->size);
            if (slashIdx != NULL)
            {
                appendString(str, "\"", 1);
                char const * blockBegin = key->str;
                for (char const * sp = key->str; sp < key->str + key->size; ++sp)
                {
                    if (* sp == '"')
                    {
                        appendString(str, blockBegin, sp - blockBegin);
                        appendString(str, "\\\"", 2);
                        blockBegin = sp + 1;
                    }
                }
                appendString(str, blockBegin, key->str + key->size - blockBegin);
                appendString(str, "\"", 1);
                return;
            }
            else
            {
                // if all digits are numbers, enquote the key so it can't be an index
                bool hasNonDigits = false;
                for (int i = 0; i < key->size && hasNonDigits == false; ++i)
                    { hasNonDigits = key->str[i] < '0' || key->str[i] > '9'; }
                if (hasNonDigits == false)
                {
                    appendString(str, "\"", 1);
                    appendString(str, key->str, key->size);
                    appendString(str, "\"", 1);
                    return;
                }
            }
        }

        appendString(str, key->str, key->size);
    }
}

void huGetNodeAddress(huNode const * node, char * dest, int * destLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == NULL || node == hu_nullNode || destLen == NULL)
        { return; }
#endif

    if (dest != NULL && * destLen == 0)
        { return; }

    // if node is root, do special return "/"
    if (node->parentNodeIdx == -1)
    { 
        * destLen = 1;
        if (dest != NULL)
            { dest[0] = '/'; }
        return;
    }

    huVector str;
    if (dest == NULL)
    {
        huInitVector(& str, 0); // counting only
    }
    else
    {
        huInitVectorPreallocated(& str, sizeof(char), dest, * destLen, false);
    }

    getNodeAddressRec(node, & str);
    * destLen = str.numElements;
}
