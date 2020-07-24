#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "humon.internal.h"


void initNode(huNode * node, huTrove const * trove)
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
    initGrowableVector(& node->childNodeIdxs, sizeof(int));
    initGrowableVector(& node->annotations, sizeof(huAnnotation));
    initGrowableVector(& node->comments, sizeof(huComment));
}


void destroyNode(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode)
        { return; }
#endif

    destroyVector(& node->childNodeIdxs);
    destroyVector(& node->annotations);
    destroyVector(& node->comments);
}


huNode const * huGetParentNode(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode)
        { return hu_nullNode; }
#endif

    return huGetNodeByIndex(node->trove, node->parentNodeIdx);
}


int huGetNumChildren(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode)
        { return 0; }
#endif

    return node->childNodeIdxs.numElements;
}


huNode const * huGetChildByIndex(huNode const * node, int childOrdinal)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode || childOrdinal < 0)
        { return hu_nullNode; }
#endif

    if (childOrdinal >= huGetNumChildren(node))
        { return hu_nullNode; }

    return huGetNodeByIndex(node->trove, 
        * (int *) getVectorElement(& node->childNodeIdxs, childOrdinal));
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
    if (node == hu_nullNode || key == NULL || keyLen < 0)
        { return hu_nullNode; }
#endif

    if (node->kind != HU_NODEKIND_DICT)
        { return hu_nullNode; }

    for (int i = 0; i < huGetNumChildren(node); ++i)
    {
        huNode const * childNode = huGetChildByIndex(node, i);
        if (keyLen == childNode->keyToken->str.size &&
            strncmp(childNode->keyToken->str.ptr, key, keyLen) == 0)
            { return childNode; }
    }

    return hu_nullNode;
}


huNode const * huGetFirstChild(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode)
        { return hu_nullNode; }
#endif

    if (huGetNumChildren(node) == 0)
        { return hu_nullNode; }

    return huGetNodeByIndex(node->trove, 
        * (int *) getVectorElement(& node->childNodeIdxs, 0));
}


huNode const * huGetNextSibling(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode)
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
    if (node == hu_nullNode)
        { return false; }
#endif

    return node->keyToken != hu_nullToken;
}


huStringView huGetNestedValue(huNode const * node)
{
    huStringView str;
    str.ptr = NULL;
    str.size = 0;

#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode)
        { return str; }
#endif

    char const * start = node->firstToken->str.ptr;
    if (node->firstToken->quoteChar != '\0')
        { start -= 1; }

    char const * end = node->lastToken->str.ptr + node->lastToken->str.size; 
    if (node->lastToken->quoteChar != '\0')
        { end += 1; }
    
    str.ptr = start;
    str.size = end - start;
    
    return str;
}


int huGetNumAnnotations(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode)
        { return 0; }
#endif

    return node->annotations.numElements;
}


huAnnotation const * huGetAnnotation(huNode const * node, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode || annotationIdx < 0)
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
    if (node == hu_nullNode || key == NULL || keyLen < 0)
        { return false; }
#endif

    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation *) node->annotations.buffer + i;
        if (keyLen == anno->key->str.size &&
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return true; }
    }

    return false;
}


huToken const * huGetAnnotationWithKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return hu_nullToken; }
#endif

    return huGetAnnotationWithKeyN(node, key, strlen(key));
}


huToken const * huGetAnnotationWithKeyN(huNode const * node, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode || key == NULL || keyLen < 0)
        { return hu_nullToken; }
#endif

    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (keyLen == anno->key->str.size &&
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return anno->value; }
    }

    return hu_nullToken;
}


int huGetNumAnnotationsWithValueZ(huNode const * node, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    return huGetNumAnnotationsWithValueN(node, value, strlen(value));
}


int huGetNumAnnotationsWithValueN(huNode const * node, char const * value, int valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode || value == NULL || valueLen < 0)
        { return 0; }
#endif

    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (valueLen == anno->value->str.size &&
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetAnnotationWithValueZ(huNode const * node, char const * value, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return hu_nullToken; }
#endif

    return huGetAnnotationWithValueN(node, value, strlen(value), annotationIdx);
}


huToken const * huGetAnnotationWithValueN(huNode const * node, char const * value, int valueLen, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode || value == NULL || valueLen < 0 || annotationIdx < 0)
        { return hu_nullToken; }
#endif

    int matches = 0;
    for (int i = 0; i < node->annotations.numElements; ++i)
    { 
        huAnnotation const * anno = (huAnnotation *) node->annotations.buffer + i;
        if (valueLen == anno->value->str.size &&
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
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
    if (node == hu_nullNode)
        { return 0; }
#endif

    return node->comments.numElements;
}


huToken const * huGetComment(huNode const * node, int commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode || commentIdx < 0)
        { return hu_nullToken; }
#endif

    if (commentIdx < node->comments.numElements)
        { return ((huComment const *) node->comments.buffer + commentIdx)->token; }
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
    if (node == hu_nullNode || containedText == NULL || containedTextLen < 0)
        { return hu_nullToken; }
#endif

    bool foundLast = startWith == NULL;

    int na = huGetNumComments(node);
    for (int i = 0; i < na; ++i)
    {
        huToken const * comm = huGetComment(node, i);
        if (foundLast && stringInString(comm->str.ptr, comm->str.size, 
                containedText, containedTextLen))
            { return comm; }

        if (foundLast == false && comm == startWith)
            { foundLast = true; }
    }

    return hu_nullToken;
}


static void eatAddressWord(huScanner * scanner, int * len, int * col)
{
    // The first character is already confirmed a word char, so, next please.
    * len += scanner->curCursor->charLength;
    * col += 1;
    nextCharacter(scanner);

    bool eating = true;
    while (eating)
    {
        //analyzeWhitespace(scanner);
        if (scanner->curCursor->isEof)
            { eating = false; }
        else if (scanner->curCursor->isNewline || scanner->curCursor->isSpace)
            { eating = false; }
        else
        {
            switch(scanner->curCursor->codePoint)
            {
            case '{': case '}': case '[': case ']':
            case ':': case '@': case '#': case '/': // adding '/' here because address
                eating = false;
                break;
            default:
                * len += scanner->curCursor->charLength;
                * col += 1;
                nextCharacter(scanner);
                break;
            }
        }
    }    
}


static void eatQuotedAddressWord(huScanner * scanner, char quoteChar, int * len, int * col)
{
    // The first character is already confirmed quoteChar, so, next please.
    * col += 1;
    nextCharacter(scanner);

    bool eating = true;
    while (eating)
    {
        //analyzeWhitespace(scanner);
        if (scanner->curCursor->isEof)
            { eating = false; }
        else if (scanner->curCursor->isNewline)
        {
            * len += scanner->curCursor->charLength;
            * col = 1;
            nextCharacter(scanner);
        }
        else
        {
            if (scanner->curCursor->codePoint == (uint32_t) quoteChar)
            {
                * col += 1;
                nextCharacter(scanner);
                eating = false;
            }
            else if (scanner->curCursor->codePoint == '\\' && 
                     scanner->nextCursor->codePoint == (uint32_t) quoteChar)  // TODO: ensure not running off the end
            {
                * col += 1;
                * len += 1;
                nextCharacter(scanner);
                * col += 1;
                * len += 1;
                nextCharacter(scanner);
            }
            else
            {
                * col += 1;
                * len += scanner->curCursor->charLength;
                nextCharacter(scanner);
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
    if (node == hu_nullNode || address == NULL || addressLen < 0)
        { return hu_nullNode; }
#endif

    // When the last node is reached in the address, we're it.
    if (addressLen == 0)
        { return node; }

    // malformed
    if (address[0] == '/')
        { return hu_nullNode; }

    huScanner scanner;
    initScanner(& scanner, NULL, address, addressLen);
    int line = 0;  // unused
    int col = 0;

    eatWs(& scanner, 1, & line, & col);
    int len = 0;
    char const * wordStart = address + col;
    bool quoted = false;
    bool doubleQuoted = false;
    switch(scanner.curCursor->codePoint)
    {
    case '\0':
        return node;
    case '"':
        eatQuotedAddressWord(& scanner, '"', & len, & col);
        wordStart += scanner.curCursor->charLength;
        quoted = true;
        doubleQuoted = true;
        break;
    case '\'':
        eatQuotedAddressWord(& scanner, '\'', & len, & col);
        wordStart += scanner.curCursor->charLength;
        quoted = true;
        break;
    case '`':
        eatQuotedAddressWord(& scanner, '`', & len, & col);
        wordStart += scanner.curCursor->charLength;
        quoted = true;
        break;
    default:
        eatAddressWord(& scanner, & len, & col);
        break;
    }

    if (len == 0)
       { return hu_nullNode; }
    
    huNode const * nextNode = hu_nullNode;
    // if '..', go up a level if we can
    if (quoted == false && len == 2 && 
        wordStart[0] == '.' && wordStart[1] == '.')
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
    
    eatWs(& scanner, 1, & line, & col);

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
static int log10i(unsigned int x)
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


static void getNodeAddressRec(huNode const * node, PrintTracker * printer)
{
    huVector * str = printer->str;

    if (node->parentNodeIdx != -1)
        { getNodeAddressRec(huGetParentNode(node), printer); }
    else
        { return; }
    
    huNode const * parentNode = huGetParentNode(node);

    appendString(printer, "/", 1);    
    
    if (parentNode->kind == HU_NODEKIND_LIST)
    {
        int numBytes = log10i((unsigned int) node->childOrdinal) + 1;
        char * nn = growVector(str, & numBytes);

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
            char * slashIdx = memchr(key->ptr, '/', key->size);
            if (slashIdx != NULL)
            {
                appendString(printer, "\"", 1);
                char const * blockBegin = key->ptr;
                for (char const * sp = key->ptr; sp < key->ptr + key->size; ++sp)
                {
                    if (* sp == '"')
                    {
                        appendString(printer, blockBegin, sp - blockBegin);
                        appendString(printer, "\\\"", 2);
                        blockBegin = sp + 1;
                    }
                }
                appendString(printer, blockBegin, key->ptr + key->size - blockBegin);
                appendString(printer, "\"", 1);
                return;
            }
            else
            {
                // if all digits are numbers, enquote the key so it can't be an index
                bool hasNonDigits = false;
                for (int i = 0; i < key->size && hasNonDigits == false; ++i)
                    { hasNonDigits = key->ptr[i] < '0' || key->ptr[i] > '9'; }
                if (hasNonDigits == false)
                {
                    appendString(printer, "\"", 1);
                    appendString(printer, key->ptr, key->size);
                    appendString(printer, "\"", 1);
                    return;
                }
            }
        }

        appendString(printer, key->ptr, key->size);
    }
}

void huGetNodeAddress(huNode const * node, char * dest, int * destLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == hu_nullNode || destLen == NULL)
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
        initVectorForCounting(& str); // counting only
    }
    else
    {
        initVectorPreallocated(& str, dest, sizeof(char), * destLen);
    }

    PrintTracker printer = {
        .trove = NULL,
        .str = & str,
        .storeParams = NULL,
        .currentDepth = 0,
        .lastPrintWasNewline = false
    };

    getNodeAddressRec(node, & printer);
    * destLen = printer.str->numElements;
}
