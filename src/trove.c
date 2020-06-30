#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


huTrove const * makeTrove(huStringView const * data, int inputTabSize)
{
    huTrove * t = malloc(sizeof(huTrove));
    if (t == NULL)
        { return hu_nullTrove; }

    t->dataStringSize = data->size;
    // Pad by 4 nulls. This lets us look ahead three bytes for a 4-byte char match.
    char * newDataString = malloc(data->size + 4);
    if (newDataString == NULL)
    {
        free(t);
        return hu_nullTrove;
    }
    memcpy(newDataString, data->ptr, data->size);
    newDataString[data->size] = '\0';
    newDataString[data->size + 1] = '\0';
    newDataString[data->size + 2] = '\0';
    newDataString[data->size + 3] = '\0';

    t->dataString = newDataString;

    huInitVector(& t->tokens, sizeof(huToken));
    huInitVector(& t->nodes, sizeof(huNode));
    huInitVector(& t->errors, sizeof(huError));

    t->inputTabSize = inputTabSize;

    huInitVector(& t->annotations, sizeof(huAnnotation));
    huInitVector(& t->comments, sizeof(huComment));

    t->lastAnnoToken = NULL;

    huTokenizeTrove(t);
    huParseTrove(t);
 
    return t;
}


huTrove const * huMakeTroveFromStringZ(char const * data, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (data == NULL)
        { return hu_nullTrove; }
#endif

    return huMakeTroveFromStringN(data, strlen(data), inputTabSize);
}


huTrove const * huMakeTroveFromStringN(char const * data, int dataLen, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (data == NULL || dataLen < 0 ||
        inputTabSize < MIN_INPUT_TAB_SIZE || inputTabSize > MAX_INPUT_TAB_SIZE)
        { return hu_nullTrove; }
#endif

    // pad by 4 nulls -- see above
    char * newData = malloc(dataLen + 4);
    if (newData == NULL)
        { return hu_nullTrove; }

    memcpy(newData, data, dataLen);
    newData[dataLen] = '\0';
    newData[dataLen + 1] = '\0';
    newData[dataLen + 2] = '\0';
    newData[dataLen + 3] = '\0';

    huStringView dataView = { newData, dataLen };
    huTrove const * newTrove = makeTrove(& dataView, inputTabSize);
    if (newTrove == NULL)
    {
        free(newData);
        return hu_nullTrove;
    }
    
    return newTrove;
}


huTrove const * huMakeTroveFromFileZ(char const * path, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return hu_nullTrove; }
#endif

    return huMakeTroveFromFileN(path, strlen(path), inputTabSize);
}


huTrove const * huMakeTroveFromFileN(char const * path, int pathLen, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL || inputTabSize < MIN_INPUT_TAB_SIZE || inputTabSize > MAX_INPUT_TAB_SIZE)
        { return hu_nullTrove; }
#endif

    FILE * fp = fopen(path, "r");
    if (fp == NULL)
        { return hu_nullTrove; }

    if (fseek(fp, 0, SEEK_END) != 0)
        { return hu_nullTrove; }

    int newDataSize = ftell(fp);
    if (newDataSize == -1L)
        { return hu_nullTrove; }

    fseek(fp, 0, SEEK_SET);
    
    char * newData = malloc(newDataSize + 4);
    if (newData == NULL)
        { return hu_nullTrove; }

    int freadRet = fread(newData, 1, newDataSize, fp);
    if (freadRet != newDataSize)
    {
        free(newData);
        return hu_nullTrove;
    }
    newData[newDataSize] = '\0';
    newData[newDataSize + 1] = '\0';
    newData[newDataSize + 2] = '\0';
    newData[newDataSize + 3] = '\0';

    huStringView dataView = { newData, newDataSize };
    huTrove const * newTrove = makeTrove(& dataView, inputTabSize);
    if (newTrove == NULL)
    {
        free(newData);
        return hu_nullTrove;
    }
    
    return newTrove;
}


void huDestroyTrove(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return; }
#endif

    huTrove * ncTrove = (huTrove *) trove;

    if (ncTrove->dataString != NULL)
    {
        free((char *) ncTrove->dataString);
        ncTrove->dataString = NULL;
        ncTrove->dataStringSize = 0;
    }

    huDestroyVector(& ncTrove->tokens);
    huDestroyVector(& ncTrove->nodes);
    huDestroyVector(& ncTrove->errors);

    huDestroyVector(& ncTrove->annotations);
    huDestroyVector(& ncTrove->comments);

    free(ncTrove);
}


int huGetNumTokens(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->tokens.numElements;
}


huToken const * huGetToken(huTrove const * trove, int tokenIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return hu_nullToken; }
#endif

    if (tokenIdx < trove->tokens.numElements)
        { return (huToken *) trove->tokens.buffer + tokenIdx; }

    return hu_nullToken;
}


huToken * allocNewToken(huTrove * trove, int kind, 
    char const * str, int size, int line, int col, int endLine, 
    int endCol, char quoteChar)
{
    int num = 1;
    huToken * newToken = huGrowVector(& trove->tokens, & num);
    if (num == 0)
        { return (huToken *) hu_nullToken; }

    newToken->kind = kind;
    newToken->quoteChar = quoteChar;
    newToken->str.ptr = str;
    newToken->str.size = size;
    newToken->line = line;
    newToken->col = col;
    newToken->endLine = endLine;
    newToken->endCol = endCol;

    //printf ("%stoken: line: %d  col: %d  len: %d  %s%s\n",
    //    darkYellow, line, col, size, huTokenKindToString(kind), off);
    
    return newToken;
}


int huGetNumNodes(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->nodes.numElements;
}


huNode const * huGetRootNode(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return hu_nullNode; }
#endif

    return trove->nodes.buffer;
}


huNode const * huGetNode(huTrove const * trove, int nodeIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return hu_nullNode; }
#endif

    if (nodeIdx >= 0 && nodeIdx < trove->nodes.numElements)
        { return (huNode *) trove->nodes.buffer + nodeIdx; }

    return hu_nullNode;
}


int huGetNumErrors(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL)
        { return 0; }
#endif

    return trove->errors.numElements;
}


huError const * huGetError(huTrove const * trove, int errorIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || errorIdx < 0)
        { return NULL; }
#endif

    if (errorIdx < huGetNumErrors(trove))
    {
        return (huError *) trove->errors.buffer + errorIdx;
    }

    return NULL;
}


int huGetNumTroveAnnotations(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL)
        { return 0; }
#endif

    return trove->annotations.numElements;
}


huAnnotation const * huGetTroveAnnotation(huTrove const * trove, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || annotationIdx < 0)
        { return NULL; }
#endif

    if (annotationIdx < trove->annotations.numElements)
    {
        return (huAnnotation *) trove->annotations.buffer + annotationIdx;
    }

    return NULL;
}


bool huTroveHasAnnotationWithKeyZ(huTrove const * trove, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key  == NULL)
        { return false; }
#endif

    return huTroveHasAnnotationWithKeyN(trove, key, strlen(key));
}


bool huTroveHasAnnotationWithKeyN(huTrove const * trove, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || key  == NULL || keyLen < 0)
        { return false; }
#endif

    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->key->str.size == keyLen && 
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return true; }
    }

    return false;
}


huToken const * huGetTroveAnnotationWithKeyZ(huTrove const * trove, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key  == NULL)
        { return hu_nullToken; }
#endif

    return huGetTroveAnnotationWithKeyN(trove, key, strlen(key));
}


huToken const * huGetTroveAnnotationWithKeyN(huTrove const * trove, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || key  == NULL || keyLen < 0)
        { return hu_nullToken; }
#endif

    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->key->str.size == keyLen && 
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return anno->value; }
    }

    return hu_nullToken;
}


int huGetNumTroveAnnotationsWithValueZ(huTrove const * trove, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    return huGetNumTroveAnnotationsWithValueN(trove, value, strlen(value));
}


int huGetNumTroveAnnotationsWithValueN(huTrove const * trove, char const * value, int valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || value  == NULL || valueLen < 0)
        { return 0; }
#endif

    int matches = 0;
    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->value->str.size == valueLen && 
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetTroveAnnotationWithValueZ(huTrove const * trove, char const * value, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (value  == NULL)
        { return hu_nullToken; }
#endif

    return huGetTroveAnnotationWithValueN(trove, value, strlen(value), annotationIdx);
}


huToken const * huGetTroveAnnotationWithValueN(huTrove const * trove, char const * value, int valueLen, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || value  == NULL || valueLen < 0 || annotationIdx < 0)
        { return hu_nullToken; }
#endif

    int matches = 0;
    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->value->str.size == valueLen && 
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
        {
            if (matches == annotationIdx)
                { return anno->key; }

            matches += 1;
        }
    }

    return hu_nullToken;
}


int huGetNumTroveComments(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->comments.numElements;
}


huToken const * huGetTroveComment(huTrove const * trove, int commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || commentIdx < 0)
        { return hu_nullToken; }
#endif

    if (commentIdx < trove->comments.numElements)
    {
        return ((huComment *) trove->comments.buffer + commentIdx)->token;
    }

    return hu_nullToken;
}


huNode const * huGetNodeByFullAddressZ(huTrove const * trove, char const * address)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
        { return hu_nullNode; }
#endif

    return huGetNodeByFullAddressN(trove, address, strlen(address));
}


huNode const * huGetNodeByFullAddressN(huTrove const * trove, char const * address, int addressLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || address == NULL || addressLen < 0)
        { return hu_nullNode; }
#endif

    // parse address; must start with '/' to start at root
    if (addressLen <= 0)
        { return hu_nullNode; }

    huCursor cur = 
        { .trove = NULL, 
          .character = address, 
          .charLength = getcharLength(address) };
    int line = 0;  // unused
    int col = 0;

    eatWs(& cur, 1, & line, & col);
    char const * wordStart = address + col;

    if (* wordStart != '/')
        { return hu_nullNode; }

    huNode const * root = huGetRootNode(trove);
    if (root->kind == HU_NODEKIND_NULL)
        { return hu_nullNode; }

    return huGetNodeByRelativeAddressN(root, wordStart + 1, addressLen - col - 1);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyZ(huTrove const * trove, char const * key, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyN(trove, key, strlen(key), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyN(huTrove const * trove, char const * key, int keyLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || key == NULL || keyLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
        bool hasOne = huHasAnnotationWithKeyN(node, key, keyLen);
        if (hasOne)
            { return node; }
    }

    return hu_nullNode;
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationValueZ(huTrove const * trove, char const * value, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationValueN(trove, value, strlen(value), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationValueN(huTrove const * trove, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
        int na = huGetNumAnnotationsWithValueN(node, value, valueLen);
        if (na > 0)
           { return node; }
    }

    return hu_nullNode;
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, strlen(key), value, strlen(value), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueNZ(huTrove const * trove, char const * key, int keyLen, char const * value, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, keyLen, value, strlen(value), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, strlen(key), value, valueLen, startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || key == NULL || keyLen < 0 || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
        huToken const * anno = huGetAnnotationWithKeyN(node, key, keyLen);
        if (anno != hu_nullToken)
        {
            if (anno->str.size == valueLen &&
                strncmp(anno->str.ptr, value, valueLen) == 0)
                { return node; }
        }
    }

    return hu_nullNode;
}


huNode const * huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesByCommentContainingN(trove, containedText, strlen(containedText), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, int containedTextLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || containedText == NULL || containedTextLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);

        int na = huGetNumComments(node);
        for (int j = 0; j < na; ++j)
        {
            huToken const * comm = huGetComment(node, j);
            if (stringInString(comm->str.ptr, comm->str.size, containedText, containedTextLen))
                { return node; }
        }
    }

    return hu_nullNode;
}


huNode * allocNewNode(huTrove * trove, int nodeKind, huToken const * firstToken)
{
    int num = 1;

    // TODO: Remove this debug hook
    int foo = huGetVectorSize(& trove->nodes);
    if (foo > 15)
        { num = 1; }

    huNode * newNode = huGrowVector(& trove->nodes, & num);
    if (num == 0)
        { return (huNode *) hu_nullNode; }

    huInitNode(newNode, trove);
    int newNodeIdx = newNode - (huNode *) trove->nodes.buffer;
    newNode->nodeIdx = newNodeIdx;
    newNode->kind = nodeKind;
    newNode->firstToken = firstToken;
    newNode->lastToken = firstToken;

#ifdef HUMON_CAVEPERSON_DEBUGGING
     printf ("%snode: nodeIdx: %d    firstToken: %d    %s%s\n",
        lightCyan, newNodeIdx, (int)(firstToken - (huToken *) trove->tokens.buffer), 
        huNodeKindToString(nodeKind), off);
#endif

    return newNode;
}


void recordTokenizeError(huTrove * trove, int errorCode, int line, int col)
{
#ifdef HUMON_ERRORS_TO_STDERR
    fprintf (stderr, "%sError%s: line: %d    col: %d    %s\n", lightRed, off, 
        line, col, huOutputErrorToString(errorCode));
#endif

    int num = 1;
    huError * error = huGrowVector(& trove->errors, & num);
    if (num)
    {
        error->errorCode = errorCode;
        error->token = NULL;
        error->line = line;
        error->col = col;
    }
}


void recordError(huTrove * trove, int errorCode, huToken const * pCur)
{
#ifdef HUMON_ERRORS_TO_STDERR
    fprintf (stderr, "%sError%s: line: %d    col: %d    %s\n", lightRed, off, 
        pCur->line, pCur->col, huOutputErrorToString(errorCode));
#endif

    int num = 1;
    huError * error = huGrowVector(& trove->errors, & num);
    if (num)
    {
        error->errorCode = errorCode;
        error->token = pCur;
        error->line = pCur->line;
        error->col = pCur->col;
    }
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"


void huTroveToString(huTrove const * trove, char * dest, int * destLength, 
    int outputFormat, int outputTabSize, huStringView const * colorTable, 
    bool printComments, char const * newline, int newlineSize)
{
    if (dest == NULL && destLength != NULL)
        { * destLength = 0; }

#ifdef HUMON_CHECK_PARAMS

    if (trove == NULL || trove == hu_nullTrove || destLength == NULL || outputFormat < 0 || outputFormat >= 3 || outputTabSize < 0 || newline == NULL)
        { return; }
#endif

    // newline must be > 0; some things need a newline like // comments
    if (newlineSize < 1)
        { return; }

    if (outputFormat == HU_OUTPUTFORMAT_XEROGRAPHIC)
    {
        if (dest == NULL)
            { * destLength = trove->dataStringSize; }
        else
            { memcpy(dest, trove->dataString, * destLength); }
    }
    else if (outputFormat == HU_OUTPUTFORMAT_PRETTY ||
             outputFormat == HU_OUTPUTFORMAT_MINIMAL)
    {
        huVector str;
        if (dest == NULL)
        {
            huInitVector(& str, 0); // counting only
        }
        else
        {
            huInitVectorPreallocated(& str, sizeof(char), dest, * destLength, false);
        }

        troveToPrettyString(trove, & str, outputFormat, outputTabSize, 
            colorTable, printComments, newline, newlineSize);
        if (dest == NULL)
            { * destLength = str.numElements; }
    }
}

#pragma GCC diagnostic pop

size_t huTroveToFileZ(huTrove const * trove, char const * path, int outputFormat, 
    int outputTabSize, huStringView const * colorTable, bool printComments, 
    char const * newline, int newlineSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return 0; }
#endif

    return huTroveToFileN(trove, path, strlen(path), outputFormat, outputTabSize, 
        colorTable, printComments, newline, newlineSize);
}

size_t huTroveToFileN(huTrove const * trove, char const * path, int pathLen, 
    int outputFormat, int outputTabSize, huStringView const * colorTable, 
    bool printComments, char const * newline, int newlineSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || path == NULL || outputFormat < 0 || outputFormat >= 3 || outputTabSize < 0 || newline == NULL || newlineSize < 1)
        { return 0; }
#endif

    int strLength = 0;
    huTroveToString(trove, NULL, & strLength, outputFormat, outputTabSize, 
        colorTable, printComments, newline, newlineSize);

    char * str = malloc(strLength + 1);
    if (str == NULL)
        { return 0; }

    huTroveToString(trove, str, & strLength, outputFormat, outputTabSize, 
        colorTable, printComments, newline, newlineSize);

    FILE * fp = fopen(path, "w");
    if (fp == NULL)
        { free(str); return 0; }

    size_t ret = fwrite(str, sizeof(char), strLength, fp);

    free(str);
    return ret;
}
