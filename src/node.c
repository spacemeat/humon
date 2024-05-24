#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "humon.internal.h"


void initNode(huNode * node, huTrove const * trove)
{
    node->trove = trove;
    node->nodeIdx = -1;
    node->kind = HU_NODEKIND_NULL;
    node->firstToken = HU_NULLTOKEN;
    node->keyToken = HU_NULLTOKEN;
    node->valueToken = HU_NULLTOKEN;
    node->lastValueToken = HU_NULLTOKEN;
    node->lastToken = HU_NULLTOKEN;
    node->childOrdinal = 0;
    node->parentNodeIdx = -1;
    initGrowableVector(& node->childNodeIdxs, sizeof(huSize_t), & trove->allocator);
    initGrowableVector(& node->annotations, sizeof(huAnnotation), & trove->allocator);
    initGrowableVector(& node->comments, sizeof(huComment), & trove->allocator);
}


void destroyNode(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return; }
#endif

    destroyVector(& node->childNodeIdxs);
    destroyVector(& node->annotations);
    destroyVector(& node->comments);
}


huNode const * huGetParent(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return HU_NULLNODE; }
#endif

    return huGetNodeByIndex(node->trove, node->parentNodeIdx);
}


huSize_t huGetNumChildren(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return 0; }
#endif

    return node->childNodeIdxs.numElements;
}


huNode const * huGetChildByIndex(huNode const * node, huSize_t childOrdinal)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || childOrdinal < 0)
        { return HU_NULLNODE; }
#endif

    if (childOrdinal >= huGetNumChildren(node))
        { return HU_NULLNODE; }

    return huGetNodeByIndex(node->trove,
        * (huSize_t *) getVectorElement(& node->childNodeIdxs, childOrdinal));
}


huNode const * huGetChildByKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huGetChildByKeyN(node, key, (huSize_t) keyLenC);
}


huNode const * huGetChildByKeyN(huNode const * node, char const * key, huSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || key == NULL || keyLen < 0)
        { return HU_NULLNODE; }
#endif

    if (node->kind != HU_NODEKIND_DICT)
        { return HU_NULLNODE; }

    for (huSize_t i = huGetNumChildren(node) - 1; i >= 0; --i)
    {
        huNode const * childNode = huGetChildByIndex(node, i);
        if (keyLen == childNode->keyToken->str.size &&
            strncmp(childNode->keyToken->str.ptr, key, keyLen) == 0)
            { return childNode; }
    }

    return HU_NULLNODE;
}


huNode const * huGetFirstChild(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return HU_NULLNODE; }
#endif

    if (huGetNumChildren(node) == 0)
        { return HU_NULLNODE; }

    return huGetNodeByIndex(node->trove,
        * (huSize_t *) getVectorElement(& node->childNodeIdxs, 0));
}


huNode const * huGetNextSibling(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return HU_NULLNODE; }
#endif

    if (node->parentNodeIdx != -1)
    {
        huNode const * parentNode = huGetParent(node);

        if (huGetNumChildren(parentNode) > node->childOrdinal + 1)
        {
           return huGetChildByIndex(parentNode, node->childOrdinal + 1);
        }
    }

    return HU_NULLNODE;
}


huNode const * huGetFirstChildWithKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huGetFirstChildWithKeyN(node, key, (huSize_t) keyLenC);
}


huNode const * huGetFirstChildWithKeyN(huNode const * node, char const * key, huSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || key == NULL || keyLen < 0)
        { return HU_NULLNODE; }
#endif

    if (node->kind != HU_NODEKIND_DICT)
        { return HU_NULLNODE; }

    huSize_t numChildren = huGetNumChildren(node);
    if (numChildren == 0)
        { return HU_NULLNODE; }

    for (huSize_t i = 0; i < numChildren; ++i)
    {
        huNode const * childNode = huGetChildByIndex(node, i);
        if (keyLen == childNode->keyToken->str.size &&
            strncmp(childNode->keyToken->str.ptr, key, keyLen) == 0)
            { return childNode; }
    }

    return HU_NULLNODE;
}


huNode const * huGetNextSiblingWithKeyZ(huNode const * node, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huGetNextSiblingWithKeyN(node, key, (huSize_t) keyLenC);
}


huNode const * huGetNextSiblingWithKeyN(huNode const * node, char const * key, huSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || key == NULL || keyLen < 0)
        { return HU_NULLNODE; }
#endif

    if (node->kind != HU_NODEKIND_DICT)
        { return HU_NULLNODE; }

    if (node->parentNodeIdx == -1)
        { return HU_NULLNODE; }

    huNode const * parentNode = huGetParent(node);
    if (parentNode == HU_NULLNODE)
        { return HU_NULLNODE; }

    huSize_t numChildren = huGetNumChildren(parentNode);

    for (huSize_t i = node->childOrdinal + 1; i < numChildren; ++i)
    {
        huNode const * childNode = huGetChildByIndex(parentNode, i);
        if (keyLen == childNode->keyToken->str.size &&
            strncmp(childNode->keyToken->str.ptr, key, keyLen) == 0)
            { return childNode; }
    }

    return HU_NULLNODE;
}


// Note, we're using huSize_t instead of huCol_t for col on purpose in these functions. col here is just the char offset into the address string.

static bool eatAddressWord(huScanner * scanner, huSize_t * wordLen)
{
    bool error = false;

    // The first character is already confirmed a word char, so, next please.
    nextCharacter(scanner);
    * wordLen = scanner->curCursor->charLength;

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError)
        {
            eating = false;
            error = true;
        }
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
                nextCharacter(scanner);
                * wordLen += scanner->curCursor->charLength;
                break;
            }
        }
    }

    return error == false;
}


static bool eatQuotedAddressWord(huScanner * scanner, char const ** word, huSize_t * wordLen)
{
    bool error = false;
    uint32_t quoteChar = scanner->curCursor->codePoint;

    // The first character is already confirmed quoteChar, so, next please.
    nextCharacter(scanner);

    * word = scanner->curCursor->character;
    * wordLen = 0;  // but not to be included in the word len

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError ||
            scanner->curCursor->isEof)
        {
            eating = false;
            error = true;
        }
        else if (scanner->curCursor->codePoint == quoteChar)
        {
            nextCharacter(scanner);
            eating = false;
        }
        else
        {
            nextCharacter(scanner);
            * wordLen += scanner->curCursor->charLength;
        }
    }

    return error == false;
}


static bool eatHeredocTag(huScanner * scanner, huSize_t * tagLen)
{
    bool error = false;

    // The first character is already confirmed caret, so, next please.
    * tagLen = scanner->curCursor->charLength;
    nextCharacter(scanner);

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError ||
            scanner->curCursor->isEof)
        {
            eating = false;
            error = true;
        }
        else
        {
            * tagLen += scanner->curCursor->charLength;

            if (scanner->curCursor->codePoint == '^')
                { eating = false; }

            nextCharacter(scanner);
        }
    }

    return error == false;
}


static bool eatHeredocWord(huScanner * scanner, char const * tag, huSize_t tagLen, huSize_t * wordLen)
{
    * wordLen = 0;
    bool error = false;

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError ||
            scanner->curCursor->isEof)
        {
            eating = false;
            error = true;
        }
        else if (strncmp(scanner->curCursor->character, tag, tagLen) == 0)
        {
            eating = false;
        }
        else
        {
            * wordLen += scanner->curCursor->charLength;
            nextCharacter(scanner);
        }
    }

    return error == false;
}


huNode const * huGetNodeByRelativeAddressZ(huNode const * node, char const * address)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
        { return HU_NULLNODE; }
#endif

    size_t addressLenC = strlen(address);
    if (addressLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huGetNodeByRelativeAddressN(node, address, (huSize_t) addressLenC);
}


huNode const * huGetNodeByRelativeAddressN(huNode const * node, char const * address, huSize_t addressLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || address == NULL || addressLen < 0)
        { return HU_NULLNODE; }
#endif

    huScanner scanner;
    initScanner(& scanner, NULL, 1, address, addressLen);

    eatWs(& scanner);

    // When the last node is reached in the address, we're it.
    if (scanner.curCursor->isEof)
        { return node; }

    // malformed
    if (scanner.curCursor->codePoint == '/')
        { return HU_NULLNODE; }

    huSize_t wslen = scanner.len;

    char const * rawWordStart = address + wslen;
    char const * wordStart = rawWordStart;
    huSize_t wordLen = 0;
    bool quoted = false;
    char quoteChar = '\0';
    bool error = false;

    switch(scanner.curCursor->codePoint)
    {
    case '\0':
        return node;
    case '"':
        wordStart += 1;
        error = ! eatQuotedAddressWord(& scanner, & wordStart, & wordLen);
        quoteChar = '"';
        break;
    case '\'':
        wordStart += 1;
        error = ! eatQuotedAddressWord(& scanner, & wordStart, & wordLen);
        quoteChar = '\'';
        break;
    case '`':
        wordStart += 1;
        error = ! eatQuotedAddressWord(& scanner, & wordStart, & wordLen);
        quoteChar = '`';
        break;
    case '^':
        {
            huSize_t tagLen = 0;
            char const * tag = scanner.curCursor->character;
            error = ! eatHeredocTag(& scanner, & tagLen);
            if (error)
                { break; }
            wordStart += tagLen;
            error = ! eatHeredocWord(& scanner, tag, tagLen, & wordLen);
            if (error)
                { break; }
            error = ! eatHeredocTag(& scanner, & tagLen);
            quoteChar = '^';
        }
        break;
    default:
        error = ! eatAddressWord(& scanner, & wordLen);
        break;
    }

    if (error)
        { return HU_NULLNODE; }

    huSize_t rawPartLen = scanner.len - wslen;
    if (rawPartLen == 0)
       { return HU_NULLNODE; }

    huNode const * nextNode = HU_NULLNODE;

    // if '..', go up a level if we can
    if (quoteChar == '\0' && wordLen == 2 &&
        wordStart[0] == '.' && wordStart[1] == '.')
            { nextNode = huGetParent(node); }
    else
    {
        if (quoteChar == '\0')
        {
            char * wordEnd;
            unsigned long long index = strtoull(wordStart, & wordEnd, 10);
            if (quoted == false && wordEnd - wordStart == wordLen && index <= maxOfType(huSize_t))
                { nextNode = huGetChildByIndex(node, (huSize_t) index); }
            else
                { nextNode = huGetChildByKeyN(node, wordStart, wordLen); }
        }
        else
            { nextNode = huGetChildByKeyN(node, wordStart, wordLen); }
    }

    // If the key or index is invalid, nextNode wil be set to HU_NULLNODE.
    if (nextNode == HU_NULLNODE)
        { return HU_NULLNODE; }

    eatWs(& scanner);

    if (scanner.curCursor->isEof)
        { return nextNode; }
    else if (scanner.curCursor->codePoint == '/')
    {
        nextCharacter(& scanner);
        return huGetNodeByRelativeAddressN(
            nextNode, address + scanner.len, addressLen - scanner.len);
    }
    else
      { return HU_NULLNODE; }
}


// This is kinda fugly. But for most cases (x < 1000) it's probably fine.
static huSize_t log10i(huSize_t a)
{
    int64_t x = (int64_t) a;
		 if (x < (int16_t)1 * 10) { return 0; }
	else if (x < (int16_t)1 * 100) { return 1; }
    else if (sizeof(huSize_t) == 1) { return 2; }
	else if (x < (int16_t)1 * 1000) { return 2; }
	else if (x < (int16_t)1 * 1000 * 10) { return 3; }
	else if (sizeof(huSize_t) == 2) { return 4; }
	else if (x < (int32_t)1 * 1000 * 100) { return 4; }
	else if (x < (int32_t)1 * 1000 * 1000) { return 5; }
	else if (x < (int32_t)1 * 1000 * 1000 * 10) { return 6; }
	else if (x < (int32_t)1 * 1000 * 1000 * 100) { return 7; }
	else if (x < (int32_t)1 * 1000 * 1000 * 1000) { return 8; }
	else if (sizeof(huSize_t) == 4) { return 9; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 10) { return 9; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 100) { return 10; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 1000) { return 11; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 1000 * 10) { return 12; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 1000 * 100) { return 13; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 1000 * 1000) { return 14; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 1000 * 1000 * 10) { return 15; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 1000 * 1000 * 100) { return 16; }
	else if (x < (int64_t)1 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000) { return 17; }
	else { return 18; }
}


static void getNodeAddressRec(huNode const * node, PrintTracker * printer)
{
    huVector * str = printer->str;

    if (node->parentNodeIdx != -1)
        { getNodeAddressRec(huGetParent(node), printer); }
    else
        { return; }

    huNode const * parentNode = huGetParent(node);

    appendString(printer, "/", 1);

    if (parentNode->kind == HU_NODEKIND_LIST)
    {
        huSize_t numBytes = log10i(node->childOrdinal) + 1;
        char * nn = growVector(str, & numBytes);

        // If we're printing the string and not just counting,
        if (str->elementSize > 0)
        {
            nn += numBytes; // set nn to past the end of the new bits so we can print it in reverse
            huSize_t cv = node->childOrdinal;
            for (huSize_t i = 0; i < numBytes; ++i)
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

        //  if key is not quoted
        //      if key contains a '/',
        //          determine the best heredoc string
        //          append heredoc string
        //          append string
        //          append heredoc string
        //      else

        bool isQuoted = node->keyToken->quoteChar != '\0';

        if (! isQuoted)
        {
            char * slashIdx = memchr(key->ptr, '/', key->size);
            if (slashIdx != NULL)
            {
#define HEREDOC_TAG_LEN (16)
                char heredocTag[HEREDOC_TAG_LEN] = "^^";
                int foundLen = 2;
                bool foundOne = ! stringInString(key->ptr, key->size, heredocTag, foundLen);
                for (int i = 0; foundOne == false; ++i)
                {
                    foundLen = snprintf(heredocTag, HEREDOC_TAG_LEN, "^%d^", i);
                    foundOne = ! stringInString(key->ptr, key->size, heredocTag, foundLen);
                }

                appendString(printer, heredocTag, foundLen);
                appendString(printer, key->ptr, key->size);
                appendString(printer, heredocTag, foundLen);
            }
            else
            {
                // if all digits are numbers, enquote the key so it can't be an index
                bool hasNonDigits = false;
                for (huSize_t i = 0; i < key->size && hasNonDigits == false; ++i)
                    { hasNonDigits = key->ptr[i] < '0' || key->ptr[i] > '9'; }
                if (hasNonDigits == false)
                {
                    appendString(printer, "'", 1);
                    appendString(printer, key->ptr, key->size);
                    appendString(printer, "'", 1);
                }
                else
                {
                    appendString(printer, key->ptr, key->size);
                }
            }
        }
        else
        {
            appendString(printer, node->keyToken->rawStr.ptr, node->keyToken->rawStr.size);
        }
    }
}


void huGetAddress(huNode const * node, char * dest, huSize_t * destLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || destLen == NULL)
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
        .serializeOptions = NULL,
        .currentDepth = 0,
        .lastPrintWasNewline = false
    };

    getNodeAddressRec(node, & printer);
    * destLen = printer.str->numElements;
}


bool huHasKey(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return false; }
#endif

    return node->keyToken != HU_NULLTOKEN;
}


huToken const * huGetKey(huNode const * node)
{
    return node->keyToken;
}


huToken const * huGetValue(huNode const * node)
{
    return node->valueToken;
}


huStringView huGetTokenStream(huNode const * node)
{
    huStringView str;
    str.ptr = NULL;
    str.size = 0;

#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return str; }
#endif

    char const * start = node->firstToken->rawStr.ptr;
    char const * end = node->lastToken->rawStr.ptr + node->lastToken->rawStr.size;
    str.ptr = start;
    str.size = (huSize_t)(end - start);

    return str;
}


huSize_t huGetNumAnnotations(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return 0; }
#endif

    return node->annotations.numElements;
}


huAnnotation const * huGetAnnotation(huNode const * node, huSize_t annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || annotationIdx < 0)
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

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return false; }

    return huHasAnnotationWithKeyN(node, key, (huSize_t) keyLenC);
}


bool huHasAnnotationWithKeyN(huNode const * node, char const * key, huSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || key == NULL || keyLen < 0)
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
        { return HU_NULLTOKEN; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLTOKEN; }

    return huGetAnnotationWithKeyN(node, key, (huSize_t) keyLenC);
}


huToken const * huGetAnnotationWithKeyN(huNode const * node, char const * key, huSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || key == NULL || keyLen < 0)
        { return HU_NULLTOKEN; }
#endif

    for (huSize_t i = 0; i < node->annotations.numElements; ++i)
    {
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (keyLen == anno->key->str.size &&
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return anno->value; }
    }

    return HU_NULLTOKEN;
}


huSize_t huGetNumAnnotationsWithValueZ(huNode const * node, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huSize_t))
        { return 0; }

    return huGetNumAnnotationsWithValueN(node, value, (huSize_t) valueLenC);
}


huSize_t huGetNumAnnotationsWithValueN(huNode const * node, char const * value, huSize_t valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || value == NULL || valueLen < 0)
        { return 0; }
#endif

    huSize_t matches = 0;
    for (huSize_t i = 0; i < node->annotations.numElements; ++i)
    {
        huAnnotation const * anno = (huAnnotation const *) node->annotations.buffer + i;
        if (valueLen == anno->value->str.size &&
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetAnnotationWithValueZ(huNode const * node, char const * value, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return HU_NULLTOKEN; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huSize_t))
        { return HU_NULLTOKEN; }

    return huGetAnnotationWithValueN(node, value, (huSize_t) valueLenC, cursor);
}


huToken const * huGetAnnotationWithValueN(huNode const * node, char const * value, huSize_t valueLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || value == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
        { return HU_NULLTOKEN; }
#endif

    for (; * cursor < node->annotations.numElements; ++ * cursor)
    {
        huAnnotation const * anno = (huAnnotation *) node->annotations.buffer + * cursor;
        if (valueLen == anno->value->str.size &&
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
        {
            * cursor += 1;
            return anno->key;
        }
    }

    return HU_NULLTOKEN;
}


huSize_t huGetNumComments(huNode const * node)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE)
        { return 0; }
#endif

    return node->comments.numElements;
}


huToken const * huGetComment(huNode const * node, huSize_t commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || commentIdx < 0)
        { return HU_NULLTOKEN; }
#endif

    if (commentIdx < node->comments.numElements)
        { return ((huComment const *) node->comments.buffer + commentIdx)->token; }
    else
        { return HU_NULLTOKEN; }
}


bool huHasCommentsContainingZ(huNode const * node, char const * containedText)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
        { return false; }
#endif

    size_t containedTextLenC = strlen(containedText);
    if (containedTextLenC > maxOfType(huSize_t))
        { return false; }

    return huHasCommentsContainingN(node, containedText, (huSize_t) containedTextLenC);
}


bool huHasCommentsContainingN(huNode const * node, char const * containedText, huSize_t containedTextLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || containedText == NULL || containedTextLen < 0)
        { return false; }
#endif

    for (huSize_t idx = 0; idx < node->comments.numElements; ++ idx)
    {
        huToken const * comm = huGetComment(node, idx);
        if (stringInString(comm->str.ptr, comm->str.size,
                containedText, containedTextLen))
            { return true; }
    }

    return false;
}


huSize_t huGetNumCommentsContainingZ(huNode const * node, char const * containedText)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
        { return 0; }
#endif

    size_t containedTextLenC = strlen(containedText);
    if (containedTextLenC > maxOfType(huSize_t))
        { return 0; }

    return huGetNumCommentsContainingN(node, containedText, (huSize_t) containedTextLenC);
}


huSize_t huGetNumCommentsContainingN(huNode const * node, char const * containedText, huSize_t containedTextLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || containedText == NULL || containedTextLen < 0)
        { return 0; }
#endif

    huSize_t matches = 0;
    for (huSize_t idx = 0; idx < node->comments.numElements; ++ idx)
    {
        huToken const * comm = huGetComment(node, idx);
        if (stringInString(comm->str.ptr, comm->str.size,
                containedText, containedTextLen))
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetCommentsContainingZ(huNode const * node, char const * containedText, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
        { return HU_NULLTOKEN; }
#endif

    size_t containedTextLenC = strlen(containedText);
    if (containedTextLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huGetCommentsContainingN(node, containedText, (huSize_t) containedTextLenC, cursor);
}


huToken const * huGetCommentsContainingN(huNode const * node, char const * containedText, huSize_t containedTextLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (node == HU_NULLNODE || containedText == NULL || containedTextLen < 0 || cursor == NULL || * cursor < 0)
        { return HU_NULLTOKEN; }
#endif

    for (; * cursor < node->comments.numElements; ++ * cursor)
    {
        huToken const * comm = huGetComment(node, * cursor);
        if (stringInString(comm->str.ptr, comm->str.size,
            containedText, containedTextLen))
        {
            * cursor += 1;
            return comm;
        }
    }

    return HU_NULLTOKEN;
}
